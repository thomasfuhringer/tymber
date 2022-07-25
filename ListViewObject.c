// ListViewObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

LRESULT CALLBACK TyListViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static PyObject*
TyListView_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyListViewObject* self = (TyListViewObject*)TyListViewType.tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->pyDataType = &PyList_Type;
		self->iAutoSizeColumn = -1;
		self->pyColumns = PyList_New(0);
		self->pyData = Py_None;
		Py_INCREF(Py_None);
		self->nSelectedRow = -1;
		self->pyOnRowChangedCB = NULL;
		self->pyOnSelectionChangedCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyListView_init(TyListViewObject* self, PyObject* args, PyObject* kwds)
{
	if (TyListViewType.tp_base->tp_init((PyObject*)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	self->hWin = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"",
		WS_CHILD | WS_TABSTOP | LVS_REPORT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (self->bVisible ? WS_VISIBLE : 0) | LVS_SINGLESEL | LVS_SHOWSELALWAYS, //| LVS_EDITLABELS
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYLISTVIEW, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}
	SendMessage(self->hWin, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT); // | LVS_EX_ONECLICKACTIVATE);

	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyListViewProc);
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g->hfDefaultFont, MAKELPARAM(FALSE, 0));
	return 0;
}

static BOOL
TyListView_AddColumn(TyListViewObject* self, Py_ssize_t nIndex, const char* strCaption, int  iWidth, int iAlign)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = nIndex;
	lvc.pszText = toW(strCaption);
	lvc.cx = iWidth;
	lvc.fmt = iAlign;
	if (ListView_InsertColumn(self->hWin, nIndex, &lvc) == -1)
		return FALSE;
	PyMem_RawFree(lvc.pszText);
	return TRUE;
}

BOOL
TyListView_UpdateRow(TyListViewObject* self, Py_ssize_t nRow)
{
	LVITEM lvItem;
	Py_ssize_t nColumn, nColumns, nRowSize, nMin;
	PyObject* pyCellData = NULL, * pyColumnDefinition, * pyType = NULL, * pyFormat = NULL, * pyRow, * pyText = NULL;

	lvItem.mask = LVIF_TEXT;
	lvItem.cchTextMax = 256;
	lvItem.stateMask = 0;
	lvItem.state = 0;
	lvItem.iItem = nRow;
	lvItem.iSubItem = 0;

	pyRow = PyList_GetItem(self->pyData, nRow);
	nColumns = PySequence_Size(self->pyColumns);
	nRowSize = PySequence_Size(pyRow);
	for (nColumn = 0; nColumn < nColumns; nColumn++)
	{
		lvItem.iSubItem = nColumn;
		if (nColumn < nRowSize) {
			pyCellData = PyList_GetItem(pyRow, nColumn);
			pyColumnDefinition = PyList_GetItem(self->pyColumns, nColumn);
			if (PyList_GetItem(pyColumnDefinition, 2) != Py_None) {
				pyFormat = NULL;
				if (PySequence_Size(pyColumnDefinition) > 3)
					pyFormat = PyList_GetItem(pyColumnDefinition, 3);
				pyType = PyList_GetItem(pyColumnDefinition, 1);
				if (!(pyText = TyFormatData(pyCellData, pyFormat)))
					return FALSE;

				lvItem.pszText = toW(PyUnicode_AsUTF8(pyText));
				if (SendMessage(self->hWin, LVM_SETITEM, 0, (LPARAM)&lvItem) == -1) {
					PyErr_SetFromWindowsErr(0);
					return FALSE;
				}
				PyMem_RawFree(lvItem.pszText);
				Py_DECREF(pyText);
			}
		}
		else {
			lvItem.pszText = L"";
			if (SendMessage(self->hWin, LVM_SETITEM, 0, (LPARAM)&lvItem) == -1) {
				PyErr_SetFromWindowsErr(0);
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL
TyListView_RenderData(TyListViewObject* self)
{
	LVITEM lvItem;
	Py_ssize_t nRow, nRows;

	SendMessage(self->hWin, LVM_DELETEALLITEMS, 0, 0);
	if (self->pyData == NULL || self->pyData == Py_None) {
		return TRUE;
	}
	lvItem.mask = LVIF_TEXT;
	lvItem.cchTextMax = 256;
	lvItem.stateMask = 0;
	lvItem.state = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText = L"";
	self->nSelectedRow = -1;
	nRows = PySequence_Size(self->pyData);
	for (nRow = 0; nRow < nRows; nRow++) {
		lvItem.iItem = nRow;
		if (SendMessage(self->hWin, LVM_INSERTITEM, 0, (LPARAM)&lvItem) == -1) {
			PyErr_SetFromWindowsErr(0);
			PyErr_Print();
			return FALSE;
		}

		if (!TyListView_UpdateRow(self, nRow))
			return FALSE;
	}
	return TRUE;
}

static PyObject*
TyListView_add_row(TyListViewObject* self, PyObject* args, PyObject* kwds)
{
	PyObject* pyData = NULL, * PyOldData;
	int iIndex = -2;
	LVITEM lvItem;
	static char* kwlist[] = { "data", "index", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist,
		&pyData,
		&iIndex))
		return NULL;

	if (!PyList_Check(pyData)) {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('data') must be a List, not '%.200s'.", pyData->ob_type->tp_name);
		return NULL;
	}

	Py_ssize_t nRows = 0;
	if (self->pyData != Py_None)
		nRows = PySequence_Size(self->pyData);
	if (iIndex <  -2 || iIndex > nRows) {
		PyErr_Format(PyExc_IndexError, "Argument 2 ('index') is out of range.");
		return NULL;
	}

	if (self->pyData == Py_None) {
		self->pyData = NULL;
		Py_DECREF(Py_None);
	}
	if (self->pyData == NULL) {
		self->pyData = PyList_New(0);
	}

	iIndex = (iIndex == -2 ? self->nSelectedRow : iIndex);
	iIndex = (iIndex == -1 ? PySequence_Size(self->pyData) : iIndex);

	lvItem.mask = LVIF_TEXT;
	lvItem.cchTextMax = 256;
	lvItem.pszText = L"";
	lvItem.stateMask = 0;
	lvItem.state = 0;
	lvItem.iItem = iIndex;
	lvItem.iSubItem = 0;

	if (SendMessage(self->hWin, LVM_INSERTITEM, 0, (LPARAM)&lvItem) == -1) {
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}

	if (PyList_Insert(self->pyData, iIndex, pyData) != 0)
		return NULL;
	Py_INCREF(pyData);

	if (!TyListView_UpdateRow(self, iIndex))
		return NULL;

	if (self->nSelectedRow > iIndex)
		self->nSelectedRow += 1;

	Py_RETURN_NONE;
}

static PyObject*
TyListView_update_row(TyListViewObject* self, PyObject* args, PyObject* kwds)
{
	PyObject* pyData = NULL, * pyOldData;
	int iIndex = -2;
	static char* kwlist[] = { "data", "index", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist,
		&pyData,
		&iIndex))
		return NULL;

	if (!PyList_Check(pyData)) {
		PyErr_Format(PyExc_TypeError, "Argument 1 ('data') must be a List, not '%.200s'.", pyData->ob_type->tp_name);
		return NULL;
	}

	Py_ssize_t nRows = PySequence_Size(self->pyData);
	if (iIndex <  -2 || iIndex > nRows) {
		PyErr_Format(PyExc_IndexError, "Argument 2 ('index') is out of range.");
		return NULL;
	}
	iIndex = (iIndex == -2 ? self->nSelectedRow : iIndex);
	if (iIndex == -1) {
		PyErr_Format(PyExc_IndexError, "Please specify a position.");
		return NULL;
	}

	//pyOldData = PyList_GetItem(self->pyData, iIndex);
	if (PyList_SetItem(self->pyData, iIndex, pyData) != 0)
		return NULL;
	Py_INCREF(pyData);

	if (!TyListView_UpdateRow(self, iIndex))
		return NULL;

	Py_RETURN_NONE;
}

static PyObject*
TyListView_delete_row(TyListViewObject* self, PyObject* args, PyObject* kwds)
{
	PyObject* pyData = NULL;
	int iIndex = -2;
	static char* kwlist[] = { "index", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist,
		&iIndex))
		return NULL;

	Py_ssize_t nRows = PySequence_Size(self->pyData);
	if (iIndex <  -2 || iIndex > nRows) {
		PyErr_Format(PyExc_IndexError, "Argument 1 ('index') is out of range.");
		return NULL;
	}

	iIndex = (iIndex == -2 ? self->nSelectedRow : iIndex);
	if (iIndex == -1) {
		PyErr_Format(PyExc_IndexError, "Please specify a position!");
		return NULL;
	}

	ListView_DeleteItem(self->hWin, iIndex);

	if (PySequence_DelItem(self->pyData, iIndex) != 0)
		return NULL;

	if (self->nSelectedRow > iIndex)
		self->nSelectedRow -= 1;
	else if (self->nSelectedRow == iIndex)
		TyListView_SelectionChanged(self, -1);

	Py_RETURN_NONE;
}

BOOL
TyListView_SelectionChanged(TyListViewObject* self, int iRow)
{
	self->nSelectedRow = iRow;
	if (self->pyOnSelectionChangedCB) {
		PyObject* pyResult = PyObject_CallFunction(self->pyOnSelectionChangedCB, "(O)", self);
		if (pyResult == NULL)
			return FALSE;
		Py_DECREF(pyResult);
	}
	return TRUE;
}

BOOL
TyListView_DoubleClick(TyListViewObject* self, int iRow)
{
	if (self->pyOnDoubleClickCB) {
		PyObject* pyResult = PyObject_CallFunction(self->pyOnDoubleClickCB, "(Oi)", self, iRow);
		if (pyResult == NULL)
			return FALSE;
		Py_DECREF(pyResult);
	}
	return TRUE;
}

BOOL
TyListView_SelectRow(TyListViewObject* self, Py_ssize_t nRow)
{
	ListView_SetItemState(self->hWin, nRow, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);
	return TRUE;
}

BOOL
TyListView_SetData(TyListViewObject* self, PyObject* pyData)
{
	if (self->pyData == pyData)
		return TRUE;
	if (!TyWidget_SetData((TyWidgetObject*)self, pyData))
		return FALSE;
	return TyListView_RenderData(self);
}

static int
TyListView_setattro(TyListViewObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "data") == 0) {
			if (self->pyColumns == NULL) {
				PyErr_SetString(PyExc_RuntimeError, "Please define columns first!");
				return -1;
			}
			if (!TyListView_SetData(self, pyValue))
				return -1;
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "columns") == 0) {
			if (!PyList_Check(pyValue)) {
				PyErr_SetString(PyExc_TypeError, "Please assign a List!");
				return -1;
			}
			if (!TyListView_SetData(self, Py_None))
				return -1;

			PyObject* pyColumn, * pyCaption, * pyDataType, * pyWidth;
			Py_ssize_t n, nLen;
			int iIndex = 0;
			nLen = PySequence_Size(pyValue);
			for (n = 0; n < nLen; n++) {
				pyColumn = PyList_GetItem(pyValue, n);
				if (!PyList_Check(pyColumn)) {
					PyErr_Format(PyExc_TypeError, "List items must be Lists with column definitions, not '%.200s'.", pyColumn->ob_type->tp_name);
					return -1;
				}
				if (PySequence_Size(pyColumn) < 3) {
					PyErr_Format(PyExc_TypeError, "Column definitions must be three items.");
					return -1;
				}
				pyWidth = PyList_GetItem(pyColumn, 2);
				if (pyWidth != Py_None) {
					pyCaption = PyList_GetItem(pyColumn, 0);
					pyDataType = PyList_GetItem(pyColumn, 1);
					if (!TyListView_AddColumn(self, iIndex, PyUnicode_AsUTF8(pyCaption), PyLong_AsLong(pyWidth), pyDataType == (PyObject*)&PyUnicode_Type ? LVCFMT_LEFT : LVCFMT_RIGHT))
						return -1;
					iIndex += 1;
				}
			}
			//ListView_SetColumnWidth(self->hWin, iIndex - 1, LVSCW_AUTOSIZE_USEHEADER);
			TyAttachObject(&self->pyColumns, pyValue);
			return 0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "row") == 0) {
			if (pyValue != Py_None && !PyLong_Check(pyValue)) {
				PyErr_Format(PyExc_TypeError, "Please assing an 'int' or None, not '%.200s'!", pyValue->ob_type->tp_name);
				return -1;
			}
			Py_ssize_t nRow = (pyValue == Py_None ? -1 : PyLong_AsLong(pyValue));
			if (!TyListView_SelectRow((TyEntryObject*)self, nRow))
				return -1;
			return 0;
		}
	}
	return TyListViewType.tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject*
TyListView_getattro(TyListViewObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "row") == 0) {
			PyErr_Clear();
			if (self->nSelectedRow == -1)
				Py_RETURN_NONE;
			else
				return PyLong_FromSize_t(self->nSelectedRow);
		}
	}
	return TyListViewType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyListView_dealloc(TyListViewObject* self)
{
	Py_XDECREF(self->pyColumns);
	TyListViewType.tp_base->tp_dealloc((TyWidgetObject*)self);
}

static PyMemberDef TyListView_members[] = {
	{ "data", T_OBJECT, offsetof(TyListViewObject, pyData), READONLY, "Data value" },
	{ "columns", T_OBJECT_EX, offsetof(TyListViewObject, pyColumns), READONLY, "List of column definitions" },
	{ "on_row_changed", T_OBJECT_EX, offsetof(TyListViewObject, pyOnRowChangedCB), 0, "Callback when row was modified" },
	{ "on_selection_changed", T_OBJECT_EX, offsetof(TyListViewObject, pyOnSelectionChangedCB), 0, "Callback when selected row changed" },
	{ "on_double_click", T_OBJECT_EX, offsetof(TyListViewObject, pyOnDoubleClickCB), 0, "Callback when row was double clicked" },
	{ NULL }
};

static PyMethodDef TyListView_methods[] = {
	{ "add_row", (PyCFunction)TyListView_add_row, METH_VARARGS | METH_KEYWORDS, "Insert a row at given index or append it at bottom." },
	{ "update_row", (PyCFunction)TyListView_update_row, METH_VARARGS | METH_KEYWORDS, "Update a row at given index or selected one." },
	{ "delete_row", (PyCFunction)TyListView_delete_row, METH_VARARGS | METH_KEYWORDS, "Delete a row at given index or selected one." },
	{ NULL }
};

PyTypeObject TyListViewType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.ListView",         /* tp_name */
	sizeof(TyListViewObject),  /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyListView_dealloc, /* tp_dealloc */
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
	TyListView_getattro,       /* tp_getattro */
	TyListView_setattro,       /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"List View widget objects", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyListView_methods,        /* tp_methods */
	TyListView_members,        /* tp_members */
	0,                         /* tp_getset */
	&TyWidgetType,             /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyListView_init, /* tp_init */
	0,                         /* tp_alloc */
	TyListView_new,            /* tp_new */
	PyObject_Free,             /* tp_free */
};

LRESULT CALLBACK
TyListViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TyListViewObject* self = (TyListViewObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if ((msg >= OCM__BASE))
		switch (msg -= OCM__BASE)
		{
		case WM_NOTIFY:
		{
			LPNMHDR nmhdr = (LPNMHDR)lParam;
			//printf("WM_NOTIFY %d %d %d \n", nmhdr->code, hwnd, LOWORD(wParam));
			if (nmhdr->code == LVN_ITEMCHANGED) {
				NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)nmhdr;
				if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVNI_SELECTED))
				{
					if (!TyListView_SelectionChanged(self, pNMListView->iItem)) {
						PyErr_Print();
					}
				}
			}
			if (nmhdr->code == NM_DBLCLK) {
				if (!TyListView_DoubleClick(self, (int)SendMessage(self->hWin, LVM_GETNEXTITEM, -1, LVNI_SELECTED))) {
					PyErr_Print();
				}
			}
		}
		}
	else
		switch (msg)
		{
			//default:
			//	break;
		}

	return CallWindowProcW(self->fnOldWinProcedure, hwnd, msg, wParam, lParam);
}