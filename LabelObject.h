#ifndef Ty_LABELOBJECT_H
#define Ty_LABELOBJECT_H

typedef struct _TyLabelObject
{
	TyWidgetObject_HEAD
		COLORREF iTextColor;
}
TyLabelObject;

extern PyTypeObject TyLabelType;

#endif /* Ty_LABELOBJECT_H */