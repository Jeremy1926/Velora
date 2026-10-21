#pragma once

#include "Player/FortPlayerStatePvP.h"

class AActor;
class UScriptStruct;
struct FGameplayTagContainer;
struct FVector;
enum class EDeathCause : uint8;

struct FDeathInfo
{
    FDeathInfo();

    FDeathInfo(const FDeathInfo&) = delete;

    ~FDeathInfo();

    static UScriptStruct* StaticStruct();

    AActor*& GetFinisherOrDowner();

    AActor** GetDowner();

    void SetDBNO(bool bInDBNO);

    EDeathCause& GetDeathCause();

    float* GetDistance();

    FVector* GetDeathLocation();

    void SetInitialized(bool bInInitialized);

    FGameplayTagContainer* GetDeathTags();

    FGameplayTagContainer* GetFinisherOrDownerTags();

    FGameplayTagContainer* GetVictimTags();

private:
    uint8 Data[0xC0];
};

class AFortPlayerStateAthena : public AFortPlayerStatePvP
{
public:
    static UClass* StaticClass();

    static EDeathCause ToDeathCause(const FGameplayTagContainer& InTags, bool bWasDBNO);

    FDeathInfo& GetDeathInfo();

    void InitializeDeathInfo(const FDeathInfo& NewDeathInfo);

    uint8 GetSquadId() const;

    uint8 GetTeamIndex() const;

    void PlayerPawnRebooted();

    int32& GetRebootCounter();
};
