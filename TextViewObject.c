// TextViewObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"
#include <richedit.h>

static HMODULE hMsftEditDll = 0;
char* markdown2rtf(const char* md, const char* img_path);

static PyObject*
TyTextView_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyTextViewObject* self = (TyTextViewObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		//self->bMarkdown = FALSE;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyTextView_init(TyTextViewObject* self, PyObject* args, PyObject* kwds)
{
	if (Py_TYPE(self)->tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	if (hMsftEditDll == 0) {
		HMODULE hMsftEditDll = LoadLibrary(L"MsftEdit.dll");
		if (hMsftEditDll == NULL) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}
	}
	self->hWin = CreateWindowExW(WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"",
		ES_MULTILINE | WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_VSCROLL | (self->bVisible ? WS_VISIBLE : 0),   // | ES_READONLY
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYTEXTVIEW, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	LRESULT CALLBACK TyTextViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

BOOL
TyTextView_RenderData(TyTextViewObject* self)
{
	SETTEXTEX se;
	se.codepage = 65001;// CP_ACP;
	se.flags = ST_DEFAULT;

	if (self->pyData == NULL || self->pyData == Py_None) {
		SendMessage(self->hWin, EM_SETTEXTEX, 0, (LPARAM)L"");
		return TRUE;
	}

	BOOL bMarkdown = PyTuple_CheckExact(self->pyData);
	if (bMarkdown) {
		PyObject* pyTextMD = PyTuple_GetItem(self->pyData, 0);
		if (pyTextMD == NULL) {
			PyErr_SetString(PyExc_TypeError, "Data needs to be a tuple of (text, path).");
			return FALSE;
		}
		PyObject* pyPath = PyTuple_GetItem(self->pyData, 1);
		if (pyPath == NULL) {
			PyErr_SetString(PyExc_TypeError, "Data needs to be a tuple of (text, path).");
			return FALSE;
		}

		const char* strTextMD = PyUnicode_AsUTF8(pyTextMD);
		const char* strPath;
		if (pyPath == Py_None)
			strPath = NULL;
		else
			strPath = PyUnicode_AsUTF8(pyPath);

		char* strTextRTF = markdown2rtf(strTextMD, strPath);
		if (strTextRTF == NULL) {
			SendMessage(self->hWin, EM_SETTEXTEX, 0, (LPARAM)L"");
			return TRUE;
		}

		SendMessageW(self->hWin, EM_SETTEXTEX, (WPARAM)&se, (LPARAM)strTextRTF);
		free(strTextRTF);
	}
	else {
		const char* strText = PyUnicode_AsUTF8(self->pyData);
		SendMessageW(self->hWin, EM_SETTEXTEX, (WPARAM)&se, (LPARAM)strText);
	}

	return TRUE;
}

BOOL
TyTextView_SetData(TyTextViewObject* self, PyObject* pyData)
{
	if (self->pyData == pyData)
		return TRUE;

	if (PyTuple_CheckExact(pyData)) {
		self->pyDataType = &PyTuple_Type;
	}
	else if (PyUnicode_Check(pyData)) {
		self->pyDataType = &PyUnicode_Type;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Please assign a tuple or str!");
		return FALSE;
	}

	if (!TyWidget_SetData((TyWidgetObject*)self, pyData))
		return FALSE;
	return TyTextView_RenderData(self, TRUE);
}

static int
TyTextView_setattro(TyTextViewObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			if (!TyTextView_SetData((TyTextViewObject*)self, pyValue))
				return -1;
			return 0;
		}
	}
	return TyTextViewType.tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyTextView_getattro(TyTextViewObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		// for later
	}
	return TyTextViewType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyTextView_dealloc(TyTextViewObject* self)
{
	TyTextViewType.tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyTextView_members[] = {
	{ "data", T_OBJECT, offsetof(TyTextViewObject, pyData), READONLY, "Data value" },
	{ "margin", T_INT, offsetof(TyTextViewObject, iMargin), 0, "Margin inside widget" },
	{ NULL }
};

static PyMethodDef TyTextView_methods[] = {
	{ NULL }
};

PyTypeObject TyTextViewType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.TextView",         /* tp_name */
	sizeof(TyTextViewObject),  /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyTextView_dealloc, /* tp_dealloc */
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
	TyTextView_getattro,       /* tp_getattro */
	TyTextView_setattro,       /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Text view widget, to render plain text, RTF or Markdown", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyTextView_methods,        /* tp_methods */
	TyTextView_members,        /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyTextView_init, /* tp_init */
	0,                         /* tp_alloc */
	TyTextView_new,            /* tp_new */
	PyObject_Free,             /* tp_free */
};