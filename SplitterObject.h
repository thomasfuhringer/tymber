// SplitterObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_SPLITTEROBJECT_H
#define Ty_SPLITTEROBJECT_H

typedef struct _TySplitterObject
{
	TyWidgetObject_HEAD
		PyObject* pyChildren;
	TyBoxObject* pyBox1;
	TyBoxObject* pyBox2;
	BOOL bVertical;
	int iPosition;
	int iSpacing;
}
TySplitterObject;

extern PyTypeObject TySplitterType;

#endif /* Ty_SPLITTEROBJECT_H */