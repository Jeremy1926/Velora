#include "pch.h"
#include "Engine/CurveTable.h"
#include "UObject/UObjectGlobals.h"

UClass* UCurveTable::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.CurveTable"));

    return Class;
}
