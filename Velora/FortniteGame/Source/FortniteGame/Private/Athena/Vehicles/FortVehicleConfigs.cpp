#include "pch.h"
#include "Athena/Vehicles/FortVehicleConfigs.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortVehicleConfigs::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleConfigs"));

    return Class;
}

const TRuntimeSizedArray<FVehicleDamageablePartConfig>& UFortVehicleConfigs::GetDamageableParts() const
{
    static auto DamageableParts = StaticClass()->FindPropertyByName(FName(TEXT("DamageableParts")));

    return *DamageableParts->ContainerPtrToValuePtr<TRuntimeSizedArray<FVehicleDamageablePartConfig>>(this);
}

UScriptStruct* FVehicleDamageablePartConfig::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.VehicleDamageablePartConfig"));

    return Struct;
}

bool FVehicleDamageablePartConfig::GetbStartsDisabled() const
{
    static auto bStartsDisabled = StaticStruct()->FindPropertyByName(FName(TEXT("bStartsDisabled")));

    return bStartsDisabled && *bStartsDisabled->ContainerPtrToValuePtr<bool>(this);
}

FName FVehicleDamageablePartConfig::GetShapeName() const
{
    static auto ShapeName = StaticStruct()->FindPropertyByName(FName(TEXT("ShapeName")));

    return TGameLayout<FName>::Load(ShapeName->ContainerPtrToValuePtr<FName>(this));
}

FName FVehicleDamageablePartConfig::GetBoneName() const
{
    static auto BoneName = StaticStruct()->FindPropertyByName(FName(TEXT("BoneName")));

    return TGameLayout<FName>::Load(BoneName->ContainerPtrToValuePtr<FName>(this));
}

float FVehicleDamageablePartConfig::GetMaxHealth() const
{
    static auto MaxHealth = StaticStruct()->FindPropertyByName(FName(TEXT("MaxHealth")));

    return *MaxHealth->ContainerPtrToValuePtr<float>(this);
}

int32 FVehicleDamageablePartConfig::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}
