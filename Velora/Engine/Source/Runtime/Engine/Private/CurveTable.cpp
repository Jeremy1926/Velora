#include "pch.h"
#include "Engine/CurveTable.h"
#include "Containers/UnrealString.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

float FRealCurve::Eval(float InTime, float InDefaultValue) const
{
    static auto Eval = Anchor::VirtualFunction<float (*)(const FRealCurve*, float, float)>(TEXT("FRealCurve::Eval"))
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
        .String(TEXT("UCurveTable::FindCurve : Row '%s' not found in CurveTable '%s' (%s)."))
        .Start()
        .Get();

    return FindCurve(this, RowName, ContextString, bWarnIfNotFound);
}
