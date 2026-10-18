#include "pch.h"
#include "UObject/UObjectArray.h"
#include "Scanner.h"

FUObjectArray& GUObjectArray = *(FUObjectArray*)(Scanner::String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. ")).Last("cmp *, [0x*]").Find("mov *, [0x*]").Target(TEXT("GUObjectArray.ObjObjects")) - offsetof(FUObjectArray, ObjObjects));
