// ButtonObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static LRESULT CALLBACK TyButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static PyObject*
TyButton_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyButtonObject* self = (TyButtonObject*)TyButtonType.tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->pyOnClickCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyButton_init(TyButtonObject* self, PyObject* args, PyObject* kwds)
{
	if (TyButtonType.tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	self->hWin = CreateWindowEx(0, L"BUTTON", L"OK",
		WS_TABSTOP | WS_CHILD | BS_TEXT | BS_PUSHBUTTON | WS_TABSTOP | (self->bVisible ? WS_VISIBLE : 0), // | BS_NOTIFY
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYBUTTON, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	if (self->pyCaption != NULL)
		if (!TyWidget_SetCaption((TyWidgetObject*)self, self->pyCaption))
			return -1;

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyButtonProc);
	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));

	return 0;
}

BOOL
TyButton_OnClick(TyButtonObject* self)
{
	if (self->pyOnClickCB) {
		PyObject* pyResult = PyObject_CallFunction(self->pyOnClickCB, "(O)", self);
		if (pyResult == NULL)
			return FALSE;
		Py_DECREF(pyResult);
	}
	return TRUE;
}

static void
TyButton_dealloc(TyButtonObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static int
TyButton_setattro(TyButtonObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "on_click") == 0) {
			if (PyCallable_Check(pyValue)) {
				Py_XINCREF(pyValue);
				Py_XDECREF(self->pyOnClickCB);
				self->pyOnClickCB = pyValue;/*
				TyAttachObject(self->pyOnClickCB, pyValue);*/
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a function!");
				return -1;
			}
		}
	}
	return Py_TYPE(self)->tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyButton_getattro(TyButtonObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {

	}
	Py_XDECREF(pyResult);
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static PyMemberDef TyButton_members[] = {
	{ "on_click", T_OBJECT_EX, offsetof(TyButtonObject, pyOnClickCB), READONLY, "On Click callback" },
	{ NULL }
};

static PyMethodDef TyButton_methods[] = {
	{ NULL }
};

PyTypeObject TyButtonType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Button",           /* tp_name */
	sizeof(TyButtonObject),    /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyButton_dealloc, /* tp_dealloc */
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
	TyButton_getattro,         /* tp_getattro */
	TyButton_setattro,         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Button widget",           /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyButton_methods,          /* tp_methods */
	TyButton_members,          /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyButton_init,   /* tp_init */
	0,                         /* tp_alloc */
	TyButton_new,              /* tp_new */
	PyObject_Free,             /* tp_free */
};

static LRESULT CALLBACK
TyButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TyButtonObject* self = (TyButtonObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case OCM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (!TyButton_OnClick(self))
				PyErr_Print();
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			if (!TyButton_OnClick(self))
				PyErr_Print();
		}
		break;

	default:
		break;
	}
	//if (uMsg >= OCM__BASE)
	//	uMsg -= OCM__BASE;
	return CallWindowProc(self->fnOldWinProcedure, hwnd, uMsg, wParam, lParam);
}