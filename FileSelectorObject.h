#ifndef Ty_FILESELECTOROBJECT_H
#define Ty_FILESELECTOROBJECT_H

typedef struct _TyFileSelectorObject
{
	PyObject_HEAD
	TCHAR* szCaption;
	TCHAR* szInitialPath;
}
TyFileSelectorObject;

extern PyTypeObject TyFileSelectorType;

#endif /* Ty_FILESELECTOROBJECT_H */