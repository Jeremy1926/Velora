#include "pch.h"
#include "UObject/Stack.h"
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
