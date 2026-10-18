#include "pch.h"
#include "FortGameDeathPenalty.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortGameDeathPenalty::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameDeathPenalty"));

    return Class;
}

float UFortGameDeathPenalty::GetSelfResurrectDurabilityPenalty() const
{
    static auto SelfResurrectDurabilityPenalty = GetClass()->FindPropertyByName(FName(TEXT("SelfResurrectDurabilityPenalty")));

    return *SelfResurrectDurabilityPenalty->ContainerPtrToValuePtr<float>(this);
}

float UFortGameDeathPenalty::GetTeammateResurrectDurabilityPenalty() const
{
    static auto TeammateResurrectDurabilityPenalty = GetClass()->FindPropertyByName(FName(TEXT("TeammateResurrectDurabilityPenalty")));

    return *TeammateResurrectDurabilityPenalty->ContainerPtrToValuePtr<float>(this);
}
