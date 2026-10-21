#pragma once

#include "Player/FortPlayerStatePvP.h"

struct FGameplayTagContainer;
enum class EDeathCause : uint8;

class AFortPlayerStateAthena : public AFortPlayerStatePvP
{
public:
    static UClass* StaticClass();

    static EDeathCause ToDeathCause(const FGameplayTagContainer& InTags, bool bWasDBNO);

    uint8 GetSquadId() const;

    uint8 GetTeamIndex() const;

    void PlayerPawnRebooted();

    int32& GetRebootCounter();
};
