#include "pch.h"
#include "AI/FortAthenaAIRuntimeParameters_Inventory.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortAthenaAIRuntimeParameters_Inventory::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaAIRuntimeParameters_Inventory"));

    return Class;
}

bool UFortAthenaAIRuntimeParameters_Inventory::GetbShouldDropCurrencyOnDeath() const
{
    static auto bShouldDropCurrencyOnDeath = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bShouldDropCurrencyOnDeath")));

    return bShouldDropCurrencyOnDeath && bShouldDropCurrencyOnDeath->GetPropertyValue_InContainer(this);
}

FName UFortAthenaAIRuntimeParameters_Inventory::GetCurrencyPayoutRowName() const
{
    static auto CurrencyPayoutRowName = GetClass()->FindPropertyByName(FName(TEXT("CurrencyPayoutRowName")));

    return CurrencyPayoutRowName ? *CurrencyPayoutRowName->ContainerPtrToValuePtr<FName>(this) : FName(NAME_None);
}
