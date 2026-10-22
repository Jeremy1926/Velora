#include "pch.h"
#include "AI/FortAthenaAIRuntimeParametersComponent.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortAthenaAIRuntimeParametersComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIRuntimeParametersComponent"));

    return Class;
}

UObject* UFortAthenaAIRuntimeParametersComponent::GetOrCreateAIRuntimeParameters(UClass* RuntimeParametersClass, int32 SkillLevel, uint8 ConflictBehavior)
{
    static auto GetOrCreateAIRuntimeParameters = Anchor::Function<UObject* (*)(UFortAthenaAIRuntimeParametersComponent*, UClass* const*, int32, uint8, uint32*)>(TEXT("UFortAthenaAIRuntimeParametersComponent::GetOrCreateAIRuntimeParameters"))
        .String(TEXT("Need a AIRuntimeParametersComponent to retrieve skillset on AI %s"))
        .Start()
        .Find("lea r9d, [*+0x1]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetOrCreateAIRuntimeParameters(this, &RuntimeParametersClass, SkillLevel, ConflictBehavior, nullptr);
}
