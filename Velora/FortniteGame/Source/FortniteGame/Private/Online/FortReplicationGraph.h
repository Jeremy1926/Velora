#pragma once

#include "ReplicationGraph.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "ReplicationGraphTypes.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "UObject/WeakObjectPtr.h"

class ABuildingActor;
class ABuildingContainer;
class ABuildingGameplayActor;
class ABuildingGameplayActorSpawnChip;
class AFortAthenaExitCraft;
class AFortGameModeAthena;
class AFortInventory;
class AFortLiveSpectatorController;
class AFortPawn;
class AFortPickup;
class AFortPlayerPawn;
class AFortPlayerPetRepState;
class AFortPlayerState;
class AFortPlayerStateAthena;
class AFortTeamHealthInfo;
class AFortTeamPrivateInfo;
class AFortWeapon;
class APlayerState;
class UFortReplicationGraphNode_AlwaysRelevantForSquad;
class UFortReplicationGraphNode_AlwaysRelevantForTeam;
class UNetConnection;
class UScriptStruct;
struct FFrame;

enum class EClassRepNodeMapping : uint32
{
    NotRouted,
    RelevantAllConnections,
    RelevantAllInsideFortVolume,
    Custom,
    Spatialize_Static,
    Spatialize_Dynamic,
    Spatialize_Dormancy,
    Instance_Dynamic,
};

struct FRepGraphActorSettingsBase
{
    static UScriptStruct* StaticStruct();

    bool GetAddClassRepInfoToMap() const;

    bool GetUseCustomClassRepInfo() const;

    bool GetAddToRPC_Multicast_OpenChannelForClassMap() const;

    bool GetRPC_Multicast_OpenChannelForClass() const;

    EClassRepNodeMapping GetClassNodeMapping() const;

    FClassReplicationInfo& GetClassRepInfo();
};

struct FRepGraphActorClassSettings : public FRepGraphActorSettingsBase
{
    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    FName GetActorClassName() const;

    UClass* GetStaticActorClass() const;
};

template<>
class TArray<FRepGraphActorClassSettings, FDefaultAllocator> : public TRuntimeSizedArray<FRepGraphActorClassSettings>
{
};

class UGlobalRepGraphActorClassSettings : public UObject
{
public:
    static UClass* StaticClass();

    TArray<FRepGraphActorClassSettings>& GetClassSettings();
};

class UFortReplicationGraphConnection : public UNetReplicationGraphConnection
{
public:
    static UClass* StaticClass();
};

class UFortReplicationGraph : public UReplicationGraph
{
public:
    static UClass* StaticClass();

    UReplicationGraphNode*& GetPlayerStateLimiterNode();

    UReplicationGraphNode*& GetReplayPlayerStateLimiterNode();

    UReplicationGraphNode_GridSpatialization2D*& GetRootGridNode();

    UReplicationGraphNode_ActorList*& GetAlwaysRelevantNode();

    UReplicationGraphNode*& GetLiveSpectatorRelevancyNode();

    TMap<FName, FActorRepListRefView>& GetAlwaysRelevantStreamingLevelActors();

    void InitializeForWorld(UWorld* World);

    void InitConnectionGraphNodes(UNetReplicationGraphConnection* ConnectionManager);

    void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo);

    void InitGlobalActorClassSettings();

    void InitGlobalGraphNodes();

    void InitDynamicSpatializationSettings();

    void InitForPlaylist(AFortGameModeAthena* GameMode);

    void OnGameStatePlaylistLoaded(FName PlaylistName, const FGameplayTagContainer& GameplayTags);

    void execOnGameStatePlaylistLoaded(FFrame& Stack, void* const Z_Param__Result);

    void OnAIClassAdditionRequest(TSubclassOf<AFortPawn> NewAIClass);

    void OnLiveSpectatorJoined(AFortLiveSpectatorController* PC);

    void OnNotifyAddReplicatedNonPlayerInventory(AActor* ParentActor, AFortInventory* Inventory);

    void OnNotifyRemoveReplicatedNonPlayerInventory(AActor* ParentActor, AFortInventory* Inventory);

    UFortReplicationGraphNode_AlwaysRelevantForTeam* GetNodeForTeamInfo(AFortTeamPrivateInfo* TeamInfo);

    void OnPlayerAddedTeam(AFortTeamPrivateInfo* TeamInfo, AFortPlayerState* PS);

    void OnPlayerRemovedTeam(AFortTeamPrivateInfo* TeamInfo, AFortPlayerState* PS);

    UFortReplicationGraphNode_AlwaysRelevantForSquad* GetNodeForSquad(uint8 SquadID);

    void OnPlayerStateSquadAssign(AFortPlayerStateAthena* PS, uint8 PreviousSquadId);

    void OnSquadSpecificContainerSpawned(int32 SquadId, ABuildingContainer* Container);

    void OnSquadSpecificContainerDestroyed(int32 SquadId, ABuildingContainer* Container);

    void OnRebootCardSpawned(AFortPlayerStateAthena* PS, ABuildingGameplayActorSpawnChip* Card);

    void OnRebootCardDestroyed(AFortPlayerStateAthena* PS, ABuildingGameplayActorSpawnChip* Card);

    void OnPawnWeaponChange(AFortPawn* Pawn, AFortWeapon* NewWeapon, AFortWeapon* OldWeapon);

    void OnPetStateChange(AFortPlayerPawn* Pawn, AFortPlayerPetRepState* NewPet, AFortPlayerPetRepState* OldPet);

    void OnBalloonStateChange(AFortPlayerPawn* Pawn, ABuildingGameplayActor* NewBalloon, ABuildingGameplayActor* OldBalloon);

    void OnPossessedPropStateChange(AFortPlayerPawn* Pawn, AActor* NewPossessedProp, AActor* OldPossessedProp);

    void OnCullDistSqChange(AActor* Actor, float DistSq);

    void OnPawnCullDistSqChange(AFortPawn* Pawn, float DistSq);

    void OnExitVehicleCullDistChange(AFortAthenaExitCraft* Craft, float DistSq);

    void OnPickupCullDistanceChanged(AFortPickup* Pickup, float Dist);

    void OnBuildingDistSqChange(ABuildingActor* Building, float Dist);

    void OnActorNetCullDistanceChange(AActor* Actor);

    void OnVehicleChanged(AFortPlayerPawn* Pawn, AActor* NewVehicle, AActor* PrevVehicle);

    void OnPlayerPawnHighFrequencyChange(AFortPlayerPawn* Pawn, bool bHighFreq);

private:
    void EnableDynamicSpatialFrequencyNode(bool bEnable);

    void AddClassRepInfo(UClass* Class, EClassRepNodeMapping Mapping);

    EClassRepNodeMapping GetClassNodeMapping(UClass* Class);

    void RegisterClassRepNodeMapping(UClass* Class);

    bool ConditionalInitClassReplicationInfo(UClass* ReplicatedClass, FClassReplicationInfo& ClassInfo);

    void InitClassReplicationInfo(FClassReplicationInfo& Info, UClass* Class, bool bSpatialize);

    EClassRepNodeMapping GetMappingPolicy(UClass* Class, AActor* Actor);

    EClassRepNodeMapping GetInstanceDynamicRepNodeMapping(AActor* Actor);

    void AddRelevantToAllConnections(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo);

    void AddRelevantToAllInsideFortVolume(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo);

    void OnAlwaysRelevantStreamingActorAdded(AActor* Actor, FGlobalActorReplicationInfo& GlobalInfo);

    void SetAllSquadNodesEnabled(bool bEnabled);

    TMap<UClass*, UReplicationGraphNode*>& GetClassToNodetoRouteTo();

    bool& GetDisableSquadNodes();

    bool& GetDisableHighFrequencyPawnRouting();

    TClassMap<EClassRepNodeMapping>& GetClassRepNodePolicies();

    TMap<FObjectKey, UFortReplicationGraphNode_AlwaysRelevantForTeam*>& GetFortTeamPrivateInfoMap();

    TMap<uint8, UFortReplicationGraphNode_AlwaysRelevantForSquad*>& GetSquadMap();

    bool& GetSpecialRelevancyEnabled();

    TArray<UClass*>& GetExplicitlySetClasses();

    UGlobalRepGraphActorClassSettings*& GetGlobalActorClassSettings();
};

struct FFortAlwaysRelevantActorInfo
{
    bool operator==(const UNetConnection* InConnection) const
    {
        return Connection == InConnection;
    }

    const UNetConnection* Connection;
    AActor* LastPawn;
    AActor* LastTetherPawn;
};

class UFortReplicationGraphNode_AlwaysRelevant_ForConnection : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void OnClientLevelVisibilityAdd(FName LevelName, UWorld* StreamingWorld);

    void FlushStreamingAlwaysRelevantList(const FNewReplicatedActorInfo& ActorInfo);

    void OnClientLevelVisibilityRemove(FName LevelName);

private:
    uint8* GetAlwaysRelevantStreamingLevelsNeedingReplication();

    int32& GetNumAlwaysRelevantStreamingLevelsNeedingReplication();

    FActorRepListRefView& GetReplicationActorList();

    TArray<FFortAlwaysRelevantActorInfo>& GetPastRelevantActors();

    bool& GetInitializedPlayerState();
};

class UFortReplicationGraphNode_AlwaysRelevantForSquad : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    uint8& GetSquadID();

    bool& GetSpecialRelevancyEnabled();

    TArray<AFortPlayerStateAthena*>& GetPlayerStates();

    TArray<ABuildingGameplayActorSpawnChip*>& GetRebootCards();

    TArray<ABuildingContainer*>& GetContainers();

    FActorRepListRefView& GetReplicationActorList();
};

class UFortReplicationGraphNode_AlwaysRelevantForTeam : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    AFortTeamPrivateInfo*& GetFortTeamPrivateInfo();

    FActorRepListRefView& GetReplicationActorList();
};

class UFortReplicationGraphNode_Custom : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void InitializeNode(UWorld* World);
};

class UFortReplicationGraphNode_AlwaysRelevantHealthForTeam : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    AFortTeamHealthInfo*& GetFortTeamHealthInfo();

    TMap<FObjectKey, TArray<TWeakObjectPtr<AFortTeamHealthInfo>>>& GetReplicationActorMap();

    FActorRepListRefView& GetReplicationActorList();
};

class UFortReplicationGraphNode_PlayerStateFrequencyLimiter : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void InitNode(UWorld* World);

    void OnPlayerStateAdded(APlayerState* AddedPlayerState);

    void OnPlayerStateRemoved(APlayerState* RemovedPlayerState);

    int32& GetMaxStatesToReplicatePerFrame();

private:
    TMap<APlayerState*, int32>& GetPlayerStateStoredListIndexMap();

    TArray<FActorRepListRefView>& GetPerFramePlayerStateLists();

    int32& GetCurrentFrameListIndex();

    TBitArray<>& GetActiveLists();
};

enum class EFortDynamicNodeType : uint8
{
    AI,
    PhysicsObject,
    Vehicle,
    NumTypes,
};

class UFortReplicationGraphNode_DynamicSpatialFrequency : public UReplicationGraphNode_DynamicSpatialFrequency
{
public:
    static UClass* StaticClass();

    static TClassMap<bool>& GetAIClasses();

    static FSettings* GetNodeSettings();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    void SetupClassToNodeMap();

protected:
    void GatherActors(const FActorRepListRefView& RepList, FGlobalActorReplicationInfoMap& GlobalMap, FPerConnectionActorInfoMap& ConnectionMap, const FConnectionGatherActorListParameters& Params, UNetConnection* NetConnection);

private:
    EFortDynamicNodeType& GetCurrentDynamicNodeType();

    TMap<EFortDynamicNodeType, TWeakObjectPtr<UFortReplicationGraphNode_DynamicSpatialFrequency>>& GetDynamicTypeToNodeMap();
};

class UFortReplicationGraphNode_LiveSpectator : public UReplicationGraphNode_ActorList
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    void GatherBroadcasterActorListsForConnection(const FConnectionGatherActorListParameters& Params);

    FActorRepListRefView& GetBroadcasterRelevancyActorList();

    TArray<FFortAlwaysRelevantActorInfo>& GetPastViewedTargets();
};

class UReplicationGraphNode_FortVolumeGrid : public UReplicationGraphNode
{
public:
    static UClass* StaticClass();

    void GatherActorListsForConnection(const FConnectionGatherActorListParameters& Params);

private:
    bool& GetIsWaitingNewStreamingLevel();

    UReplicationGraphNode*& GetFortVolumeGrid2D();

    UReplicationGraphNode*& GetFortVolumeGlobalRelevancyNode();
};
