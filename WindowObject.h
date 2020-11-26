#ifndef Ty_WINDOWOBJECT_H
#define Ty_WINDOWOBJECT_H

#define TyWIDGET_CENTER      (CW_USEDEFAULT-1)
#define TyWINDOWBKGCOLOR     COLOR_MENU // COLOR_WINDOW

#define TyWindowObject_HEAD  \
		PyObject_HEAD \
		HWND hWin; \
		RECT rc; \
		HWND hwndParent; \
		PyObject* pyKey; \
		int cxMin; \
		int cyMin; \
		PyObject* pyChildren; \
		PyObject* pyCaption; \
		PyObject* pyFocusWidget; \
		PyObject* pyOnFocusChangeCB; \
		PyObject* pyToolBar; \
		int iToolBarHeight; \
		PyObject* pyBeforeCloseCB; \
		PyObject* pyOnCloseCB;

typedef struct _TyImageObject TyImageObject;
typedef struct _TyWindowObject
{
	TyWindowObject_HEAD
		int iStatusBarHeight;
	PyObject* pyStatusBar;
	BOOL bModal;
	BOOL bOkPressed;
	HWND hMdiArea; // necessary for DefFrameProcW()
	TyImageObject* pyIcon;
}
TyWindowObject;

extern PyTypeObject TyWindowType;
BOOL WindowClass_Init();
LRESULT DefParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, PyObject* self);

#endif