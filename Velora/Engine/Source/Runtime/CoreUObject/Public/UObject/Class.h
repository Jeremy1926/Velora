#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Misc/EngineVersion.h"
#include "UObject/Field.h"

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

    void InitializeStruct(void* Dest, int32 ArrayDim = 1) const;

    void DestroyStruct(void* Dest, int32 ArrayDim = 1) const;

    FORCEINLINE UStruct* GetSuperStruct() const
    {
        return GetStruct()->SuperStruct;
    }

    FORCEINLINE int32 GetPropertiesSize() const
    {
        return FField::StaticClass() ? ((const UStruct*)((const uint8*)GetStruct() + sizeof(FField*)))->PropertiesSize : GetStruct()->PropertiesSize;
    }

    FORCEINLINE int32 GetMinAlignment() const
    {
        return FField::StaticClass() ? ((const UStruct*)((const uint8*)GetStruct() + sizeof(FField*)))->MinAlignment : GetStruct()->MinAlignment;
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
public:
    void CopyScriptStruct(void* Dest, void const* Src, int32 ArrayDim = 1) const;
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

    void SetNativeFunc(Native InFunc);
};

class UEnum : public UField
{
public:
    int64 GetValueByName(FName InName) const;

    FString GetNameStringByValue(int64 InValue) const;

private:
    FString CppType;
    TArray<TPair<FName, int64>> Names;
};

template<typename T>
UEnum* StaticEnum();
