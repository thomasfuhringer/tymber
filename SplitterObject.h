// SplitterObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_SPLITTEROBJECT_H
#define Ty_SPLITTEROBJECT_H

typedef struct _TySplitterObject
{
	TyBoxObject_HEAD
		TyBoxObject* pyBox1;
	TyBoxObject* pyBox2;
	BOOL bVertical;
	int iPosition;
	int iSpacing;
	BOOL bSplitterMoving;
}
TySplitterObject;

extern PyTypeObject TySplitterType;

#endif /* Ty_SPLITTEROBJECT_H */