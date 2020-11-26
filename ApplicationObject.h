// ApplicationObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_APPLICATIONOBJECT_H
#define Ty_APPLICATIONOBJECT_H

typedef struct _TyApplicationObject
{
	PyObject_HEAD
		PyObject* pyWindow; // main window
	PyObject* pyMenu; // main menu
	PyObject* pyChildren;
}
TyApplicationObject;

extern PyTypeObject TyApplicationType;

#endif