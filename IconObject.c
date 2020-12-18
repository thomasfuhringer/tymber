#include "Tymber.h"

static PyObject*
TyIcon_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyIconObject* self = (TyIconObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->hIconLarge = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyIcon_init(TyIconObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "source", NULL };
	PyObject* pySource = NULL;
	self->hWin = g->hIcon;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist,
		&pySource))
		return -1;

	if (pySource) {
		if (PyUnicode_Check(pySource)) {
			LPWSTR szFileName = toW(PyUnicode_AsUTF8(pySource));
			LPWSTR szExtension = PathFindExtensionW(szFileName);

			if (lstrcmpi(szExtension, L".ico") == 0) {
				self->hWin = LoadImageW(0, szFileName, IMAGE_ICON, 0, 0, LR_LOADFROMFILE); //LR_DEFAULTSIZE | 
				if (self->hWin == 0) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else {
				PyErr_Format(PyExc_RuntimeError, "The file ending must be .ico.");
				return -1;
			}
			PyMem_RawFree(szFileName);
		}
		else if (PyObject_TypeCheck(pySource, &PyBytes_Type)) {
			char* sData = PyBytes_AsString(pySource);
			int iSize = PyBytes_GET_SIZE(pySource);
			int iOffset = LookupIconIdFromDirectoryEx(sData, TRUE, 0, 0, LR_DEFAULTCOLOR);
			//printf("iOffset %d\n", iOffset);
			if (iOffset != 0)
				self->hWin = CreateIconFromResourceEx(sData + iOffset, 0, TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
			if (self->hWin == 0) printf("Failed to generate Icon.\n");
		}
		else if (PyObject_TypeCheck(pySource, g->pyEnumType)) {
			if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "window")) {
				if (ExtractIconExW(L"SHELL32.DLL", -3, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "file_open")) {
				if (ExtractIconExW(L"SHELL32.DLL", -235, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "file_new")) {
				if (ExtractIconExW(L"SHELL32.DLL", -152, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "save")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16761, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "ok")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16802, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "no")) {
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "copy")) {
				if (ExtractIconExW(L"SHELL32.DLL", -243, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "cut")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16762, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "paste")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16763, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "find")) {
				if (ExtractIconExW(L"SHELL32.DLL", -323, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "properties")) {
				if (ExtractIconExW(L"SHELL32.DLL", -174, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "settings")) {
				self->hWin = ExtractIconW(g->hInstance, L"SHELL32.DLL", -16826);
				if (ExtractIconExW(L"SHELL32.DLL", -16826, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "window")) {
				if (ExtractIconExW(L"SHELL32.DLL", -3, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "undo")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -5315, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "redo")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -5311, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "right")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16805, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "up")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16749, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "down")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16750, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "refresh")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16739, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "close")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -97, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "exit")) {
				if (ExtractIconExW(L"SHELL32.DLL", -28, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "delete")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -89, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "help")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -99, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "information")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -81, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "warning")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -1403, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "lock")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -1304, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "edit")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -5306, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Parameter 1 ('source') specifies a StockItem that is not available.");
				return -1;
			}
		}
		else {
			PyErr_SetString(PyExc_TypeError, "Argument 1 ('source') can only be a byte object, a string with the file name or a StockItem enum member.");
			return -1;
		}
		SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	}
	return 0;
}

static void
TyIcon_dealloc(TyIconObject* self)
{
	DestroyIcon(self->hWin);
	if (self->hIconLarge)
		DestroyIcon(self->hIconLarge);

	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyIcon_members[] = {
	{ NULL }
};

static PyMethodDef TyIcon_methods[] = {
	{ NULL }
};

PyTypeObject TyIconType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Icon",             /* tp_name */
	sizeof(TyIconObject),      /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyIcon_dealloc, /* tp_dealloc */
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
	"Icon object",             /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyIcon_methods,            /* tp_methods */
	TyIcon_members,            /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyIcon_init,     /* tp_init */
	0,                         /* tp_alloc */
	TyIcon_new,                /* tp_new */
	PyObject_Free,             /* tp_free */
};