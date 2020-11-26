// StatusBar.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static LPWSTR szStatusBarClass = L"TyStatusBarClass";

static PyObject*
TyStatusBar_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyStatusBarObject* self = (TyStatusBarObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyStatusBar_init(TyStatusBarObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "window", "borders", NULL };
	PyObject* pyWindow = NULL, * pyBorders = NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist,
		&pyWindow,
		&pyBorders))
		return -1;

	if (PyObject_TypeCheck(pyWindow, &TyWindowType)) {
		if (((TyWindowObject*)pyWindow)->pyStatusBar != Py_None) {
			PyErr_Format(PyExc_RuntimeError, "The parent Window already has a StatusBar.");
			return -1;
		}
		TyAttachObject(&((TyWindowObject*)pyWindow)->pyStatusBar, self);
		self->hwndParent = ((TyWindowObject*)pyWindow)->hWin;  // frequently used for resizing
	}
	else {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('window') must be a Window, not '%.200s'.", pyWindow->ob_type->tp_name);
		return -1;
	}

	if (pyBorders == NULL) {
		self->iParts = 1;
		self->iaRightEdges[0] = -1;
	}
	else if (PyList_Check(pyBorders)) {
		self->iParts = PyList_Size(pyBorders) + 1;
		if (self->iParts > TySTATUSBAR_MAX_PARTS) {
			PyErr_Format(PyExc_RuntimeError, "Not more than %d parts are allowed.", TySTATUSBAR_MAX_PARTS);
			return -1;
		}

		PyObject* pyBorder;
		Py_ssize_t n, nLen;
		nLen = PySequence_Size(pyBorders);
		for (n = 0; n < nLen; n++) {
			pyBorder = PyList_GetItem(pyBorders, n);
			if (PyLong_Check(pyBorder)) {
				self->iaParts[n] = PyLong_AsLong(pyBorder);
			}
			else {
				PyErr_Format(PyExc_RuntimeError, "Argument 2 ('borders') must be a List of integers. Item %d is not.", n);
				return -1;
			}
		}

		self->iaRightEdges[self->iParts - 1] = -1;
	}
	else {
		PyErr_Format(PyExc_TypeError, "Argument 2 ('borders') must be a List, not '%.200s'.", pyBorders->ob_type->tp_name);
		return -1;
	}

	self->hWin = CreateWindowExW(0, STATUSCLASSNAME, L"",
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
		0, 0, 0, 0,
		((TyWindowObject*)pyWindow)->hWin, (HMENU)IDC_TYSTATUSBAR, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	TyStatusBar_Reposition(self);
	RECT rc;
	GetWindowRect(self->hWin, &rc);
	((TyWindowObject*)pyWindow)->iStatusBarHeight = rc.bottom - rc.top;

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

BOOL
TyStatusBar_Reposition(TyStatusBarObject* self)
{
	RECT rect;
	//TransformRectToAbs(&self->rc, self->hwndParent, &rcParent);
	TyWidget_CalculateRect(self, &rect);

	for (int iPart = 0; iPart < self->iParts - 1; iPart++)
		if (self->iaParts[iPart] < 0)
			self->iaRightEdges[iPart] = rect.right + self->iaParts[iPart];
		else
			self->iaRightEdges[iPart] = self->iaParts[iPart];

	SendMessageW(self->hWin, SB_SETPARTS, (WPARAM)self->iParts, (LPARAM)self->iaRightEdges);
	SendMessageW(self->hWin, WM_SIZE, 0, 0);
	return TRUE;
}

PyObject*
TyStatusBar_SetText(TyStatusBarObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "text", "part", NULL };
	PyObject* pyText;
	int iPart = 0;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist,
		&pyText,
		&iPart))
		return NULL;

	if (pyText != Py_None && !PyUnicode_Check(pyText)) {
		PyErr_Format(PyExc_TypeError, "Argument 1 must be a str or None, not '%.200s'.", pyText->ob_type->tp_name);
		return -1;
	}

	if (iPart >= self->iParts) {
		PyErr_Format(PyExc_RuntimeError, "The StatusBar has no more than %d parts.", self->iParts);
		return NULL;
	}

	BOOL bR;
	if (pyText == Py_None)
		bR = SendMessage(self->hWin, SB_SETTEXT, iPart, L"");
	else {
		LPWSTR szText = toW(PyUnicode_AsUTF8(pyText));
		bR = SendMessage(self->hWin, SB_SETTEXT, iPart, (LPARAM)szText);
		PyMem_RawFree(szText);
	}

	if (!bR) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	Py_RETURN_NONE;
}

PyObject*
TyStatusBar_GetText(TyStatusBarObject* self, PyObject* arg)
{
	if (!PyLong_Check(arg)) {
		PyErr_Format(PyExc_TypeError, "Argument must be an integer, not '%.200s'.", arg->ob_type->tp_name);
		return NULL;
	}
	int iPart = PyLong_AsLong(arg);

	if (iPart > self->iParts) {
		PyErr_Format(PyExc_RuntimeError, "The StatusBar has no more than %d parts.", self->iParts);
		return NULL;
	}

	int iLength = LOWORD(SendMessage(self->hWin, SB_GETTEXTLENGTHW, iPart, 0));
	if (iLength == 0)
		Py_RETURN_NONE;
	LPWSTR szText = HAlloc(iLength + 1);

	if (SendMessage(self->hWin, SB_GETTEXT, iPart, (LPARAM)szText) == 0) {
		HFree(szText);
		Py_RETURN_NONE;
	}

	char* strText = toU8(szText);
	HFree(szText);
	PyObject* pyText = PyUnicode_FromString(strText);
	PyMem_RawFree(strText);

	return pyText;
}

static void
TyStatusBar_dealloc(TyStatusBarObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyStatusBar_members[] = {
	{ NULL }
};

static PyMethodDef TyStatusBar_methods[] = {
	{ "set_text", (PyCFunction)TyStatusBar_SetText, METH_VARARGS | METH_KEYWORDS, "Set the text for the given part of the StatusBar." },
	{ "get_text", (PyCFunction)TyStatusBar_GetText, METH_O, "Get the text set at the given part of the StatusBar." },
	{ NULL }
};

PyTypeObject TyStatusBarType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.StatusBar",        /* tp_name */
	sizeof(TyStatusBarObject), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyStatusBar_dealloc, /* tp_dealloc */
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
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	"StatusBar widget objects", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyStatusBar_methods,       /* tp_methods */
	TyStatusBar_members,       /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyStatusBar_init, /* tp_init */
	0,                         /* tp_alloc */
	TyStatusBar_new,           /* tp_new */
};


static LRESULT CALLBACK TyStatusBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL
TyStatusBarType_Init()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TyStatusBarProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g->hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szStatusBarClass;
	wc.hIconSm = NULL;

	if (!RegisterClassEx(&wc))
	{
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}
	return TRUE;
}

static LRESULT CALLBACK
TyStatusBarProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TyStatusBarObject* self = NULL;
	switch (uMsg)
	{
	case WM_ERASEBKGND: {
		self = (TyStatusBarObject*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
		if (self == NULL)
			break;
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect((HDC)wParam, &rc, g->hBkgBrush);
		return 1L;
	}
	}
	return DefParentProc(hwnd, uMsg, wParam, lParam, self);
}