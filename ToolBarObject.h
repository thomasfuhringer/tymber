// ToolBarObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_TOOLBAROBJECT_H
#define Ty_TOOLBAROBJECT_H

#define TyTOOLBAR_MAX_BUTTONS     20
#define TyTOOLBAR_IMGLST           0

typedef struct _TyToolBarObject
{
	PyObject_HEAD
		HWND hWin;
	int iButtons;
	HIMAGELIST hImageList;
}
TyToolBarObject;

extern PyTypeObject TyToolBarType;

BOOL TyToolBarType_Init();
#endif