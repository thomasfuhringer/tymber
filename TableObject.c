﻿// TableObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"

#define TyTABLE_ROW_INDICATOR_WIDTH 16
LRESULT CALLBACK TyTableProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static PyObject *
TyTable_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	TyTableObject* self = (TyTableObject*)TyTableType.tp_base->tp_new(type, args, kwds);
	if (self != NULL) {
		self->bTable = TRUE;
		self->bPointer = TRUE;
		self->bShowRecordIndicator = TRUE;
		self->nColumns = 0;
		self->iAutoSizeColumn = -1;
		self->pyColumns = PyList_New(0);
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyTable_init(TyTableObject *self, PyObject *args, PyObject *kwds)
{
	//OutputDebugString(L"\n-Table init-\n");
	if (TyTableType.tp_base->tp_init((PyObject *)self, args, kwds) < 0)
		return -1;

	RECT rect;
	TransformRectToAbs(&self->rc, gArgs.hwndParent, &rect);

	self->hWin = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"",
		WS_CHILD | WS_TABSTOP | LVS_REPORT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | (gArgs.bVisible ? WS_VISIBLE : 0) | LVS_SINGLESEL | LVS_SHOWSELALWAYS, //| LVS_EDITLABELS
		rect.left, rect.top, rect.right, rect.bottom,
		self->hwndParent, (HMENU)IDC_TYTABLE, g->hInstance, NULL);

	if (self->hWin == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	SendMessage(self->hWin, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);// | LVS_EX_ONECLICKACTIVATE);

	self->fnOldWinProcedure = (WNDPROC)SetWindowLongPtrW(self->hWin, GWLP_WNDPROC, (LONG_PTR)TyTableProc);
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	SendMessage(self->hWin, WM_SETFONT, (WPARAM)g.hfDefaultFont, MAKELPARAM(FALSE, 0));
	//ShowWindow(self->hWin, TRUE);

	// row indicator column
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.pszText = L"";
	lvc.cx = TyTABLE_ROW_INDICATOR_WIDTH;
	lvc.fmt = LVCFMT_CENTER;

	if (ListView_InsertColumn(self->hWin, 0, &lvc) == -1) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	//OutputDebugString(L"\n-Table init through-\n");
	return 0;
}

static PyObject *
TyTable_add_column(TyTableObject* self, PyObject *args, PyObject *kwds)
{
	static char *kwlist[] = { "caption", "width", "data", "type", "format", "widget", "autoSize", NULL };
	int iWidth;
	BOOL bAutoSize;
	PyObject* pyCaption = NULL, *pyDataName = NULL, *pyDynasetColumn = NULL, *pyType = NULL, *pyFormat = NULL, *pyWidget = NULL;
	TyTableColumnObject* pyColumn = NULL;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi|OOOOp", kwlist,
		&pyCaption,
		&iWidth,
		&pyDataName,
		&pyType,
		&pyFormat,
		&pyWidget,
		&bAutoSize))
		return NULL;

	if (!PyUnicode_Check(pyCaption)) {
		PyErr_SetString(PyExc_TypeError, "Parameter 1 ('caption') must be a string.");
		return NULL;
	}

	if (pyDataName) {
		if ((pyDynasetColumn = PyDict_GetItem(self->pyDynaset->pyColumns, pyDataName)) == NULL)
			return PyErr_Format(PyExc_ValueError, "DataColumn '%s' does not exist in bound Dynaset'.", PyUnicode_AsUTF8(pyDataName));

		PyObject* pyDataType = PyStructSequence_GetItem(pyDynasetColumn, TYDYNASETCOLUMN_TYPE);
		if (pyType) {
			if (pyType != pyDataType) {
				PyErr_SetString(PyExc_ValueError, "Parameter 4 ('type') is conflicting with data type of bound DataColumn.");
				return NULL;
			}
		}
		else {
			pyType = pyDataType;
			Py_INCREF(pyType);
		}

		if (!pyFormat) {
			pyFormat = PyStructSequence_GetItem(pyDynasetColumn, TYDYNASETCOLUMN_FORMAT);
		}
	}

	if (pyType) {
		if (!PyType_Check(pyType)) {
			PyErr_SetString(PyExc_TypeError, "Parameter 4 ('type') must be a Type Object.");
			return NULL;
		}
	}
	else {
		pyType = (PyObject*)&PyUnicode_Type;
	}

	if (pyFormat) {
		if (pyFormat != Py_None && !PyUnicode_Check(pyFormat)) {
			PyErr_SetString(PyExc_TypeError, "Parameter 5 ('format') must be a string.");
			return NULL;
		}
	}
	else {
		pyFormat = Py_None;
	}

	//PyObject *pyArgs = Py_BuildValue("(sis)", "x", iWidth, "y");
	//XX(&TyTableColumnType);
	if (!(pyColumn = PyObject_CallObject((PyObject*)&TyTableColumnType, NULL)))
		return NULL;
	//XX(pyColumn);
	Py_INCREF(pyDynasetColumn);
	Py_INCREF(pyType);
	Py_INCREF(pyFormat);
	pyColumn->pyDynasetColumn = pyDynasetColumn;
	pyColumn->pyType = pyType;
	pyColumn->pyFormat = pyFormat;
	pyColumn->iIndex = (int)self->nColumns++;
	pyColumn->pyTable = self;

	if (pyWidget) {
		if (!PyObject_TypeCheck(pyWidget, &TyWidgetType)) {
			PyErr_SetString(PyExc_TypeError, "Parameter 6 ('widget') must be a Widget.");
			return NULL;
		}
		Py_INCREF(self);
		Py_INCREF(pyWidget);
		pyColumn->pyWidget = (TyWidgetObject*)pyWidget;
		pyColumn->pyWidget->pyParent = (TyWidgetObject*)self;
		pyColumn->pyWidget->pyWindow = self->pyWindow;
		pyColumn->pyWidget->pyDynaset = self->pyDynaset;
		pyColumn->pyWidget->pyDataColumn = self->pyDataColumn;
	}

	if (PyList_Append(self->pyColumns, pyColumn) == -1) {
		return NULL;
	}

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = pyColumn->iIndex + 1;
	lvc.pszText = toW(PyUnicode_AsUTF8(pyCaption));
	lvc.cx = iWidth;
	lvc.fmt = pyType == (PyObject*)&PyUnicode_Type ? LVCFMT_LEFT : LVCFMT_RIGHT;

	if (ListView_InsertColumn(self->hWin, pyColumn->iIndex + 1, &lvc) == -1) {
		return NULL;
	}

	if (bAutoSize)
		self->iAutoSizeColumn = lvc.iSubItem;

	return (PyObject*)pyColumn;
}

static PyObject *
TyTable_refresh(TyTableObject* self)
{
	PyObject* pyData = NULL, *pyRow, *pyDynasetColumn = NULL, *pyText = NULL;
	TyTableColumnObject* pyTableColumn = NULL;
	Py_ssize_t nColumn;
	int iRow, iColumn;
	//PyObject *pyData = TyWidget_PullData((TyWidgetObject*)self);

	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.cchTextMax = 256;
	lvItem.stateMask = 0;
	lvItem.state = 0;

	SendMessage(self->hWin, LVM_DELETEALLITEMS, 0, 0);

	for (iRow = 0; iRow < self->pyDynaset->nRows; iRow++) {
		pyRow = PyList_GetItem(self->pyDynaset->pyRows, (Py_ssize_t)iRow);

		if (PyStructSequence_GetItem(pyRow, TYDYNASETROW_DELETE) == Py_True)
			lvItem.pszText = L"Х";
		else if (PyStructSequence_GetItem(pyRow, TYDYNASETROW_NEW) == Py_True)
			lvItem.pszText = L"☼";
		else if (PyStructSequence_GetItem(pyRow, TYDYNASETROW_DATAOLD) != Py_None)
			lvItem.pszText = L"Ҩ";
		else
			lvItem.pszText = L"";

		lvItem.iItem = iRow;
		lvItem.iSubItem = 0;

		if (SendMessage(self->hWin, LVM_INSERTITEM, 0, (LPARAM)&lvItem) == -1) {
			PyErr_SetFromWindowsErr(0);
			return NULL;
		}

		for (iColumn = 1; iColumn < self->nColumns + 1; iColumn++) // Add SubItems in a loop
		{
			//ShowInts(L"X", iRow, iColumn);
			pyTableColumn = (TyTableColumnObject*)PyList_GetItem(self->pyColumns, (Py_ssize_t)iColumn - 1);
			pyDynasetColumn = pyTableColumn->pyDynasetColumn;
			pyData = TyDynaset_GetData(self->pyDynaset, (Py_ssize_t)iRow, pyDynasetColumn);
			if (!(pyText = TyFormatData(pyData, pyTableColumn->pyFormat)))
				return NULL;

			//lvItem.iItem = iRow;
			lvItem.iSubItem = iColumn;
			lvItem.pszText = toW(PyUnicode_AsUTF8(pyText));

			if (SendMessage(self->hWin, LVM_SETITEM, 0, (LPARAM)&lvItem) == -1) {
				PyErr_SetFromWindowsErr(0);
				return NULL;
			}
			PyMem_RawFree(lvItem.pszText);
			Py_DECREF(pyText);
		}
	}
	Py_RETURN_TRUE;
}


static PyObject *
TyTable_refresh_cell(TyTableObject* self, PyObject *args)
{
	PyObject* pyData = NULL, *pyDynasetColumn = NULL, *pyText = NULL;
	TyTableColumnObject* pyTableColumn = NULL;
	Py_ssize_t nColumn, nRow;
	OutputDebugString(L"\n-Table refresh cell\n");

	if (!PyArg_ParseTuple(args, "nO", &nRow, &pyDynasetColumn)) {
		return NULL;
	}

	for (nColumn = 0; nColumn < self->nColumns; nColumn++) {
		pyTableColumn = (TyTableColumnObject*)PyList_GetItem(self->pyColumns, nColumn);
		if (pyTableColumn->pyDynasetColumn == pyDynasetColumn)
			break;
		pyTableColumn = NULL;
	}

	if (pyTableColumn == NULL)
		Py_RETURN_FALSE;

	pyData = TyDynaset_GetData(self->pyDynaset, nRow, pyDynasetColumn);
	if (!(pyText = TyFormatData(pyData, pyTableColumn->pyFormat)))
		return NULL;

	LVITEM lvItem;
	lvItem.mask = LVIF_TEXT;
	lvItem.cchTextMax = 256;
	lvItem.stateMask = 0;
	lvItem.state = 0;
	lvItem.iItem = (int)nRow;
	lvItem.iSubItem = (int)nColumn + 1;
	lvItem.pszText = toW(PyUnicode_AsUTF8(pyText));

	if (SendMessage(self->hWin, LVM_SETITEM, 0, (LPARAM)&lvItem) == -1) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	PyMem_RawFree(lvItem.pszText);
	Py_DECREF(pyText);

	OutputDebugString(L"\n- Table cell Refre t\n");
	Py_RETURN_TRUE;
}

static PyObject *
TyTable_refresh_row_pointer(TyTableObject* self, PyObject *args)
{
	ListView_SetItemState(self->hWin, (INT)self->pyDynaset->nRow, LVIS_FOCUSED, LVIS_FOCUSED);// LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	//ListView_SetSelectionMark(self->hWin, (INT)self->pyDynaset->nRow);
	Py_RETURN_NONE;
}
/*
BOOL
TyTable_SelectionChanged(TyTableObject* self, int iRow)
{
//ShowLastError(L"TyTable_SelectionChanged");
if (TyDynaset_SetRow(self->pyDynaset, (Py_ssize_t)iRow)) {
// set blue bar
return TRUE;
}
return FALSE;
}
int
TyTable_Notify(TyTableObject* self, LPNMHDR nmhdr)
{
}
*/

static int
TyTable_setattro(TyTableObject* self, PyObject* pyAttributeName, PyObject *pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "showRowIndicator") == 0) {
			if (pyValue == Py_True) {
				ListView_SetColumnWidth(self->hWin, 0, TyTABLE_ROW_INDICATOR_WIDTH);
				self->bShowRecordIndicator = TRUE;
			}
			else {
				ListView_SetColumnWidth(self->hWin, 0, 0);
				self->bShowRecordIndicator = FALSE;
			}
			return 0;
		}
	}
	return TyTableType.tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static void
TyTable_dealloc(TyTableObject* self)
{
	Py_XDECREF(self->pyColumns);
	Py_TYPE(self)->tp_base->tp_dealloc((TyWidgetObject *)self);
}

static PyMemberDef TyTable_members[] = {
	{ "columns", T_OBJECT_EX, offsetof(TyTableObject, pyColumns), READONLY, "List of TableColumns" },
	{ NULL }  /* Sentinel */
};

static PyMethodDef TyTable_methods[] = {
	{ "add_column", (PyCFunction)TyTable_add_column, METH_VARARGS | METH_KEYWORDS, "Add a column" },
	{ "refresh", (PyCFunction)TyTable_refresh, METH_NOARGS, "Pull fresh data" },
	{ "refresh_cell", (PyCFunction)TyTable_refresh_cell, METH_VARARGS, "Pull fresh data one cell" },
	{ "refresh_row_pointer", (PyCFunction)TyTable_refresh_row_pointer, METH_NOARGS, "Update highlight of selected row" },
	{ NULL }  /* Sentinel */
};

PyTypeObject TyTableType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Table",               /* tp_name */
	sizeof(TyTableObject),     /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyTable_dealloc, /* tp_dealloc */
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
	TyTable_setattro,          /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT,        /* tp_flags */
	"Table widget objects ('List View' on Windows)", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyTable_methods,           /* tp_methods */
	TyTable_members,           /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyTable_init,    /* tp_init */
	0,                         /* tp_alloc */
	TyTable_new,               /* tp_new */
};

// https://zairon.wordpress.com/2007/11/06/editable-listview-control/
// http://cboard.cprogramming.com/windows-programming/122733-%5Bc%5D-editing-subitems-listview-win32-api.html

LRESULT CALLBACK TyTableProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RECT    rc, rcOld;
	RECT   *pRc;
	POINT   pt;
	BOOL	bOldstate;
	TyTableObject* self = (TyTableObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg)
	{
	case WM_NOTIFY:
		//return TyTable_Notify((TyWidgetObject*)GetWindowLongPtr(((LPNMHDR)lParam)->hwndFrom, GWLP_USERDATA), (LPNMHDR)lParam);
	{
		LPNMHDR nmhdr = (LPNMHDR)lParam;
		int iIndex;
		if (nmhdr->code == LVN_ITEMCHANGED) {
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)nmhdr;
			if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uNewState & LVNI_SELECTED))
			{
				//if (!TyTable_SelectionChanged(self, pNMListView->iItem)) {
				if (!TyDynaset_SetRow(self->pyDynaset, (Py_ssize_t)pNMListView->iItem))
					ShowPythonError();
			}

			/*
			//LVIS_SELECTED
			iIndex = (int)SendMessage(self->hWin, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
			if (iIndex == -1)
			return FALSE;
			ListView_SetItemState(self->hWin, iIndex, 0, LVIS_SELECTED | LVIS_FOCUSED);

			ShowInts(L"X", iIndex, 0);
			//TyTable_SelectionChanged(self, iIndex);
			*/
		}
		else if (nmhdr->code == LVN_ITEMCHANGING) {
			if (self->pyDynaset->bFrozen)
				return FALSE;
		}
		else if (nmhdr->code == NM_DBLCLK) {
			if (self->pyDynaset->bFrozen)
				return 0;
			else {
				iIndex = (int)SendMessage(self->hWin, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
				ShowInts(L"\n Doubl klicked I", iIndex, 0);
				if (iIndex != -1)
				{
					ShowInts(L"\n Doubl klick I", iIndex, 0);
					return TRUE;
				}
			}
		}

		else if (nmhdr->code == NM_CLICK) {
			if (self->pyDynaset->bFrozen)
				return 0;
			/*else {
				iIndex = (int)SendMessage(self->hWin, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
				if (iIndex != -1)
				{
				ShowInts(L"\n sing klick I", iIndex, 0);
				}
				return TRUE;
				}*/
		}

		else if (nmhdr->code == NM_CUSTOMDRAW) { // https://msdn.microsoft.com/en-us/library/windows/desktop/ff919573(v=vs.85).aspx
			LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)nmhdr;// lParam;

			switch (lplvcd->nmcd.dwDrawStage) {
			case CDDS_PREPAINT:
				return CDRF_NOTIFYITEMDRAW;

			case CDDS_ITEMPREPAINT:
				return CDRF_NOTIFYSUBITEMDRAW;

			case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
				/*lplvcd->clrTextBk = GetBkColorForSubItem(lplvcd->nmcd.dwItemSpec,
														 lplvcd->nmcd.lItemlParam,
														 lplvcd->iSubItem));*/
				if (lplvcd->iSubItem == 0)
					lplvcd->clrTextBk = RGB(230, 230, 230); // gray GetSysColor(COLOR_SCROLLBAR);//
				else if (lplvcd->iSubItem == 1)
					lplvcd->clrTextBk = RGB(255, 255, 255);

				return CDRF_NEWFONT;
			}
		}
		/*else  if (nmhdr->code == LVN_ENDLABELEDIT){
			LVITEM LvItem;
			LV_DISPINFO* dispinfo = (LV_DISPINFO*)nmhdr;
			LvItem.iItem = dispinfo->item.iItem;
			LvItem.iSubItem = dispinfo->item.iSubItem;
			LvItem.pszText = dispinfo->item.pszText;
			SendMessage(self->hWin, LVM_SETITEMTEXT, (WPARAM)LvItem.iItem, (LPARAM)&LvItem);
			}*/
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		//if (hEdit != NULL){ SendMessage(hEdit, WM_KILLFOCUS, 0, 0); };
		LVHITTESTINFO itemclicked;
		long x, y;
		x = (long)LOWORD(lParam);
		y = (long)HIWORD(lParam);
		itemclicked.pt.x = x;
		itemclicked.pt.y = y;
		int lResult = ListView_SubItemHitTest(self->hWin, &itemclicked);
		if (lResult != -1) {
			if (!TyDynaset_SetRow(self->pyDynaset, (Py_ssize_t)itemclicked.iItem)) {
				ShowPythonError();
				return 0;
			}
			if (self->pyDynaset->bLocked)
				break;

			//ShowInts(L"Subitem C", itemclicked.iItem, itemclicked.iSubItem);

			TyTableColumnObject* pyTableColumn = (TyTableColumnObject*)PyList_GetItem(self->pyColumns, (Py_ssize_t)itemclicked.iSubItem - 1);
			if (pyTableColumn->pyWidget) {
				//self->iFocusRow = itemclicked.iItem;
				//self->iFocusColumn = itemclicked.iSubItem - 1;

				if (!TyEntry_RepresentCell(pyTableColumn->pyWidget))
					ShowPythonError();
			}
		}
		//return 0;
		break;
	}
	/*default:
		break;*/
	}
	return CallWindowProcW(self->fnOldWinProcedure, hwnd, msg, wParam, lParam);
}


/* TableColumn -----------------------------------------------------------------------*/

static PyObject *
TyTableColumn_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	TyTableColumnObject* self = (TyTableColumnObject *)TyTableColumnType.tp_alloc(type, 0);

	if (self != NULL) {
		self->pyTable = NULL;
		self->pyDynasetColumn = NULL;
		self->pyType = NULL;
		self->pyFormat = NULL;
		self->pyWidget = NULL;
		self->iIndex = -1;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static void
TyTableColumn_dealloc(TyTableColumnObject* self)
{
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMemberDef TyTableColumn_members[] = {
	{ "data", T_OBJECT_EX, offsetof(TyTableColumnObject, pyDynasetColumn), READONLY, "Bound DynasetColumn" },
	{ "type", T_OBJECT_EX, offsetof(TyTableColumnObject, pyType), READONLY, "Data type" },
	{ "format", T_OBJECT_EX, offsetof(TyTableColumnObject, pyFormat), READONLY, "Display format" },
	{ "widget", T_OBJECT_EX, offsetof(TyTableColumnObject, pyWidget), READONLY, "Edit widget" },
	{ NULL }  /* Sentinel */
};

static PyMethodDef TyTableColumn_methods[] = {
	{ NULL }  /* Sentinel */
};

PyTypeObject TyTableColumnType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"pylax.TableColumn",       /* tp_name */
	sizeof(TyTableColumnObject), /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyTableColumn_dealloc, /* tp_dealloc */
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
	"Colunm for a Table widget", /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyTableColumn_methods,     /* tp_methods */
	TyTableColumn_members,     /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	0,                         /* tp_init */
	0,                         /* tp_alloc */
	TyTableColumn_new,         /* tp_new */
};