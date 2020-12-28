#ifndef TyUTILITIES_H
#define TyUTILITIES_H

char* toU8(const LPWSTR pszTextUTF16);
LPWSTR toW(const char* strTextUTF8);
BOOL TyAttachObject(PyObject** ppyMember, PyObject* pyObject);

// Conveniance Wrappers
LPVOID HAlloc(_In_ SIZE_T dwBytes);
BOOL   HFree(_In_ LPVOID pMem);

// For debugging purposes
void XX(PyObject* pyObject);

#endif 
