// EntryObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_ENTRYOBJECT_H
#define Ty_ENTRYOBJECT_H

typedef struct _TyEntryObject
{
	TyWidgetObject_HEAD
		PyObject* pyOnLeaveCB;
	PyObject* pyOnKeyCB;
	BOOL bPassword;
	BOOL bMultiline;
}
TyEntryObject;

extern PyTypeObject TyEntryType;

BOOL TyEntry_FocusIn(TyEntryObject* self);
BOOL TyEntry_FocusOut(TyEntryObject* self);
BOOL TyEntry_Parse(TyEntryObject* self);

LRESULT CALLBACK TyEntryProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif