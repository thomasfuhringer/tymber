// WidgetObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_WIDGETOBJECT_H
#define Ty_WIDGETOBJECT_H

#define TyWIDGET_CENTER      (CW_USEDEFAULT-1)
#define TyWINDOWBKGCOLOR     COLOR_MENU // COLOR_WINDOW

#define TyWidgetObject_HEAD  \
		PyObject_HEAD \
		HWND hWin; \
		RECT rc; \
		HWND hwndParent; \
		PyObject* pyKey; \
		TyWidgetObject* pyParent; \
		BOOL bVisible; \
		WNDPROC fnOldWinProcedure; \
		BOOL bReadOnly; \
	    PyObject* pyCaption; \
		PyObject* pyData; \
		PyTypeObject* pyDataType; \
		PyObject* pyAlignHorizontal; \
		PyObject* pyAlignVertical; \
		PyObject* pyFormat; \
		PyObject* pyFormatEdit; \
		PyObject* pyTabNext; \
		PyObject* pyVerifyCB; \
		PyObject* pyOnChangedCB; \
		TyWindowObject* pyWindow;

#define TY_FRAME WS_EX_STATICEDGE

typedef struct _TyWindowObject TyWindowObject;
typedef struct _TyWidgetObject TyWidgetObject;

typedef struct _TyWidgetObject
{
	TyWidgetObject_HEAD
}
TyWidgetObject;

extern PyTypeObject TyWidgetType;

BOOL TyWidget_SetCaption(TyWidgetObject* self, PyObject* pyText);
BOOL TyWidget_SetData(TyWidgetObject* self, PyObject* pyData);

PyObject* TyFormatData(PyObject* pyData, PyObject* pyFormat);
PyObject* TyParseString(LPCSTR strText, PyTypeObject* pyDataType, PyObject* pyFormat);
BOOL CALLBACK TyWidgetSizeEnumProc(HWND hwndChild, LPARAM lParam);
BOOL TransformRectToAbs(_In_ RECT rcRel, _In_ RECT rcParent, _Out_ PRECT rcAbs);
BOOL TyWidget_CalculateRect(TyWidgetObject* self, _Out_ PRECT rcAbs);
BOOL TyWidget_MoveWindow(TyWidgetObject* self);
BOOL TyWidget_FocusIn(TyWidgetObject* self);
BOOL TyWidget_FocusOut(TyWidgetObject* self);
BOOL TyWidget_FocusTo(TyWidgetObject* self);

typedef enum { ALIGN_NONE, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_TOP, ALIGN_BOTTOM, ALIGN_BLOCK } TyAlignEnum;

#endif