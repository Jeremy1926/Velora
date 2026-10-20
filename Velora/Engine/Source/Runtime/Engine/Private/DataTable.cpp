#include "pch.h"
#include "Engine/DataTable.h"
#include "UObject/UObjectGlobals.h"

UClass* UDataTable::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.DataTable"));

    return Class;
}
