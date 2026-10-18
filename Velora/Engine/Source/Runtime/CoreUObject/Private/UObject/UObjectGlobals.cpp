#include "pch.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FObjectInitializer::AssertIfInConstructor(UObject* Outer, const TCHAR* ErrorMessage)
{
    static auto AssertIfInConstructor = (void (*)(UObject*, const TCHAR*))Scanner::String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead.")).Call().Get(TEXT("FObjectInitializer::AssertIfInConstructor"));

    AssertIfInConstructor(Outer, ErrorMessage);
}

UObject* StaticFindObject(UClass* ObjectClass, UObject* InObjectPackage, const TCHAR* OrigInName, bool ExactClass)
{
    static auto StaticFindObject = (UObject* (*)(UClass*, UObject*, const TCHAR*, bool))Scanner::Reference(Scanner::String(TEXT("Illegal call to StaticFindObject() while collecting garbage!")).Target(), "lea rax, [0x*]; mov r9d, 0x1; mov edx, 0x*; mov [rsp+0x20], rax; call 0x*; lea r9, [0x*]; mov r8d, 0x*; lea rdx, [0x*]; lea rcx, [0x*]; call 0x*; cmp *, 0xffffffffffffffff").Start().Get(TEXT("StaticFindObject"));

    return StaticFindObject(ObjectClass, InObjectPackage, OrigInName, ExactClass);
}

UObject* StaticLoadObject(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32 LoadFlags, UPackageMap* Sandbox, bool bAllowObjectReconciliation)
{
    static auto StaticLoadObject = (UObject* (*)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32, UPackageMap*, bool))Scanner::String(TEXT("Failed to find object '{ClassName} {OuterName}.{ObjectName}'")).Start().Get(TEXT("StaticLoadObject"));

    return StaticLoadObject(Class, InOuter, Name, Filename, LoadFlags, Sandbox, bAllowObjectReconciliation);
}

UObject* StaticConstructObject_Internal(UClass* Class, UObject* InOuter, FName Name, EObjectFlags SetFlags, EInternalObjectFlags InternalSetFlags, UObject* Template, bool bCopyTransientsFromClassDefaults, FObjectInstancingGraph* InstanceGraph, bool bAssumeTemplateIsArchetype)
{
    static auto StaticConstructObject_Internal = (UObject* (*)(UClass*, UObject*, FName, EObjectFlags, EInternalObjectFlags, UObject*, bool, FObjectInstancingGraph*, bool))Scanner::String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead.")).Call(2).Get(TEXT("StaticConstructObject_Internal"));

    return StaticConstructObject_Internal(Class, InOuter, Name, SetFlags, InternalSetFlags, Template, bCopyTransientsFromClassDefaults, InstanceGraph, bAssumeTemplateIsArchetype);
}
