// BoxObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_BOXOBJECT_H
#define Ty_BOXOBJECT_H


#define TyBoxObject_HEAD  \
		TyWidgetObject_HEAD \
		PyObject* pyChildren;

typedef struct _TyBoxObject
{
	TyBoxObject_HEAD
}
TyBoxObject;

extern PyTypeObject TyBoxType;

BOOL TyBoxType_Init();
#endif