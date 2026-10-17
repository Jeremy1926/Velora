#include "pch.h"
#include "UObject/Class.h"
#include "Scanner.h"

UProperty* UStruct::FindPropertyByName(FName InName) const
{
    static auto FindPropertyByName = (UProperty* (*)(const UStruct*, FName))Scanner::FindPattern("48 8B 41 58 48 85 C0 74 ? 0F 1F 80 00 00 00 00 48 39 50 18 74 ? 48 8B 40 50 48 85 C0 75 ? 33 C0 C3");

    return FindPropertyByName(this, InName);
}
