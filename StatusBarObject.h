// StatusBar.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_STATUSBAROBJECT_H
#define Ty_STATUSBAROBJECT_H

#define TySTATUSBAR_MAX_PARTS     8

typedef struct _TyStatusBarObject
{
	PyObject_HEAD
		HWND hWin;
	RECT rc;
	HWND hwndParent;
	int iParts;
	int iaRightEdges[TySTATUSBAR_MAX_PARTS - 1];
	int iaParts[TySTATUSBAR_MAX_PARTS];
}
TyStatusBarObject;

extern PyTypeObject TyStatusBarType;

BOOL TyStatusBarType_Init();
#endif