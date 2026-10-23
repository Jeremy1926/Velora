#pragma once

#include "Player/FortPlayerStatePvP.h"

class AActor;
class AFortPlayerPawnAthena;
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

    int32* GetDeathCircumstance();

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

    static int32 ToDeathCircumstance(AFortPlayerPawnAthena* Pawn);

    FDeathInfo& GetDeathInfo();

    void InitializeDeathInfo(const FDeathInfo& NewDeathInfo);

    void ClientReportKill(AFortPlayerStateAthena* Player);

    void ClientReportTeamKill(int32 TeamKills);

    int32& GetKillScore();

    int32& GetTeamKillScore();

    uint8 GetSquadId() const;

    uint8 GetTeamIndex() const;

    int32 GetPlace() const;

    void PlayerPawnRebooted();

    int32& GetRebootCounter();

    bool IsTeamOutOfGame() const;
};
