// Module.c  | Tymber © 2020 by Thomas Führinger
#include <Tymber.h>
#include "Version.h"

TyGlobals* g;
static ULONG_PTR gdiplusToken;

BOOL
MessageU(const LPCSTR strMessage, const LPCSTR strTitle, UINT uType)
{
	LPWSTR szMessage = toW(strMessage);
	LPWSTR szTitle = toW(strTitle);
	int result = MessageBox(0, szMessage, szTitle, uType);
	PyMem_RawFree(szMessage);
	PyMem_RawFree(szTitle);
	return result;
}

PyObject*
Tymber_message(PyObject* self, PyObject* args)
{
	const char* strMessage, * strTitle = NULL;

	if (!PyArg_ParseTuple(args, "s|s", &strMessage, &strTitle))
		return NULL;

	if (strTitle == NULL)
		strTitle = "Information";

	MessageU(strMessage, strTitle, 0);
	Py_RETURN_NONE;
}

#define MAX_SETTING_SIZE     255
PyObject*
Tymber_settings_set(PyObject* self, PyObject* args)
{
	const char* strKey1, * strKey2, * strSubKey;
	PyObject* pyValue;

	if (!PyArg_ParseTuple(args, "sssO",
		&strKey1,
		&strKey2,
		&strSubKey,
		&pyValue))
		return NULL;

	if (!PyBytes_Check(pyValue)) {
		PyErr_Format(PyExc_TypeError, "Argument 3 must be a Bytes object, not '%.200s'.", pyValue->ob_type->tp_name);
		return NULL;
	}

	HKEY hKey;
	LPWSTR szStr;
	TCHAR szKey[256];
	lstrcpy(szKey, L"SOFTWARE\\");
	szStr = toW(strKey1);
	lstrcat(szKey, szStr);
	PyMem_RawFree(szStr);
	lstrcat(szKey, L"\\");
	szStr = toW(strKey2);
	lstrcat(szKey, szStr);
	PyMem_RawFree(szStr);

	size_t nSize = PyBytes_Size(pyValue);
	if (nSize > MAX_SETTING_SIZE) {
		PyErr_SetString(PyExc_RuntimeError, "Size of data for Setting may not exceed %d.", MAX_SETTING_SIZE);
		return NULL;
	}

	if (RegCreateKeyEx(HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}

	szStr = toW(strSubKey);
	if (RegSetValueEx(hKey, szStr, 0, REG_BINARY, (PBYTE)PyBytes_AsString(pyValue), nSize) != ERROR_SUCCESS)
	{
		PyErr_SetFromWindowsErr(0);
		RegCloseKey(hKey);
		return NULL;
	}
	PyMem_RawFree(szStr);

	RegCloseKey(hKey);
	Py_RETURN_NONE;
}

PyObject*
Tymber_settings_get(PyObject* self, PyObject* args)
{
	const char* strKey1, * strKey2, * strSubKey;
	PyObject* pyValue = NULL;

	if (!PyArg_ParseTuple(args, "sss",
		&strKey1,
		&strKey2,
		&strSubKey))
		return NULL;

	LSTATUS response;
	HKEY hKey;
	LPWSTR szStr;
	TCHAR szKey[256];
	lstrcpy(szKey, L"SOFTWARE\\");
	szStr = toW(strKey1);
	lstrcat(szKey, szStr);
	PyMem_RawFree(szStr);
	lstrcat(szKey, L"\\");
	szStr = toW(strKey2);
	lstrcat(szKey, szStr);
	PyMem_RawFree(szStr);

	char  strData[MAX_SETTING_SIZE + 1];
	DWORD	dwSizeData = MAX_SETTING_SIZE;

	response = RegOpenKeyExW(HKEY_CURRENT_USER, szKey, 0, KEY_QUERY_VALUE, &hKey);
	if (response == ERROR_PATH_NOT_FOUND || response == ERROR_FILE_NOT_FOUND)
		Py_RETURN_NONE;

	if (response != ERROR_SUCCESS) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}

	szStr = toW(strSubKey);
	response = RegQueryValueExW(hKey, szStr, 0, NULL, strData, (LPBYTE)(&dwSizeData));
	if (response == ERROR_PATH_NOT_FOUND || response == ERROR_FILE_NOT_FOUND) {
		RegCloseKey(hKey);
		Py_RETURN_NONE;
	}

	if (response != ERROR_SUCCESS) {
		PyErr_SetFromWindowsErr(0);
		RegCloseKey(hKey);
		return NULL;
	}
	RegCloseKey(hKey);
	PyMem_RawFree(szStr);

	if ((pyValue = PyBytes_FromStringAndSize(strData, dwSizeData)) == NULL)
		return NULL;
	return pyValue;
}

static PyMethodDef TymberMethods[] = {
	{ "message", Tymber_message, METH_VARARGS, "Show message box." },
	{ "set_setting", Tymber_settings_set, METH_VARARGS, "Save a Byte object to settings." },
	{ "get_setting", Tymber_settings_get, METH_VARARGS, "Load a Byte object from settings." },
	{ NULL, NULL, 0, NULL }
};

static PyModuleDef TyModule = {
	PyModuleDef_HEAD_INIT,
	"tymber",
	"GUI toolkit",
	-1,
	TymberMethods
};

PyMODINIT_FUNC
PyInit_tymber(void)
{
	g = (TyGlobals*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, sizeof(struct _TyGlobals));

	g->hInstance = GetModuleHandle(NULL);
	g->szAppName = L"Tymber";
	g->pyApp = Py_None;
	g->hIcon = ExtractIconW(g->hInstance, L"Tymber.pyd", -1 * IDI_LOGO);
	g->hfDefaultFont = GetStockObject(DEFAULT_GUI_FONT);
	g->hMenu = CreateMenu();
	g->hHeap = GetProcessHeap();
	g->hCursorWestEast = LoadCursor(NULL, MAKEINTRESOURCEW(IDC_SIZEWE));    // for Splitter
	g->hCursorNorthSouth = LoadCursor(NULL, MAKEINTRESOURCEW(IDC_SIZENS));
	g->hBkgBrush = GetSysColorBrush(TyWINDOWBKGCOLOR);
	Py_INCREF(Py_None);

	if (ExtractIconExW(L"SHELL32.DLL", -3, NULL, &g->hIconMdi, 1) == UINT_MAX) {
		PyErr_SetFromWindowsErr(0);
		return -1;
	}
	/* not working, related to this: https://bugs.python.org/issue5019
	g->hDLLcomctl32 = GetModuleHandleW("COMCTL32.DLL");
	if (g->hDLLcomctl32 == 0) {
		PyErr_SetFromWindowsErr(0);
		return NULL;
	}
	*/
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInputDef, NULL);

	PyObject* pyModule;
	pyModule = PyImport_ImportModule("enum");
	if (pyModule == NULL) {
		PyErr_SetString(PyExc_RuntimeError, "Can not import Python module 'enum'");
		return NULL;
	}
	g->pyEnumType = PyObject_GetAttrString(pyModule, "Enum");
	if (!g->pyEnumType) {
		PyErr_SetString(PyExc_RuntimeError, "Can not load Python type 'Enum'");
		return NULL;
	}

	pyModule = PyModule_Create(&TyModule);
	if (pyModule == NULL)
		return NULL;

	if (!WindowClass_Init())
		return NULL;
	if (!TyCanvasType_Init())
		return NULL;
	if (!TyBoxType_Init())
		return NULL;
	if (!TyToolBarType_Init())
		return NULL;
	if (!TyStatusBarType_Init())
		return NULL;
	if (!TyImageViewType_Init())
		return NULL;

	if (PyType_Ready(&TyApplicationType) < 0)
		return NULL;

	if (PyType_Ready(&TyWindowType) < 0)
		return NULL;

	if (PyType_Ready(&TyMenuType) < 0)
		return NULL;

	if (PyType_Ready(&TyMenuItemType) < 0)
		return NULL;

	if (PyType_Ready(&TyToolBarType) < 0)
		return NULL;

	if (PyType_Ready(&TyStatusBarType) < 0)
		return NULL;

	if (PyType_Ready(&TyMdiWindowType) < 0)
		return NULL;

	if (PyType_Ready(&TyWidgetType) < 0)
		return NULL;

	if (PyType_Ready(&TyIconType) < 0)
		return NULL;

	if (PyType_Ready(&TyButtonType) < 0)
		return NULL;

	if (PyType_Ready(&TyLabelType) < 0)
		return NULL;

	if (PyType_Ready(&TyEntryType) < 0)
		return NULL;

	if (PyType_Ready(&TyMdiAreaType) < 0)
		return NULL;

	if (PyType_Ready(&TyCanvasType) < 0)
		return NULL;

	if (PyType_Ready(&TyBoxType) < 0)
		return NULL;

	if (PyType_Ready(&TySplitterType) < 0)
		return NULL;

	if (PyType_Ready(&TyTabType) < 0)
		return NULL;

	if (PyType_Ready(&TyTabPageType) < 0)
		return NULL;

	if (PyType_Ready(&TyImageViewType) < 0)
		return NULL;

	if (PyType_Ready(&TyComboBoxType) < 0)
		return NULL;

	if (PyType_Ready(&TyListViewType) < 0)
		return NULL;

	Py_INCREF(&TyApplicationType);
	Py_INCREF(&TyWindowType);
	Py_INCREF(&TyMenuType);
	Py_INCREF(&TyMenuItemType);
	Py_INCREF(&TyToolBarType);
	Py_INCREF(&TyStatusBarType);
	Py_INCREF(&TyMdiWindowType);
	Py_INCREF(&TyWidgetType);
	Py_INCREF(&TyIconType);
	Py_INCREF(&TyButtonType);
	Py_INCREF(&TyLabelType);
	Py_INCREF(&TyEntryType);
	Py_INCREF(&TyMdiAreaType);
	Py_INCREF(&TyCanvasType);
	Py_INCREF(&TyBoxType);
	Py_INCREF(&TySplitterType);
	Py_INCREF(&TyTabType);
	Py_INCREF(&TyTabPageType);
	Py_INCREF(&TyImageViewType);
	Py_INCREF(&TyComboBoxType);
	Py_INCREF(&TyListViewType);

	PyModule_AddObject(pyModule, "Application", (PyObject*)&TyApplicationType);
	PyModule_AddObject(pyModule, "Window", (PyObject*)&TyWindowType);
	PyModule_AddObject(pyModule, "Menu", (PyObject*)&TyMenuType);
	PyModule_AddObject(pyModule, "MenuItem", (PyObject*)&TyMenuItemType);
	PyModule_AddObject(pyModule, "ToolBar", (PyObject*)&TyToolBarType);
	PyModule_AddObject(pyModule, "StatusBar", (PyObject*)&TyStatusBarType);
	PyModule_AddObject(pyModule, "MdiWindow", (PyObject*)&TyMdiWindowType);
	PyModule_AddObject(pyModule, "Widget", (PyObject*)&TyWidgetType);
	PyModule_AddObject(pyModule, "Icon", (PyObject*)&TyIconType);
	PyModule_AddObject(pyModule, "Button", (PyObject*)&TyButtonType);
	PyModule_AddObject(pyModule, "Label", (PyObject*)&TyLabelType);
	PyModule_AddObject(pyModule, "Entry", (PyObject*)&TyEntryType);
	PyModule_AddObject(pyModule, "MdiArea", (PyObject*)&TyMdiAreaType);
	PyModule_AddObject(pyModule, "Canvas", (PyObject*)&TyCanvasType);
	PyModule_AddObject(pyModule, "Box", (PyObject*)&TyBoxType);
	PyModule_AddObject(pyModule, "Splitter", (PyObject*)&TySplitterType);
	PyModule_AddObject(pyModule, "Tab", (PyObject*)&TyTabType);
	PyModule_AddObject(pyModule, "TabPage", (PyObject*)&TyTabPageType);
	PyModule_AddObject(pyModule, "ImageView", (PyObject*)&TyImageViewType);
	PyModule_AddObject(pyModule, "ComboBox", (PyObject*)&TyComboBoxType);
	PyModule_AddObject(pyModule, "ListView", (PyObject*)&TyListViewType);

	if (PyDict_SetItemString(TyWidgetType.tp_dict, "default_coordinate", PyLong_FromLong(CW_USEDEFAULT)) == -1)
		return NULL;
	if (PyDict_SetItemString(TyWidgetType.tp_dict, "center", PyLong_FromLong(TyWIDGET_CENTER)) == -1)
		return NULL;

	// named tuples

	// enumerations	
	PyObject* pyArgs = Py_BuildValue("(ss)", "Align", "left right center top bottom block");
	g->pyAlignEnum = PyObject_CallObject(g->pyEnumType, pyArgs);
	PyModule_AddObject(pyModule, "Align", g->pyAlignEnum);
	Py_DECREF(pyArgs);

	pyArgs = Py_BuildValue("(ss)", "ImageFormat", "ico bmp png jpg");
	g->pyImageFormatEnum = PyObject_CallObject(g->pyEnumType, pyArgs);
	PyModule_AddObject(pyModule, "ImageFormat", g->pyImageFormatEnum);
	Py_DECREF(pyArgs);

	pyArgs = Py_BuildValue("(ss)", "StockIcon", "file_open file_new save close exit delete help information warning lock edit ok no copy cut paste find undo redo right up down properties settings refresh window");
	g->pyStockIconEnum = PyObject_CallObject(g->pyEnumType, pyArgs);
	PyModule_AddObject(pyModule, "StockIcon", g->pyStockIconEnum);
	Py_DECREF(pyArgs);

	PyListObject* pyKeyValues = PyList_New(0);
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "enter", VK_RETURN));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "tab", VK_TAB));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "escape", VK_ESCAPE));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "delete", VK_DELETE));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "space", VK_SPACE));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "left", VK_LEFT));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "right", VK_RIGHT));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "up", VK_UP));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "down", VK_DOWN));
	PyList_Append(pyKeyValues, Py_BuildValue("(si)", "f1", VK_F1));

	pyArgs = Py_BuildValue("(sO)", "Key", pyKeyValues);
	g->pyKeyEnum = PyObject_CallObject(g->pyEnumType, pyArgs);
	PyModule_AddObject(pyModule, "Key", g->pyKeyEnum);
	Py_DECREF(pyArgs);
	Py_DECREF(pyKeyValues);

	// other
	PyObject* pyDict = PyModule_GetDict(pyModule);
	LPCSTR str = toU8(COPYRIGHT);
	PyObject* pyValue = PyUnicode_FromString(str);
	if (pyValue) {
		PyDict_SetItemString(pyDict, "copyright", pyValue);
		Py_DECREF(pyValue);
	}
	PyMem_RawFree(str);

	pyValue = Py_BuildValue("(iiis)", VER_MAJOR, VER_MINOR, VER_MICRO, VER_RELEASE_LEVEL);
	if (pyValue) {
		PyDict_SetItemString(pyDict, "version_info", pyValue);
		Py_DECREF(pyValue);
	}

	PyModule_AddObject(pyModule, "app", (PyObject*)g->pyApp);
	PyModule_AddObject(pyModule, "native", PyLong_FromLong(g->hInstance));
	g->pyModule = pyModule;
	return pyModule;
}