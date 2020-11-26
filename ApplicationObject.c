// ApplicationObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

static PyObject*
TyApplication_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyApplicationObject* self = (TyApplicationObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->pyWindow = NULL;
		self->pyMenu = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyApplication_init(TyApplicationObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "window", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist,
		&self->pyWindow))
		return -1;

	if (!PyObject_TypeCheck(self->pyWindow, &TyWindowType)) {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('window') must be a Window, not '%.200s'.", self->pyWindow->ob_type->tp_name);
		return -1;
	}
	Py_INCREF(self->pyWindow);

	if (g->pyApp = Py_None) {
		TyAttachObject(&g->pyApp, self);
		PyObject* pyDict = PyModule_GetDict(g->pyModule);
		PyDict_SetItemString(pyDict, "app", self);
	}
	else {
		PyErr_Format(PyExc_RuntimeError, "An instance of Application already exist. Only one can be created.");
		return -1;
	}
	self->pyChildren = PyDict_New();
	return 0;
}

PyObject*
TyApplication_run(TyApplicationObject* self, PyObject* args, PyObject* kwds)
{
	PyObject* pyResult = PyObject_CallMethod(self->pyWindow, "run", NULL);
	Py_DECREF(pyResult);
	Py_RETURN_NONE;
}

static int
TyApplication_setattro(TyApplicationObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {

	}
	return PyObject_GenericSetAttr((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyApplication_getattro(TyApplicationObject* self, PyObject* pyAttributeName)
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
	return TyApplicationType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyApplication_dealloc(TyApplicationObject* self)
{
	Py_XDECREF(self->pyChildren);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMemberDef TyApplication_members[] = {
	{ "window", T_OBJECT, offsetof(TyApplicationObject, pyWindow), READONLY, "Main window" },
	{ "menu", T_OBJECT, offsetof(TyApplicationObject, pyMenu), READONLY, "Main menu" },
	{ "children", T_OBJECT, offsetof(TyApplicationObject, pyChildren), READONLY, "Global objects" },
	{ NULL }  /* Sentinel */
};

static PyMethodDef TyApplication_methods[] = {
	{ "run", (PyCFunction)TyApplication_run, METH_VARARGS | METH_KEYWORDS, "Run the app" },
	{ NULL }  /* Sentinel */
};

PyTypeObject TyApplicationType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Application",      /* tp_name */
	sizeof(TyApplicationObject), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyApplication_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,					       /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	0,                         /* tp_str */
	TyApplication_getattro,    /* tp_getattro */
	TyApplication_setattro,    /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Application singleton",   /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyApplication_methods,     /* tp_methods */
	TyApplication_members,     /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyApplication_init,  /* tp_init */
	0,                         /* tp_alloc */
	TyApplication_new,         /* tp_new */
	PyObject_Free,             /* tp_free */
};