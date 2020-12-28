// MdiAreaObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"
#define IDM_FIRSTCHILD    50000

static PyObject*
TyMdiArea_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyMdiAreaObject* self = (TyMdiAreaObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->bMaximized = FALSE;
		self->pyChildren = PyDict_New();
		self->pyOnActivatedCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyMdiArea_init(TyMdiAreaObject* self, PyObject* args, PyObject* kwds)
{
	if (TyMdiAreaType.tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	if (!TyWidget_CalculateRect(self, &rect))
		return -1;

	CLIENTCREATESTRUCT  MDIClientCreateStruct;
	MDIClientCreateStruct.hWindowMenu = NULL;
	MDIClientCreateStruct.idFirstChild = IDM_FIRSTCHILD;

	if ((self->hWin = CreateWindowExW(WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT, L"MDICLIENT", L"",
		WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL,// | MDIS_ALLCHILDSTYLES,
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYMDIAREA, g->hInstance, (void*)&MDIClientCreateStruct)) == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	if (PyObject_TypeCheck(self->pyParent, &TyWindowType)) {
		if (((TyWindowObject*)self->pyParent)->hMdiArea) {
			PyErr_Format(PyExc_TypeError, "The parent window already has an MdiArea.");
			return -1;
		}
		((TyWindowObject*)self->pyParent)->hMdiArea = self->hWin; // necessary for DefFrameProcW()
	}
	else {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('parent') must be a Window, not '%.200s'.", ((PyObject*)self->pyParent)->ob_type->tp_name);
		return -1;
	}

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

static PyObject*
TyMdiArea_tile(TyMdiAreaObject* self)
{
	SendMessage(self->hWin, WM_MDITILE, MDITILE_HORIZONTAL, 0);
	Py_RETURN_NONE;
}

static int
TyMdiArea_setattro(TyMdiAreaObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "menu") == 0) {
			if (!PyObject_TypeCheck(pyValue, &TyMenuType)) {
				PyErr_SetString(PyExc_TypeError, "Assign a Menu!");
				return -1;
			}
			SendMessage(self->hWin, WM_MDISETMENU, NULL, ((TyMenuObject*)pyValue)->hWin);
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "maximized") == 0) {
			if (!PyBool_Check(pyValue)) {
				PyErr_SetString(PyExc_TypeError, "Assign a Bool!");
				return -1;
			}
			self->bMaximized = (pyValue == Py_True);
			BOOLEAN bMaximized = TRUE;
			HWND hActiveChild = (HWND)SendMessage(self->hWin, WM_MDIGETACTIVE, 0, &bMaximized);
			if ((hActiveChild == 0) && (PyDict_Size(self->pyChildren) > 0)) {  // if no child is active just maximize the first
				PyObject* pyKey, * pyValue;
				Py_ssize_t nPos = 0;
				PyDict_Next(self->pyChildren, &nPos, &pyKey, &pyValue);
				hActiveChild = ((TyMdiWindowObject*)pyValue)->hWin;
			}
			if (hActiveChild)
				SendMessage(self->hWin, self->bMaximized ? WM_MDIMAXIMIZE : WM_MDIRESTORE, hActiveChild, NULL);
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "active_child") == 0) {
			if (!PyObject_TypeCheck(pyValue, &TyMdiWindowType)) {
				PyErr_SetString(PyExc_TypeError, "Assign an MdiWindow!");
				return -1;
			}
			SendMessage(self->hWin, WM_MDIACTIVATE, ((TyMenuObject*)pyValue)->hWin, NULL);
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "on_activated") == 0) {
			if (PyCallable_Check(pyValue)) {
				Py_XINCREF(pyValue);
				Py_XDECREF(self->pyOnActivatedCB);
				self->pyOnActivatedCB = pyValue;
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a function!");
				return -1;
			}
		}
	}

	return TyMdiAreaType.tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject* // new ref
TyMdiArea_getattro(TyMdiAreaObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult, * pyAttribute;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "menu") == 0) {
			PyErr_Clear();
			Py_RETURN_NONE; // To do
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "maximized") == 0) {
			PyErr_Clear();
			//BOOLEAN bMaximized = FALSE;
			HWND hActiveChild = (HWND)SendMessage(self->hWin, WM_MDIGETACTIVE, 0, &self->bMaximized);
			return PyBool_FromLong(self->bMaximized);
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "active_child") == 0) {
			PyErr_Clear();
			HWND hActiveChild = (HWND)SendMessage(self->hWin, WM_MDIGETACTIVE, 0, NULL);
			if (hActiveChild == 0)
				Py_RETURN_NONE;
			TyMdiWindowObject* pyChild = (TyMdiWindowObject*)GetWindowLongPtr(hActiveChild, GWLP_USERDATA);
			Py_INCREF(pyChild);
			return pyChild;
		}
		if (PyDict_Contains(self->pyChildren, pyAttributeName)) {
			PyErr_Clear();
			pyAttribute = PyDict_GetItem(self->pyChildren, pyAttributeName);
			Py_INCREF(pyAttribute);
			return pyAttribute;
		}
	}
	Py_XDECREF(pyResult);
	return TyMdiAreaType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyMdiArea_dealloc(TyMdiAreaObject* self)
{
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyMdiArea_members[] = {
	{ "children", T_OBJECT, offsetof(TyMdiAreaObject, pyChildren), READONLY, "Child windows" },
	{ "on_activated", T_OBJECT_EX, offsetof(TyMdiAreaObject, pyOnActivatedCB), READONLY, "On Activated callback" },
	{ NULL }
};

static PyMethodDef TyMdiArea_methods[] = {
	{ "tile", (PyCFunction)TyMdiArea_tile, METH_NOARGS, "Arrange child windows in a tile format." },
	{ NULL }
};

PyTypeObject TyMdiAreaType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.MdiArea",          /* tp_name */
	sizeof(TyMdiAreaObject),   /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyMdiArea_dealloc, /* tp_dealloc */
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
	TyMdiArea_getattro,        /* tp_getattro */
	TyMdiArea_setattro,        /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	"Client area for MdiWindow", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyMdiArea_methods,         /* tp_methods */
	TyMdiArea_members,         /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyMdiArea_init,  /* tp_init */
	0,                         /* tp_alloc */
	TyMdiArea_new,             /* tp_new */
};