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

class UScriptStruct : public UStruct
{
};

class UClass : public UStruct
{
public:
    static UClass* StaticClass();

    UObject* GetDefaultObject(bool bCreateIfNeeded = true) const;

    template<class T>
    T* GetDefaultObject() const
    {
        return (T*)GetDefaultObject();
    }

    UObject* InternalCreateDefaultObjectWrapper() const;
};

class UFunction : public UStruct
{
public:
    Native GetNativeFunc() const;
};
