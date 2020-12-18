// WindowObject.c  | Tymber © 2020 by Thomas Führinger
#include "Tymber.h"
#include "Resource.h"

// Forward declarations
static PyObject* TyWindow_run(TyWindowObject* self);
static PyObject* TyWindow_close(TyWindowObject* self);

static PyObject*
TyWindow_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyWindowObject* self = (TyWindowObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->cxMin = 320;
		self->cyMin = 240;
		self->rc.left = TyWIDGET_CENTER;
		self->rc.top = TyWIDGET_CENTER;
		self->rc.right = 320;
		self->rc.bottom = 240;
		self->pyIcon = NULL;
		self->bModal = FALSE;
		self->pyBeforeCloseCB = NULL;
		self->pyOnCloseCB = NULL;
		self->hMdiArea = NULL;
		self->iStatusBarHeight = 0;
		self->iToolBarHeight = 0;
		self->pyFocusWidget = NULL;
		self->pyOnFocusChangeCB = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyWindow_init(TyWindowObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "caption", "left", "top", "width", "height", "visible", NULL };
	PyObject* pyCaption = NULL;
	BOOL bVisible = TRUE;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|Oiiiip", kwlist,
		&pyCaption,
		&self->rc.left,
		&self->rc.top,
		&self->rc.right,
		&self->rc.bottom,
		&bVisible))
		return -1;

	HWND hwndParent = GetDesktopWindow();
	if (g->pyApp != Py_None && g->pyApp->pyWindow)
		hwndParent = ((TyWindowObject*)g->pyApp->pyWindow)->hWin;

	RECT rect;
	TyWidget_CalculateRect(self, &rect);

	if ((self->hWin = CreateWindowExW(WS_EX_CONTROLPARENT, L"TyWindowClass", L"<new>",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_POPUP | (bVisible ? WS_VISIBLE : 0),
		rect.left, rect.top, rect.right, rect.bottom,
		hwndParent, 0, g->hInstance, NULL)) == NULL) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}

	if (pyCaption != NULL) {
		LPCSTR strText = PyUnicode_AsUTF8(pyCaption);
		LPWSTR szText = toW(strText);
		BOOL bSuccessful = SendMessage(self->hWin, WM_SETTEXT, (WPARAM)0, (LPARAM)szText);
		PyMem_RawFree(szText);
		if (!bSuccessful) {
			PyErr_SetFromWindowsErr(0);
			return -1;
		}
	}

	Py_INCREF(Py_None);
	Py_INCREF(Py_None);
	self->pyChildren = PyDict_New();
	self->pyToolBar = Py_None;
	self->pyStatusBar = Py_None;

	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return 0;
}

static int
TyWindow_setattro(TyWindowObject* self, PyObject* pyAttributeName, PyObject* pyValue)
{
	if (PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "visible") == 0) {
			ShowWindow(self->hWin, PyObject_IsTrue(pyValue) ? SW_SHOW : SW_HIDE);
			return  0;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "before_close") == 0) {
			if (PyCallable_Check(pyValue)) {
				Py_XINCREF(pyValue);
				Py_XDECREF(self->pyBeforeCloseCB);
				self->pyBeforeCloseCB = pyValue;
				return 0;
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Assigned object must be callable.");
				return -1;
			}
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "position") == 0) {
			if (!PyTuple_Check(pyValue)) {
				PyErr_SetString(PyExc_TypeError, "Assigned value must be a tuple.");
				return -1;
			}
			if (PyTuple_Size(pyValue) != 4) {
				PyErr_SetString(PyExc_TypeError, "Assigned value must be tuple of four integers.");
				return -1;
			}
			int x = PyLong_AsLong(PyTuple_GetItem(pyValue, 0));
			int y = PyLong_AsLong(PyTuple_GetItem(pyValue, 1));
			int w = PyLong_AsLong(PyTuple_GetItem(pyValue, 2));
			int h = PyLong_AsLong(PyTuple_GetItem(pyValue, 3));
			MoveWindow(self->hWin, x, y, w, h, TRUE);

			return 0;
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "maximized") == 0) {
			if (!PyBool_Check(pyValue)) {
				PyErr_SetString(PyExc_TypeError, "Assigned value must be Bool.");
				return -1;
			}
			if (pyValue == Py_True)
				ShowWindow(self->hWin, SW_MAXIMIZE);
			else
				ShowWindow(self->hWin, SW_SHOWNORMAL);

			return 0;
		}

		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "icon") == 0) {
			if (pyValue == Py_None) {
				SendMessageW(self->hWin, WM_SETICON, ICON_SMALL, 0);
				SendMessageW(self->hWin, WM_SETICON, ICON_BIG, 0);
			}
			else if (!PyObject_TypeCheck(pyValue, &TyIconType)) {
				PyErr_SetString(PyExc_TypeError, "Please assign an 'Icon'!");
				return -1;
			}
			Py_XDECREF(self->pyIcon);
			Py_INCREF(pyValue);
			self->pyIcon = (TyIconObject*)pyValue;
			SendMessageW(self->hWin, WM_SETICON, ICON_SMALL, (LPARAM)self->pyIcon->hWin);
			SendMessageW(self->hWin, WM_SETICON, ICON_BIG, (LPARAM)self->pyIcon->hIconLarge);
			return 0;
		}
	}
	return Py_TYPE(self)->tp_base->tp_setattro((PyObject*)self, pyAttributeName, pyValue);
}

static PyObject* // new ref
TyWindow_getattro(TyWindowObject* self, PyObject* pyAttributeName)
{
	PyObject* pyResult, * pyAttribute;
	pyResult = PyObject_GenericGetAttr((PyObject*)self, pyAttributeName);
	if (pyResult == NULL && PyErr_ExceptionMatches(PyExc_AttributeError) && PyUnicode_Check(pyAttributeName)) {
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "before_close") == 0) {
			PyErr_Clear();
			Py_INCREF(self->pyBeforeCloseCB);
			return self->pyBeforeCloseCB;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "visible") == 0) {
			PyErr_Clear();
			if (GetWindowLong(self->hWin, GWL_STYLE) & WS_VISIBLE)
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "position") == 0) {
			PyErr_Clear();
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(self->hWin, &wp);
			PyObject* pyRectangle = PyTuple_Pack(4, PyLong_FromLong(wp.rcNormalPosition.left), PyLong_FromLong(wp.rcNormalPosition.top), PyLong_FromLong(wp.rcNormalPosition.right - wp.rcNormalPosition.left), PyLong_FromLong(wp.rcNormalPosition.bottom - wp.rcNormalPosition.top));
			/*
			 RECT rcWin;
			 GetWindowRect(self->hWin, &rcWin);
			 MapWindowPoints(HWND_DESKTOP, GetParent(self->hWin), (LPPOINT)&rcWin, 2);
			 PyObject* pyRectangle = PyTuple_Pack(4, PyLong_FromLong(rcWin.left), PyLong_FromLong(rcWin.top), PyLong_FromLong(rcWin.right - rcWin.left), PyLong_FromLong(rcWin.bottom - rcWin.top));
			 */
			return pyRectangle;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "maximized") == 0) {
			PyErr_Clear();
			WINDOWPLACEMENT wp;
			wp.length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(self->hWin, &wp);
			if ((wp.showCmd & SW_MAXIMIZE) == SW_MAXIMIZE)
				Py_RETURN_TRUE;
			else
				Py_RETURN_FALSE;
		}
		if (PyUnicode_CompareWithASCIIString(pyAttributeName, "size") == 0) {
			PyErr_Clear();
			RECT rcClient;
			GetClientRect(self->hWin, &rcClient);
			PyObject* pyRectangle = PyTuple_Pack(2, PyLong_FromLong(rcClient.right), PyLong_FromLong(rcClient.bottom));
			return pyRectangle;
		}
		if (PyDict_Contains(self->pyChildren, pyAttributeName)) {
			PyErr_Clear();
			pyAttribute = PyDict_GetItem(self->pyChildren, pyAttributeName);
			Py_INCREF(pyAttribute);
			return pyAttribute;
		}
	}
	Py_XDECREF(pyResult);
	return TyWindowType.tp_base->tp_getattro((PyObject*)self, pyAttributeName);
}

static void
TyWindow_dealloc(TyWindowObject* self)
{
	//printf("TyWindow_dealloc\n");
	Py_XDECREF(self->pyChildren);
	if (self->hWin) {
		if (!DestroyWindow(self->hWin)) {
			PyErr_SetFromWindowsErr(0);
		}
	}
	Py_XDECREF(self->pyBeforeCloseCB);
	Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMemberDef TyWindow_members[] = {
	{ "children", T_OBJECT, offsetof(TyWindowObject, pyChildren), READONLY, "Child widgets" },
	{ "tool_bar", T_OBJECT, offsetof(TyWindowObject, pyToolBar), READONLY, "ToolBar" },
	{ "status_bar", T_OBJECT, offsetof(TyWindowObject, pyStatusBar), READONLY, "StatusBar" },
	{ "icon", T_OBJECT, offsetof(TyWindowObject, pyIcon), READONLY, "Icon" },
	{ "min_width", T_INT, offsetof(TyWindowObject, cxMin), 0, "Window can not be resized smaller" },
	{ "min_height", T_INT, offsetof(TyWindowObject, cyMin), 0, "Window can not be resized smaller" },
	{ "focus", T_OBJECT, offsetof(TyWindowObject, pyFocusWidget), READONLY, "Widget that has the keyboard focus." },
	{ "before_close", T_OBJECT, offsetof(TyWindowObject, pyBeforeCloseCB), 0, "Callback before closing, if returns False abort closing" },
	{ "on_close", T_OBJECT, offsetof(TyWindowObject, pyOnCloseCB), 0, "Callback on closing" },
	{ "on_focus_change", T_OBJECT, offsetof(TyWindowObject, pyOnFocusChangeCB), 0, "Callback if keyboard focus moves to new widget" },
	{ NULL }
};

static PyMethodDef TyWindow_methods[] = {
	{ "run", (PyCFunction)TyWindow_run, METH_NOARGS, "Run as dialog (modal)." },
	{ "close", (PyCFunction)TyWindow_close, METH_NOARGS, "Hide and return from modal state." },
	{ NULL }
};

PyTypeObject TyWindowType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Window",           /* tp_name */
	sizeof(TyWindowObject),    /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyWindow_dealloc, /* tp_dealloc */
	0,                         /* tp_print */
	0,                         /* tp_getattr */
	0,                         /* tp_setattr */
	0,                         /* tp_reserved */
	0,                         /* tp_repr */
	0,                         /* tp_as_number */
	0,                         /* tp_as_sequence */
	0,                         /* tp_as_mapping */
	0,                         /* tp_hash  */
	0,                         /* tp_call */
	0,                         /* tp_str */
	TyWindow_getattro,         /* tp_getattro */
	TyWindow_setattro,         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Simply a window",         /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyWindow_methods,          /* tp_methods */
	TyWindow_members,          /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyWindow_init,   /* tp_init */
	0,                         /* tp_alloc */
	TyWindow_new,              /* tp_new */
	PyObject_Free,             /* tp_free */
};

static LRESULT CALLBACK
TyWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL
WindowClass_Init()
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = TyWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g->hInstance;
	wc.hIcon = g->hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(TyWINDOWBKGCOLOR + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"TyWindowClass";
	wc.hIconSm = g->hIcon;

	if (!RegisterClassEx(&wc)) {
		PyErr_SetFromWindowsErr(0);
		return FALSE;
	}
	return TRUE;
}

static LPMINMAXINFO lpMMI;

static LRESULT CALLBACK
TyWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PyObject* pyResult = NULL;
	TyWidgetObject* pyWidget;
	WORD wCommandIdentifier;

	TyWindowObject* self = (TyWindowObject*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (self) {
		switch (uMsg)
		{
		case WM_COMMAND:
			wCommandIdentifier = LOWORD(wParam);
			if (wCommandIdentifier >= FIRST_CUSTOM_MENU_ID && wCommandIdentifier <= MAX_CUSTOM_MENU_ID) {
				TyMenuItemObject* pyMenuItem = g->pyaMenuItem[wCommandIdentifier - FIRST_CUSTOM_MENU_ID];
				if (pyMenuItem->pyOnClickCB && (PyObject_CallObject(pyMenuItem->pyOnClickCB, NULL) == NULL)) {
					PyErr_Print();
					return 0;
				}
			}
			switch (HIWORD(wParam))
			{
			case EN_SETFOCUS:
				pyWidget = (TyWidgetObject*)GetWindowLongPtr(lParam, GWLP_USERDATA);
				if (pyWidget && !TyWidget_FocusIn(pyWidget))
					PyErr_Print();
				break;

			case EN_KILLFOCUS:
				pyWidget = (TyWidgetObject*)GetWindowLongPtr(lParam, GWLP_USERDATA);
				//printf("EN_KILLFOCUS %p \n", pyWidget);
				if (pyWidget && !TyWidget_FocusOut(pyWidget))
					PyErr_Print();
			}
			break;

		case WM_GETMINMAXINFO:
			lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = self->cxMin;
			lpMMI->ptMinTrackSize.y = self->cyMin;
			break;

		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED)
			{
				EnumChildWindows(hwnd, TyWidgetSizeEnumProc, (LPARAM)0);
				if (!PyObject_TypeCheck((PyObject*)self, &TyMdiWindowType))
					return 0;
			}

			break;

		case WM_MDIACTIVATE:
			if (hwnd != wParam) {
				PyObject* pyOnActivated = ((TyMdiAreaObject*)((TyMdiWindowObject*)self)->pyParent)->pyOnActivatedCB;
				if (pyOnActivated) {
					PyObject* pyArgs = PyTuple_Pack(1, (PyObject*)self);
					Py_INCREF(self);
					PyObject* pyResult = PyObject_CallObject(pyOnActivated, pyArgs);
					if (pyResult == NULL)
						return FALSE;
					Py_DECREF(pyResult);
				}
			}
			break;

		case WM_QUERYENDSESSION:
		case WM_CLOSE:
			if (self->pyBeforeCloseCB && self->pyBeforeCloseCB != Py_None) {
				PyObject* pyArgs = PyTuple_Pack(1, (PyObject*)self);
				PyObject* r = PyObject_CallObject(self->pyBeforeCloseCB, pyArgs);
				if (r == NULL) {
					PyErr_Print();
					return 0;
				}
				Py_XDECREF(pyArgs);
				BOOL bCloseNot = (r == Py_False);
				Py_XDECREF(r);
				if (bCloseNot)
					return 0;
			}

			if (self->bModal) {
				self->bModal = FALSE;
			}

			if (PyObject_TypeCheck(self, &TyMdiWindowType)) {
				TyMdiAreaObject* pyParent = (TyMdiAreaObject*)((TyMdiWindowObject*)self)->pyParent;
				PyObject* pyChildren = PyObject_GetAttrString(pyParent, "children");
				if (PyDict_Contains(pyChildren, ((TyMdiWindowObject*)self)->pyKey) && PyDict_DelItem(pyChildren, ((TyMdiWindowObject*)self)->pyKey) == -1) {
					PyErr_Print();
					return 0;
				}
				Py_DECREF(pyChildren);

				PyObject* pyOnActivated = pyParent->pyOnActivatedCB;
				if (pyOnActivated) {
					PyObject* pyResult = PyObject_CallFunction(self->pyOnCloseCB, "(O)", Py_None);
					if (pyResult == NULL)
						return FALSE;
					Py_DECREF(pyResult);
				}
			}
			else
				ShowWindow(self->hWin, SW_HIDE);

			if (self->pyOnCloseCB && self->pyOnCloseCB != Py_None) {
				PyObject* pyResult = PyObject_CallFunction(self->pyOnCloseCB, "(O)", self);
				if (pyResult == NULL) {
					PyErr_Print();
					return 0;
				}
				Py_XDECREF(pyResult);
			}
			return 0;

		case WM_DESTROY:
			//printf("WM_DESTROY\n");
			return 0;

			self->hWin = NULL;
			Py_DECREF(self);
			return 0;
		}
	}
	return DefParentProc(hwnd, uMsg, wParam, lParam, self);
}

// Reflect messages to child
LRESULT
DefParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, PyObject* self)
{
	TyWidgetObject* pyWidget;
	switch (uMsg) {
	case WM_NOTIFY:
	{
		NMHDR* nmhdr = (NMHDR*)lParam;
		if (nmhdr->hwndFrom != NULL)
			return SendMessage(nmhdr->hwndFrom, uMsg + OCM__BASE, wParam, lParam);
		break;
	}

	// Control's HWND in LPARAM
	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case EN_SETFOCUS:
			pyWidget = (TyWidgetObject*)GetWindowLongPtr(lParam, GWLP_USERDATA);
			if (pyWidget && !TyWidget_FocusIn(pyWidget))
				PyErr_Print();
			break;

		case EN_KILLFOCUS:
			pyWidget = (TyWidgetObject*)GetWindowLongPtr(lParam, GWLP_USERDATA);
			//printf("EN_KILLFOCUS %p \n", pyWidget);
			if (pyWidget && !TyWidget_FocusOut(pyWidget))
				PyErr_Print();
		}
		//break;
	case WM_CTLCOLORBTN:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
	case WM_VKEYTOITEM:
	case WM_CHARTOITEM:
		if (lParam != 0)
			return SendMessage((HWND)lParam, uMsg + OCM__BASE, wParam, lParam);
		break;
	}
	if ((uMsg >= OCM__BASE))
		uMsg -= OCM__BASE;

	// pick the right DefXxxProcW
	if (self != NULL) {
		if (PyObject_TypeCheck(self, &TyMdiWindowType))
			return DefMDIChildProcW(hwnd, uMsg, wParam, lParam);
		if (PyObject_TypeCheck(self, &TyWindowType) && ((TyWindowObject*)self)->hMdiArea) {
			return DefFrameProcW(hwnd, ((TyWindowObject*)self)->hMdiArea, uMsg, wParam, lParam);
		}
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static PyObject*
TyWindow_run(TyWindowObject* self)
{
	BOOL r = ShowWindow(self->hWin, SW_SHOW);

	self->bModal = TRUE;
	MSG msg;
	//HACCEL hAccelTable = LoadAccelerators(g->hInstance, MAKEINTRESOURCE(IDS_LOGO));
	while (GetMessage(&msg, NULL, 0, 0) > 0 && self->bModal)
	{
		//if (!IsDialogMessage(self->hWin, &msg)) { //&& !TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		//}
	}

	Py_RETURN_NONE;
}

static PyObject*
TyWindow_close(TyWindowObject* self)
{
	SendMessage(self->hWin, WM_CLOSE, (WPARAM)0, (LPARAM)0);
	Py_RETURN_NONE;
}