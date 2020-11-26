// BoxObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_BOXOBJECT_H
#define Ty_BOXOBJECT_H

typedef struct _TyBoxObject
{
	TyWidgetObject_HEAD
		PyObject* pyChildren;
}
TyBoxObject;

extern PyTypeObject TyBoxType;

BOOL TyBoxType_Init();
#endif