#include "Tymber.h"

static PyObject*
TyFileSelector_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyFileSelectorObject* self = (TyFileSelectorObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyFileSelector_init(TyFileSelectorObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "caption", "path",NULL };
	LPCSTR strCaption, strInitialPath = NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist,
		&strCaption,
		&strInitialPath))
		return -1;

	self->szCaption = toW(strCaption);
	self->szInitialPath = toW(strInitialPath);

	return 0;
}

static PyObject*
TyFileSelector_run(TyFileSelectorObject* self)
{
	OPENFILENAME ofn;
	//wchar_t szFilter[] = L"Bitmap (*.BMP)\0*.bmp\0";
	wchar_t szOpenFileNamePath[MAX_PATH];
	PyObject* pyFileNamePath;

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = ((TyWindowObject*)g->pyApp->pyWindow)->hWin;
	ofn.lpstrFilter = 0;// szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = szOpenFileNamePath;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;
	if (self->szInitialPath)
		ofn.lpstrInitialDir = self->szInitialPath;
	ofn.lpstrTitle = self->szCaption;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; //OFN_HIDEREADONLY | OFN_CREATEPROMPT ;
	ofn.lpstrDefExt = 0;// L"bmp";

	if (GetOpenFileNameW(&ofn) == TRUE) {
		pyFileNamePath = PyUnicode_FromWideChar(szOpenFileNamePath, -1);
		return pyFileNamePath;
	}
	Py_RETURN_NONE;
}

static void
TyFileSelector_dealloc(TyFileSelectorObject* self)
{
	PyMem_RawFree(self->szCaption);
	PyMem_RawFree(self->szInitialPath);
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyFileSelector_members[] = {
	{ NULL }
};

static PyMethodDef TyFileSelector_methods[] = {
	{ "run", (PyCFunction)TyFileSelector_run, METH_NOARGS, "Run as dialog (modal)." },
	{ NULL }
};

PyTypeObject TyFileSelectorType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.FileSelector",     /* tp_name */
	sizeof(TyFileSelectorObject), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyFileSelector_dealloc, /* tp_dealloc */
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
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"FileSelector object",     /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyFileSelector_methods,    /* tp_methods */
	TyFileSelector_members,    /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyFileSelector_init, /* tp_init */
	0,                         /* tp_alloc */
	TyFileSelector_new,        /* tp_new */
	PyObject_Free,             /* tp_free */
};