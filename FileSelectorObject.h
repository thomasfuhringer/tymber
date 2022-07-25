#ifndef Ty_FILESELECTOROBJECT_H
#define Ty_FILESELECTOROBJECT_H

typedef struct _TyFileSelectorObject
{
	PyObject_HEAD
	TCHAR* szCaption;
	TCHAR* szInitialPath;
	TCHAR* szExtension;
	TCHAR* szName;
	BOOL bSave;
}
TyFileSelectorObject;

extern PyTypeObject TyFileSelectorType;

#endif