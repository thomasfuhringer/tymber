// WidgetObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static PyObject*
TyWidget_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyWidgetObject* self = (TyWidgetObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->hWin = 0;
		self->fnOldWinProcedure = NULL;
		self->pyParent = NULL;
		self->rc.left = 10;
		self->rc.top = 10;
		self->rc.right = 80;
		self->rc.bottom = 22;
		self->bVisible = TRUE;
		self->bReadOnly = FALSE;
		self->pyWindow = NULL;
		self->pyData = Py_None;
		Py_INCREF(Py_None);
		self->pyDataType = NULL;
		self->pyCaption = NULL;
		self->pyAlignHorizontal = NULL;
		self->pyAlignVertical = NULL;
		self->pyFormat = NULL;
		self->pyFormatEdit = NULL;
		self->pyTabNext = NULL;
		self->pyVerifyCB = NULL;
		self->pyOnChangedCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyWidget_init(TyWidgetObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "parent", "key", "left", "top", "width", "height", "caption", "data_type", "format", "visible", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|iiiiOOOp", kwlist,
		&self->pyParent,
		&self->pyKey,
		&self->rc.left,
		&self->rc.top,
		&self->rc.right,
		&self->rc.bottom,
		&self->pyCaption,
		&self->pyDataType,
		&self->pyFormat,
		&self->bVisible))
		return -1;

	if (!PyUnicode_Check(self->pyKey)) {
		PyErr_Format(PyExc_TypeError, "Argument 2 ('key') must be a string, not '%.200s'.", self->pyKey->ob_type->tp_name);
		return -1;
	}

	if (self->pyCaption) {
		if (!PyUnicode_Check(self->pyCaption)) {
			PyErr_Format(PyExc_TypeError, "Argument 7 ('caption') must be a string, not '%.200s'.", self->pyCaption->ob_type->tp_name);
			return -1;
		}
		Py_INCREF(self->pyCaption);
	}

	if (self->pyDataType) {
		if (!PyObject_TypeCheck(self->pyDataType, &PyType_Type)) {
			PyErr_Format(PyExc_TypeError, "Argument 8 ('data_type') must be a data type.");
			return -1;
		}
	}
	else
		self->pyDataType = &PyUnicode_Type;

	if (self->pyFormat) {
		if (!PyUnicode_Check(self->pyFormat)) {
			PyErr_Format(PyExc_TypeError, "Argument 6 ('format') must be a string, not '%.200s'.", self->pyFormat->ob_type->tp_name);
			return -1;
		}
		Py_INCREF(self->pyFormat);
	}
	else {
		Py_INCREF(Py_None);
		self->pyFormat = Py_None;
	}

	if (PyObject_HasAttrString(self->pyParent, "children")) {
		PyObject* pyChildren = PyObject_GetAttrString(self->pyParent, "children");
		if (PyDict_GetItemString(pyChildren, PyUnicode_AsUTF8(self->pyKey)) != NULL) {
			PyErr_Format(PyExc_AttributeError, "Parent widget already contains a widget with this key.");
			return -1;
		}
		if (PyDict_SetItem(pyChildren, self->pyKey, self) == -1)
			return -1;
		self->hwndParent = ((TyWidgetObject*)self->pyParent)->hWin;  // frequently used for resizing

		if (PyObject_TypeCheck(self->pyParent, &TyWindowType) || PyObject_TypeCheck(self->pyParent, &TyMdiWindowType))
			self->pyWindow = self->pyParent;
		else
			self->pyWindow = ((TyWidgetObject*)self->pyParent)->pyWindow;
	}
	else {
		PyErr_Format(PyExc_TypeError, "Parent widget of type %.200s can not contain widgets.", ((PyObject*)self->pyParent)->ob_type->tp_name);
		return -1;
	}

	return 0;
}

static BOOL CALLBACK
CloseEnumProc(HWND hwnd, LPARAM lParam)
{
	EnumChildWindows(hwnd, CloseEnumProc, 0);
	TyWidgetObject* pyWidget = (TyWidgetObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pyWidget != NULL)
		Py_DECREF(pyWidget);
	return TRUE;
}

static void
TyWidget_dealloc(TyWidgetObject* self)
{
	if (self->hWin) {
		EnumChildWindows(self->hWin, CloseEnumProc, 0);
		DestroyWindow(self->hWin);
		self->hWin = 0;
	}

	Py_XDECREF(self->pyData);
	Py_XDECREF(self->pyDataType);
	Py_XDECREF(self->pyFormat);
	Py_XDECREF(self->pyFormatEdit);
	Py_XDECREF(self->pyAlignHorizontal);
	Py_XDECREF(self->pyAlignVertical);

	Py_TYPE(self)->tp_free((PyObject*)self);
}

BOOL
TyWidget_SetCaption(TyWidgetObject* self, PyObject* pyText)
{
	LPCSTR strText = PyUnicode_AsUTF8(pyText);
	LPWSTR szText = toW(strText);
	if (SendMessage(self->hWin, WM_SETTEXT, (WPARAM)0, (LPARAM)szText)) {
		PyMem_RawFree(szText);
		return TRUE;
	}
	else {
		return TRUE; // something's not working with SendMessage
		PyErr_SetFromWindowsErr(0);
		PyMem_RawFree(szText);
		return FALSE;
	}
}

static int
TyWidget_setattro(TyWidgetObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "caption") == 0) {
			return TyWidget_SetCaption(self, pyValue) ? 0 : -1;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "visible") == 0) {
			self->bVisible = PyObject_IsTrue(pyValue);
			ShowWindow(self->hWin, self->bVisible ? SW_SHOW : SW_HIDE);
			return  0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "frame") == 0) {
			LONG lStyle = GetWindowLong(self->hWin, GWL_EXSTYLE);
			if (PyObject_IsTrue(pyValue))
				lStyle |= TY_FRAME;
			else
				lStyle &= ~TY_FRAME;
			SetWindowLong(self->hWin, GWL_EXSTYLE, lStyle);
			SetWindowPos(self->hWin, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
			return  0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "tab_next") == 0) {

			if (PyObject_TypeCheck(pyValue, &TyWidgetType) && (((TyWidgetObject*)pyValue)->pyWindow == self->pyWindow)) {
				self->pyTabNext = pyValue;
				return  0;
			}
			else {
				PyErr_Format(PyExc_TypeError, "Please assign a widget from same window!");
				return -1;
			}
		}
	}
	if (PyObject_GenericSetAttr((PyObject*)self, pyAttributeName, pyValue) < 0)
		return -1;
	return  0;
}


static PyObject*
TyWidget_getattro(TyWidgetObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "visible") == 0) {
			PyErr_Clear();
			if (self->hWin && GetWindowLong(self->hWin, GWL_STYLE) & WS_VISIBLE)
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}/*
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "frame") == 0) {
			PyErr_Clear();
			LONG lStyle = GetWindowLong(self->hWin, GWL_EXSTYLE);
			if (lStyle & TY_FRAME)
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}*/
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "position") == 0) {
			PyErr_Clear();
			RECT rcWin;
			GetWindowRect(self->hWin, &rcWin);
			MapWindowPoints(HWND_DESKTOP, GetParent(self->hWin), (LPPOINT)&rcWin, 2);
			PyObject* pyRectangle = PyTuple_Pack(4, PyLong_FromLong(rcWin.left), PyLong_FromLong(rcWin.top), PyLong_FromLong(rcWin.right), PyLong_FromLong(rcWin.bottom));
			return pyRectangle;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "size") == 0) {
			PyErr_Clear();
			RECT rcClient;
			GetClientRect(self->hWin, &rcClient);
			PyObject* pyRectangle = PyTuple_Pack(2, PyLong_FromLong(rcClient.right), PyLong_FromLong(rcClient.bottom));
			return pyRectangle;
		}
	}
	Py_XDECREF(pyResult);
	//return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
	return TyWidgetType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static PyObject*
TyWidget_move(TyWidgetObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "left", "top", "width", "height", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iiii", kwlist,
		&self->rc.left,
		&self->rc.top,
		&self->rc.right,
		&self->rc.bottom))
		return NULL;

	if (TyWidget_MoveWindow(self))
		Py_RETURN_TRUE;
	Py_RETURN_FALSE;
}

BOOL
TyWidget_FocusIn(TyWidgetObject* self)
{
	self->pyWindow->pyFocusWidget = self;
	if (PyObject_TypeCheck(self, &TyEntryType) && !TyEntry_FocusIn(self))
		return FALSE;
	if (self->pyWindow->pyOnFocusChangeCB) {
		PyObject* pyResult = PyObject_CallFunction(self->pyWindow->pyOnFocusChangeCB, "(O)", self);
		if (pyResult == NULL)
			return FALSE;
		else
			Py_DECREF(pyResult);
	}
	return TRUE;
}

BOOL
TyWidget_FocusOut(TyWidgetObject* self)
{
	PyObject* pyResult;
	if (PyObject_HasAttrString(self, "on_leave")) {
		PyObject* pyCB = PyObject_GetAttrString(self, "on_leave");
		if (pyCB && pyCB != Py_None) {
			PyObject* pyArgs = PyTuple_Pack(1, (PyObject*)self);
			pyResult = PyObject_CallObject(pyCB, pyArgs);
			if (pyResult == NULL)
				return FALSE;

			BOOL bLeaveNot = (pyResult == Py_False);
			Py_DECREF(pyResult);
			if (bLeaveNot) {
				SetFocus(self->hWin);
				return TRUE;
			}
		}
	}

	//pyResult = PyObject_CallMethod(self, "parse", NULL);
	if (PyObject_TypeCheck(self, &TyEntryType)) {
		return TyEntry_FocusOut(self);
	}
	self->pyWindow->pyFocusWidget = NULL;
	return TRUE;
}

BOOL
TyWidget_FocusTo(TyWidgetObject* self)
{
	SetFocus(self->hWin);
	self->pyWindow->pyFocusWidget = NULL;
	return TRUE;
}

PyObject*
TyFormatData(PyObject* pyData, PyObject* pyFormat)
{
	PyObject* pyText = NULL;
	if (pyData == NULL || pyData == Py_None) {
		pyText = PyUnicode_New(0, 0);
		return pyText;
	}

	if (PyUnicode_Check(pyData)) {
		pyText = pyData;
		Py_INCREF(pyText);
	}

	PyDateTime_IMPORT;
	if (PyDateTime_Check(pyData)) {
		if (pyFormat && pyFormat != Py_None) {
			if (!(pyText = PyObject_CallMethod(pyFormat, "format", "(O)", pyData))) {
				return NULL;
			}
		}
		else
			if (!(pyText = PyObject_CallMethod(g->pyStdDateTimeFormat, "format", "(O)", pyData)))
				return NULL;
	}
	else if (PyLong_Check(pyData) || PyFloat_Check(pyData)) {
		if (pyFormat && pyFormat != Py_None) {
			if (!(pyText = PyObject_CallMethod(pyFormat, "format", "(O)", pyData))) {
				return NULL;
			}
		}
		else
			if (!(pyText = PyObject_Str(pyData)))
				return NULL;
	}

	if (pyText == NULL) {
		PyErr_Format(PyExc_TypeError, "This data type can not be formatted: '%s'.", PyUnicode_AsUTF8(PyObject_Repr((PyObject*)pyData->ob_type)));
		return NULL;
	}

	return pyText;
}

PyObject*
TyParseString(LPCSTR strText, PyTypeObject* pyDataType, PyObject* pyFormat)
{
	PyObject* pyData = NULL, * pyString;
	PyDateTime_IMPORT;
	if (pyDataType == &PyUnicode_Type) {
		pyData = PyUnicode_FromString(strText);
	}
	else if (pyDataType == &PyLong_Type) {
		pyData = PyLong_FromString(strText, NULL, 0);
	}
	else if (pyDataType == &PyFloat_Type) {
		pyString = PyUnicode_FromString(strText);
		pyData = PyFloat_FromString(pyString);
		Py_XDECREF(pyString);
	}
	else if (pyDataType == PyDateTimeAPI->DateTimeType) {
		pyData = PyObject_CallMethod((PyObject*)pyDataType, "strptime", "ss", strText, "%Y-%m-%d");
	}
	else {
		pyString = PyObject_Repr((PyObject*)pyDataType);
		PyErr_Format(PyExc_TypeError, "Unsupported data type in Edit: '%s'.", PyUnicode_AsUTF8(pyString));
		Py_XDECREF(pyString);
	}
	return pyData;
}

BOOL
TyWidget_SetData(TyWidgetObject* self, PyObject* pyData)
{
	if (pyData != Py_None && Py_TYPE(pyData) != self->pyDataType) {
		PyErr_Format(PyExc_TypeError, "Please assign a '%.200s', not '%.200s'!", self->pyDataType->tp_name, Py_TYPE(pyData)->tp_name);
		return FALSE;
	}

	if (PyObject_RichCompareBool(self->pyData, pyData, Py_EQ))
		return TRUE;

	TyAttachObject(&self->pyData, pyData);

	if (self->pyOnChangedCB && self->pyOnChangedCB != Py_None) {
		PyObject* pyArgs = PyTuple_Pack(1, self);
		PyObject* pyResult = PyObject_CallObject(self->pyOnChangedCB, pyArgs);
		if (pyResult == NULL)
			return FALSE;
		Py_DECREF(pyResult);
	}
	return TRUE;
}

static PyMemberDef TyWidget_members[] = {
	{ "native_widget", T_INT, offsetof(TyWidgetObject, hWin), READONLY, "Win32 handle of the MS-Windows window" },
	{ "left", T_INT, offsetof(TyWidgetObject, rc.left), READONLY, "Distance from left edge of parent, if negative from right" },
	{ "top", T_INT, offsetof(TyWidgetObject, rc.top), READONLY, "Distance from top edge of parent, if negative from bottom" },
	{ "right", T_INT, offsetof(TyWidgetObject, rc.right), READONLY, "Distance from left or, if zero or negative, from right edge of parent of right edge" },
	{ "bottom", T_INT, offsetof(TyWidgetObject, rc.bottom), READONLY, "Distance from top or, if zero or negative, from bottom edge of parent of bottom edge" },
	{ "data_type", T_OBJECT, offsetof(TyWidgetObject, pyDataType), READONLY, "Data type the widget can hold" },
	{ "edit_format", T_OBJECT, offsetof(TyWidgetObject, pyFormatEdit), 0, "Format when editing" },
	{ "format", T_OBJECT, offsetof(TyWidgetObject, pyFormat), 0, "Format for display" },
	{ "parent", T_OBJECT, offsetof(TyWidgetObject, pyParent), READONLY, "Parent widget" },
	{ "window", T_OBJECT, offsetof(TyWidgetObject, pyWindow), READONLY, "Parent window" },
	{ "key", T_OBJECT, offsetof(TyWidgetObject, pyKey), READONLY, "Key in parent's child dict" },
	{ "tab_next", T_OBJECT, offsetof(TyWidgetObject, pyTabNext), READONLY, "Widget to jump to on tab" },
	{ "on_changed", T_OBJECT, offsetof(TyWidgetObject, pyOnChangedCB), 0, "Call back when data has changed" },
	{ "read_only", T_BOOL, offsetof(TyWidgetObject, bReadOnly), READONLY, "Data can not be edited." },
	{ "align_horiz", T_OBJECT, offsetof(TyWidgetObject, pyAlignHorizontal), READONLY, "Horizontal alignment" },
	{ "align_vert", T_OBJECT, offsetof(TyWidgetObject, pyAlignVertical), READONLY, "Vertical alignment" },
	{ NULL }
};

static PyMethodDef TyWidget_methods[] = {
	{ "move", (PyCFunction)TyWidget_move, METH_VARARGS | METH_KEYWORDS, "Reposition widget to given coordinates." },
	{ NULL }
};

PyTypeObject TyWidgetType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Widget",           /* tp_name */
	sizeof(TyWidgetObject),    /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyWidget_dealloc, /* tp_dealloc */
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
	TyWidget_getattro,         /* tp_getattro */
	TyWidget_setattro,         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"The parent of all widgets", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyWidget_methods,          /* tp_methods */
	TyWidget_members,          /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyWidget_init,   /* tp_init */
	0,                         /* tp_alloc */
	TyWidget_new,              /* tp_new */
	PyObject_Free,             /* tp_free */
	0,                         /*tp_is_gc*/
};

BOOL
TyWidget_MoveWindow(TyWidgetObject* self)
{
	RECT rect;
	TyWidget_CalculateRect(self, &rect);
	if (MoveWindow(self->hWin, rect.left, rect.top, rect.right, rect.bottom, TRUE) == 0) {
		PyErr_SetFromWindowsErr(0);
		PyErr_Print();
		return FALSE;
	}
	//EnumChildWindows(self->hWin, TyWidgetSizeEnumProc, (LPARAM)0);
	return TRUE;
}

BOOL TyStatusBar_Reposition(TyStatusBarObject* self);
BOOL CALLBACK
TyWidgetSizeEnumProc(HWND hwndChild, LPARAM lParam)
{
	TyWidgetObject* pyWidget = (TyWidgetObject*)GetWindowLongPtr(hwndChild, GWLP_USERDATA);
	if (pyWidget == NULL)
		return TRUE;
	if (PyObject_TypeCheck((PyObject*)pyWidget, &TyMdiWindowType))
		return FALSE;
	if (PyObject_TypeCheck(pyWidget, &TyStatusBarType)) {
		TyStatusBar_Reposition(pyWidget);
		return TRUE;
	}
	if (IsWindow(pyWidget->hWin)) {
		return TyWidget_MoveWindow(pyWidget);
	}
	return TRUE;
}

BOOL
TyWidget_CalculateRect(TyWidgetObject* self, _Out_ PRECT rcAbs)
{
	RECT rcRel = self->rc;
	RECT rcParent;
	BOOL bSucess;
	if (PyObject_TypeCheck((PyObject*)self, &TyWindowType))
		bSucess = GetClientRect(GetDesktopWindow(), &rcParent);
	else
		bSucess = GetClientRect(self->hwndParent, &rcParent);
	if (!bSucess) {
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}

	// Adjust for height of tool and status bar 
	if (PyObject_TypeCheck((PyObject*)self, &TyWidgetType) && (PyObject_TypeCheck(self->pyParent, &TyWindowType) || PyObject_TypeCheck(self->pyParent, &TyMdiWindowType))) {
		rcParent.top += ((TyWindowObject*)self->pyParent)->iToolBarHeight;
		if (PyObject_TypeCheck((PyObject*)self->pyParent, &TyWindowType))
			rcParent.bottom -= ((TyWindowObject*)self->pyParent)->iStatusBarHeight;
	}

	// and Tab (quick fix)
	if (PyObject_TypeCheck((PyObject*)self, &TyTabType))
		rcRel.top += 26;
	return TransformRectToAbs(rcRel, rcParent, rcAbs);
}

BOOL
TransformRectToAbs(_In_ RECT rcRel, _In_ RECT rcParent, _Out_ PRECT rcAbs)
{
	if (rcRel.left == TyWIDGET_CENTER)
		rcAbs->left = (rcParent.right - rcRel.right) / 2;
	else
		rcAbs->left = (rcRel.left >= 0 || rcRel.left == CW_USEDEFAULT ? rcRel.left : rcParent.right + rcRel.left);
	rcAbs->right = (rcRel.right > 0 || rcRel.right == CW_USEDEFAULT ? rcRel.right : rcParent.right + rcRel.right - rcAbs->left);
	if (rcRel.top == TyWIDGET_CENTER)
		rcAbs->top = (rcParent.bottom - rcRel.bottom) / 2;
	else
		rcAbs->top = (rcRel.top >= 0 || rcRel.top == CW_USEDEFAULT ? rcRel.top + rcParent.top : rcParent.bottom + rcRel.top);
	rcAbs->bottom = (rcRel.bottom > 0 || rcRel.bottom == CW_USEDEFAULT ? rcRel.bottom : rcParent.bottom + rcRel.bottom - rcAbs->top);

	return TRUE;
}