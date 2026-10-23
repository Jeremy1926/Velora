#include "pch.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FObjectInitializer::AssertIfInConstructor(UObject* Outer, const TCHAR* ErrorMessage)
{
    static auto AssertIfInConstructor = Anchor::Function<void (*)(UObject*, const TCHAR*)>(TEXT("FObjectInitializer::AssertIfInConstructor"))
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."))
        .FirstOf("call 0x*", "jmp 0x*")
        .Is("call 0x*")
        .Call()
        .Or()
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."), "lea rdx, [0x*]; call 0x*")
        .Call()
        .Or()
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSubobject<> instead."))
        .Start()
        .Get();

    AssertIfInConstructor(Outer, ErrorMessage);
}

UObject* StaticFindObject(UClass* ObjectClass, UObject* InObjectPackage, const TCHAR* OrigInName, bool ExactClass)
{
    static auto StaticFindObject = Anchor::Function<UObject* (*)(UClass*, UObject*, const TCHAR*, bool)>(TEXT("StaticFindObject"))
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"), "lea rax, [0x*]; mov r9d, 0x1; mov edx, 0x*; mov [rsp+0x20], rax; call 0x*; lea r9, [0x*]; mov r8d, 0x*; lea rdx, [0x*]; lea rcx, [0x*]; call 0x*; cmp *, 0xffffffffffffffff")
        .Start()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"), "lea rsi, [0x*]; mov r9d, 0x1; mov [rbp+0x*], rax; lea r8, [rbp+0x*]; mov [rsp+0x20], rsi; mov edx, 0x*; lea rcx, [0x*]; call 0x*; mov r9, rsi; lea rdx, [0x*]; mov r8d, 0x*; lea rcx, [0x*]; call 0x*; cmp *, 0xffffffffffffffff")
        .Start()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"), "lea rax, [0x*]; mov r9d, 0x1; lea r8, [0x*]; mov [rsp+0x20], rax; mov edx, 0x*; lea rcx, [0x*]; call 0x*; call 0x*; cmp *, 0xffffffffffffffff")
        .Start()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"))
        .Start()
        .Or()
        .Pattern("45 33 F6 4C 8B E1 45 0F B6 E9 49 8B F8 41 8B C6 41 8B CE 48 83 FA FF 0F 84")
        .Start()
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Get();

    return StaticFindObject(ObjectClass, InObjectPackage, OrigInName, ExactClass);
}

UObject* StaticLoadObject(UClass* Class, UObject* InOuter, const TCHAR* Name, const TCHAR* Filename, uint32 LoadFlags, UPackageMap* Sandbox, bool bAllowObjectReconciliation, FUObjectSerializeContext* InSerializeContext)
{
    static auto StaticLoadObject = Anchor::Function<UObject* (*)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32, UPackageMap*, bool, FUObjectSerializeContext*)>(TEXT("StaticLoadObject"))
        .String(TEXT("Failed to find object '{ClassName} {OuterName}.{ObjectName}'"))
        .Start()
        .Get();

    return StaticLoadObject(Class, InOuter, Name, Filename, LoadFlags, Sandbox, bAllowObjectReconciliation, InSerializeContext);
}

UObject* StaticConstructObject_Internal(UClass* Class, UObject* InOuter, FName Name, EObjectFlags SetFlags, EInternalObjectFlags InternalSetFlags, UObject* Template, bool bCopyTransientsFromClassDefaults, FObjectInstancingGraph* InstanceGraph, bool bAssumeTemplateIsArchetype)
{
    static auto StaticConstructObject_InternalWithParams = Anchor::Optional<UObject* (*)(const void*)>(TEXT("StaticConstructObject_Internal(FStaticConstructObjectParameters)"))
        .String(TEXT("STAT_ConstructObject"))
        .Start()
        .Find("mov *, [rcx]; lea rax, [0x*]; mov *, [rcx+0x8]")
        .Start()
        .Or()
        .String(TEXT("STAT_ConstructObject"), "lea rax, [0x*]; mov *, [rcx+0x8]")
        .Start()
        .Get();

    if (StaticConstructObject_InternalWithParams)
    {
        struct
        {
            UClass* Class;
            UObject* Outer;
            FName Name;
            EObjectFlags SetFlags;
            EInternalObjectFlags InternalSetFlags;
            bool bCopyTransientsFromClassDefaults;
            bool bAssumeTemplateIsArchetype;
            UObject* Template;
            FObjectInstancingGraph* InstanceGraph;
            void* ExternalPackage;
            uint8 PropertyInitCallback[0x40];
            void* SubobjectOverrides;
        } Params { Class, InOuter, Name, SetFlags, InternalSetFlags, bCopyTransientsFromClassDefaults, bAssumeTemplateIsArchetype, Template, InstanceGraph, nullptr };

        return StaticConstructObject_InternalWithParams(&Params);
    }

    static auto StaticConstructObject_Internal = Anchor::Function<UObject* (*)(UClass*, UObject*, FName, EObjectFlags, EInternalObjectFlags, UObject*, bool, FObjectInstancingGraph*, bool)>(TEXT("StaticConstructObject_Internal"))
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."))
        .FirstOf("call 0x*", "jmp 0x*")
        .Is("call 0x*")
        .Call(2)
        .Or()
        .String(TEXT("STAT_ConstructObject"))
        .Start()
        .Or()
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."), "lea rdx, [0x*]; call 0x*")
        .Call(2)
        .Get();

    return StaticConstructObject_Internal(Class, InOuter, Name, SetFlags, InternalSetFlags, Template, bCopyTransientsFromClassDefaults, InstanceGraph, bAssumeTemplateIsArchetype);
}
