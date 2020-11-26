#include "Tymber.h"

static HBITMAP PyBytes_AsBitmap(PyObject* pyBytes);

static PyObject*
TyImage_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	TyImageObject* self = (TyImageObject*)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->pyImageFormat = NULL;
		self->hIconLarge = NULL;
		return (PyObject*)self;
	}
	else
		return NULL;
}

static int
TyImage_init(TyImageObject* self, PyObject* args, PyObject* kwds)
{
	static char* kwlist[] = { "source", NULL };
	PyObject* pySource = NULL;
	self->hWin = g->hIcon;

	if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist,
		&pySource))
		return -1;

	if (pySource) {
		if (PyUnicode_Check(pySource)) {
			LPWSTR szFileName = toW(PyUnicode_AsUTF8(pySource));
			LPWSTR szExtension = PathFindExtensionW(szFileName);

			if (lstrcmpi(szExtension, L".ico") == 0) {
				self->hWin = LoadImageW(0, szFileName, IMAGE_ICON, 0, 0, LR_LOADFROMFILE); //LR_DEFAULTSIZE | 
				if (self->hWin == 0) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
				self->pyImageFormat = PyObject_GetAttrString(g->pyImageFormatEnum, "ico");
				Py_INCREF(self->pyImageFormat);
			}
			else if (lstrcmpi(szExtension, L".bmp") == 0) {
				self->hWin = (HBITMAP)LoadImageW(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
				if (self->hWin == 0) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
				self->pyImageFormat = PyObject_GetAttrString(g->pyImageFormatEnum, "bmp");
				Py_INCREF(self->pyImageFormat);
			}
			PyMem_RawFree(szFileName);
		}
		else if (PyObject_TypeCheck(pySource, &PyBytes_Type)) {
			self->hWin = PyBytes_AsBitmap(pySource);
			if (self->hWin == 0) {
				return -1;
			}
			self->pyImageFormat = PyObject_GetAttrString(g->pyImageFormatEnum, "bmp");
			Py_INCREF(self->pyImageFormat);
		}
		else if (PyObject_TypeCheck(pySource, g->pyEnumType)) {
			if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "window")) {
				if (ExtractIconExW(L"SHELL32.DLL", -3, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "file_open")) {
				if (ExtractIconExW(L"SHELL32.DLL", -235, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "file_new")) {
				if (ExtractIconExW(L"SHELL32.DLL", -152, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "save")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16761, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "ok")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16802, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "no")) {
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "copy")) {
				self->hWin = ExtractIconW(g->hInstance, L"SHELL32.DLL", -243);
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "cut")) {
				self->hWin = ExtractIconW(g->hInstance, L"SHELL32.DLL", -16762);
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "paste")) {
				self->hWin = ExtractIconW(g->hInstance, L"SHELL32.DLL", -16763);
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "find")) {
				self->hWin = ExtractIconW(g->hInstance, L"SHELL32.DLL", -23);
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "properties")) {
				self->hWin = ExtractIconW(g->hInstance, L"SHELL32.DLL", -174);
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "settings")) {
				self->hWin = ExtractIconW(g->hInstance, L"SHELL32.DLL", -16826);
				if (ExtractIconExW(L"SHELL32.DLL", -200, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "window")) {
				if (ExtractIconExW(L"SHELL32.DLL", -3, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "undo")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -5315, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "redo")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -5311, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "right")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16805, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "up")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16817, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "refresh")) {
				if (ExtractIconExW(L"SHELL32.DLL", -16739, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "close")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -97, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "exit")) {
				if (ExtractIconExW(L"SHELL32.DLL", -28, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "delete")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -89, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "help")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -99, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "information")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -81, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "warning")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -1403, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "lock")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -1304, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else if (pySource == PyObject_GetAttrString(g->pyStockIconEnum, "edit")) {
				if (ExtractIconExW(L"IMAGERES.DLL", -5306, &self->hIconLarge, &self->hWin, 1) == UINT_MAX) {
					PyErr_SetFromWindowsErr(0);
					return -1;
				}
			}
			else {
				PyErr_SetString(PyExc_TypeError, "Parameter 1 ('source') specifies a StockItem that is not available.");
				return -1;
			}
			self->pyImageFormat = PyObject_GetAttrString(g->pyImageFormatEnum, "ico");
			Py_INCREF(self->pyImageFormat);
		}
		else {
			PyErr_SetString(PyExc_TypeError, "Argument 1 ('source') can only be a byte object, a string with the file name or a StockItem enum member.");
			return -1;
		}
		SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	}

	return 0;
}

static BOOL
TyImage_Load(TyImageObject* self, LPCSTR strFileName)
{
	LPWSTR szFileName;
	szFileName = toW(strFileName);
	self->hWin = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	PyMem_RawFree(szFileName);
	SetWindowLongPtr(self->hWin, GWLP_USERDATA, (LONG_PTR)self);
	return TRUE;
}

static PyObject*
TyImage_load(TyImageObject* self, PyObject* args)
{
	const char* strFileName;

	if (!PyArg_ParseTuple(args, "s",
		&strFileName))
		return NULL;

	if (!TyImage_Load(self, strFileName))
		return NULL;

	Py_RETURN_NONE;
}

static PyObject*
TyImage_as_bytes(TyImageObject* self)
// from https://phvu.wordpress.com/2009/07/09/serialize-and-deserialize-bitmap-object-in-mfcwin32/
{
	if (!PyObject_TypeCheck(self, &TyImageType)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be an Image object.");
		return NULL;
	}
	//HBITMAP hBitmap;
	int len;

	BITMAP bmpObj;
	HDC hDCScreen;
	int iRet;
	DWORD dwBmpSize;

	BITMAPFILEHEADER    bmfHeader;
	LPBITMAPINFO        lpbi;
	const DWORD dwSizeOfBmfHeader = sizeof(BITMAPFILEHEADER);
	DWORD dwSizeOfBmInfo = sizeof(BITMAPINFO);

	hDCScreen = GetDC(NULL);
	GetObject(self->hWin, sizeof(BITMAP), &bmpObj);

	lpbi = HAlloc(dwSizeOfBmInfo + 8);
	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

	// Gets the "bits" from the bitmap and copies them into a buffer
	// which is pointed to by lpbi
	iRet = GetDIBits(hDCScreen, self->hWin, 0, (UINT)bmpObj.bmHeight, NULL, lpbi, DIB_RGB_COLORS);
	assert(iRet > 0);

	// only 16 and 32 bit images are supported.
	assert(lpbi->bmiHeader.biBitCount == 16 || lpbi->bmiHeader.biBitCount == 32);
	if (lpbi->bmiHeader.biCompression == BI_BITFIELDS)
		dwSizeOfBmInfo += 8;

	dwBmpSize = lpbi->bmiHeader.biSizeImage;
	char* lpbitmap = (char*)HAlloc(dwBmpSize);

	iRet = GetDIBits(hDCScreen, self->hWin, 0, (UINT)bmpObj.bmHeight, lpbitmap, lpbi, DIB_RGB_COLORS);
	assert(iRet > 0);

	DWORD dwSizeofDIB = dwBmpSize + dwSizeOfBmfHeader + dwSizeOfBmInfo;
	bmfHeader.bfOffBits = (DWORD)dwSizeOfBmfHeader + (DWORD)dwSizeOfBmInfo;
	bmfHeader.bfSize = dwSizeofDIB;
	bmfHeader.bfType = 0x4D42; //BM
	bmfHeader.bfReserved1 = bmfHeader.bfReserved2 = 0;

	char* arrData = (char*)HAlloc(dwSizeofDIB);
	memcpy(arrData, &bmfHeader, dwSizeOfBmfHeader);
	memcpy(arrData + dwSizeOfBmfHeader, lpbi, dwSizeOfBmInfo);
	memcpy(arrData + dwSizeOfBmfHeader + dwSizeOfBmInfo, lpbitmap, dwBmpSize);


	HFree(lpbi);
	HFree(lpbitmap);
	ReleaseDC(NULL, hDCScreen);

	len = dwSizeofDIB;
	PyObject* pyBytes = PyBytes_FromStringAndSize(arrData, len);
	HFree(arrData);
	return  pyBytes;
}

static HBITMAP
PyBytes_AsBitmap(PyObject* pyBytes)
{
	PBITMAPFILEHEADER    bmfHeader;
	PBITMAPINFO    pbi;
	HDC            hDC;
	HBITMAP        hBmpRet;
	int            iRet;
	//char        *lpbitmap;
	int            iSizeOfBmInfo;
	const int    iSizeOfBmfHeader = sizeof(BITMAPFILEHEADER);

	if (!PyObject_TypeCheck(pyBytes, &PyBytes_Type)) {
		PyErr_SetString(PyExc_TypeError, "Argument must be a bytes object.");
		return NULL;
	}

	char* arrData = PyBytes_AsString(pyBytes);
	Py_ssize_t nLen = PyBytes_Size(pyBytes);


	// get the BITMAPFILEHEADER
	bmfHeader = (PBITMAPFILEHEADER)arrData;
	arrData += iSizeOfBmfHeader;

	// get the BITMAPINFO
	iSizeOfBmInfo = bmfHeader->bfOffBits - iSizeOfBmfHeader;
	pbi = (PBITMAPINFO)arrData;
	arrData += iSizeOfBmInfo;

	assert(pbi->bmiHeader.biSizeImage == nLen - iSizeOfBmfHeader - iSizeOfBmInfo);

	// create the output DDB bitmap
	hDC = GetDC(NULL);
	hBmpRet = CreateCompatibleBitmap(hDC,
		pbi->bmiHeader.biWidth, pbi->bmiHeader.biHeight);
	assert(hBmpRet);

	// set the image...
	iRet = SetDIBits(hDC, hBmpRet, 0,
		pbi->bmiHeader.biHeight,
		arrData,
		pbi, DIB_RGB_COLORS);
	assert(iRet > 0);

	ReleaseDC(NULL, hDC);
	return hBmpRet;
}

static PyObject*
TyImage_FromBytes(PyObject* pyBytes)
{
	TyImageObject* self = (TyImageObject*)TyImage_new(&TyImageType, NULL, NULL);
	self->hWin = PyBytes_AsBitmap(pyBytes);
	return (PyObject*)self;
}

static void
TyImage_dealloc(TyImageObject* self)
{
	if (self->pyImageFormat = PyObject_GetAttrString(g->pyImageFormatEnum, "bmp"))
		DeleteObject(self->hWin);
	else if (self->pyImageFormat == PyObject_GetAttrString(g->pyImageFormatEnum, "ico")) {
		DestroyIcon(self->hWin);
		if (self->hIconLarge)
			DestroyIcon(self->hIconLarge);
	}
	Py_TYPE(self)->tp_base->tp_dealloc((PyObject*)self);
}

static PyMemberDef TyImage_members[] = {
	{ "format", T_OBJECT, offsetof(TyImageObject, pyImageFormat), READONLY, "ImageFormat" },
	{ NULL }
};

static PyMethodDef TyImage_methods[] = {
	{ "load", (PyCFunction)TyImage_load, METH_VARARGS, "Load from file" },
	{ "bytes", (PyCFunction)TyImage_as_bytes, METH_NOARGS, "Convert to bytes object." },
	{ NULL }
};

PyTypeObject TyImageType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tymber.Image",            /* tp_name */
	sizeof(TyImageObject),     /* tp_basicsize */
	0,                         /* tp_itemsize */
	(destructor)TyImage_dealloc, /* tp_dealloc */
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
	0,                         /* tp_getattro */
	0,                         /* tp_setattro */
	0,                         /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
	"Image object",            /* tp_doc */
	0,                         /* tp_traverse */
	0,                         /* tp_clear */
	0,                         /* tp_richcompare */
	0,                         /* tp_weaklistoffset */
	0,                         /* tp_iter */
	0,                         /* tp_iternext */
	TyImage_methods,           /* tp_methods */
	TyImage_members,           /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)TyImage_init,    /* tp_init */
	0,                         /* tp_alloc */
	TyImage_new,               /* tp_new */
	PyObject_Free,             /* tp_free */
};