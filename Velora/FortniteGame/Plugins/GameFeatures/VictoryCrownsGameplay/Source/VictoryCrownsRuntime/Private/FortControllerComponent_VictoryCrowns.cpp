#include "pch.h"
#include "FortControllerComponent_VictoryCrowns.h"
#include "FortGameStateComponent_VictoryCrowns.h"
#include "Player/FortPlayerController.h"
#include "Player/FortPlayerControllerZone.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Pawns/FortPlayerPawn.h"
#include "Items/FortWorldItem.h"
#include "Items/FortWorldItemDefinition.h"
#include "Online/FortMcpProfile.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "FortKismetLibrary.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortControllerComponent_VictoryCrowns::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns"));

    return Class;
}

void UFortControllerComponent_VictoryCrowns::BeginPlay()
{
    DETOUR(UFortControllerComponent_VictoryCrowns, BeginPlay, Anchor::Hook()
        .If(StaticClass())
        .VirtualMatch(TEXT("/Script/Engine.Default__ActorComponent"), "mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rax, [rcx+0x10]; mov rdi, rcx; mov ecx, [rax+0x*]; bt ecx, 0x12")
        .LogSlot(TEXT("UActorComponent::BeginPlay"))
        .InVTable(TEXT("/Script/VictoryCrownsRuntime.Default__FortControllerComponent_VictoryCrowns")));

    Original();

    auto PlayerController = GetPlayerController();

    if (!PlayerController)
        return;

    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("HandlePawnChanged")));

    PlayerController->GetOnPossessedPawnChanged().AddUnique(Delegate);

    if (auto PlayerControllerZone = Cast<AFortPlayerControllerZone>(PlayerController))
        PlayerControllerZone->GetOnPlayerPlaceChanged().AddUObject(this, &UFortControllerComponent_VictoryCrowns::HandlePlayerPlaceChanged);

    if (auto Pawn = PlayerController->GetPawn())
        HandlePawnChanged(nullptr, Pawn);

    if (auto GameStateComponent = UFortGameStateComponent_VictoryCrowns::Get(this))
        GameStateComponent->AddPlayerState(Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()));

    BindOnAthenaProfileInitialized();
}

void UFortControllerComponent_VictoryCrowns::execHandlePawnChanged(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_VictoryCrowns, execHandlePawnChanged, Anchor::Hook()
        .If(StaticClass())
        .InExec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:HandlePawnChanged")));

    APawn* OldPawn = nullptr;
    APawn* NewPawn = nullptr;

    Stack.StepCompiledIn(&OldPawn);
    Stack.StepCompiledIn(&NewPawn);
    Stack.Code += !!Stack.Code;

    HandlePawnChanged(OldPawn, NewPawn);
}

void UFortControllerComponent_VictoryCrowns::execOnAthenaProfileInitialized(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_VictoryCrowns, execOnAthenaProfileInitialized, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:OnAthenaProfileInitialized"))
        .Optional()
        .InExec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:OnAthenaProfileInitialized")));

    Stack.Code += !!Stack.Code;

    OnAthenaProfileInitialized();
}

void UFortControllerComponent_VictoryCrowns::execCacheAccountItemData(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortControllerComponent_VictoryCrowns, execCacheAccountItemData, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:CacheAccountItemData"))
        .Optional()
        .InExec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:CacheAccountItemData")));

    Stack.Code += !!Stack.Code;

    CacheAccountItemData();
}

void UFortControllerComponent_VictoryCrowns::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("HandleEnterAircraft")));

    if (auto OldPlayerPawn = Cast<AFortPlayerPawn>(OldPawn))
        OldPlayerPawn->GetOnEnteredAircraft().Remove(Delegate);

    auto NewPlayerPawn = Cast<AFortPlayerPawn>(NewPawn);

    if (!NewPlayerPawn)
        return;

    NewPlayerPawn->GetOnEnteredAircraft().AddUnique(Delegate);

    if (GetShouldGrantCrownOnSpawn())
        GrantVictoryCrownItem();
}

void UFortControllerComponent_VictoryCrowns::HandlePlayerPlaceChanged()
{
    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetOwner());

    if (!PlayerController)
        return;

    if (PlayerController->HasAuthority() && !GetWonCrownInMatch() && CurrentPlaylistSupportsVictoryCrowns() && WasOwnerQualifyingWinner())
    {
        auto& VictoryCrownAccountData = GetVictoryCrownAccountData();
        const bool bWonRoyalRoyale = GetCrownInPlayerInventory() != nullptr;

        VictoryCrownAccountData.has_victory_crown = true;
        VictoryCrownAccountData.total_victory_crowns_bestowed_count++;

        if (bWonRoyalRoyale)
            VictoryCrownAccountData.total_royal_royales_achieved_count++;

        SetWonCrownInMatch(true);
        SetWonRoyalRoyale(bWonRoyalRoyale);
    }

    PlayerController->ForceNetUpdate();
}

bool UFortControllerComponent_VictoryCrowns::WasOwnerQualifyingWinner() const
{
    static auto MaxRankingForCrownAward = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/VictoryCrownsRuntime.VictoryCrownPlaylistData")))->FindPropertyByName(FName(TEXT("MaxRankingForCrownAward")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());
    auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;
    auto RowData = PlayerState ? FindCurrentPlaylistRow() : nullptr;
    const int32 Place = PlayerState ? PlayerState->GetPlace() : 0;

    return RowData && Place > 0 && Place <= *MaxRankingForCrownAward->ContainerPtrToValuePtr<int32>(RowData);
}

void UFortControllerComponent_VictoryCrowns::SetWonCrownInMatch(bool bNewWonCrownInMatch)
{
    if (bNewWonCrownInMatch != GetWonCrownInMatch())
        GetWonCrownInMatch() = bNewWonCrownInMatch;
}

void UFortControllerComponent_VictoryCrowns::SetWonRoyalRoyale(bool bNewWonRoyalRoyale)
{
    if (bNewWonRoyalRoyale != GetWonRoyalRoyale())
        GetWonRoyalRoyale() = bNewWonRoyalRoyale;
}

void UFortControllerComponent_VictoryCrowns::BindOnAthenaProfileInitialized()
{
    auto PlayerController = GetPlayerController();

    if (!PlayerController)
        return;

    if (PlayerController->GetAthenaProfile())
    {
        OnAthenaProfileInitialized();
        return;
    }

    FScriptDelegate Delegate;
    Delegate.BindUFunction(this, FName(TEXT("OnAthenaProfileInitialized")));

    PlayerController->GetOnMcpProfilesInitializedEvent().AddUnique(Delegate);
}

void UFortControllerComponent_VictoryCrowns::OnAthenaProfileInitialized()
{
    CacheAccountItemData();

    if (!GetVictoryCrownAccountData().has_victory_crown || !CurrentPlaylistSupportsVictoryCrowns())
        return;

    GetShouldGrantCrownOnSpawn() = true;

    auto PlayerController = GetPlayerController();

    if (PlayerController && Cast<AFortPlayerPawn>(PlayerController->GetPawn()))
        GrantVictoryCrownItem();
}

void UFortControllerComponent_VictoryCrowns::CacheAccountItemData()
{
    static auto VictoryCrownAccountItemDefinition = StaticClass()->FindPropertyByName(FName(TEXT("VictoryCrownAccountItemDefinition")));

    auto PlayerController = GetPlayerController();
    auto AthenaProfile = PlayerController ? PlayerController->GetAthenaProfile() : nullptr;
    auto AccountItemDefinition = *VictoryCrownAccountItemDefinition->ContainerPtrToValuePtr<UFortItemDefinition*>(this);

    if (!AthenaProfile || !AccountItemDefinition)
        return;

    TArray<UFortAccountItem*> AccountItems;
    AthenaProfile->GetItemInstancesForTemplate(AccountItemDefinition->GetPersistentName(), AccountItems);

    if (auto AccountItem = AccountItems.Num() ? Cast<UFortVictoryCrownAccountItem>(AccountItems[0]) : nullptr)
        SetCrownAccountData(AccountItem->GetVictoryCrownAccountData());
}

void UFortControllerComponent_VictoryCrowns::SetCrownAccountData(const FVictoryCrownAccountData& NewCrownAccountData)
{
    GetVictoryCrownAccountData() = NewCrownAccountData;
}

bool UFortControllerComponent_VictoryCrowns::CurrentPlaylistSupportsVictoryCrowns() const
{
    return FindCurrentPlaylistRow() != nullptr;
}

uint8* UFortControllerComponent_VictoryCrowns::FindCurrentPlaylistRow() const
{
    static auto VictoryCrownPlaylistData = StaticClass()->FindPropertyByName(FName(TEXT("VictoryCrownPlaylistData")));
    static auto PlaylistName = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylist")))->FindPropertyByName(FName(TEXT("PlaylistName")));

    auto DataTable = *VictoryCrownPlaylistData->ContainerPtrToValuePtr<UDataTable*>(this);
    auto World = GetWorld();
    auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
    auto Playlist = GameState ? GameState->GetCurrentPlaylist() : nullptr;

    if (!DataTable || !DataTable->RowStruct || !Playlist)
        return nullptr;

    auto RowName = PlaylistName->GetPropertyValue_InContainer<FName>(Playlist);
    auto RowData = RowName != NAME_None ? DataTable->RowMap.Find(RowName) : nullptr;

    return RowData ? *RowData : nullptr;
}

void UFortControllerComponent_VictoryCrowns::GrantVictoryCrownItem()
{
    static auto CrownInventoryItemClass = StaticClass()->FindPropertyByName(FName(TEXT("CrownInventoryItemClass")));

    GetShouldGrantCrownOnSpawn() = false;

    auto PlayerController = GetPlayerController();

    if (!PlayerController || GetCrownInPlayerInventory())
        return;

    UFortKismetLibrary::K2_GiveItemToPlayer(PlayerController, CrownInventoryItemClass->ContainerPtrToValuePtr<TSoftObjectPtr<UFortWorldItemDefinition>>(this)->LoadSynchronous(), 1);
}

const UFortWorldItem* UFortControllerComponent_VictoryCrowns::GetCrownInPlayerInventory() const
{
    static auto GetCrownInPlayerInventory = Anchor::Function<const UFortWorldItem* (*)(const UFortControllerComponent_VictoryCrowns*)>(TEXT("UFortControllerComponent_VictoryCrowns::GetCrownInPlayerInventory"))
        .Exec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:GetCrownInPlayerInventory"))
        .Find("call 0x*; mov [*], rax")
        .Target()
        .Get();

    return GetCrownInPlayerInventory(this);
}

AFortPlayerController* UFortControllerComponent_VictoryCrowns::GetPlayerController() const
{
    auto PlayerController = Cast<AFortPlayerController>(GetOwner());

    return PlayerController && PlayerController->HasAuthority() ? PlayerController : nullptr;
}

bool& UFortControllerComponent_VictoryCrowns::GetShouldGrantCrownOnSpawn()
{
    static auto bShouldGrantCrownOnSpawn = Anchor::Offset(TEXT("UFortControllerComponent_VictoryCrowns::bShouldGrantCrownOnSpawn"))
        .Exec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:HandleEnterAircraft"))
        .Find("setnz al; mov [*+0x*], al")
        .Find("mov [*+0x*], al")
        .Or()
        .Exec(TEXT("/Script/VictoryCrownsRuntime.FortControllerComponent_VictoryCrowns:HandleEnterAircraft"))
        .Find("add [*+0x20], *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("mov byte ptr [*+0x*], 0x1")
        .Disp();

    return *(bool*)((uint8_t*)this + bShouldGrantCrownOnSpawn);
}

FVictoryCrownAccountData& UFortControllerComponent_VictoryCrowns::GetVictoryCrownAccountData()
{
    static auto VictoryCrownAccountData = StaticClass()->FindPropertyByName(FName(TEXT("VictoryCrownAccountData")));

    return *VictoryCrownAccountData->ContainerPtrToValuePtr<FVictoryCrownAccountData>(this);
}

bool& UFortControllerComponent_VictoryCrowns::GetWonCrownInMatch()
{
    static auto bWonCrownInMatch = StaticClass()->FindPropertyByName(FName(TEXT("bWonCrownInMatch")));

    return *bWonCrownInMatch->ContainerPtrToValuePtr<bool>(this);
}

bool& UFortControllerComponent_VictoryCrowns::GetWonRoyalRoyale()
{
    static auto bWonRoyalRoyale = StaticClass()->FindPropertyByName(FName(TEXT("bWonRoyalRoyale")));

    return *bWonRoyalRoyale->ContainerPtrToValuePtr<bool>(this);
}

FMulticastScriptDelegate& UFortControllerComponent_VictoryCrowns::GetOnVictoryCrownStatusChanged()
{
    static auto OnVictoryCrownStatusChanged = StaticClass()->FindPropertyByName(FName(TEXT("OnVictoryCrownStatusChanged")));

    return *OnVictoryCrownStatusChanged->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}
