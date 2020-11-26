// EntryObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

#define TyENTRY_BUTTON_WIDTH  18

static PyObject*
TyEntry_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyEntryObject* self = (TyEntryObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->pyOnLeaveCB = NULL;
		self->pyOnKeyCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyEntry_init(TyEntryObject* self, PyObject* args, PyObject* kwds)
{
	if (Py_TYPE(self)->tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	int iAlignment = (self->pyDataType == &PyUnicode_Type) ? 0 : ES_RIGHT;

	self->hWin = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
		WS_CHILD | WS_TABSTOP | ES_AUTOVSCROLL | ES_AUTOHSCROLL | iAlignment | (self->bVisible ? WS_VISIBLE : 0), //| ES_MULTILINE | ES_WANTRETURN
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYENTRY, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyEntryProc);
	if (self->pyCaption != NULL)
		if (!TyWidget_SetCaption((TyWidgetObject*)self, self->pyCaption))
			return -1;
	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));
	TyEntry_Parse(self);
	return 0;
}

BOOL
TyEntry_RenderData(TyEntryObject* self, BOOL bFormat)
{
	if (self->pyData == NULL || self->pyData == Py_None) {
		SendMessage(self->hWin, WM_SETTEXT, 0, (LPARAM)L"");
		return TRUE;
	}

	PyObject* pyText = TyFormatData(self->pyData, bFormat ? self->pyFormat : (self->pyFormatEdit ? self->pyFormatEdit : Py_None));
	if (pyText == NULL) {
		return FALSE;
	}

	LPCSTR strText = PyUnicode_AsUTF8(pyText);
	LPWSTR szText = toW(strText);
	Py_DECREF(pyText);
	if (SendMessage(self->hWin, WM_SETTEXT, 0, (LPARAM)szText)) {
		PyMem_RawFree(szText);
		return TRUE;
	}
	else {
		PyMem_RawFree(szText);
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}
}

BOOL
TyEntry_SetData(TyEntryObject* self, PyObject* pyData)
{
	if (self->pyData == pyData)
		return TRUE;
	if (!TyWidget_SetData((TyWidgetObject*)self, pyData))
		return FALSE;
	return TyEntry_RenderData(self, TRUE);
}

static PyObject*
TyEntry_get_input_string(TyEntryObject* self)
{
	TCHAR szText[1024];
	PyObject* pyInput;
	GetWindowText(self->hWin, szText, 1024);
	char* strText = toU8(szText);
	if (lstrlenW(szText) > 0)
		pyInput = PyUnicode_FromString(strText);
	else
		pyInput = PyUnicode_New(0, 0);
	PyMem_RawFree(strText);
	return pyInput;
}

static PyObject*  // new ref
TyEntry_get_input_data(TyEntryObject* self)
{
	TCHAR szText[1024];
	PyObject* pyData;
	int iLen = GetWindowText(self->hWin, szText, 1024);
	if (iLen == 0)
		Py_RETURN_NONE;
	char* strText = toU8(szText);
	if ((pyData = TyParseString(strText, self->pyDataType, NULL)) == NULL)
		return NULL;
	PyMem_RawFree(strText);
	return pyData;
}

BOOL
TyEntry_Parse(TyEntryObject* self)
{
	PyObject* pyData = NULL;
	if ((pyData = TyEntry_get_input_data(self)) == NULL)
		return FALSE;

	if (pyData)
		return TyEntry_SetData(self, pyData);
	return FALSE;
}

BOOL
TyEntry_FocusIn(TyEntryObject* self)
{
	return TyEntry_RenderData(self, FALSE); // render unformatted
}

BOOL
TyEntry_FocusOut(TyEntryObject* self)
{
	if (!TyEntry_Parse(self)) {
		if (PyErr_ExceptionMatches(PyExc_ValueError)) {
			PyErr_Clear();
			MessageBox(0, L"Invalid Input", L"Error", 0);
			SetFocus(self->hWin);
			return TRUE;
		}
		else
			return FALSE;
	}
	return TRUE;
}

static PyObject*  // new ref
TyEntry_commit(TyEntryObject* self)
{
	if (TyEntry_FocusOut(self))
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;
}

static int
TyEntry_setattro(TyEntryObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			if (!TyEntry_SetData((TyEntryObject*)self, pyValue))
				return -1;
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "align_horiz") == 0) {
			LONG_PTR pAlign = ES_LEFT;
			switch (PyLong_AsLong(PyObject_GetAttrString(pyValue, "value")))
			{
			case ALIGN_LEFT:
				pAlign = ES_LEFT;
				break;
			case ALIGN_RIGHT:
				pAlign = ES_RIGHT;
				break;
			case ALIGN_CENTER:
				pAlign = ES_CENTER;
				break;
			}
			SetWindowLongPtr(self->hWin, GWL_STYLE, GetWindowLongPtr(self->hWin, GWL_STYLE) & ~(ES_CENTER | ES_RIGHT) | pAlign);

			self->pyAlignHorizontal = pyValue;
			return 0;
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "input") == 0) {
			if (PyUnicode_Check(pyValue)) {
				LPCSTR strText = PyUnicode_AsUTF8(pyValue);
				LPWSTR szText = toW(strText);
				SendMessage(self->hWin, WM_SETTEXT, 0, (LPARAM)szText);
				PyMem_RawFree(szText);
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a str!");
				return -1;
			}
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "read_only") == 0) {
			if (PyBool_Check(pyValue)) {
				self->bReadOnly = PyObject_IsTrue(pyValue);
				if (SendMessage(self->hWin, EM_SETREADONLY, self->bReadOnly, 0) == 0) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a bool!");
				return -1;
			}
			return 0;
		}
	}
	return TyEntryType.tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject* // new ref
TyEntry_getattro(TyEntryObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "input_string") == 0) {
			PyErr_Clear();
			return TyEntry_get_input_string(self);
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "input_data") == 0) {
			PyErr_Clear();
			return TyEntry_get_input_data(self);
		}
	}
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyEntry_dealloc(TyEntryObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyEntry_members[] = {
	{ "data", T_OBJECT, offsetof(TyEntryObject, pyData), READONLY, "Data value" },
	{ "on_leave", T_OBJECT, offsetof(TyEntryObject, pyOnLeaveCB), 0, "Callback, return True if ready for focus to move on." },
	{ "on_key", T_OBJECT, offsetof(TyEntryObject, pyOnKeyCB), 0, "Callback when key is pressed" },
	{ NULL }
};

static PyMethodDef TyEntry_methods[] = {
	{ "commit", (PyCFunction)TyEntry_commit, METH_NOARGS, "Validate input and update data attribute. Return False if input is invalid." },
	{ NULL }
};

PyTypeObject TyEntryType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Entry",            /* tp_name */
	sizeof(TyEntryObject),     /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyEntry_dealloc, /* tp_dealloc */
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
	TyEntry_getattro,          /* tp_getattro */
	TyEntry_setattro,          /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Data entry widget",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyEntry_methods,           /* tp_methods */
	TyEntry_members,           /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyEntry_init,    /* tp_init */
	0,                         /* tp_alloc */
	TyEntry_new,               /* tp_new */
	PyObject_Free,             /* tp_free */
};

LRESULT CALLBACK
TyEntryProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TyEntryObject* self = (TyEntryObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	PyObject* pyKey;

	switch (msg)
	{
	case WM_KEYDOWN:
		//printf("WM_KEYDOWN %p \n", wParam);
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
			if (TyEntry_FocusOut(self)) {
				TyWidget_FocusTo(self->pyTabNext);
			}
		}
		/*switch (wParam)
		{
		case VK_RETURN:
			printf("VK_RETURN %p \n", self);
			break;
		}*/
		break;

	default:
		break;
	}
	return CallWindowProc(self->fnOldWinProcedure, hwnd, msg, wParam, lParam);
}