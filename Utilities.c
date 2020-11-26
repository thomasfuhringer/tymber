// Utilities.c  | Tymber © 2020 by Thomas Führinger
#include <Tymber.h>
#include "Resource.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <Shlobj.h>

LPVOID HAlloc(_In_ SIZE_T dwBytes)
{
	return HeapAlloc(g->hHeap, HEAP_GENERATE_EXCEPTIONS, dwBytes);
}

BOOL HFree(_In_ LPVOID pMem)
{
	return HeapFree(g->hHeap, 0, pMem);
}

LPCSTR toU8(const LPWSTR szUTF16)
{
	if (szUTF16 == NULL)
		return NULL;
	if (*szUTF16 == L'\0')
		return '\0';

	int cbUTF8 = WideCharToMultiByte(CP_UTF8, 0, szUTF16, -1, NULL, 0, NULL, NULL);
	if (cbUTF8 == 0) {
		PyErr_SetString(PyExc_RuntimeError, "Sting converson to wide character failed.");
		return NULL;
	}
	LPCSTR strTextUTF8 = (LPCSTR)PyMem_RawMalloc(cbUTF8);
	int result = WideCharToMultiByte(CP_UTF8, 0, szUTF16, -1, strTextUTF8, cbUTF8, NULL, NULL);
	if (result == 0) {
		PyErr_SetString(PyExc_RuntimeError, "Sting converson to wide character failed.");
		return NULL;
	}
	return strTextUTF8;
}

LPWSTR toW(const LPCSTR strTextUTF8)
{
	if (strTextUTF8 == NULL)
		return NULL;
	if (*strTextUTF8 == '\0')
		return L'\0';

	int cchUTF16 = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, strTextUTF8, -1, NULL, 0); // request buffer size
	if (cchUTF16 == 0) {
		PyErr_SetString(PyExc_RuntimeError, "Sting converson to wide character failed.");
		return NULL;
	}
	LPWSTR szUTF16 = (LPWSTR)PyMem_RawMalloc(cchUTF16 * sizeof(WCHAR));
	int result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, strTextUTF8, -1, szUTF16, cchUTF16);
	if (result == 0) {
		PyErr_SetString(PyExc_RuntimeError, "Sting converson to wide character failed.");
		return NULL;
	}
	return szUTF16;
}

BOOL
TyAttachObject(PyObject** ppyMember, PyObject* pyObject)
{
	PyObject* tmp = *ppyMember;
	Py_INCREF(pyObject);
	*ppyMember = pyObject;
	Py_XDECREF(tmp);
	return TRUE;
}

void XX(PyObject* pyObject) // for debuggin purposes
{
	if (pyObject) {
		char* key = "";
		if (PyObject_TypeCheck(pyObject, &TyWidgetType)) {
			key = PyUnicode_AsUTF8(((TyWidgetObject*)pyObject)->pyKey);
		}
		PyObject* objectsRepresentation = PyObject_Repr(pyObject);
		const char* str = PyUnicode_AsUTF8(objectsRepresentation);
		printf("%s %s %11d refs\n", str, key, pyObject->ob_refcnt);
		Py_DECREF(objectsRepresentation);
	}
	else {
		TCHAR szBuffer[255];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, szBuffer, sizeof(szBuffer) / sizeof(TCHAR), 0);
		printf("NULL %ws\n", szBuffer);
	}
}