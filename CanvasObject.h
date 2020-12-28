// CanvasObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_CANVASOBJECT_H
#define Ty_CANVASOBJECT_H

#define MAX_PIXBUFFERS 2

typedef struct _TyCanvasObject
{
	TyWidgetObject_HEAD
		RECT rcClient;
	int iActiveBuffer;
	int iLiveBuffer;
	HDC hDC[MAX_PIXBUFFERS];
	HBITMAP hBM[MAX_PIXBUFFERS];
	Graphics pGraphics[MAX_PIXBUFFERS];
	GpFontFamily* pFontFamily;
	GpFont* pFont;
	GpStringFormat* pStringFormat;
	GpPen* pPen;
	GpBrush* pBrush;
	PyObject* pyOnResizeCB;
	PyObject* pyOnLMouseUpCB;
	PyObject* pyOnLMouseDownCB;
	PyObject* pyOnMouseWheelCB;
	PyObject* pyOnMouseMoveCB;
}
TyCanvasObject;

extern PyTypeObject TyCanvasType;

BOOL TyCanvasType_Init();

#endif /* Ty_CANVASOBJECT_H */