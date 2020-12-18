// ImageView.c  | Tymber © 2020 by Thomas Führinger

#include "Tymber.h"
#include "Resource.h"

static BITMAP bitmap;

static PyObject*
TyImageView_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyImageViewObject* self = (TyImageViewObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->bStretch = TRUE;
		self->bFill = FALSE;
		self->pyOnLMouseDownCB = NULL;
		self->pyOnRMouseDownCB = NULL;
		self->pyOnMouseWheelCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyImageView_init(TyImageViewObject* self, PyObject* args, PyObject* kwds)
{
	if (Py_TYPE(self)->tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);
	self->hWin = CreateWindowExW(0, L"TyImageViewClass", L"",
		WS_CHILD | WS_VISIBLE,
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TyImageView, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	self->pyDataType = NULL;
	//Py_INCREF(self->pyDataType);

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

BOOL
TyImageView_RenderData(TyImageViewObject* self, BOOL bFormat)
{
	InvalidateRect(self->hWin, NULL, TRUE);
	return TRUE;
}

BOOL
TyImageView_SetData(TyImageViewObject* self, PyObject* pyData)
{
	if (self->pyData == pyData)
		return TRUE;
	if (!TyWidget_SetData((TyWidgetObject*)self, pyData))
		return FALSE;

	return TyImageView_RenderData(self, TRUE);
}

static int
TyImageView_setattro(TyImageViewObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			if (pyValue != Py_None && !PyUnicode_Check(pyValue) && !PyBytes_Check(pyValue)) {
				PyErr_Format(PyExc_TypeError, "Please assign either a 'str' holding the file name or a 'bytes' object.");
				return -1;
			}
			if (!TyImageView_SetData(self, pyValue))
				return -1;
			return 0;
		}
	}
	return Py_TYPE(self)->tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyImageView_getattro(TyImageViewObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		// for later
	}
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyImageView_dealloc(TyImageViewObject* self)
{
	DestroyWindow(self->hWin);
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyImageView_members[] = {
	{ "data", T_OBJECT, offsetof(TyImageViewObject, pyData), READONLY, "Data value" },
	{ "stretch", T_BOOL, offsetof(TyImageViewObject, bStretch), 0, "Stretch or squeeze to fit into widget, keeping aspect ratio." },
	//{ "fill", T_BOOL, offsetof(TyImageViewObject, bFill), 0, "Stretch to fit into widget, filling all available space (distorting)." },
	{ NULL }
};

static PyMethodDef TyImageView_methods[] = {
	{ NULL }
};

PyTypeObject TyImageViewType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.ImageView",        /* tp_name */
	sizeof(TyImageViewObject), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyImageView_dealloc, /* tp_dealloc */
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
	TyImageView_getattro,      /* tp_getattro */
	TyImageView_setattro,      /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Displays an Image.",      /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyImageView_methods,       /* tp_methods */
	TyImageView_members,       /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyImageView_init, /* tp_init */
	0,                         /* tp_alloc */
	TyImageView_new,           /* tp_new */
	PyObject_Free,             /* tp_free */
};


LRESULT CALLBACK TyImageViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL
TyImageViewType_Init()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TyImageViewWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g->hInstance;
	wc.hIcon = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"TyImageViewClass";
	wc.hIconSm = NULL;

	if (!RegisterClassEx(&wc)) {
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}
	return TRUE;
}

static
LRESULT CALLBACK TyImageViewWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int iWidth, iHeight;
	RECT rcClient;
	int iOrg;
	PAINTSTRUCT paintStruct;
	HDC hDC;
	PyObject* pyImage, * pyResult;
	GpImage* pImage;
	Graphics pGraphics;
	GpStatus status = GenericError;
	TyImageViewObject* self = (TyImageViewObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (self) {
		switch (uMsg)
		{
		case WM_PAINT:
			if (self->pyData && self->pyData != Py_None) {
				pyImage = (PyObject*)self->pyData;
				GetClientRect(self->hWin, &rcClient);
				iWidth = rcClient.right;
				iHeight = rcClient.bottom;

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

				hDC = BeginPaint(hwnd, &paintStruct);
				status = GdipCreateFromHDC(hDC, &pGraphics);

				UINT uWidth;
				UINT uHeight;
				GpBitmap* pImageTmp;
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
				if (self->bStretch)
					GdipDrawImageRectRectI(pGraphics, pImage, iOffsetX, iOffsetY, iWidth, iHeight,
						0, 0, uWidth, uHeight, UnitPixel, pImageAttributes, NULL, NULL);
				else
					GdipDrawImageI(pGraphics, pImage, 0, 0);
				GdipDisposeImageAttributes(pImageAttributes);

				GdipDisposeImage(pImage);
				GdipDeleteGraphics(pGraphics);

				EndPaint(hwnd, &paintStruct);
			}
			break;

		case WM_RBUTTONDOWN:
			if (self->pyOnRMouseDownCB) {
				pyResult = PyObject_CallFunction(self->pyOnRMouseDownCB, "(Oii)", self, LOWORD(lParam), HIWORD(lParam));
				if (pyResult == NULL) {
					PyErr_Print();
					MessageBox(NULL, L"Error in Python script", L"Error", MB_ICONERROR);
				}
				else
					Py_DECREF(pyResult);
			}
			return 0L;

		case WM_LBUTTONDOWN:
			if (self->pyOnLMouseDownCB) {
				pyResult = PyObject_CallFunction(self->pyOnLMouseDownCB, "(Oii)", self, LOWORD(lParam), HIWORD(lParam));
				if (pyResult == NULL) {
					PyErr_Print();
					MessageBox(NULL, L"Error in Python script", L"Error", MB_ICONERROR);
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
					MessageBox(NULL, L"Error in Python script", L"Error", MB_ICONERROR);
				}
				else
					Py_DECREF(pyResult);
			}
			return 0L;

		case WM_ERASEBKGND:
			return 1;

		case WM_DESTROY:
			Py_DECREF(self);
			return 0;
		}

		// pick the right DefXxxProcW
		if (PyObject_TypeCheck((PyObject*)self, &TyMdiWindowType))
			return DefMDIChildProcW(hwnd, uMsg, wParam, lParam);
		if (PyObject_TypeCheck((PyObject*)self, &TyWindowType) && ((TyWindowObject*)self)->hMdiArea) {
			return DefFrameProcW(hwnd, ((TyWindowObject*)self)->hMdiArea, uMsg, wParam, lParam);
		}
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}