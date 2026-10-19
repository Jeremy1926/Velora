#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"
#include "Misc/EngineVersion.h"

class UProperty;
struct FFrame;

typedef void (UObject::*Native)(FFrame& TheStack, void* const Z_Param__Result);

class UField : public UObject
{
public:
    UField* Next;
};

class FStructBaseChain
{
private:
    FStructBaseChain** StructBaseChainArray;
    int32 NumStructBasesInChainMinusOne;
};

class UStruct : public UField
{
public:
    UProperty* FindPropertyByName(FName InName) const;

    FORCEINLINE UStruct* GetSuperStruct() const
    {
        return GetStruct()->SuperStruct;
    }

    FORCEINLINE int32 GetPropertiesSize() const
    {
        return GetStruct()->PropertiesSize;
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
    FORCEINLINE const UStruct* GetStruct() const
    {
        auto& Version = FEngineVersion::Current();

        return Version.GetMajor() > 4 || Version.GetMinor() >= 22 ? (const UStruct*)((const uint8*)this + sizeof(FStructBaseChain)) : this;
    }

    UStruct* SuperStruct;
    UField* Children;
    int32 PropertiesSize;
    int32 MinAlignment;
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
