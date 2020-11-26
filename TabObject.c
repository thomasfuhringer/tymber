// TabObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static PyObject*
TyTab_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyTabObject* self = (TyTabObject*)TyTabType.tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->pyCurrentPage = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static LRESULT CALLBACK TyTabProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int
TyTab_init(TyTabObject* self, PyObject* args, PyObject* kwds)
{
	if (TyTabType.tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	self->hWin = CreateWindowEx(0, WC_TABCONTROLW, L"",
		WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,// | TCS_BOTTOM,
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, g->hMenu, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	self->pyChildren = PyDict_New();

	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));
	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyTabProc);

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);

	return 0;
}

static TyTabPageObject*
TyTab_GetPageByIndex(TyTabObject* self, int iIndex)
{
	PyObject* pyKey, * pyValue, * pyPage = NULL;
	Py_ssize_t nPos = 0;

	while (PyDict_Next(self->pyChildren, &nPos, &pyKey, &pyValue) && !pyPage) {
		if (((TyTabPageObject*)pyValue)->iIndex == iIndex)
			pyPage = pyValue;
	}
	return pyPage;
}

static PyObject*
TyTab_getattro(TyTabObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult, * pyAttribute;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyDict_Contains(self->pyChildren, pyAttributeName)) {
			PyErr_Clear();
			pyAttribute = PyDict_GetItem(self->pyChildren, pyAttributeName);
			Py_INCREF(pyAttribute);
			return pyAttribute;
		}
	}
	return TyTabType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyTab_dealloc(TyTabObject* self)
{
	Py_DECREF(self->pyChildren);
	DestroyWindow(self->hWin);
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyTab_members[] = {
	{ "children", T_OBJECT, offsetof(TyTabObject, pyChildren), READONLY, "Tab pages" },
	{ NULL }
};

static PyMethodDef TyTab_methods[] = {
	{ NULL }
};

PyTypeObject TyTabType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Tab",              /* tp_name */
	sizeof(TyTabObject),       /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyTab_dealloc, /* tp_dealloc */
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
	TyTab_getattro,            /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Tab widget objects",      /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyTab_methods,             /* tp_methods */
	TyTab_members,             /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyTab_init,      /* tp_init */
	0,                         /* tp_alloc */
	TyTab_new,                 /* tp_new */
	PyObject_Free,             /* tp_free */
};

static BOOL TyTabPage_GotSelected(TyTabPageObject* self);

static LRESULT CALLBACK
TyTabProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TyTabObject* self = (TyTabObject*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	switch (msg)
	{
	case OCM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case TCN_SELCHANGING:
			return FALSE;

		case TCN_SELCHANGE:
		{
			int iIndex = TabCtrl_GetCurSel(self->hWin);
			TyTabPage_GotSelected(TyTab_GetPageByIndex(self, iIndex));
			break;
		}
		}
	}
	}
	return CallWindowProcW(self->fnOldWinProcedure, hwnd, msg, wParam, lParam);
}


/* TabPage ----------------------------------------------------------------------- */

static PyObject*
TyTabPage_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyTabPageObject* self = (TyTabPageObject*)TyTabPageType.tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->iIndex = -1;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyTabPage_init(TyTabPageObject* self, PyObject* args, PyObject* kwds)
{
	if (TyTabPageType.tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	if (!PyObject_TypeCheck(self->pyParent, &TyTabType)) {
		PyErr_Format(PyExc_TypeError, "Argument 'parent' must be a Tab, not '%.200s'.", ((PyObject*)self->pyParent)->ob_type->tp_name);
		return -1;
	}

	TCITEM tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = -1;
	LPCSTR strText;
	if (self->pyCaption != NULL) {
		strText = PyUnicode_AsUTF8(self->pyCaption);
	}
	else
		strText = "<New>";
	tie.pszText = toW(strText);

	self->iIndex = TabCtrl_InsertItem(self->pyParent->hWin, PyDict_Size(((TyTabObject*)self->pyParent)->pyChildren), &tie);
	if (self->iIndex == -1) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}
	PyMem_RawFree(tie.pszText);

	self->rc.left = 2;
	self->rc.top = 24;
	self->rc.right = -5;
	self->rc.bottom = -5;
	TyWidget_MoveWindow(self);

	if (PyDict_Size(((TyTabObject*)self->pyParent)->pyChildren) == 1)
		TyTabPage_GotSelected(self);
	else
		ShowWindow(self->hWin, SW_HIDE);

	return 0;
}

static BOOL
TyTabPage_GotSelected(TyTabPageObject* self)
{
	TyTabObject* pyTab = (TyTabObject*)self->pyParent;

	if (pyTab->pyCurrentPage)
		ShowWindow(pyTab->pyCurrentPage->hWin, SW_HIDE);
	pyTab->pyCurrentPage = self;
	ShowWindow(self->hWin, SW_SHOW);
	return TRUE;
}

static PyObject*
TyTabPage_getattro(TyApplicationObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult, * pyAttribute;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyDict_Contains(self->pyChildren, pyAttributeName)) {
			PyErr_Clear();
			pyAttribute = PyDict_GetItem(self->pyChildren, pyAttributeName);
			Py_INCREF(pyAttribute);
			return pyAttribute;
		}
	}
	return TyTabPageType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyTabPage_dealloc(TyTabPageObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((TyTabPageObject*)self);
}

static PyMemberDef TyTabPage_members[] = {
	{ "children", T_OBJECT, offsetof(TyTabPageObject, pyChildren), READONLY, "Child widgets" },
	{ NULL }
};

static PyMethodDef TyTabPage_methods[] = {
	{ NULL }
};

PyTypeObject TyTabPageType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.TabPage",          /* tp_name */
	sizeof(TyTabPageObject),   /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyTabPage_dealloc, /* tp_dealloc */
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
	TyTabPage_getattro,        /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"TabPage widget objects",  /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyTabPage_methods,         /* tp_methods */
	TyTabPage_members,         /* tp_members */
	0,                         /* tp_getset */
	&TyBoxType,                /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyTabPage_init,  /* tp_init */
	0,                         /* tp_alloc */
	TyTabPage_new,             /* tp_new */
	PyObject_Free,             /* tp_free */
};