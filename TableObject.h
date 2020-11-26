// TableObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_TABLEOBJECT_H
#define Ty_TABLEOBJECT_H

typedef struct _TyTableObject
{
	TyWidgetObject_HEAD
		PyObject* pyColumns; // PyList
	Py_ssize_t nColumns;
	int iAutoSizeColumn;
	BOOL bShowRecordIndicator;
	//int iFocusRow;
	//int iFocusColumn;
}
TyTableObject;

typedef struct _TyTableColumnObject
{
	PyObject_HEAD
		TyTableObject* pyTable;
	PyObject* pyDynasetColumn;
	int iIndex;
	PyObject* pyType;
	PyObject* pyFormat;
	TyWidgetObject* pyWidget;
}
TyTableColumnObject;

extern PyTypeObject TyTableType;
extern PyTypeObject TyTableColumnType;

BOOL TyTable_SelectionChanged(TyTableObject* self, int iRow);
//int TyTable_Notify(TyTableObject* self, LPNMHDR nmhdr);

#endif /* !Ty_TABLEOBJECT_H */

