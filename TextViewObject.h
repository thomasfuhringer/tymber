// TextViewObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_TEXTVIEWOBJECT_H
#define Ty_TEXTVIEWOBJECT_H

typedef struct _TyTextViewObject
{
	TyWidgetObject_HEAD
		PyObject* pyOnClickLinkCB;
}
TyTextViewObject;

extern PyTypeObject TyTextViewType;

#endif