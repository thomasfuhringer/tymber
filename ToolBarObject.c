// ToolBarObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static LPWSTR szToolBarClass = L"TyToolBarClass";

static PyObject*
TyToolBar_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyToolBarObject* self = (TyToolBarObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->iButtons = 1;
		self->hImageList = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyToolBar_init(TyToolBarObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "window", NULL };
	PyObject* pyWindow = NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist,
		&pyWindow))
		return -1;

	const int iBitmapSize = 16;

	if (PyObject_TypeCheck(pyWindow, &TyWindowType)) {
		if (((TyWindowObject*)pyWindow)->pyToolBar != Py_None) {
			PyErr_Format(PyExc_RuntimeError, "The parent Window already has a ToolBar.");
			return -1;
		}
		TyAttachObject(&((TyWindowObject*)pyWindow)->pyToolBar, self);
	}
	else {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('window') must be a Window, not '%.200s'.", pyWindow->ob_type->tp_name);
		return -1;
	}

	self->hWin = CreateWindowExW(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_WRAPABLE | TBSTYLE_LIST,// | TBSTYLE_FLAT,
		0, 0, 0, 0,
		((TyWindowObject*)pyWindow)->hWin, (HMENU)IDC_TYTOOLBAR, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	self->hImageList = ImageList_Create(iBitmapSize, iBitmapSize, ILC_COLOR32 | ILC_MASK, 1, 20);

	SendMessage(self->hWin, TB_SETIMAGELIST, (WPARAM)TyTOOLBAR_IMGLST, (LPARAM)self->hImageList);
	SendMessage(self->hWin, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(self->hWin, TB_AUTOSIZE, 0, 0);
	SendMessage(self->hWin, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_MIXEDBUTTONS);
	ShowWindow(self->hWin, TRUE);

	RECT rc;
	GetWindowRect(self->hWin, &rc);
	((TyWindowObject*)pyWindow)->iToolBarHeight = rc.bottom - rc.top;
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

BOOL
TyToolBar_Reposition(TyToolBarObject* self)
{
	SendMessage(self->hWin, TB_AUTOSIZE, 0, 0);
	return TRUE;
}

PyObject*
TyToolBar_AppendItem(TyToolBarObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "item", NULL };
	TyMenuItemObject* pyItem;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist,
		&pyItem))
		return NULL;

	if (!PyObject_TypeCheck(pyItem, &TyMenuItemType)) {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('item') must be a MenuItem, not '%.200s'.", ((PyObject*)pyItem)->ob_type->tp_name);
		return -1;
	}

	int iImageIndex = ImageList_AddIcon(self->hImageList, pyItem->pyIcon->hWin);
	LPCSTR strCaption = PyUnicode_AsUTF8(pyItem->pyCaption);
	LPWSTR szCaption = toW(strCaption);

	TBBUTTON aButtons[1] =
	{
		{ iImageIndex, pyItem->iIdentifier, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, (DWORD_PTR)pyItem, (INT_PTR)szCaption }
	};
	HFree(szCaption);
	if (!SendMessage(self->hWin, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&aButtons)) {
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	};
	pyItem->pyToolBar = self;
	Py_RETURN_NONE;
}

PyObject*
TyToolBar_AppendSeparator(TyToolBarObject* self, PyObject* arg)
{
	TBBUTTON aButtons[1] =
	{
		{ 3, 0, 0, TBSTYLE_SEP, { 0 }, 0, 0 }
	};
	SendMessage(self->hWin, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&aButtons);
	Py_RETURN_NONE;
}

PyObject*
TyToolBar_SetEnabled(TyToolBarObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "item", "enabled", NULL };
	TyMenuItemObject* pyItem;
	BOOL bEnabled;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "Ob", kwlist,
		&pyItem,
		&bEnabled))
		return NULL;

	if (!PyObject_TypeCheck(pyItem, &TyMenuItemType)) {
		PyErr_Format(PyExc_TypeError, "Argument 2 ('item') must be a MenuItem, not '%.200s'.", ((PyObject*)pyItem)->ob_type->tp_name);
		return NULL;
	}

	if (!SendMessage(self->hWin, TB_ENABLEBUTTON, pyItem->iIdentifier, MAKELONG(bEnabled, 0))) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	Py_RETURN_NONE;
}

static void
TyToolBar_dealloc(TyToolBarObject* self)
{
	DestroyWindow(self->hWin);
	ImageList_Destroy(self->hImageList);
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyToolBar_members[] = {
	{ "children", T_OBJECT, offsetof(TyWindowObject, pyChildren), READONLY, "Child widgets" },
	{ NULL }
};

static PyMethodDef TyToolBar_methods[] = {
	{ "append_item", (PyCFunction)TyToolBar_AppendItem, METH_VARARGS | METH_KEYWORDS, "Appends a MenuItem the ToolBar" },
	{ "append_separator", (PyCFunction)TyToolBar_AppendSeparator, METH_NOARGS, "Appends a separator item the ToolBar." },
	{ "set_enabled", (PyCFunction)TyToolBar_SetEnabled, METH_VARARGS | METH_KEYWORDS, "Enables or disabled a button." },
	{ NULL }
};

PyTypeObject TyToolBarType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.ToolBar",          /* tp_name */
	sizeof(TyToolBarObject),   /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyToolBar_dealloc, /* tp_dealloc */
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
	"ToolBar widget objects",  /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyToolBar_methods,         /* tp_methods */
	TyToolBar_members,         /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyToolBar_init,  /* tp_init */
	0,                         /* tp_alloc */
	TyToolBar_new,             /* tp_new */
};