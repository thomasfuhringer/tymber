// ListViewObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_LISTVIEWOBJECT_H
#define Ty_LISTVIEWOBJECT_H

typedef struct _TyListViewObject
{
	TyWidgetObject_HEAD
		PyListObject* pyColumns; // PyList
	Py_ssize_t nSelectedRow;
	int iAutoSizeColumn;
	PyObject* pyOnRowChangedCB;
	PyObject* pyOnSelectionChangedCB;
	PyObject* pyOnDoubleClickCB;
	//BOOL bShowRecordIndicator;
	//int iFocusRow;
	//int iFocusColumn;
}
TyListViewObject;
extern PyTypeObject TyListViewType;

/*
typedef struct _TyTableColumnObject
{
	PyObject_HEAD
		TyListViewObject* pyTable;
	PyObject* pyDynasetColumn;
	int iIndex;
	PyObject* pyType;
	PyObject* pyFormat;
	TyWidgetObject* pyWidget;
}
TyTableColumnObject;

extern PyTypeObject TyTableColumnType;
*/
BOOL TyListView_SelectionChanged(TyListViewObject* self, int iRow);
//int TyListView_Notify(TyListViewObject* self, LPNMHDR nmhdr);

#endif