// CanvasObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static	PAINTSTRUCT ps;
static	GpStatus status = GenericError;

static PyObject*
TyCanvas_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyCanvasObject* self = (TyCanvasObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->iActiveBuffer = 0;
		self->iLiveBuffer = 0;
		self->pyOnResizeCB = NULL;
		self->pyOnLMouseUpCB = NULL;
		self->pyOnLMouseDownCB = NULL;
		self->pyOnMouseMoveCB = NULL;
		self->pyOnMouseWheelCB = NULL;
		self->hDC[0] = 0;
		self->hDC[1] = 0;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyCanvas_init(TyCanvasObject* self, PyObject* args, PyObject* kwds)
{
	if (Py_TYPE(self)->tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	TyWidget_CalculateRect(self, &self->rcClient);
	self->hWin = CreateWindowEx(0, L"TyCanvasClass", L"",
		WS_CHILD | WS_VISIBLE,
		self->rcClient.left, self->rcClient.top, self->rcClient.right, self->rcClient.bottom,
		self->hwndParent, (HMENU)IDC_TYCANVAS, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	if (!TyCanvas_RenewBuffer(self, 0, self->rcClient.right, self->rcClient.bottom))
		return -1;

	SelectObject(self->hDC[0], g->hfDefaultFont);

	GdipCreatePen1(MakeARGB(255, 0, 0, 0), 1, UnitPixel, &self->pPen);
	GdipCreateSolidFill(MakeARGB(255, 0, 0, 0), &self->pBrush);
	//GdipCreateFontFamilyFromName(L"Arial", NULL, &self->pFontFamily);
	//GdipCreateFont(self->pFontFamily[0], 18.0, 0, UnitPixel, &self->pFont[0]);
	status = GdipCreateFontFromDC(self->hDC[self->iActiveBuffer], &self->pFont);
	if (Ok != status) {
		PyErr_Format(PyExc_RuntimeError, "Cannot create font.");
		return -1;
	}
	status = GdipStringFormatGetGenericDefault(&self->pStringFormat);
	if (Ok != status) {
		PyErr_Format(PyExc_RuntimeError, "Cannot create format.");
		return -1;
	}

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

static PyObject*
TyCanvas_set_pen(TyCanvasObject* self, PyObject* args, PyObject* kwds)
{
	int iRed = 0, iGreen = 0, iBlue = 0, iAlpha = 255, iWidth = 1;
	static char* kwlist[] = { "red", "green", "blue", "alpha","width", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iiiii", kwlist,
		&iRed,
		&iGreen,
		&iBlue,
		&iAlpha,
		&iWidth))
		return NULL;

	GdipDeletePen(self->pPen);
	GdipDeleteBrush(self->pBrush);
	GdipCreatePen1(MakeARGB(iAlpha, iRed, iGreen, iBlue), iWidth, UnitPixel, &self->pPen);
	if (status != Ok) {
		PyErr_Format(PyExc_RuntimeError, "Cannot create pen.");
		return NULL;
	}
	status = GdipCreateSolidFill(MakeARGB(iAlpha, iRed, iGreen, iBlue), &self->pBrush);
	if (status != Ok) {
		PyErr_Format(PyExc_RuntimeError, "Cannot create brush.");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_ellipse(TyCanvasObject* self, PyObject* args)
{
	int iX1, iY1, iX2, iY2;
	BOOL bFill = FALSE;
	if (!PyArg_ParseTuple(args, "iiii|b",
		&iX1,
		&iY1,
		&iX2,
		&iY2,
		&bFill))
		return NULL;

	if (bFill)
		status = GdipFillEllipseI(self->pGraphics[self->iActiveBuffer], self->pBrush, iX1, iY1, iX2, iY2);
	else
		status = GdipDrawEllipseI(self->pGraphics[self->iActiveBuffer], self->pPen, iX1, iY1, iX2, iY2);
	if (status != Ok) {
		PyErr_Format(PyExc_RuntimeError, "Cannot draw ellipse.");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_line(TyCanvasObject* self, PyObject* args)
{
	int iX1, iY1, iX2, iY2;
	if (!PyArg_ParseTuple(args, "iiii",
		&iX1,
		&iY1,
		&iX2,
		&iY2))
		return NULL;

	status = GdipDrawLineI(self->pGraphics[self->iActiveBuffer], self->pPen, iX1, iY1, iX2, iY2);
	if (status != Ok) {
		PyErr_Format(PyExc_RuntimeError, "Cannot draw line.");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_rectangle(TyCanvasObject* self, PyObject* args)
{
	int iX1, iY1, iX2, iY2;
	BOOL bFill = FALSE;
	if (!PyArg_ParseTuple(args, "iiii|b",
		&iX1,
		&iY1,
		&iX2,
		&iY2,
		&bFill))
		return NULL;

	if (bFill)
		status = GdipFillRectangleI(self->pGraphics[self->iActiveBuffer], self->pBrush, iX1, iY1, iX2, iY2);
	else
		status = GdipDrawRectangleI(self->pGraphics[self->iActiveBuffer], self->pPen, iX1, iY1, iX2, iY2);
	if (status != Ok) {
		PyErr_Format(PyExc_RuntimeError, "Cannot draw rectangle.");
		return NULL;
	}

	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_polygon(TyCanvasObject* self, PyObject* args)
{
	PyObject* pyPoints;
	BOOL bFill = FALSE;
	if (!PyArg_ParseTuple(args, "O|b",
		&pyPoints,
		&bFill))
		return NULL;

	if (!PyList_Check(pyPoints)) {
		PyErr_Format(PyExc_TypeError, "Argument 1 must be a list of lists.");
		return NULL;
	}

	Py_ssize_t n, iPoints = PyList_Size(pyPoints);
	GpPoint point;
	GpPoint* paPoints = (GpPoint*)PyMem_RawMalloc(sizeof(GpPoint) * iPoints);
	PyObject* pyPoint, * pyX, * pyY;
	for (n = 0; n < iPoints; n++) {
		pyPoint = PyList_GetItem(pyPoints, n);
		if (PyList_Check(pyPoint)) {
			pyX = PyList_GetItem(pyPoint, 0);
			pyY = PyList_GetItem(pyPoint, 1);
			point.X = PyLong_AsLong(pyX);
			point.Y = PyLong_AsLong(pyY);
			paPoints[n] = point;
		}
		else {
			PyErr_Format(PyExc_RuntimeError, "Argument 2 1 must be a list of lists. Item %d is not.", n);
			return NULL;
		}
	}

	if (bFill)
		status = GdipFillPolygonI(self->pGraphics[self->iActiveBuffer], self->pBrush, paPoints, iPoints, FillModeAlternate);
	else
		status = GdipDrawPolygonI(self->pGraphics[self->iActiveBuffer], self->pPen, paPoints, iPoints);
	if (status != Ok) {
		PyErr_Format(PyExc_RuntimeError, "Cannot draw polygon.");
		return NULL;
	}
	PyMem_Free(paPoints);
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_text(TyCanvasObject* self, PyObject* args)
{
	int iX, iY, iX2, iY2;
	const LPCSTR strText;
	LPWSTR szText;

	if (!PyArg_ParseTuple(args, "iiiis",
		&iX,
		&iY,
		&iX2,
		&iY2,
		&strText))
		return NULL;

	szText = toW(strText);
	status = GdipDrawString(self->pGraphics[self->iActiveBuffer], szText, -1, self->pFont, &((GpRectF) { iX, iY, iX2, iY2 }), self->pStringFormat, self->pBrush);
	if (Ok != status) {
		PyErr_Format(PyExc_RuntimeError, "Cannot draw string.");
		return NULL;
	}

	PyMem_RawFree(szText);
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_image(TyCanvasObject* self, PyObject* args)
{
	int iX, iY, iWidth = 0, iHeight = 0, iOrg;
	PyObject* pyImage;
	GpImage* pImage;

	if (!PyArg_ParseTuple(args, "iiO|ii",
		&iX,
		&iY,
		&pyImage,
		&iWidth,
		&iHeight))
		return NULL;

	if (!PyUnicode_Check(pyImage) && !PyBytes_Check(pyImage)) {
		PyErr_Format(PyExc_TypeError, "Argument 3 must be either a 'str' holding the file name or a 'bytes' object.");
		return NULL;
	}

	if (PyUnicode_Check(pyImage)) {
		LPCSTR strText = PyUnicode_AsUTF8(pyImage);
		LPWSTR szText = toW(strText);
		status = GdipLoadImageFromFile(szText, &pImage);
		PyMem_RawFree(szText);
	}
	else if (PyBytes_Check(pyImage)) {
		IStream* pStream = SHCreateMemStream((BYTE*)PyBytes_AsString(pyImage), PyBytes_GET_SIZE(pyImage));
		status = GdipLoadImageFromStream(pStream, &pImage);
		IUnknown_AtomicRelease((void**)&pStream);
	}
	if (Ok != status) {
		PyErr_Format(PyExc_RuntimeError, "Cannot load image.");
		return NULL;
	}

	if (iWidth == 0 && iHeight == 0) {
		GdipDrawImageI(self->pGraphics[self->iActiveBuffer], pImage, iX, iY);
	}
	else {
		UINT uWidth;
		UINT uHeight;
		double dAspectRatio;
		double dPictureAspectRatio;
		int iOffsetX = 0;
		int iOffsetY = 0;

		GdipGetImageWidth(pImage, &uWidth);
		GdipGetImageHeight(pImage, &uHeight);
		dPictureAspectRatio = (double)uWidth / uHeight;

		if (iWidth == 0) {
			iWidth = uWidth * iHeight / uHeight;
		}
		else if (iHeight == 0) {
			iHeight = uHeight * iWidth / uWidth;
		}
		else {
			dAspectRatio = (double)iWidth / iHeight;
			if (dPictureAspectRatio > dAspectRatio)
			{
				iOrg = iHeight;
				iHeight = (int)(iWidth / dPictureAspectRatio);
				iOffsetY = (iOrg - iHeight) / 2;
			}
			else if (dPictureAspectRatio < dAspectRatio)
			{
				iOrg = iWidth;
				iWidth = (int)(iHeight * dPictureAspectRatio);
				iOffsetX = (iOrg - iWidth) / 2;
			}
		}

		GpImageAttributes* pImageAttributes;
		GdipCreateImageAttributes(&pImageAttributes);
		GdipDrawImageRectRectI(self->pGraphics[self->iActiveBuffer], pImage, iX + iOffsetX, iY + iOffsetY, iWidth, iHeight,
			0, 0, uWidth, uHeight, UnitPixel, pImageAttributes, NULL, NULL);
		GdipDisposeImageAttributes(pImageAttributes);
	}
	GdipDisposeImage(pImage);
	Py_RETURN_NONE;
}

static BOOL
TyCanvas_DeleteBuffer(TyCanvasObject* self, int iBuffer)
{
	if (self->hDC[iBuffer]) {
		status = GdipDeleteGraphics(self->pGraphics[iBuffer]);
		if (Ok != status) {
			PyErr_Format(PyExc_TypeError, "Cannot delete Graphics.");
			return FALSE;
		}
		if (!DeleteObject(self->hBM[iBuffer])) {
			PyErr_SetFromWindowsErr(0);
			return FALSE;
		}

		if (!DeleteDC(self->hDC[iBuffer])) {
			PyErr_SetFromWindowsErr(0);
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL
TyCanvas_RenewBuffer(TyCanvasObject* self, int iBuffer, int iWidth, int iHeight)
{
	if (!TyCanvas_DeleteBuffer(self, iBuffer))
		return FALSE;

	HDC hDC = GetDC(self->hWin);
	if (hDC == 0) {
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}
	self->hBM[iBuffer] = CreateCompatibleBitmap(hDC, iWidth, iHeight);
	self->hDC[iBuffer] = CreateCompatibleDC(hDC);
	SelectObject(self->hDC[iBuffer], self->hBM[iBuffer]);
	BitBlt(self->hDC[iBuffer], 0, 0, iWidth, iHeight, NULL, 0, 0, WHITENESS);
	GdipCreateFromHDC(self->hDC[iBuffer], &self->pGraphics[iBuffer]);
	ReleaseDC(self->hWin, hDC);
	return TRUE;
}

static PyObject*
TyCanvas_renew_buffer(TyCanvasObject* self, PyObject* args)
{
	int iBuffer = self->iActiveBuffer;
	int iWidth = self->rcClient.right, iHeight = self->rcClient.bottom;
	if (!PyArg_ParseTuple(args, "|iii",
		&iBuffer,
		&iWidth,
		&iHeight))
		return NULL;

	if (!TyCanvas_RenewBuffer(self, iBuffer, iWidth, iHeight))
		return NULL;

	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_resize_buffer(TyCanvasObject* self, PyObject* args)
{
	int iBuffer = self->iActiveBuffer;
	int iWidth = self->rcClient.right, iHeight = self->rcClient.bottom, iX = 0, iY = 0;
	if (!PyArg_ParseTuple(args, "|iiiii",
		&iBuffer,
		&iWidth,
		&iHeight,
		&iX,
		&iY))
		return NULL;

	if (self->hDC[iBuffer] == 0) {
		if (!TyCanvas_RenewBuffer(self, iBuffer, iWidth, iHeight)) {
			return NULL;
		}
	}
	else {
		status = GdipDeleteGraphics(self->pGraphics[iBuffer]);
		if (Ok != status) {
			PyErr_Format(PyExc_TypeError, "Cannot delete Graphics.");
			return NULL;
		}

		HBITMAP hBmOld = self->hBM[iBuffer];
		HDC hDcOld = self->hDC[iBuffer];
		HDC hDC = GetDC(self->hWin);
		self->hBM[iBuffer] = CreateCompatibleBitmap(hDC, iWidth, iHeight);
		self->hDC[iBuffer] = CreateCompatibleDC(hDC);
		SelectObject(self->hDC[iBuffer], self->hBM[iBuffer]);
		BitBlt(self->hDC[iBuffer], 0, 0, iWidth, iHeight, NULL, 0, 0, WHITENESS);
		BitBlt(self->hDC[iBuffer], iX, iY, iWidth, iHeight, hDcOld, 0, 0, SRCCOPY);

		GdipCreateFromHDC(self->hDC[iBuffer], &self->pGraphics[iBuffer]);
		ReleaseDC(self->hWin, hDC);

		if (!DeleteDC(hDcOld) || !DeleteObject(hBmOld)) {
			PyErr_SetFromWindowsErr(0);
			return NULL;
		}
	}
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_copy_buffer(TyCanvasObject* self, PyObject* args)
{
	int iBuffer1, iBuffer2;
	if (!PyArg_ParseTuple(args, "ii",
		&iBuffer1,
		&iBuffer2))
		return NULL;

	if (self->hDC[iBuffer1] == 0) {
		PyErr_SetString(PyExc_IndexError, "Source buffer is still empty.");
		return NULL;
	}

	// Initialize buffer if it is not yet
	if (self->hDC[iBuffer2] == 0 && !TyCanvas_RenewBuffer(self, iBuffer2, self->rcClient.right, self->rcClient.bottom))
		return NULL;

	BITMAP BitMap1, BitMap2;
	GetObject(self->hBM[iBuffer1], sizeof(BITMAP), (LPVOID)&BitMap1);
	GetObject(self->hBM[iBuffer2], sizeof(BITMAP), (LPVOID)&BitMap2);

	BitBlt(self->hDC[iBuffer2], 0, 0, BitMap2.bmWidth, BitMap2.bmHeight, NULL, 0, 0, WHITENESS);
	BitBlt(self->hDC[iBuffer2], 0, 0, BitMap1.bmWidth, BitMap1.bmHeight, self->hDC[iBuffer1], 0, 0, SRCCOPY);

	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_clear_buffer(TyCanvasObject* self, PyObject* args)
{
	int iBuffer = self->iActiveBuffer;
	if (!PyArg_ParseTuple(args, "|i",
		&iBuffer))
		return NULL;

	if (self->hDC[iBuffer] == 0) {
		if (!TyCanvas_RenewBuffer(self, iBuffer, self->rcClient.right, self->rcClient.bottom)) {
			return NULL;
		}
	}
	else {
		BITMAP BitMap;
		GetObject(self->hBM[iBuffer], sizeof(BITMAP), (LPVOID)&BitMap);
		BitBlt(self->hDC[iBuffer], 0, 0, BitMap.bmWidth, BitMap.bmHeight, NULL, 0, 0, WHITENESS);
	}
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_refresh(TyCanvasObject* self)
{
	RedrawWindow(self->hWin, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	Py_RETURN_NONE;
}

static int
TyCanvas_setattro(TyCanvasObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "active_buffer") == 0) {
			if (PyLong_Check(pyValue)) {
				int iIndex = PyLong_Check(pyValue);
				if (iIndex < 0 || iIndex >= MAX_PIXBUFFERS) {
					PyErr_SetString(PyExc_IndexError, "Index out of range.");
					return -1;
				}

				if (self->hDC[iIndex] == 0 && !TyCanvas_RenewBuffer(self, iIndex, self->rcClient.right, self->rcClient.bottom))
					return -1;
				self->iActiveBuffer = iIndex;
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "PLease assign an int specifying the index of the buffer.");
				return -1;
			}
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "live_buffer") == 0) {
			if (PyLong_Check(pyValue)) {
				int iIndex = PyLong_Check(pyValue);
				if (iIndex < 0 || iIndex >= MAX_PIXBUFFERS) {
					PyErr_SetString(PyExc_IndexError, "Index out of range.");
					return -1;
				}

				if (self->hDC[iIndex] == 0 && !TyCanvas_RenewBuffer(self, iIndex, self->rcClient.right, self->rcClient.bottom))
					return -1;
				self->iLiveBuffer = iIndex;
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "PLease assign an int specifying the index of the buffer.");
				return -1;
			}
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "anti_alias") == 0) {
			if (PyBool_Check(pyValue)) {
				GdipSetSmoothingMode(self->pGraphics[self->iActiveBuffer], pyValue == Py_True ? SmoothingModeAntiAlias : SmoothingModeNone);
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a bool!");
				return -1;
			}
		}
	}
	return Py_TYPE(self)->tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyCanvas_getattro(TyCanvasObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {

	}
	Py_XDECREF(pyResult);
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyCanvas_dealloc(TyCanvasObject* self)
{
	if (!TyCanvas_DeleteBuffer(self, 0)) {
		PyErr_SetFromWindowsErr(0);
		PyErr_Print();
	};
	if (!TyCanvas_DeleteBuffer(self, 1)) {
		PyErr_SetFromWindowsErr(0);
		PyErr_Print();
	};

	GdipDeleteStringFormat(self->pStringFormat);
	GdipDeleteFont(self->pFont);
	GdipDeleteFontFamily(self->pFontFamily);
	GdipDeletePen(self->pPen);
	GdipDeleteBrush(self->pBrush);
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyCanvas_members[] = {
	{ "active_buffer", T_INT, offsetof(TyCanvasObject, iActiveBuffer), READONLY, "Index of the bitmap buffer drawing operations work on" },
	{ "live_buffer", T_INT, offsetof(TyCanvasObject, iLiveBuffer), READONLY, "Index of the bitmap buffer that is displayed when the widget gets repainted by the OS" },
	{ "on_mouse_move", T_OBJECT_EX, offsetof(TyCanvasObject, pyOnMouseMoveCB), 0, "On mouse move callback" },
	{ "on_mouse_wheel", T_OBJECT_EX, offsetof(TyCanvasObject, pyOnMouseWheelCB), 0, "On mouse wheel rotated callback" },
	{ "on_l_button_down", T_OBJECT_EX, offsetof(TyCanvasObject, pyOnLMouseDownCB), 0, "On left mouse button down callback" },
	{ "on_l_button_up", T_OBJECT_EX, offsetof(TyCanvasObject, pyOnLMouseUpCB), 0, "On left mouse button up callback" },
	{ "on_resize", T_OBJECT_EX, offsetof(TyCanvasObject, pyOnResizeCB), 0, "Callback when size changed" },
	{ NULL }
};

static PyMethodDef TyCanvas_methods[] = {
	{ "set_pen", (PyCFunction)TyCanvas_set_pen, METH_VARARGS | METH_KEYWORDS, "Sets RGBA color and thickness of pen." },
	{ "ellipse", (PyCFunction)TyCanvas_ellipse, METH_VARARGS, "Draws a point." },
	{ "line", (PyCFunction)TyCanvas_line, METH_VARARGS, "Draws a line." },
	{ "rectangle", (PyCFunction)TyCanvas_rectangle, METH_VARARGS, "Draws a rectangle." },
	{ "polygon", (PyCFunction)TyCanvas_polygon, METH_VARARGS, "Draws a polygon." },
	{ "text", (PyCFunction)TyCanvas_text, METH_VARARGS, "Draws a text string." },
	{ "image", (PyCFunction)TyCanvas_image, METH_VARARGS, "Places a bitmap." },
	{ "renew_buffer", (PyCFunction)TyCanvas_renew_buffer, METH_VARARGS, "Creates a new buffer." },
	{ "resize_buffer", (PyCFunction)TyCanvas_resize_buffer, METH_VARARGS, "Adjusts the size of the buffer." },
	{ "copy_buffer", (PyCFunction)TyCanvas_copy_buffer, METH_VARARGS, "Copy bitmap data from one buffer do the other." },
	{ "clear_buffer", (PyCFunction)TyCanvas_clear_buffer, METH_VARARGS, "Paint the buffer white." },
	{ "refresh", (PyCFunction)TyCanvas_refresh, METH_NOARGS, "Trigger the paint process." },
	{ NULL }
};

PyTypeObject TyCanvasType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Canvas",           /* tp_name */
	sizeof(TyCanvasObject),    /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyCanvas_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	0,                         /* tp_str */
	TyCanvas_getattro,         /* tp_getattro */
	TyCanvas_setattro,         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Canvas, a widget for direct painting", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyCanvas_methods,          /* tp_methods */
	TyCanvas_members,          /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyCanvas_init,   /* tp_init */
	0,                         /* tp_alloc */
	TyCanvas_new,              /* tp_new */
};


LRESULT CALLBACK TyCanvasWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL TyCanvasType_Init()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;// CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TyCanvasWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g->hInstance;
	wc.hIcon = 0;// g->hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"TyCanvasClass";
	wc.hIconSm = 0;

	if (!RegisterClassEx(&wc)) {
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}
	return TRUE;
}

static
LRESULT CALLBACK TyCanvasWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PyObject* pyResult;
	HDC hDC;
	RECT rect;

	TyCanvasObject* self = (TyCanvasObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (self) {
		switch (uMsg)
		{

		case WM_PAINT:
		{
			// Swap in the live buffer
			hDC = BeginPaint(self->hWin, &ps);
			SetBkMode(hDC, TRANSPARENT);
			BitBlt(hDC, 0, 0, self->rcClient.right, self->rcClient.bottom, self->hDC[self->iLiveBuffer], 0, 0, SRCCOPY);
			EndPaint(self->hWin, &ps);
			return 0;
		}

		case WM_SIZE:
			self->rcClient.right = LOWORD(lParam);
			self->rcClient.bottom = HIWORD(lParam);
			if (self->pyOnResizeCB) {
				pyResult = PyObject_CallFunction(self->pyOnResizeCB, "(O)", self);
				if (pyResult == NULL) {
					PyErr_Print();
				}
				else Py_DECREF(pyResult);
			}
			//break;
			return 0;

		case WM_ERASEBKGND:
			return 1;

		case WM_LBUTTONDOWN:
			if (self->pyOnLMouseDownCB) {
				pyResult = PyObject_CallFunction(self->pyOnLMouseDownCB, "(Oii)", self, LOWORD(lParam), HIWORD(lParam));
				if (pyResult == NULL) {
					PyErr_Print();
					MessageBox(NULL, L"Error in on_l_mouse_down callback", L"Error", MB_ICONERROR);
				}
				else
					Py_DECREF(pyResult);
			}
			return 0L;

		case WM_LBUTTONUP:
			if (self->pyOnLMouseUpCB) {
				pyResult = PyObject_CallFunction(self->pyOnLMouseUpCB, "(Oii)", self, LOWORD(lParam), HIWORD(lParam));
				if (pyResult == NULL) {
					PyErr_Print();
					MessageBox(NULL, L"Error in on_l_mouse_up callback", L"Error", MB_ICONERROR);
				}
				else
					Py_DECREF(pyResult);
			}
			return 0L;

		case WM_MOUSEMOVE:
			if (self->pyOnMouseMoveCB) {
				pyResult = PyObject_CallFunction(self->pyOnMouseMoveCB, "(Oii)", self, LOWORD(lParam), HIWORD(lParam));
				if (pyResult == NULL) {
					PyErr_Print();
					MessageBox(NULL, L"Error in  on_mouse_move callback", L"Error", MB_ICONERROR);
				}
				else
					Py_DECREF(pyResult);
			}
			return 0L;

		case WM_MOUSEWHEEL:
			if (self->pyOnMouseWheelCB) {
				POINT pt;
				pt.x = GET_X_LPARAM(lParam);
				pt.y = GET_Y_LPARAM(lParam);
				ScreenToClient(hwnd, &pt);
				pyResult = PyObject_CallFunction(self->pyOnMouseWheelCB, "(Oiii)", self, GET_WHEEL_DELTA_WPARAM(wParam), pt.x, pt.y);
				if (pyResult == NULL) {
					PyErr_Print();
					MessageBox(NULL, L"Error in  on_mouse_wheel callback", L"Error", MB_ICONERROR);
				}
				else
					Py_DECREF(pyResult);
			}
			return 0L;
		}
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}