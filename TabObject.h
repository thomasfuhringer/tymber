// TabObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_TABOBJECT_H
#define Ty_TABOBJECT_H

typedef struct _TyTabPageObject TyTabPageObject;
typedef struct _TyTabObject
{
	TyWidgetObject_HEAD
		PyObject* pyChildren;
	TyTabPageObject* pyCurrentPage;
}
TyTabObject;

extern PyTypeObject TyTabType;


typedef struct _TyTabPageObject
{
	TyBoxObject_HEAD
		int iIndex;
}
TyTabPageObject;

extern PyTypeObject TyTabPageType;

#endif /* !Ty_TABOBJECT_H */