#pragma once

#include "Components/ActorComponent.h"

class UObject;

enum class EAthenaGamePhase : uint8;
enum class EAthenaGamePhaseStep : uint8;

class UFortGameStateComponent_BattleRoyaleGamePhaseLogic : public UActorComponent
{
public:
    static UClass* StaticClass();

    static UFortGameStateComponent_BattleRoyaleGamePhaseLogic* Get(const UObject* WorldContextObject);

    EAthenaGamePhase GetGamePhase() const;

    EAthenaGamePhaseStep GetGamePhaseStep() const;

    void SetSafeZonesStartTime(float InSafeZonesStartTime);
};
