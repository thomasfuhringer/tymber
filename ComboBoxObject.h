#ifndef Ty_COMBOBOXOBJECT_H
#define Ty_COMBOBOXOBJECT_H

typedef struct _TyComboBoxObject
{
	TyWidgetObject_HEAD
		PyObject* pyItems; // PyList
	BOOL bNoneSelectable;
	PyObject* pyOnLeaveCB;
	PyObject* pyOnKeyCB;
}
TyComboBoxObject;

extern PyTypeObject TyComboBoxType;

BOOL TyComboBox_Selected(TyComboBoxObject* self, int iItem);
BOOL TyComboBox_Entering(TyComboBoxObject* self);
BOOL TyComboBox_Changed(TyComboBoxObject* self);
BOOL TyComboBox_Left(TyComboBoxObject* self);

#endif