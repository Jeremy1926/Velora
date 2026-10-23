#pragma once

#include "FortGameStateZone.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameplayTagContainer.h"
#include "Delegates/Delegate.h"
#include "Math/Vector.h"
#include "UObject/ScriptDelegates.h"

class AController;
class APlayerState;
class AFortAthenaMapInfo;
class AFortBroadcastSpectatorInfo;
class AFortPawn;
class AFortSafeZoneIndicator;
class AFortVolumeManager;
class UFortWorldItemDefinition;
class UScriptStruct;

enum class EAthenaGamePhase : uint8
{
    None,
    Setup,
    Warmup,
    Aircraft,
    SafeZones,
    EndGame,
};

enum class EAthenaGamePhaseStep : uint8;

enum class EFortSafeZoneState : uint8
{
    None,
    Starting,
    Holding,
    Shrinking,
};

struct FGameMemberInfo : public FFastArraySerializerItem
{
    uint8 SquadId;
    uint8 TeamIndex;
    FUniqueNetIdRepl MemberUniqueId;
};

struct FGameMemberInfoArray : public FFastArraySerializer
{
    static UScriptStruct* StaticStruct();

    TArray<FGameMemberInfo>& GetMembers();
};

class AFortGameStateAthena : public AFortGameStateZone
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, APlayerState*>& GetNotifyOnPlayerStateAdded();

    static TMulticastDelegate<void, APlayerState*>& GetNotifyOnPlayerStateRemoved();

    EAthenaGamePhase GetGamePhase() const;

    int32 GetTeamsLeft() const;

    EAthenaGamePhaseStep GetGamePhaseStep() const;

    int32 GetSafeZonePhase() const;

    AFortSafeZoneIndicator* GetSafeZoneIndicator() const;

    AFortBroadcastSpectatorInfo* GetBroadcastSpectatorInfo() const;

    float GetSafeZonePauseTime() const;

    bool IsSafeZoneTimerPaused() const;

    void SetSafeZoneTimerPaused(bool bPaused);

    void StartPlaylistTimer(float Seconds, bool bOption);

    const void* GetSafeZoneInterface() const;

    EFortSafeZoneState GetSafeZoneState() const;

    FVector GetSafeZoneCenter() const;

    float GetSafeZoneRadius() const;

    UObject* GetCurrentPlaylist() const;

    int32 GetAvgPlayerMMR() const;

    AFortAthenaMapInfo* GetMapInfo() const;

    void OnPlaylistLootTableAssetsDataLoadCompleted();

    void ApplyPlaylistDataTables();

    FGameplayTagContainer GetAthenaPlaylistContextTags() const;

    FMulticastScriptDelegate& GetOnCurrentPlaylistLoaded();

    FMulticastScriptDelegate& GetGamePhaseStepChanged();

    FGameMemberInfoArray& GetGameMemberInfoArray();

    bool GetPlaylistDataIsLoaded() const;

    bool GetIsLargeTeamGame() const;

    bool IsCreativeOrPlaygroundModePlaylist() const;

    bool IsBattleLabModePlaylist() const;

    AFortVolumeManager* GetVolumeManager() const;

    void SetCurrentPlaylistName(FName PlaylistName);

    void OnRep_CurrentPlaylistData();

    void OnRep_CurrentPlaylistInfo();

    void StreamAdditionalPlaylistLevels();

    static bool AmmoDroppedFromContainersForWeaponPickup();

    bool AdjustedGravityMultiplier(AFortPawn* Pawn, float& NewGravityVal, float& NewVehicleGravityVal, float& NewJumpZ, float& NewJumpHorizontalAcceleration, float& NewJumpHorizontalVelocity);

    void ShouldDropOrDestroyItem(bool* bCurrentCanDropValue, bool* bCurrentCanDestroyValue, AController* Controller, const UFortWorldItemDefinition* ItemDef);
};
