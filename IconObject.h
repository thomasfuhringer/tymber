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

typedef enum { TyIconFORMAT_ICO, TyIconFORMAT_BMP, TyIconFORMAT_JPG } TyIconFormatEnum;

#endif /* Ty_ICONOBJECT_H */