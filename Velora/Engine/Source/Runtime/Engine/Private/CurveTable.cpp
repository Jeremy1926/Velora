#include "pch.h"
#include "Engine/CurveTable.h"
#include "Containers/UnrealString.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

float FRealCurve::Eval(float InTime, float InDefaultValue) const
{
    static auto Eval = Anchor::VirtualFunction<float (*)(const FRealCurve*, float, float)>(TEXT("FRealCurve::Eval"))
        .Pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ? 65 48 8B 04 25 58 00 00 00 48 8D 35 ? ? ? ? 48 8B F9 0F 29 74 24 ? B9 ? ? ? ? 49 8B D8 0F 28 F1")
        .Call()
        .Find("xorps xmm2, xmm2; movaps xmm1, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Or()
        .String(TEXT("FScalableFloat::GetValueAtLevel"))
        .Start()
        .Call()
        .Find("xorps xmm2, xmm2; movaps xmm1, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    return Eval(this, InTime, InDefaultValue);
}

UClass* UCurveTable::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.CurveTable"));

    return Class;
}

FRealCurve* UCurveTable::FindCurve(FName RowName, const FString& ContextString, bool bWarnIfNotFound) const
{
    static auto FindCurve = Anchor::Function<FRealCurve* (*)(const UCurveTable*, FName, const FString&, bool)>(TEXT("UCurveTable::FindCurve"))
        .If(!::Anchor::Exists().String(TEXT("UCurveTable::FindCurve : Row '%s' not found in CurveTable '%s' (%s).")).Start().Has("mov r10, [rcx]; lea rax, [0x*]").Get())
        .String(TEXT("UCurveTable::FindCurve : Row '%s' not found in CurveTable '%s' (%s)."))
        .Start()
        .Or()
        .String(TEXT("UCurveTable::FindCurve : Row '%s' not found in CurveTable '%s' (%s)."), "lea rdx, [0x*]; lea rcx, [0x*]; call 0x*; lea rcx, [rsp+0x*]")
        .Start()
        .Is("mov [rsp+0x8], rbx; mov [rsp+0x18], rsi; mov [rsp+0x10], edx")
        .Get();

    return FindCurve(this, RowName, ContextString, bWarnIfNotFound);
}
