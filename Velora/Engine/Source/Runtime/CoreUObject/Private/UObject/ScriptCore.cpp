#include "pch.h"
#include "UObject/Stack.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

void FFrame::Step(UObject* Context, void* const Result)
{
    static auto Step = Anchor::Optional<void (*)(FFrame*, UObject*, void*)>(TEXT("FFrame::Step"))
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("mov rdx, [rdx+0x18]; lea r8, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("48 8B 41 20 4C 8B D2 48 8B D1 44 0F B6 08 48 FF C0 48 89 41 20 41 8B C1 4C 8D 0D ? ? ? ? 49 8B CA 49 FF 24 C1")
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("mov rdx, [*+0x18]; lea r8, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("mov rdx, [*+0x18]; lea r8, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (Step)
    {
        Step(this, Context, Result);
        return;
    }

    static auto GNatives = Anchor::Data<void (**)(UObject*, FFrame&, void*)>(TEXT("GNatives"))
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("movzx e*, byte ptr [rax]; lea r*, [0x*]")
        .Find("lea r*, [0x*]")
        .Target()
        .Get();

    auto B = *Code++;
    GNatives[B](Context, *this, Result);
}

void FFrame::StepExplicitProperty(void* const Result, UProperty* Property)
{
    static auto StepExplicitProperty = Anchor::Function<void (*)(FFrame*, void*, UProperty*)>(TEXT("FFrame::StepExplicitProperty"))
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("mov [rdx+0x80], rax; lea rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("mov [*+0x80], rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("mov [*+0x88], rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 41 8B 40 30 49 8B D8 48 8B F2 48 8B F9 48 0F BA E0 08 72")
        .Or()
        .Pattern("41 8B 40 30 4D 8B C8 48 0F BA E0 08 73 ? 48 8B 81 80 00 00 00")
        .Get();

    StepExplicitProperty(this, Result, Property);
}

UField*& FFrame::GetPropertyChainForCompiledIn()
{
    return *(UField**)((uint8*)&PropertyChainForCompiledIn + (FEngineVersion::Current().GetChangelist() >= 19598943 ? sizeof(uint8*) : 0));
}

template<>
template<>
void TMulticastScriptDelegate<FWeakObjectPtr>::ProcessMulticastDelegate<UObject>(void* Parameters) const
{
    static auto ProcessMulticastDelegate = Anchor::Function<void (*)(const TMulticastScriptDelegate<FWeakObjectPtr>*, void*)>(TEXT("TMulticastScriptDelegate::ProcessMulticastDelegate"))
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .LogSlot(TEXT("UActorComponent::Deactivate"))
        .Entry(TEXT("/Script/Engine.Default__ActorComponent"))
        .Find("mov rcx, [rsp+0x*]; test rcx, rcx; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Call()
        .Or()
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/Engine.Default__ActorComponent"))
        .Call(2)
        .Or()
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/Engine.Default__ActorComponent"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov rcx, [rsp+0x*]; test rcx, rcx; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Call()
        .Or()
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/Engine.Default__ActorComponent"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov rcx, [rsp+0x*]; test rcx, rcx; jz 0x*")
        .Call()
        .Or()
        .Pattern("48 89 5C 24 18 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 8B 59 08 45 33 F6 4C 8B EA 4C 8B F9 85 DB 7F")
        .Or()
        .Pattern("48 89 5C 24 18 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 8B 59 08 4C 8B EA 4C 8B F9 85 DB")
        .Or()
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/Engine.Default__ActorComponent"))
        .Find("jmp 0x*")
        .Target()
        .Find("jnz 0x*")
        .Target()
        .Find("mov rcx, [rbp-0x*]; test rcx, rcx; jz 0x*")
        .Call()
        .Get();

    ProcessMulticastDelegate(this, Parameters);
}

UFunction* UObject::FindFunctionChecked(FName InName) const
{
    static auto FindFunctionByName = Anchor::Optional<UFunction* (*)(const UClass*, FName, int32)>(TEXT("UClass::FindFunctionByName"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Find("mov rcx, [rcx+0x10]; mov r8d, 0x1")
        .Call()
        .Get();

    if (FindFunctionByName)
        return FindFunctionByName(GetClass(), InName, 1);

    static auto FindFunctionChecked = Anchor::Function<UFunction* (*)(const UObject*, FName)>(TEXT("UObject::FindFunctionChecked"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Is("mov edx, [0x*]; mov rcx, *; call 0x*; mov r9, [*]; mov rcx, *; mov rdx, rax")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Find("call 0x*")
        .Target()
        .Get();

    return FindFunctionChecked(this, InName);
}

void UObject::ProcessEvent(UFunction* Function, void* Parms)
{
    static auto ProcessEvent = Anchor::VirtualFunction<void (*)(UObject*, UFunction*, void*)>(TEXT("UObject::ProcessEvent"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Is("mov edx, [0x*]; mov rcx, *; call 0x*; mov r9, [*]; mov rcx, *; mov rdx, rax")
        .Find("call [r9+0x*]")
        .Or()
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Find("mov rax, [rcx]; mov *, rcx; mov edx, [0x*]")
        .Find("mov r*, [rax+0x*]")
        .Or()
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Find("jmp [r*+0x*]")
        .Or()
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Find("mov r*, [r*+0x*]")
        .Get();

    ProcessEvent(this, Function, Parms);
}
