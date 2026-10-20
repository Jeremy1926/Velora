#pragma once

#include "FortGameStateZone.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/OnlineReplStructs.h"

class AFortAthenaMapInfo;
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

    AFortAthenaMapInfo* GetMapInfo() const;

    FGameMemberInfoArray& GetGameMemberInfoArray();

    void SetCurrentPlaylistName(FName PlaylistName);

    void OnRep_CurrentPlaylistData();

    void OnRep_CurrentPlaylistInfo();

    void StreamAdditionalPlaylistLevels();
};
