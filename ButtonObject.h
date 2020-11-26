// ButtonObject.h  | Tymber © 2020 by Thomas Führinger
#ifndef Ty_BUTTONOBJECT_H
#define Ty_BUTTONOBJECT_H

typedef struct _TyButtonObject
{
	TyWidgetObject_HEAD
		PyObject* pyOnClickCB;
}
TyButtonObject;

extern PyTypeObject TyButtonType;

BOOL TyButton_OnClick(TyButtonObject* self);

#endif /* !Ty_BUTTONOBJECT_H */