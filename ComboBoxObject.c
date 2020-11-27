// ComboBoxObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static LRESULT CALLBACK TyComboBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static PyObject*
TyComboBox_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyComboBoxObject* self = (TyComboBoxObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->pyItems = NULL;
		self->bNoneSelectable = TRUE;
		self->pyOnLeaveCB = NULL;
		self->pyOnKeyCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyComboBox_init(TyComboBoxObject* self, PyObject* args, PyObject* kwds)
{
	if (Py_TYPE(self)->tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	int iAlignment = 0;
	if (self->pyDataType != &PyUnicode_Type)
		iAlignment = ES_RIGHT;

	self->hWin = CreateWindowExW(WS_EX_CLIENTEDGE, WC_COMBOBOX, L"",
		CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_TABSTOP | WS_CHILD | WS_OVERLAPPED | (self->bVisible ? WS_VISIBLE : 0) | iAlignment,
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYCOMBOBOX, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyComboBoxProc);

	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));

	if ((self->pyItems = PyList_New(0)) == NULL)
		return -1;

	return 0;
}

static PyObject*
TyComboBox_append(TyComboBoxObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "value", "key", NULL };
	PyObject* pyValue = NULL, * pyKey = NULL, * pyItem = NULL, * pyRepr = NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist,
		&pyValue,
		&pyKey))
		return NULL;

	if (pyKey == NULL)
		pyKey = pyValue;

	pyItem = PyTuple_Pack(2, pyValue, pyKey);
	if (PyList_Append(self->pyItems, pyItem) == -1)
		return NULL;

	if (PyUnicode_Check(pyValue)) {
		pyRepr = pyValue;
		Py_INCREF(pyRepr);
	}
	else
		pyRepr = PyObject_Repr(pyValue);

	LPCSTR strText = PyUnicode_AsUTF8(pyRepr);
	LPWSTR szText = toW(strText);
	SendMessage(self->hWin, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)szText);
	PyMem_RawFree(szText);
	Py_DECREF(pyRepr);

	Py_RETURN_NONE;
}

BOOL
TyComboBox_Selected(TyComboBoxObject* self, int iItem)
{
	PyObject* pyItem, * pyData;
	if (iItem == CB_ERR)
		pyData = Py_None;
	else {
		pyItem = PyList_GetItem(self->pyItems, iItem);
		pyData = PyTuple_GET_ITEM(pyItem, 1);
	}

	return TyComboBox_SetData(self, pyData);
}

BOOL
TyComboBox_SetData(TyComboBoxObject* self, PyObject* pyData)
{
	if (self->pyData == pyData)
		return TRUE;

	Py_ssize_t nIndex;
	if (self->bNoneSelectable && pyData == Py_None)
		nIndex = -1;
	else {
		Py_ssize_t nLen = PySequence_Size(self->pyItems);
		BOOL bFound = FALSE;
		PyObject* pyItem, * pyKey;
		for (nIndex = 0; nIndex < nLen; nIndex++) {
			pyItem = PyList_GetItem(self->pyItems, nIndex);
			pyKey = PyTuple_GET_ITEM(pyItem, 1);
			if (PyObject_RichCompareBool(pyKey, pyData, Py_EQ)) {
				bFound = TRUE;
				break;
			}
		}

		if (!bFound) {
			PyObject* pyRepr = PyObject_Repr(pyData);
			PyErr_Format(PyExc_AttributeError, "Value %s is not among selectable items in ComboBox.", PyUnicode_AsUTF8(pyRepr));
			Py_XDECREF(pyRepr);
			return FALSE;
		}
	}

	if (!TyWidget_SetData((TyWidgetObject*)self, pyData))
		return FALSE;

	SendMessage(self->hWin, CB_SETCURSEL, (WPARAM)nIndex, (LPARAM)0);
	return TRUE;
}

static int
TyComboBox_setattro(TyComboBoxObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			if (!TyComboBox_SetData(self, pyValue))
				return -1;
			return 0;
		}
	}
	return TyComboBoxType.tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyComboBox_getattro(TyComboBoxObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		// for later
	}
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyComboBox_dealloc(TyComboBoxObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyComboBox_members[] = {
	{ "none_selectable", T_BOOL, offsetof(TyComboBoxObject, bNoneSelectable), 0, "It is possible to make no selection." },
	{ "data", T_OBJECT, offsetof(TyComboBoxObject, pyData), READONLY, "Data value" },
	{ "on_leave", T_OBJECT, offsetof(TyComboBoxObject, pyOnLeaveCB), 0, "Callback, return True if ready for focus to move on." },
	{ "on_key", T_OBJECT, offsetof(TyComboBoxObject, pyOnKeyCB), 0, "Callback when key is pressed" },
	{ NULL }
};

static PyMethodDef TyComboBox_methods[] = {
	{ "append", (PyCFunction)TyComboBox_append, METH_VARARGS | METH_KEYWORDS, "Append selectable item." },
	{ NULL }
};

PyTypeObject TyComboBoxType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.ComboBox",         /* tp_name */
	sizeof(TyComboBoxObject),  /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyComboBox_dealloc, /* tp_dealloc */
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
	TyComboBox_getattro,       /* tp_getattro */
	TyComboBox_setattro,       /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"ComboBox objects",        /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyComboBox_methods,        /* tp_methods */
	TyComboBox_members,        /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyComboBox_init, /* tp_init */
	0,                         /* tp_alloc */
	TyComboBox_new,            /* tp_new */
	PyObject_Free,             /* tp_free */
};

static LRESULT CALLBACK
TyComboBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TyComboBoxObject* self = (TyComboBoxObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	PyObject* pyKey;

	switch (msg)
	{
	case WM_KEYDOWN:
		if (self->pyOnKeyCB) {
			pyKey = PyObject_CallFunction(g->pyKeyEnum, "(i)", wParam);
			if (pyKey != NULL) {
				PyObject* pyResult = PyObject_CallFunction(self->pyOnKeyCB, "(OO)", pyKey, self);
				if (pyResult == NULL)
					PyErr_Print();
				else
					Py_DECREF(pyResult);
				Py_DECREF(pyKey);
			}
			else
				PyErr_Clear();
		}

		if (wParam == VK_TAB && self->pyTabNext) {
			//if (TyComboBox_FocusOut(self)) 
			TyWidget_FocusTo(self->pyTabNext);
		}

		break;

	case OCM_COMMAND:
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			if (!TyComboBox_Selected(self, ItemIndex)) {
				PyErr_Print();
				MessageBox(NULL, L"Error ", L"Error", MB_ICONERROR);
			}
			return 0;
		}
		break;

	default:
		break;
	}
	return CallWindowProcW(self->fnOldWinProcedure, hwnd, msg, wParam, lParam);
}