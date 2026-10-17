#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"

class UProperty;
struct FFrame;

typedef void (UObject::*Native)(FFrame& TheStack, void* const Z_Param__Result);

class UField : public UObject
{
public:
    UField* Next;
};

class UStruct : public UField
{
public:
    UProperty* FindPropertyByName(FName InName) const;

    FORCEINLINE UStruct* GetSuperStruct() const
    {
        return SuperStruct;
    }

    bool IsChildOf(const UStruct* SomeBase) const
    {
        for (const UStruct* Struct = this; Struct; Struct = Struct->GetSuperStruct())
        {
            if (Struct == SomeBase)
                return true;
        }

        return false;
    }

private:
    UStruct* SuperStruct;
};

class UClass : public UStruct
{
};

class UFunction : public UStruct
{
public:
    Native GetNativeFunc() const;
};
