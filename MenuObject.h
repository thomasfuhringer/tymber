// MenuObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_MENUOBJECT_H
#define Ty_MENUOBJECT_H

typedef struct _TyMenuObject
{
	PyObject_HEAD
		HMENU hWin;
	PyObject* pyParent;
	PyObject* pyKey;
	PyObject* pyCaption;
	PyObject* pyItems;
	PyObject* pyIcon;
}
TyMenuObject;

extern PyTypeObject TyMenuType;


typedef struct _TyIconObject TyIconObject;
typedef struct _TyToolBarObject TyToolBarObject;
typedef struct _TyMenuItemObject
{
	PyObject_HEAD
		UINT_PTR iIdentifier;
	TyMenuObject* pyParent;
	PyObject* pyKey;
	PyObject* pyCaption;
	PyObject* pyOnClickCB;
	TyIconObject* pyIcon;
	TyToolBarObject* pyToolBar;
}
TyMenuItemObject;

extern PyTypeObject TyMenuItemType;

void TyMenuItems_DeleteAll();

#endif

