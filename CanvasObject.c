// CanvasObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static	PAINTSTRUCT ps;

static PyObject *
TyCanvas_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyCanvasObject* self = (TyCanvasObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->pyOnPaintCB = NULL;
		self->hDC = 0;
		self->hPen = 0;
		self->hBrush = 0;
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

	RECT rect;
	TyWidget_CalculateRect(self, &rect);
	self->hWin = CreateWindowEx(0, L"TyCanvasClass", L"",
		WS_CHILD | WS_VISIBLE,
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYCANVAS, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	if (self->pyCaption != NULL)
		if (!TyWidget_SetCaption((TyWidgetObject*)self, self->pyCaption))
			return -1;
	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));

	return 0;
}

static PyObject*
TyCanvas_point(TyCanvasObject* self, PyObject *args)
{
	int iX, iY;
	RECT rcClient;

	if (self->hDC == 0) {
		PyErr_SetString(PyExc_RuntimeError, "This method works only inside an 'on_paint' callback.");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "ii",
		&iX,
		&iY))
		return NULL;

	GetClientRect(self->hWin, &rcClient);
	if (iX < 0)
		iX += rcClient.right;
	if (iY < 0)
		iY += rcClient.bottom;

	COLORREF c = SetPixel(self->hDC, iX, iY, RGB(1, 1, 1));
	if (c == 1) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_move_to(TyCanvasObject* self, PyObject *args)
{
	int iX, iY;
	RECT rcClient;

	if (self->hDC == 0) {
		PyErr_SetString(PyExc_RuntimeError, "This method works only inside an 'on_paint' callback.");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "ii",
		&iX,
		&iY))
		return NULL;

	GetClientRect(self->hWin, &rcClient);
	if (iX < 0)
		iX += rcClient.right;
	if (iY < 0)
		iY += rcClient.bottom;

	if (MoveToEx(self->hDC, iX, iY, (LPPOINT)NULL) == 0) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_line_to(TyCanvasObject* self, PyObject *args)
{
	int iX, iY;
	RECT rcClient;

	if (self->hDC == 0) {
		PyErr_SetString(PyExc_RuntimeError, "This method works only inside an 'on_paint' callback.");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "ii",
		&iX,
		&iY))
		return NULL;

	GetClientRect(self->hWin, &rcClient);
	if (iX < 0)
		iX += rcClient.right;
	if (iY < 0)
		iY += rcClient.bottom;

	if (LineTo(self->hDC, iX, iY) == 0) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_rectangle(TyCanvasObject* self, PyObject *args)
{
	RECT rcRel, rcAbs;
	int iRadius = -1;
	BOOL bResult;

	if (self->hDC == 0) {
		PyErr_SetString(PyExc_RuntimeError, "This method works only inside an 'on_paint' callback.");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "iiii|i",
		&rcRel.left,
		&rcRel.top,
		&rcRel.right,
		&rcRel.bottom,
		&iRadius))
		return NULL;

	RECT rcParent;
	GetClientRect(self->hWin, &rcParent);
	TransformRectToAbs(rcRel, rcParent, &rcAbs);

	if (iRadius == -1)
		bResult = Rectangle(self->hDC, rcAbs.left, rcAbs.top, rcAbs.right, rcAbs.bottom);
	else
		bResult = RoundRect(self->hDC, rcAbs.left, rcAbs.top, rcAbs.right, rcAbs.bottom, iRadius, iRadius);

	if (bResult)
		Py_RETURN_NONE;
	else {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
}

static PyObject*
TyCanvas_text(TyCanvasObject* self, PyObject *args)
{
	int iX, iY;
	const LPCSTR strText;
	LPWSTR szText;
	RECT rcClient;

	if (self->hDC == 0) {
		PyErr_SetString(PyExc_RuntimeError, "This method works only inside an 'on_paint' callback.");
		return NULL;
	}

	if (!PyArg_ParseTuple(args, "iis",
		&iX,
		&iY,
		&strText))
		return NULL;

	GetClientRect(self->hWin, &rcClient);
	if (iX < 0)
		iX += rcClient.right;
	if (iY < 0)
		iY += rcClient.bottom;

	szText = toW(strText);
	if (TextOut(self->hDC, iX, iY, szText, _tcslen(szText)) == 0) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	PyMem_RawFree(szText);
	Py_RETURN_NONE;
}

static PyObject*
TyCanvas_repaint(TyCanvasObject* self)
{
	RedrawWindow(self->hWin, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
	Py_RETURN_NONE;
}

static int
TyCanvas_setattro(TyCanvasObject* self, PyObject* pyAttributeName, PyObject *pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "on_paint") == 0) {
			if (PyCallable_Check(pyValue)) {
				Py_XINCREF(pyValue);
				Py_XDECREF(self->pyOnPaintCB);
				self->pyOnPaintCB = pyValue;
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
				return -1;
			}
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "text_color") == 0) {
			if (PyTuple_Check(pyValue)) {
				if (self->hDC == 0) {
					PyErr_SetString(PyExc_RuntimeError, "text_color can only be assigned inside an 'on_paint' callback.");
					return -1;
				}
				int iR, iG, iB;
				PyObject* pyColorComponent = PyTuple_GetItem(pyValue, 0);
				if (pyColorComponent == NULL)
					return -1;
				iR = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 1);
				if (pyColorComponent == NULL)
					return -1;
				iG = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 2);
				if (pyColorComponent == NULL)
					return -1;
				iB = PyLong_AsLong(pyColorComponent);

				SetTextColor(self->hDC, RGB(iR, iG, iB));
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a tuple of RGB values!");
				return -1;
			}
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "pen_color") == 0) {
			if (PyTuple_Check(pyValue)) {
				if (self->hDC == 0) {
					PyErr_SetString(PyExc_RuntimeError, "pen_color can only be assigned inside an 'on_paint' callback.");
					return -1;
				}
				int iR, iG, iB;
				PyObject* pyColorComponent = PyTuple_GetItem(pyValue, 0);
				if (pyColorComponent == NULL)
					return -1;
				iR = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 1);
				if (pyColorComponent == NULL)
					return -1;
				iG = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 2);
				if (pyColorComponent == NULL)
					return -1;
				iB = PyLong_AsLong(pyColorComponent);

				if (self->hPen)
					DeleteObject(self->hPen);
				self->hPen = CreatePen(PS_SOLID, 1, RGB(iR, iG, iB));
				SelectObject(self->hDC, self->hPen);
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a tuple of RGB values!");
				return -1;
			}
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "fill_color") == 0) {
			if (PyTuple_Check(pyValue)) {
				if (self->hDC == 0) {
					PyErr_SetString(PyExc_RuntimeError, "fill_color can only be assigned inside an 'on_paint' callback.");
					return -1;
				}
				int iR, iG, iB;
				PyObject* pyColorComponent = PyTuple_GetItem(pyValue, 0);
				if (pyColorComponent == NULL)
					return -1;
				iR = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 1);
				if (pyColorComponent == NULL)
					return -1;
				iG = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 2);
				if (pyColorComponent == NULL)
					return -1;
				iB = PyLong_AsLong(pyColorComponent);

				if (self->hBrush)
					DeleteObject(self->hBrush);
				self->hBrush = CreateSolidBrush(RGB(iR, iG, iB));
				SelectObject(self->hDC, self->hBrush);
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a tuple of RGB values!");
				return -1;
			}
		}
	}
	return Py_TYPE(self)->tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject *
TyCanvas_getattro(TyCanvasObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "on_click") == 0) {
			PyErr_Clear();
			return self->pyOnPaintCB;
		}
	}
	Py_XDECREF(pyResult);
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyCanvas_dealloc(TyCanvasObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject *)self);
}

static PyMemberDef TyCanvas_members[] = {
	{ NULL }
};

static PyMethodDef TyCanvas_methods[] = {
	{ "point", (PyCFunction)TyCanvas_point, METH_VARARGS, "Draws a point." },
	{ "move_to", (PyCFunction)TyCanvas_move_to, METH_VARARGS, "Moves to a point." },
	{ "line_to", (PyCFunction)TyCanvas_line_to, METH_VARARGS, "Draws a line." },
	{ "rectangle", (PyCFunction)TyCanvas_rectangle, METH_VARARGS, "Draws a rectangle." },
	{ "text", (PyCFunction)TyCanvas_text, METH_VARARGS, "Draws a text string." },
	{ "repaint", (PyCFunction)TyCanvas_repaint, METH_NOARGS, "Trigger the paint process." },
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
	wc.style = CS_HREDRAW | CS_VREDRAW;
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
	TyCanvasObject* self = (TyCanvasObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{

	case WM_PAINT:
	{
		if (self->pyOnPaintCB) {
			self->hDC = BeginPaint(self->hWin, &ps);
			SelectObject(self->hDC, g->hfDefaultFont);
			SetBkMode(self->hDC, TRANSPARENT);
			PyObject* pyArgs = PyTuple_Pack(1, (PyObject*)self);
			Py_INCREF(self);
			PyObject* pyResult = PyObject_CallObject(self->pyOnPaintCB, pyArgs);
			//ReleaseDC (self->hWin, self->hDC); 
			EndPaint(self->hWin, &ps);
			self->hDC = 0;
			if (pyResult == NULL) {
				PyErr_Print();
				MessageBox(NULL, L"Error in Python script", L"Error", MB_ICONERROR);
			}
			else {
				Py_DECREF(pyResult);
			}
		}
		return 0;
	}

	case WM_SIZE:
		return 0;		
	}

	// pick the right DefXxxProcW
	if (self) {
		if (PyObject_TypeCheck((PyObject*)self, &TyMdiWindowType))
			return DefMDIChildProcW(hwnd, uMsg, wParam, lParam);
		if (PyObject_TypeCheck((PyObject*)self, &TyWindowType) && ((TyWindowObject*)self)->hMdiArea) {
			return DefFrameProcW(hwnd, ((TyWindowObject*)self)->hMdiArea, uMsg, wParam, lParam);
		}
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}