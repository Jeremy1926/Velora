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

int32 FVehicleDamageablePartConfig::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}
