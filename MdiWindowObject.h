#ifndef Ty_MDIWINDOWOBJECT_H
#define Ty_MDIWINDOWOBJECT_H

typedef struct _TyMdiWindowObject
{
	TyWindowObject_HEAD
		PyObject* pyParent;
	PyObject* pyStatusBar;
	TyIconObject* pyIcon;
}
TyMdiWindowObject;

extern PyTypeObject TyMdiWindowType;

#endif