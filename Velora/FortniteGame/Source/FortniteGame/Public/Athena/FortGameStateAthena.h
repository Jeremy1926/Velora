#pragma once

#include "FortGameStateZone.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameplayTagContainer.h"
#include "Math/Vector.h"
#include "UObject/ScriptDelegates.h"

class AFortAthenaMapInfo;
class AFortPawn;
class AFortVolumeManager;
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

    EAthenaGamePhase GetGamePhase() const;

    int32 GetTeamsLeft() const;

    EAthenaGamePhaseStep GetGamePhaseStep() const;

    int32 GetSafeZonePhase() const;

    const void* GetSafeZoneInterface() const;

    EFortSafeZoneState GetSafeZoneState() const;

    FVector GetSafeZoneCenter() const;

    float GetSafeZoneRadius() const;

    UObject* GetCurrentPlaylist() const;

    int32 GetAvgPlayerMMR() const;

    AFortAthenaMapInfo* GetMapInfo() const;

    void OnPlaylistLootTableAssetsDataLoadCompleted();

    FGameplayTagContainer GetAthenaPlaylistContextTags() const;

    FMulticastScriptDelegate& GetOnCurrentPlaylistLoaded();

    FGameMemberInfoArray& GetGameMemberInfoArray();

    bool GetPlaylistDataIsLoaded() const;

    AFortVolumeManager* GetVolumeManager() const;

    void SetCurrentPlaylistName(FName PlaylistName);

    void OnRep_CurrentPlaylistData();

    void OnRep_CurrentPlaylistInfo();

    void StreamAdditionalPlaylistLevels();

    static bool AmmoDroppedFromContainersForWeaponPickup();

    bool AdjustedGravityMultiplier(AFortPawn* Pawn, float& NewGravityVal, float& NewVehicleGravityVal, float& NewJumpZ, float& NewJumpHorizontalAcceleration, float& NewJumpHorizontalVelocity);
};
