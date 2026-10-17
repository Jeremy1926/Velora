#include "pch.h"
#include "UObject/UObjectBaseUtility.h"
#include "UObject/Class.h"

bool UObjectBaseUtility::IsA(const UClass* SomeBase) const
{
    return GetClass()->IsChildOf(SomeBase);
}
