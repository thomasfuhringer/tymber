// MenuObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

void Bitmap_SwapBackgroundColor(HBITMAP hbmp);
static iNextIdentifier = FIRST_CUSTOM_MENU_ID;

static PyObject*
TyMenu_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyMenuObject* self = (TyMenuObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->hWin = 0;
		self->pyIcon = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyMenu_init(TyMenuObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "parent", "key", "caption", "icon", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|O", kwlist,
		&self->pyParent,
		&self->pyKey,
		&self->pyCaption,
		&self->pyIcon))
		return -1;

	if (!PyUnicode_Check(self->pyKey)) {
		PyErr_Format(PyExc_TypeError, "Parameter 2 ('key') must be a string, not '%.200s'.", self->pyKey->ob_type->tp_name);
		return -1;
	}
	Py_INCREF(self->pyKey);

	LPWSTR szCaption = NULL;
	if (PyUnicode_Check(self->pyCaption)) {
		LPCSTR strCaption = PyUnicode_AsUTF8(self->pyCaption);
		szCaption = toW(strCaption);
	}
	else {
		PyErr_Format(PyExc_TypeError, "Parameter 3 ('caption') must be a string, not '%.200s'.", self->pyCaption->ob_type->tp_name);
		return -1;
	}
	Py_INCREF(self->pyCaption);

	if (PyObject_TypeCheck(self->pyParent, &TyApplicationType)) {
		if (((TyApplicationObject*)self->pyParent)->pyMenu) {
			PyErr_SetString(PyExc_RuntimeError, "The Application already has a Menu.");
			return -1;
		}
		self->hWin = g->hMenu;
		if (SetMenu(((TyWindowObject*)((TyApplicationObject*)self->pyParent)->pyWindow)->hWin, self->hWin) == 0) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}
		//SendMessage(((TyWindowObject*)((TyApplicationObject*)self->pyParent)->pyWindow)->hWin, WM_SIZE, 0, 0);

		if (!DrawMenuBar(((TyWindowObject*)((TyApplicationObject*)self->pyParent)->pyWindow)->hWin)) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}

		TyAttachObject(&((TyApplicationObject*)self->pyParent)->pyMenu, self);
		//TyWidget_MoveWindow(((TyWidgetObject*)((TyApplicationObject*)self->pyParent)->pyWindow));
		//UpdateWindow(((TyWindowObject*)((TyApplicationObject*)self->pyParent)->pyWindow)->hWin);
		//SendMessage(((TyWindowObject*)((TyApplicationObject*)self->pyParent)->pyWindow)->hWin, WM_SIZE, 0, 0);
		//EnumChildWindows(((TyWindowObject*)((TyApplicationObject*)self->pyParent)->pyWindow)->hWin, TyWidgetSizeEnumProc, (LPARAM)0);
	}
	else if (PyObject_TypeCheck(self->pyParent, &TyMenuType)) {

		if (PyDict_Contains(((TyMenuObject*)self->pyParent)->pyItems, self->pyKey)) {
			PyErr_SetString(PyExc_RuntimeError, "The parent Menu already contains an item with this key.");
			return -1;
		}

		if ((self->hWin = CreateMenu()) == NULL) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}
		if (AppendMenuW(((TyMenuObject*)self->pyParent)->hWin, MF_POPUP, (UINT_PTR)self->hWin, szCaption) == 0) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}

		if (PyDict_SetItem(((TyMenuObject*)self->pyParent)->pyItems, self->pyKey, self) == -1) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Parameter 1 ('parent') must be the Application or a Menu.");
		return -1;
	}

	if (self->pyIcon) {
		if (!PyObject_TypeCheck(self->pyIcon, &TyIconType)) {
			PyErr_Format(PyExc_TypeError, "Parameter 4 ('icon') must an Image, not '%.200s'.", self->pyIcon->ob_type->tp_name);
			return -1;
		}
		Py_INCREF(self->pyIcon);
	}

	HFree(szCaption);
	self->pyItems = PyDict_New();
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

static PyObject*
TyMenu_getattro(TyMenuObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult, * pyAttribute;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {

		if (PyDict_Contains(self->pyItems, pyAttributeName)) {
			PyErr_Clear();
			pyAttribute = PyDict_GetItem(self->pyItems, pyAttributeName);
			Py_INCREF(pyAttribute);
			return pyAttribute;
		}
	}
	Py_XDECREF(pyResult);
	return Py_TYPE(self)->tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static PyObject*
TyMenu_GetWindow(TyMenuObject* self)
{
	if (PyObject_TypeCheck(self->pyParent, &TyApplicationType))
		return ((TyApplicationObject*)self->pyParent)->pyWindow;
	else
		return TyMenu_GetWindow(self->pyParent);
}

static void
TyMenu_dealloc(TyMenuObject* self)
{
	Py_DECREF(self->pyCaption);
	Py_DECREF(self->pyIcon);
	Py_DECREF(self->pyItems);
	Py_TYPE(self)->tp_base->tp_dealloc((TyWidgetObject*)self);
}

static PyMemberDef TyMenu_members[] = {
	{ "caption", T_OBJECT, offsetof(TyMenuObject, pyCaption), READONLY, "Caption" },
	{ "children", T_OBJECT, offsetof(TyMenuObject, pyItems), READONLY, "Global objects" },
	{ NULL }
};

static PyMethodDef TyMenu_methods[] = {
	{ NULL }  /* Sentinel */
};

PyTypeObject TyMenuType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Menu",              /* tp_name */
	sizeof(TyMenuObject),      /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyMenu_dealloc, /* tp_dealloc */
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
	TyMenu_getattro,           /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	"Menu object",             /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyMenu_methods,            /* tp_methods */
	TyMenu_members,            /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyMenu_init,     /* tp_init */
	0,                         /* tp_alloc */
	TyMenu_new,                /* tp_new */
};

/* MenuItem -----------------------------------------------------------------------*/

static PyObject*
TyMenuItem_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyMenuItemObject* self = (TyMenuItemObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->pyOnClickCB = NULL;
		self->pyToolBar = NULL;
		self->pyIcon = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyMenuItem_init(TyMenuItemObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "parent", "key", "caption", "on_click", "icon", NULL };

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOOO|O", kwlist,
		&self->pyParent,
		&self->pyKey,
		&self->pyCaption,
		&self->pyOnClickCB,
		&self->pyIcon))
		return -1;

	Py_INCREF(self->pyCaption);

	if (!PyObject_TypeCheck(self->pyParent, &TyMenuType)) {
		PyErr_SetString(PyExc_TypeError, "Parameter 1 ('parent') must be a Menu.");
		return -1;
	}

	if (!PyUnicode_Check(self->pyKey)) {
		PyErr_Format(PyExc_TypeError, "Parameter 2 ('key') must be a string, not '%.200s'.", self->pyKey->ob_type->tp_name);
		return -1;
	}
	Py_INCREF(self->pyKey);

	if (!PyUnicode_Check(self->pyCaption)) {
		PyErr_SetString(PyExc_TypeError, "Parameter 3 ('caption') must be a string.");
		return -1;
	}
	Py_INCREF(self->pyCaption);

	if (!PyCallable_Check(self->pyOnClickCB)) {
		PyErr_SetString(PyExc_TypeError, "Parameter 4 ('on_click') must be callable");
		return -1;
	}
	Py_INCREF(self->pyOnClickCB);

	self->iIdentifier = iNextIdentifier++;
	if (self->iIdentifier > MAX_CUSTOM_MENU_ID) {
		PyErr_SetString(PyExc_RuntimeError, "No more MenuItem can be created.");
		return -1;
	}

	g->pyaMenuItem[self->iIdentifier - FIRST_CUSTOM_MENU_ID] = self;

	LPCSTR strCaption = PyUnicode_AsUTF8(self->pyCaption);
	LPWSTR szCaption = toW(strCaption);

	if (AppendMenuW(((TyMenuObject*)self->pyParent)->hWin, MF_STRING, self->iIdentifier, szCaption) == 0) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}
	HFree(szCaption);

	DrawMenuBar(((TyWindowObject*)TyMenu_GetWindow(self->pyParent))->hWin);

	if (self->pyIcon) {
		if (!PyObject_TypeCheck(self->pyIcon, &TyIconType)) {
			PyErr_Format(PyExc_TypeError, "Parameter 5 ('icon') must an Image, not '%.200s'.", ((PyObject*)self->pyIcon)->ob_type->tp_name);
			return -1;
		}
		Py_INCREF(self->pyIcon);

		ICONINFO iconinfo;
		GetIconInfo(self->pyIcon->hWin, &iconinfo);
		HBITMAP hBitmap = iconinfo.hbmColor;
		//Bitmap_SwapBackgroundColor(hBitmap);

		if (!SetMenuItemBitmaps(((TyMenuObject*)self->pyParent)->hWin, self->iIdentifier, MF_BYCOMMAND, hBitmap, hBitmap)) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}
	}

	return 0;
}

static void
TyMenuItem_dealloc(TyMenuItemObject* self)
{
	Py_DECREF(self->pyCaption);
	if (self->pyIcon)
		Py_DECREF(self->pyIcon);
	if (self->pyOnClickCB)
		Py_DECREF(self->pyOnClickCB);
	Py_TYPE(self)->tp_base->tp_dealloc((TyWidgetObject*)self);
}

static PyMemberDef TyMenuItem_members[] = {
	{ "caption", T_OBJECT, offsetof(TyMenuItemObject, pyCaption), READONLY, "Caption" },
	{ "on_click", T_OBJECT, offsetof(TyMenuItemObject, pyOnClickCB), READONLY, "'on_click' callback" },
	{ NULL }  /* Sentinel */
};

static PyMethodDef TyMenuItem_methods[] = {
	{ NULL }  /* Sentinel */
};

PyTypeObject TyMenuItemType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.MenuItem",           /* tp_name */
	sizeof(TyMenuItemObject),  /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyMenuItem_dealloc, /* tp_dealloc */
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
	"MenuItem object",         /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyMenuItem_methods,        /* tp_methods */
	TyMenuItem_members,        /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyMenuItem_init, /* tp_init */
	0,                         /* tp_alloc */
	TyMenuItem_new,            /* tp_new */
};

void
Bitmap_SwapBackgroundColor(HBITMAP hbmp)
{
	if (!hbmp)
		return;
	HDC hdc = GetDC(HWND_DESKTOP);
	BITMAP bm;
	GetObject(hbmp, sizeof(bm), &bm);
	BITMAPINFO bi = { 0 };
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = bm.bmWidth;
	bi.bmiHeader.biHeight = bm.bmHeight;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;

	DWORD* pixels = (DWORD*)PyMem_RawMalloc(bm.bmWidth * bm.bmHeight);
	GetDIBits(hdc, hbmp, 0, bm.bmHeight, pixels, &bi, DIB_RGB_COLORS);

	//assume that the color at (0,0) is the background color
	DWORD color_old = pixels[0];

	//this is the new background color
	DWORD bk = GetSysColor(COLOR_MENU);

	//swap RGB with BGR
	DWORD color_new = RGB(GetBValue(bk), GetGValue(bk), GetRValue(bk));

	for (int i = 0; i < (bm.bmWidth * bm.bmHeight); i++)
		if (pixels[i] == color_old)
			pixels[i] = color_new;

	SetDIBits(hdc, hbmp, 0, bm.bmHeight, pixels, &bi, DIB_RGB_COLORS);
	ReleaseDC(HWND_DESKTOP, hdc);
	PyMem_RawFree(pixels);
}