#include "pch.h"
#include "Player/FortControllerRequirement.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortControllerRequirement::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerRequirement"));

    return Class;
}

bool UFortControllerRequirement::IsRequirementMet(const AController* Controller) const
{
    static auto IsRequirementMet = Anchor::Function<bool (*)(const UFortControllerRequirement*, const AController*)>(TEXT("UFortControllerRequirement::IsRequirementMet"))
        .Native(TEXT("/Script/FortniteGame.FortControllerRequirement:IsRequirementMet"))
        .Get();

    return IsRequirementMet(this, Controller);
}
