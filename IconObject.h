#ifndef Ty_ICONOBJECT_H
#define Ty_ICONOBJECT_H

typedef struct _TyIconObject
{
	PyObject_HEAD
		HANDLE hWin;
	HICON   hIconLarge;
}
TyIconObject;

extern PyTypeObject TyIconType;

#endif /* Ty_ICONOBJECT_H */