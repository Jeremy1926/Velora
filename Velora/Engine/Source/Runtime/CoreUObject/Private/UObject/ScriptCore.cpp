#include "pch.h"
#include "UObject/Stack.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FFrame::Step(UObject* Context, void* const Result)
{
    static auto Step = Anchor::Function<void (*)(FFrame*, UObject*, void*)>(TEXT("FFrame::Step"))
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
        .Find("mov rdx, [rdx+0x18]; lea r8, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("48 8B 41 20 4C 8B D2 48 8B D1 44 0F B6 08 48 FF C0 48 89 41 20 41 8B C1 4C 8D 0D ? ? ? ? 49 8B CA 49 FF 24 C1")
        .Get();

    Step(this, Context, Result);
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
        .Get();

    StepExplicitProperty(this, Result, Property);
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
        .Call(2)
        .Or()
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/Engine.Default__ActorComponent"))
        .Find("mov rcx, [rsp+0x*]; test rcx, rcx; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Call()
        .Get();

    ProcessMulticastDelegate(this, Parameters);
}
