// CheckBoxObject.h  | Tymber © 2022 by Thomas Führinger
#ifndef Ty_CHECKBOXOBJECT_H
#define Ty_CHECKBOXOBJECT_H

typedef struct _TyCheckBoxObject
{
	TyWidgetObject_HEAD
		PyObject* pyOnClickCB;
}
TyCheckBoxObject;

extern PyTypeObject TyCheckBoxType;

BOOL TyCheckBox_OnClick(TyCheckBoxObject* self);

#endif