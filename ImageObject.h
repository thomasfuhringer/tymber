#ifndef Ty_IMAGEOBJECT_H
#define Ty_IMAGEOBJECT_H

typedef struct _TyImageObject
{
	PyObject_HEAD
		HANDLE hWin;
	PyObject* pyImageFormat;
	HICON   hIconLarge;
}
TyImageObject;

extern PyTypeObject TyImageType;

typedef enum { TyIMAGEFORMAT_ICO, TyIMAGEFORMAT_BMP, TyIMAGEFORMAT_JPG } TyImageFormatEnum;

#endif /* Ty_IMAGEOBJECT_H */