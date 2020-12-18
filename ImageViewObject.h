#ifndef Ty_IMAGEVIEWOBJECT_H
#define Ty_IMAGEVIEWOBJECT_H

typedef struct _TyImageViewObject
{
	TyWidgetObject_HEAD
	BOOL bStretch;
	BOOL bFill;
	PyObject* pyOnLMouseDownCB;
	PyObject* pyOnRMouseDownCB;
	PyObject* pyOnMouseWheelCB;
}
TyImageViewObject;

extern PyTypeObject TyImageViewType;

BOOL TyImageView_SetData(TyImageViewObject* self, PyObject* pyData);

#endif /* Ty_IMAGEVIEWOBJECT_H */

