// CanvasObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_CANVASOBJECT_H
#define Ty_CANVASOBJECT_H

typedef struct _TyCanvasObject
{
	TyWidgetObject_HEAD
		HDC hDC;
	HBRUSH hBrush;
	HPEN hPen;
	PyObject* pyOnPaintCB;
}
TyCanvasObject;

extern PyTypeObject TyCanvasType;

BOOL TyCanvasType_Init();

#endif /* Ty_CANVASOBJECT_H */