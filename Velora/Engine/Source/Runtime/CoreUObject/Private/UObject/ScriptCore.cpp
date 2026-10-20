#include "pch.h"
#include "UObject/Stack.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void FFrame::Step(UObject* Context, void* const Result)
{
    static auto Step = Scanner::Function<void (*)(FFrame*, UObject*, void*)>(Scanner::Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid")).Find("mov rdx, [rdx+0x18]; lea r8, *; call 0x*").Find("call 0x*").Target(TEXT("FFrame::Step")));

    Step(this, Context, Result);
}

void FFrame::StepExplicitProperty(void* const Result, UProperty* Property)
{
    static auto StepExplicitProperty = Scanner::Function<void (*)(FFrame*, void*, UProperty*)>(Scanner::Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid")).Find("mov [rdx+0x80], rax; lea rdx, *; call 0x*").Find("call 0x*").Target(TEXT("FFrame::StepExplicitProperty")));

    StepExplicitProperty(this, Result, Property);
}

template<>
template<>
void TMulticastScriptDelegate<FWeakObjectPtr>::ProcessMulticastDelegate<UObject>(void* Parameters) const
{
    static auto ProcessMulticastDelegate = []
    {
        auto Deactivate = Scanner::Exec(TEXT("/Script/Engine.ActorComponent:Deactivate")).Find("jmp [rax+0x*]").Slot(TEXT("UActorComponent::Deactivate"));
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__ActorComponent"));

        return Scanner::Function<void (*)(const TMulticastScriptDelegate<FWeakObjectPtr>*, void*)>(Scanner::At(Scanner::Virtual<const void*>(DefaultObject, Deactivate)).Call(2).Get(TEXT("TMulticastScriptDelegate::ProcessMulticastDelegate")));
    }();

    ProcessMulticastDelegate(this, Parameters);
}
