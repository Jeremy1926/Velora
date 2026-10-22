#include "pch.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UObject::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Object"));

    return Class;
}

UWorld* UObject::GetWorld() const
{
    static auto GetWorld = Anchor::VirtualFunction<UWorld* (*)(const UObject*)>(TEXT("UObject::GetWorld"))
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Start()
        .Find("call *")
        .Is("call [*+0x*]")
        .Or()
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Start()
        .Find("call *")
        .Target()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameSession"))
        .Get();

    return GetWorld(this);
}

bool UObject::NeedsLoadForServer() const
{
    static auto NeedsLoadForClient = Anchor::VirtualFunction<bool (*)(const UObject*)>(TEXT("UObject::NeedsLoadForClient"))
        .String(TEXT("Unable to spawn class '%s' due to client/server context."))
        .Last("call 0x*; test al, al")
        .Call()
        .Find("jmp [rax+0x*]")
        .Or()
        .String(TEXT("Unable to spawn class '%s' due to client/server context."))
        .Start()
        .Find("call [rax+0x*]; test al, al")
        .Get(sizeof(void*));

    return NeedsLoadForClient(this);
}

bool UObject::CanCreateInCurrentContext(UObject* Template)
{
    return Template->NeedsLoadForServer();
}

void UObject::PatchCanCreateInCurrentContext()
{
    static auto NeedsLoadForClientCall = Anchor::Address()
        .String(TEXT("Unable to spawn class '%s' due to client/server context."))
        .Start()
        .Find("call [rax+0x*]; test al, al")
        .Get();

    static auto bPatched = NeedsLoadForClientCall && Anchor::Patch(TEXT("UObject::CanCreateInCurrentContext"), NeedsLoadForClientCall + 2, { (uint8_t)(NeedsLoadForClientCall[2] + sizeof(void*)) });

    (void)bPatched;
}

UObject* UObject::GetArchetypeFromRequiredInfo(UClass* Class, UObject* Outer, FName Name, EObjectFlags ObjectFlags)
{
    static auto GetArchetypeFromRequiredInfoImpl = Anchor::Function<UObject* (*)(UClass*, UObject*, FName, EObjectFlags, bool)>(TEXT("GetArchetypeFromRequiredInfoImpl"))
        .String(TEXT("Component archetype is not the CDO nor a default subobject of my class. Me = %s, Component = %s, Archetype = %s"))
        .Last("xor r14b, r14b")
        .Call()
        .Find("mov r9d, *; mov r8, *; mov rdx, *; mov rcx, *; call 0x*")
        .Call()
        .Or()
        .Pattern("44 8B 4F 08 4C 8B 47 18 48 8B 57 20 48 8B 4F 10 E8")
        .Call()
        .Or()
        .String(TEXT("NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead."))
        .Call(2)
        .Call()
        .Get();

    return GetArchetypeFromRequiredInfoImpl(Class, Outer, Name, ObjectFlags, false);
}

bool UObject::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
    static auto CallRemoteFunction = Anchor::VirtualFunction<bool (*)(UObject*, UFunction*, void*, FOutParmRec*, FFrame*)>(TEXT("UObject::CallRemoteFunction"))
        .String(TEXT("NotifyEventTriggered: Unable to find function '%s'"))
        .Start()
        .Find("call [*+0x*]")
        .Entry(TEXT("/Script/CoreUObject.Default__Object"))
        .Find("test dword ptr [*], 0x400")
        .Find("call [*+0x*]", 2)
        .Get();

    return CallRemoteFunction(this, Function, Parms, OutParms, Stack);
}
