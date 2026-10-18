#pragma once

#include "Items/FortWorldItemDefinition.h"
#include "Engine/DataTable.h"

class UCurveTable;

struct FFortBaseWeaponStats : public FTableRowBase
{
    static UScriptStruct* StaticStruct();

    int32 BaseLevel;
    float DmgPB;
    float DmgMid;
    float DmgLong;
    float DmgMaxRange;
    float EnvDmgPB;
    float EnvDmgMid;
    float EnvDmgLong;
    float EnvDmgMaxRange;
    float ImpactDmgPB;
    float ImpactDmgMid;
    float ImpactDmgLong;
    float ImpactDmgMaxRange;
    bool bForceControl;
    float RngPB;
    float RngMid;
    float RngLong;
    float RngMax;
    UCurveTable* DmgScaleTable;
    FName DmgScaleTableRow;
    float DmgScale;
    UCurveTable* EnvDmgScaleTable;
    FName EnvDmgScaleTableRow;
    float EnvDmgScale;
    UCurveTable* ImpactDmgScaleTable;
    FName ImpactDmgScaleTableRow;
    float ImpactDmgScale;
    FName SurfaceRatioRowName;
    float DamageZone_Light;
    float DamageZone_Normal;
    float DamageZone_Critical;
    float DamageZone_Vulnerability;
    float KnockbackMagnitude;
    float MidRangeKnockbackMagnitude;
    float LongRangeKnockbackMagnitude;
    float KnockbackZAngle;
    float StunTime;
    float StunScale;
    UDataTable* Durability;
    FName DurabilityRowName;
    float DurabilityScale;
    float DurabilityPerUse;
    float DiceCritChance;
    float DiceCritDamageMultiplier;
    float ReloadTime;
    float ReloadScale;
    bool bReloadIndividualBullets;
    bool bReloadInterruptIsImmediate;
    int32 ClipSize;
    float ClipScale;
    int32 InitialClips;
    int32 CartridgePerFire;
    int32 AmmoCostPerFire;
    int32 MaxAmmoCostPerFire;
    float MinChargeTime;
    float MaxChargeTime;
    float ChargeDownTime;
    float MinChargeDamageMultiplier;
    float MaxChargeDamageMultiplier;
};

class UFortWeaponItemDefinition : public UFortWorldItemDefinition
{
public:
    static UClass* StaticClass();

    template <class T>
    T* GetWeaponStats() const
    {
        return GetWeaponStatHandle().GetRow<T>(TEXT("UFortWeaponItemDefinition::GetWeaponStats"));
    }

    int32 GetBaseClipSizeWithMods(const FFortBaseWeaponStats* WeaponStats, int32 Level) const;

    const FDataTableRowHandle& GetWeaponStatHandle() const;
};
