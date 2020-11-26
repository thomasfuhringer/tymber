// MdiAreaObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_MDIAREAOBJECT_H
#define Ty_MDIAREAOBJECT_H

typedef struct _TyMdiAreaObject
{
	TyWidgetObject_HEAD
	BOOL bMaximized;
		PyObject* pyChildren;
		PyObject* pyOnActivatedCB;
}
TyMdiAreaObject;

extern PyTypeObject TyMdiAreaType;

#endif
