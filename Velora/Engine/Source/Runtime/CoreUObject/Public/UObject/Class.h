#pragma once

#include "UObject/Object.h"
#include "UObject/NameTypes.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Misc/EngineVersion.h"
#include "UObject/Field.h"

class UFunction;
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
    struct FEncryptedLayout
    {
        Anchor::FCipher SuperStruct;
        Anchor::FCipher PropertiesSize;
        Anchor::FCipher MinAlignment;
        Anchor::FCipher PropertyLink;
    };

    static const FEncryptedLayout* GetEncryptedLayout();

    UProperty* FindPropertyByName(FName InName) const;

    void InitializeStruct(void* Dest, int32 ArrayDim = 1) const;

    void DestroyStruct(void* Dest, int32 ArrayDim = 1) const;

    FORCEINLINE UStruct* GetSuperStruct() const
    {
        auto Layout = GetEncryptedLayout();

        return Layout ? Layout->SuperStruct.Load<UStruct*>(this) : GetStruct()->SuperStruct;
    }

    FORCEINLINE int32 GetPropertiesSize() const
    {
        if (auto Layout = GetEncryptedLayout())
            return Layout->PropertiesSize.Load<int32>(this);

        return FField::StaticClass() ? ((const UStruct*)((const uint8*)GetStruct() + sizeof(FField*)))->PropertiesSize : GetStruct()->PropertiesSize;
    }

    FORCEINLINE int32 GetMinAlignment() const
    {
        if (auto Layout = GetEncryptedLayout())
            return Layout->MinAlignment.Load<int32>(this);

        return FField::StaticClass() ? ((const UStruct*)((const uint8*)GetStruct() + sizeof(FField*)))->MinAlignment : GetStruct()->MinAlignment;
    }

    FORCEINLINE UProperty* GetPropertyLink() const
    {
        if (auto Layout = GetEncryptedLayout())
            return Layout->PropertyLink.Load<UProperty*>(this);

        return FField::StaticClass() ? ((const UStruct*)((const uint8*)GetStruct() + sizeof(FField*)))->PropertyLink : GetStruct()->PropertyLink;
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
    TArray<uint8> Script;
    UProperty* PropertyLink;
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

    UFunction* FindFunctionByName(FName InName) const;
};

class UFunction : public UStruct
{
public:
    Native GetNativeFunc() const;

    void SetNativeFunc(Native InFunc);

    EFunctionFlags GetFunctionFlags() const;
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
