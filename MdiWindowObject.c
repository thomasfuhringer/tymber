// MdiWindowObject.c  | Tymber © 2020  by Thomas Führinger
#include "Tymber.h"
#include "Resource.h"

// Forward declarations

static PyObject*
TyMdiWindow_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyMdiWindowObject* self = (TyMdiWindowObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->cxMin = 160;
		self->cyMin = 120;
		self->rc.left = TyWIDGET_CENTER;
		self->rc.top = TyWIDGET_CENTER;
		self->rc.right = 320;
		self->rc.bottom = 240;
		self->pyCaption = NULL;
		//self->pyIcon = NULL;
		self->iToolBarHeight = 0;
		self->pyFocusWidget = NULL;
		self->pyOnFocusChangeCB = NULL;
		self->pyBeforeCloseCB = NULL;
		self->pyOnCloseCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyMdiWindow_init(TyMdiWindowObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "parent", "key", "left", "top", "width", "height", "caption", "visible", NULL };
	BOOL bVisible = TRUE;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO|iiiiOp", kwlist,
		&self->pyParent,
		&self->pyKey,
		&self->rc.left,
		&self->rc.top,
		&self->rc.right,
		&self->rc.bottom,
		&self->pyCaption,
		&bVisible))
		return -1;

	if (PyObject_TypeCheck(self->pyParent, &TyMdiAreaType)) {
		PyObject* pyChildren = PyObject_GetAttrString(self->pyParent, "children");
		if (PyDict_SetItem(pyChildren, self->pyKey, self) == -1)
			return -1;
		Py_DECREF(pyChildren);
		self->hwndParent = ((TyMdiAreaObject*)self->pyParent)->hWin;
	}
	else {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('parent') must be a MdiArea, not '%.200s'.", self->pyParent->ob_type->tp_name);
		return -1;
	}

	if (!PyUnicode_Check(self->pyKey)) {
		PyErr_SetString(PyExc_TypeError, "Argument 2 ('key') must be a string.");
		return -1;
	}

	LPWSTR szCaption = NULL;
	if (self->pyCaption) {
		if (PyUnicode_Check(self->pyCaption)) {
			LPCSTR strText = PyUnicode_AsUTF8(self->pyCaption);
			szCaption = toW(strText);
		}
		else {
			PyErr_SetString(PyExc_TypeError, "Argument 7 ('caption') must be a string, not '%.200s'.", self->pyCaption->ob_type->tp_name);
			return -1;
		}
	}

	RECT rect;
	if (!TyWidget_CalculateRect(self, &rect))
		return -1;

	MDICREATESTRUCT MdiCreate;
	MdiCreate.szClass = L"TyWindowClass";
	MdiCreate.szTitle = szCaption;
	MdiCreate.hOwner = g->hInstance;
	MdiCreate.x = self->rc.left;
	MdiCreate.y = self->rc.top;
	MdiCreate.cx = self->rc.right;
	MdiCreate.cy = self->rc.bottom;
	//MdiCreate.style = ((TyMdiAreaObject*)self->pyParent)->bMaximized ? WS_MAXIMIZE : 0; //WS_EX_CLIENTEDGE | WS_EX_MDICHILD | WS_EX_CONTROLPARENT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	MdiCreate.style = WS_EX_CONTROLPARENT | WS_EX_MDICHILD; //WS_EX_CLIENTEDGE  | WS_EX_CONTROLPARENT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	MdiCreate.lParam = 0;

	self->hWin = (HWND)SendMessageW(self->hwndParent, WM_MDICREATE, 0, (LPARAM)&MdiCreate);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}
	if (szCaption)
		PyMem_RawFree(szCaption);

	Py_INCREF(Py_None);
	Py_INCREF(Py_None);
	self->pyChildren = PyDict_New();
	self->pyToolBar = Py_None;
	self->pyStatusBar = Py_None;

	SendMessage(self->hwndParent, WM_MDIACTIVATE, self->hWin, NULL);
	ShowWindow(self->hWin, ((TyMdiAreaObject*)self->pyParent)->bMaximized ? SW_SHOWMAXIMIZED : SW_SHOW);
	SendMessage(self->hwndParent, WM_SETREDRAW, TRUE, 0);
	RedrawWindow(self->hwndParent, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);

	SendMessageW(self->hWin, WM_SETICON, ICON_SMALL, g->hIconMdi);
	SendMessageW(self->hWin, WM_SETICON, ICON_BIG, g->hIconMdi);
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

static int
TyMdiWindow_setattro(TyMdiWindowObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "before_close") == 0) {
			if (PyCallable_Check(pyValue)) {
				Py_XINCREF(pyValue);
				Py_XDECREF(self->pyBeforeCloseCB);
				self->pyBeforeCloseCB = pyValue;
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
				return -1;
			}
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "icon") == 0) {
			if (pyValue == Py_None) {
				SendMessageW(self->hWin, WM_SETICON, ICON_SMALL, 0);
				SendMessageW(self->hWin, WM_SETICON, ICON_BIG, 0);
			}
			else if (!PyObject_TypeCheck(pyValue, &TyIconType)) {
				PyErr_SetString(PyExc_TypeError, "Please assign an 'Icon'!");
				return -1;
			}
			Py_XDECREF(self->pyIcon);
			Py_INCREF(pyValue);
			self->pyIcon = (TyIconObject*)pyValue;
			SendMessageW(self->hWin, WM_SETICON, ICON_SMALL, (LPARAM)self->pyIcon->hWin);
			SendMessageW(self->hWin, WM_SETICON, ICON_BIG, (LPARAM)self->pyIcon->hWin);
			return 0;
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "position") == 0) {
			if (!PyTuple_Check(pyValue)) {
				PyErr_SetString(PyExc_TypeError, "Assigned value must be a tuple.");
				return -1;
			}
			if (PyTuple_Size(pyValue) != 4) {
				PyErr_SetString(PyExc_TypeError, "Assigned value must be tuple of four integers.");
				return -1;
			}
			int x = PyLong_AsLong(PyTuple_GetItem(pyValue, 0));
			int y = PyLong_AsLong(PyTuple_GetItem(pyValue, 1));
			int w = PyLong_AsLong(PyTuple_GetItem(pyValue, 2));
			int h = PyLong_AsLong(PyTuple_GetItem(pyValue, 3));
			MoveWindow(self->hWin, x, y, w, h, TRUE);

			return 0;
		}
	}
	return TyMdiWindowType.tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject* // new ref
TyMdiWindow_getattro(TyMdiWindowObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult, * pyAttribute;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "before_close") == 0) {
			PyErr_Clear();
			Py_INCREF(self->pyBeforeCloseCB);
			return self->pyBeforeCloseCB;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "position") == 0) {
			PyErr_Clear();
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(self->hWin, &wp);
			PyObject* pyRectangle = PyTuple_Pack(4, PyLong_FromLong(wp.rcNormalPosition.left), PyLong_FromLong(wp.rcNormalPosition.top), PyLong_FromLong(wp.rcNormalPosition.right - wp.rcNormalPosition.left), PyLong_FromLong(wp.rcNormalPosition.bottom - wp.rcNormalPosition.top));
			return pyRectangle;
		}
		if (PyDict_Contains(self->pyChildren, pyAttributeName)) {
			PyErr_Clear();
			pyAttribute = PyDict_GetItem(self->pyChildren, pyAttributeName);
			Py_INCREF(pyAttribute);
			return pyAttribute;
		}
	}
	Py_XDECREF(pyResult);
	return TyMdiWindowType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyMdiWindow_dealloc(TyMdiWindowObject* self)
{
	Py_XDECREF(self->pyChildren);
	if (self->hWin) {
		SendMessage(self->hwndParent, WM_MDIDESTROY, self->hWin, 0);
		self->hWin = 0;
	}
	Py_XDECREF(self->pyBeforeCloseCB);
	Py_XDECREF(self->pyOnCloseCB);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMemberDef TyMdiWindow_members[] = {
	{ "children", T_OBJECT, offsetof(TyMdiWindowObject, pyChildren), READONLY, "Child widgets" },
	{ "tool_bar", T_OBJECT, offsetof(TyMdiWindowObject, pyToolBar), READONLY, "ToolBar" },
	{ "status_bar", T_OBJECT, offsetof(TyMdiWindowObject, pyStatusBar), READONLY, "StatusBar" },
	{ "icon", T_OBJECT, offsetof(TyMdiWindowObject, pyIcon), READONLY, "Icon" },
	{ "parent", T_OBJECT, offsetof(TyMdiWindowObject, pyParent), READONLY, "Parent widget" },
	{ "key", T_OBJECT, offsetof(TyMdiWindowObject, pyKey), READONLY, "Key in parent's child dict" },
	{ "focus", T_OBJECT, offsetof(TyMdiWindowObject, pyFocusWidget), READONLY, "Widget that has the keyboard focus." },
	{ "min_width", T_INT, offsetof(TyMdiWindowObject, cxMin), 0, "Window can not be resized smaller" },
	{ "min_height", T_INT, offsetof(TyMdiWindowObject, cyMin), 0, "Window can not be resized smaller" },
	{ "before_close", T_OBJECT, offsetof(TyMdiWindowObject, pyBeforeCloseCB), 0, "Callback before closing" },
	{ "on_close", T_OBJECT, offsetof(TyMdiWindowObject, pyOnCloseCB), 0, "Callback on closing" },
	{ "on_focus_change", T_OBJECT, offsetof(TyMdiWindowObject, pyOnFocusChangeCB), 0, "Callback if keyboard focus moves to new widget" },
	{ NULL }
};

static PyMethodDef TyMdiWindow_methods[] = {
	{ NULL }
};

PyTypeObject TyMdiWindowType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.MdiWindow",        /* tp_name */
	sizeof(TyMdiWindowObject), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyMdiWindow_dealloc, /* tp_dealloc */
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
	TyMdiWindow_getattro,      /* tp_getattro */
	TyMdiWindow_setattro,      /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"MDI client window",       /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyMdiWindow_methods,       /* tp_methods */
	TyMdiWindow_members,       /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyMdiWindow_init, /* tp_init */
	0,                         /* tp_alloc */
	TyMdiWindow_new,           /* tp_new */
	PyObject_Free,             /* tp_free */
};