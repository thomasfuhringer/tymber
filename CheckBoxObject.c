// CheckBoxObject.c  | Tymber © 2022 by Thomas Führinger
#include "Tymber.h"

static LRESULT CALLBACK TyCheckBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static PyObject*
TyCheckBox_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyCheckBoxObject* self = (TyCheckBoxObject*)TyCheckBoxType.tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->pyDataType = &PyBool_Type;
		self->pyOnClickCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyCheckBox_init(TyCheckBoxObject* self, PyObject* args, PyObject* kwds)
{
	if (TyCheckBoxType.tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	self->hWin = CreateWindowEx(0, L"BUTTON", L"Checkbox",
		BS_3STATE | WS_TABSTOP | WS_CHILD | (self->bVisible ? WS_VISIBLE : 0),
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYBUTTON, g->hInstance, NULL);
	SendMessage(self->hWin, BM_SETCHECK, BST_INDETERMINATE, 0);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	if (self->pyCaption != NULL)
		if (!TyWidget_SetCaption((TyWidgetObject*)self, self->pyCaption))
			return -1;

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyCheckBoxProc);
	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));

	return 0;
}

BOOL
TyCheckBox_OnClick(TyCheckBoxObject* self)
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
TyCheckBox_dealloc(TyCheckBoxObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

BOOL
TyCheckBox_SetData(TyComboBoxObject* self, PyObject* pyData)
{
	if (self->pyData == pyData)
		return TRUE;

	if (!PyBool_Check(pyData) && !(pyData == Py_None)) {
		PyErr_Format(PyExc_AttributeError, "Please assign a Bool or None!");
		return FALSE;
	}

	if (!TyWidget_SetData((TyWidgetObject*)self, pyData))
		return FALSE;

	if (pyData == Py_None) {
		SendMessage(self->hWin, BM_SETCHECK, BST_INDETERMINATE, 0);
	}
	else {
		SendMessage(self->hWin, BM_SETCHECK, pyData == Py_True ? BST_CHECKED : BST_UNCHECKED, 0);
	}
	return TRUE;
}

static int
TyCheckBox_setattro(TyCheckBoxObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "on_click") == 0) {
			if (PyCallable_Check(pyValue)) {
				Py_XINCREF(pyValue);
				Py_XDECREF(self->pyOnClickCB);
				self->pyOnClickCB = pyValue;
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a function!");
				return -1;
			}
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			if (!TyCheckBox_SetData(self, pyValue))
				return -1;
			return 0;
		}
	}
	return Py_TYPE(self)->tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyCheckBox_getattro(TyCheckBoxObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {

	}
	Py_XDECREF(pyResult);
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static PyMemberDef TyCheckBox_members[] = {
	{ "data", T_OBJECT, offsetof(TyCheckBoxObject, pyData), READONLY, "Data value" },
	{ "on_click", T_OBJECT_EX, offsetof(TyCheckBoxObject, pyOnClickCB), READONLY, "On Click callback" },
	{ NULL }
};

static PyMethodDef TyCheckBox_methods[] = {
	{ NULL }
};

PyTypeObject TyCheckBoxType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.CheckBox",         /* tp_name */
	sizeof(TyCheckBoxObject),  /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyCheckBox_dealloc, /* tp_dealloc */
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
	TyCheckBox_getattro,       /* tp_getattro */
	TyCheckBox_setattro,       /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"CheckBox widget",         /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyCheckBox_methods,        /* tp_methods */
	TyCheckBox_members,        /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyCheckBox_init, /* tp_init */
	0,                         /* tp_alloc */
	TyCheckBox_new,            /* tp_new */
	PyObject_Free,             /* tp_free */
};

static LRESULT CALLBACK
TyCheckBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TyCheckBoxObject* self = (TyButtonObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case OCM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			if (self->pyOnClickCB == NULL || self->pyOnClickCB == Py_None) {
				if (!TyCheckBox_SetData(self, self->pyData == Py_False ? Py_True : Py_False))
					return -1;
			}
			else
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
	return CallWindowProc(self->fnOldWinProcedure, hwnd, uMsg, wParam, lParam);
}