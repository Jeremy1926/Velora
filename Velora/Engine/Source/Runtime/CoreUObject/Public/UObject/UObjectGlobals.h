#pragma once

#include "HAL/Platform.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectMacros.h"

class UClass;
class UObject;
class UPackageMap;

class FObjectInitializer
{
public:
    static void AssertIfInConstructor(UObject* Outer, const TCHAR* ErrorMessage);
};

UObject* GetTransientPackage();

UObject* StaticFindObject(UClass* Class, UObject* InOuter, const TCHAR* Name, bool ExactClass = false);

UObject* StaticLoadObject(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename = nullptr, uint32 LoadFlags = LOAD_None, UPackageMap* Sandbox = nullptr, bool bAllowObjectReconciliation = true, struct FUObjectSerializeContext* InSerializeContext = nullptr);

template<class T>
inline T* LoadObject(UObject* Outer, const TCHAR* Name, const TCHAR* Filename = nullptr, uint32 LoadFlags = LOAD_None, UPackageMap* Sandbox = nullptr)
{
    return (T*)StaticLoadObject(T::StaticClass(), Outer, Name, Filename, LoadFlags, Sandbox);
}

UObject* StaticConstructObject_Internal(UClass* Class, UObject* InOuter, FName Name = NAME_None, EObjectFlags SetFlags = RF_NoFlags, EInternalObjectFlags InternalSetFlags = EInternalObjectFlags::None, UObject* Template = nullptr, bool bCopyTransientsFromClassDefaults = false, struct FObjectInstancingGraph* InstanceGraph = nullptr, bool bAssumeTemplateIsArchetype = false);

template<class T>
T* NewObject(UObject* Outer, UClass* Class, FName Name = NAME_None, EObjectFlags Flags = RF_NoFlags, UObject* Template = nullptr, bool bCopyTransientsFromClassDefaults = false, FObjectInstancingGraph* InInstanceGraph = nullptr)
{
    if (Name == NAME_None)
    {
        FObjectInitializer::AssertIfInConstructor(Outer, TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."));
    }

    return static_cast<T*>(StaticConstructObject_Internal(Class, Outer, Name, Flags, EInternalObjectFlags::None, Template, bCopyTransientsFromClassDefaults, InInstanceGraph));
}
