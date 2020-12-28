// BoxObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static LPWSTR szBoxClass = L"TyBoxClass";

static PyObject*
TyBox_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyBoxObject* self = TyBoxType.tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyBox_init(TyBoxObject* self, PyObject* args, PyObject* kwds)
{
	if (TyBoxType.tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	if (!TyWidget_CalculateRect(self, &rect))
		return -1;

	self->hWin = CreateWindowExW(0, szBoxClass, L"",
		WS_CHILD | WS_CLIPSIBLINGS | WS_TABSTOP | WS_VISIBLE,
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYBOX, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	self->pyChildren = PyDict_New();
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

static PyObject*
TyBox_getattro(TyBoxObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult, * pyAttribute;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "children") == 0) {
			PyErr_Clear();
			Py_INCREF(self->pyChildren);
			return self->pyChildren;
		}
		if (PyDict_Contains(self->pyChildren, pyAttributeName)) {
			PyErr_Clear();
			pyAttribute = PyDict_GetItem(self->pyChildren, pyAttributeName);
			Py_INCREF(pyAttribute);
			return pyAttribute;
		}
	}
	Py_XDECREF(pyResult);
	return TyBoxType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyBox_dealloc(TyBoxObject* self)
{
	Py_DECREF(self->pyChildren);
	DestroyWindow(self->hWin);
	TyBoxType.tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyBox_members[] = {
	//{ "children", T_OBJECT, offsetof(TyBoxObject, pyChildren), READONLY, "Child widgets" }, // caused problem with reference count
	{ NULL }
};

static PyMethodDef TyBox_methods[] = {
	{ NULL }
};

PyTypeObject TyBoxType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Box",              /* tp_name */
	sizeof(TyBoxObject),       /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyBox_dealloc, /* tp_dealloc */
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
	TyBox_getattro,            /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Box widget",              /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyBox_methods,             /* tp_methods */
	TyBox_members,             /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyBox_init,      /* tp_init */
	0,                         /* tp_alloc */
	TyBox_new,                 /* tp_new */
	PyObject_Free,             /* tp_free */
};


static LRESULT CALLBACK TyBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL
TyBoxType_Init()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TyBoxProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g->hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szBoxClass;
	wc.hIconSm = NULL;

	if (!RegisterClassEx(&wc))
	{
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}
	return TRUE;
}

static LRESULT CALLBACK
TyBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TyBoxObject* self = NULL;
	switch (uMsg)
	{
	case WM_ERASEBKGND: {
		self = (TyBoxObject*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
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