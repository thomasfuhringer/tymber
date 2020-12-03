// Tymber.h  |  Tymber © 2020 by Thomas Führinger <thomasfuhringer@live.com>

#ifndef TYMBER_H
#define TYMBER_H

// Ignore unreferenced parameters, since they are very common
// when implementing callbacks.
#pragma warning(disable : 4100)

#ifndef WINVER                  // Minimum platform is Windows XP
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT            // Minimum platform is Windows XP
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS          // Minimum platform is Windows XP
#define _WIN32_WINDOWS 0x0501
#endif

#pragma once
#pragma execution_character_set("utf-8")

#pragma comment(lib,"comctl32.lib")

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#define IDI_LOGO                111
#define IDB_PLACEHOLDER         112
#define IDC_TYBUTTON	       7101
#define IDC_TYLABEL  	       7102
#define IDC_TYCOMBOBOX 	       7103
#define IDC_TYLISTVIEW         7104
#define IDC_TYTABLECOL 	       7105
#define IDC_TYCANVAS  	       7106
#define IDC_TYENTRY  	       7107
#define IDC_TYIMAGEVIEW	       7108
#define IDC_TYBOX   	       7109
#define IDC_TYTOOLBAR  	       7110
#define IDC_TYTAB   	       7111
#define IDC_TYMDIAREA  	       7112
#define IDC_TYSTATUSBAR	       7113 

#define IDM_WINDOWMENUPOS      3
#define FIRST_CUSTOM_MENU_ID   700
#define MAX_CUSTOM_MENU_ID     770

#include <Python.h>
#include <datetime.h>

// C RunTime Header Files
#include <stdlib.h>
#include <tchar.h>
#include <Strsafe.h>

// Windows Header Files
#include <windows.h>
#include <Commctrl.h>
#include <OleCtl.h>
#include <Shlwapi.h>
#include <structmember.h>

// Tymber Classes
#include "Version.h"
#include "Utilities.h"
#include "ApplicationObject.h"
#include "WindowObject.h"
#include "MdiWindowObject.h"
#include "WidgetObject.h"
#include "ButtonObject.h"
#include "LabelObject.h"
#include "EntryObject.h"
#include "MdiAreaObject.h"
#include "CanvasObject.h"
#include "MenuObject.h"
#include "StatusBarObject.h"
#include "ToolBarObject.h"
#include "ImageObject.h"
#include "MenuObject.h"
#include "BoxObject.h"
#include "SplitterObject.h"
#include "TabObject.h"
#include "ImageViewObject.h"
#include "ComboBoxObject.h"
#include "ListViewObject.h"

typedef struct _TyMenuItemObject TyMenuItemObject;
typedef struct _TyApplicationObject TyApplicationObject;
typedef struct _TyGlobals
{
	HINSTANCE hInstance;
	TCHAR* szAppName;
	TyApplicationObject* pyApp;
	HMODULE hDLLcomctl32;
	HICON hIcon;
	HICON hIconMdi;
	HMENU hMenu;
	TyMenuItemObject* pyaMenuItem[MAX_CUSTOM_MENU_ID - FIRST_CUSTOM_MENU_ID + 1]; // map identifiers to pyMenuItems
	HCURSOR hCursorWestEast;
	HCURSOR hCursorNorthSouth;
	HGDIOBJ hfDefaultFont;
	HBRUSH hBkgBrush;
	HANDLE hHeap;
	TCHAR* szWindowClass;
	PyObject* pyModule;
	PyObject* pyCopyFunction;
	PyObject* pyEnumType;
	PyObject* pyStdDateTimeFormat;
	PyObject* pyAlignEnum;
	PyObject* pyImageFormatEnum;
	PyObject* pyStockIconEnum;
	PyObject* pyKeyEnum;
}
TyGlobals;

extern TyGlobals* g;

#endif