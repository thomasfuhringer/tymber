#include "Tymber.h"

static PyObject*
TyLabel_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyLabelObject* self = (TyLabelObject*)type->tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->iTextColor = 0;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static LRESULT CALLBACK TyLabelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static int
TyLabel_init(TyLabelObject* self, PyObject* args, PyObject* kwds)
{
	if (Py_TYPE(self)->tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	if (!TyWidget_CalculateRect(self, &rect))
		return -1;
	self->hWin = CreateWindowEx(0, L"STATIC", L"",
		WS_CHILD | (self->bVisible ? WS_VISIBLE : 0) | SS_CENTERIMAGE,
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYLABEL, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	if (self->pyCaption != NULL)
		if (!TyWidget_SetCaption((TyWidgetObject*)self, self->pyCaption))
			return -1;
	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));
	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyLabelProc);
	return 0;
}

BOOL
TyLabel_SetData(TyLabelObject* self, PyObject* pyData)
{
	return TyWidget_SetData((TyWidgetObject*)self, pyData);
}

static int
TyLabel_setattro(TyLabelObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "caption") == 0) {
			return TyWidget_SetCaption((TyWidgetObject*)self, pyValue) ? 0 : -1;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			return TyLabel_SetData(self, pyValue) ? 0 : -1;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "align_h") == 0) {
			LONG_PTR pAlign = GetWindowLongPtr(self->hWin, GWL_STYLE);
			PyObject* pyAlign = PyObject_GetAttrString(pyValue, "value");
			switch (PyLong_AsLong(pyAlign))
			{
			case ALIGN_LEFT:
				pAlign = pAlign | SS_LEFT;
				break;
			case ALIGN_RIGHT:
				pAlign = pAlign | SS_RIGHT;
				break;
			case ALIGN_CENTER:
				pAlign = pAlign | SS_CENTER;
				break;
			}
			SetWindowLongPtr(self->hWin, GWL_STYLE, pAlign);
			TyAttachObject(&self->pyAlignVertical, pyAlign);
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "align_v") == 0) {
			LONG_PTR pAlign = GetWindowLongPtr(self->hWin, GWL_STYLE);
			PyObject* pyAlign = PyObject_GetAttrString(pyValue, "value");
			switch (PyLong_AsLong(pyAlign))
			{
			case ALIGN_TOP:
				//pAlign = pAlign | SS_TOP ; 
				break;
			case ALIGN_BOTTOM:
				//pAlign = pAlign | SS_BOTTOM ; 
				break;
			case ALIGN_CENTER:
				pAlign = pAlign | SS_CENTERIMAGE;
				break;
			}
			SetWindowLongPtr(self->hWin, GWL_STYLE, pAlign);
			TyAttachObject(&self->pyAlignVertical, pyAlign);
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "text_color") == 0) {
			if (PyTuple_Check(pyValue)) {
				int iR, iG, iB;
				PyObject* pyColorComponent = PyTuple_GetItem(pyValue, 0);
				if (pyColorComponent == NULL)
					return -1;
				iR = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 1);
				if (pyColorComponent == NULL)
					return -1;
				iG = PyLong_AsLong(pyColorComponent);
				pyColorComponent = PyTuple_GetItem(pyValue, 2);
				if (pyColorComponent == NULL)
					return -1;
				iB = PyLong_AsLong(pyColorComponent);

				self->iTextColor = RGB(iR, iG, iB);
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assign a tuple of RGB values!");
				return -1;
			}
		}
	}
	return TyLabelType.tp_base->tp_setattro((TyWidgetObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyLabel_getattro(TyLabelObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			TCHAR szText[1024];
			GetWindowText(self->hWin, szText, 1024);
			LPCSTR strText = toU8(szText);
			PyObject* pyText = PyUnicode_FromString(strText);
			PyMem_RawFree(strText);
			PyErr_Clear();
			return pyText;
		}
	}
	Py_XDECREF(pyResult);
	return TyLabelType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyLabel_dealloc(TyLabelObject* self)
{
	TyLabelType.tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyLabel_members[] = {
	{ NULL }
};

static PyMethodDef TyLabel_methods[] = {
	{ NULL }
};

PyTypeObject TyLabelType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Label",            /* tp_name */
	sizeof(TyLabelObject),     /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyLabel_dealloc, /* tp_dealloc */
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
	TyLabel_getattro,          /* tp_getattro */
	TyLabel_setattro,          /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Label widget",            /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyLabel_methods,           /* tp_methods */
	TyLabel_members,           /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyLabel_init,    /* tp_init */
	0,                         /* tp_alloc */
	TyLabel_new,               /* tp_new */
	PyObject_Free,             /* tp_free */
};

static LRESULT CALLBACK
TyLabelProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TyLabelObject* self = (TyLabelObject*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	if (self) {
		switch (msg)
		{
		case OCM_CTLCOLORSTATIC: {
			HDC hDC = (HDC)wParam;
			SetBkMode(hDC, TRANSPARENT);
			if (self->iTextColor) {
				SetTextColor(hDC, self->iTextColor);
			}
			return g->hBkgBrush;
		}
		default:
			break;
		}
		return CallWindowProcW(self->fnOldWinProcedure, hwnd, msg, wParam, lParam);
	}
}