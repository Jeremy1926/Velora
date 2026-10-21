#include "pch.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Pawns/FortPlayerPawn.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FFortAthenaLoadout::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaLoadout"));

    return Struct;
}

UAthenaCharacterItemDefinition*& FFortAthenaLoadout::GetCharacter()
{
    static auto Character = StaticStruct()->FindPropertyByName(FName(TEXT("Character")));

    return *Character->ContainerPtrToValuePtr<UAthenaCharacterItemDefinition*>(this);
}

UAthenaGliderItemDefinition*& FFortAthenaLoadout::GetGlider()
{
    static auto Glider = StaticStruct()->FindPropertyByName(FName(TEXT("Glider")));

    return *Glider->ContainerPtrToValuePtr<UAthenaGliderItemDefinition*>(this);
}

UAthenaSkyDiveContrailItemDefinition*& FFortAthenaLoadout::GetSkyDiveContrail()
{
    static auto SkyDiveContrail = StaticStruct()->FindPropertyByName(FName(TEXT("SkyDiveContrail")));

    return *SkyDiveContrail->ContainerPtrToValuePtr<UAthenaSkyDiveContrailItemDefinition*>(this);
}

UAthenaBackpackItemDefinition*& FFortAthenaLoadout::GetBackpack()
{
    static auto Backpack = StaticStruct()->FindPropertyByName(FName(TEXT("Backpack")));

    return *Backpack->ContainerPtrToValuePtr<UAthenaBackpackItemDefinition*>(this);
}

UAthenaPickaxeItemDefinition*& FFortAthenaLoadout::GetPickaxe()
{
    static auto Pickaxe = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaLoadout")))->FindPropertyByName(FName(TEXT("Pickaxe")));

    return *Pickaxe->ContainerPtrToValuePtr<UAthenaPickaxeItemDefinition*>(this);
}

UClass* AFortPlayerControllerAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerAthena"));

    return Class;
}

void AFortPlayerControllerAthena::ServerRestartPlayer_Implementation()
{
    DETOUR(AFortPlayerControllerAthena, ServerRestartPlayer_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/Engine.PlayerController:ServerRestartPlayer"))
        .Find("jmp [rax+*]")
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena")));

    Super::ServerRestartPlayer_Implementation();
}

void AFortPlayerControllerAthena::OnPawnDied(AFortPlayerPawn* KilledPawn, float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser, AController* DBNOFinisher)
{
    DETOUR(AFortPlayerControllerAthena, OnPawnDied, Anchor::Hook()
        .String(TEXT("FortGameModeAthena: Player [%s] removed from alive players list*"))
        .Start()
        .Has("cmp al, 0x12")
        .Optional()
        .String(TEXT("Deaths"))
        .Start()
        .Find("movaps xmm6, xmm*")
        .Is("movaps xmm6, xmm2")
        .Start()
        .InVirtual(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"), TEXT("AFortPlayerControllerAthena::OnPawnDied"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena")));

    Original(KilledPawn, Damage, InTags, EffectContext, EventInstigator, DamageCauser, DBNOFinisher);

    auto World = GetWorld();
    auto GameMode = Cast<AFortGameModeAthena>(World->GetAuthGameMode());
    auto GameState = Cast<AFortGameStateAthena>(World->GetGameState());

    if (!GameMode || !GameState || GameState->GetGamePhase() <= EAthenaGamePhase::Warmup || !GetMarkedAlive() || GameState->IsRespawningAllowed(Cast<AFortPlayerState>(GetPlayerState())))
        return;

    auto& PlayerDeathReport = GetPlayerDeathReport();

    GameMode->RemoveFromAlivePlayers(this, PlayerDeathReport.GetKillerPlayerState(), PlayerDeathReport.GetKillerPawn(), PlayerDeathReport.GetKillerWeapon(), AFortPlayerStateAthena::ToDeathCause(InTags, KilledPawn && KilledPawn->GetWasDBNOOnDeath()), false);
}

bool AFortPlayerControllerAthena::GetMarkedAlive() const
{
    static auto bMarkedAlive = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bMarkedAlive")));

    return bMarkedAlive->GetPropertyValue_InContainer(this);
}

FFortAthenaLoadout* AFortPlayerControllerAthena::GetCustomizationLoadout()
{
    static auto Property = GetClass()->FindPropertyByName(FName(TEXT("CustomizationLoadout")));
    static auto CustomizationLoadout = Property ? Property : GetClass()->FindPropertyByName(FName(TEXT("CosmeticLoadoutPC")));

    return CustomizationLoadout ? CustomizationLoadout->ContainerPtrToValuePtr<FFortAthenaLoadout>(this) : nullptr;
}

void AFortPlayerControllerAthena::ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution)
{
    static auto ClientEquipItem = Anchor::Function<void (*)(AFortPlayerControllerAthena*, const FGuid&, bool)>(TEXT("AFortPlayerControllerAthena::ClientEquipItem"))
        .Event(TEXT("ClientEquipItem"))
        .Get();

    ClientEquipItem(this, ItemGuid, bForceExecution);
}

void AFortPlayerControllerAthena::ClientReportPhaseFound(const FString& Phase)
{
    static auto ClientReportPhaseFound = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerAthena:ClientReportPhaseFound"));

    struct
    {
        FString Phase;
    } Parms{ Phase };

    CallRemoteFunction(ClientReportPhaseFound, &Parms, nullptr, nullptr);
}

UAthenaMarkerComponent* AFortPlayerControllerAthena::GetMarkerComponent() const
{
    static auto MarkerComponent = GetClass()->FindPropertyByName(FName(TEXT("MarkerComponent")));

    return *MarkerComponent->ContainerPtrToValuePtr<UAthenaMarkerComponent*>(this);
}

uint8 AFortPlayerControllerAthena::GetSquadId() const
{
    auto FortPlayerState = Cast<AFortPlayerStateAthena>(GetPlayerState());

    return FortPlayerState ? FortPlayerState->GetSquadId() : 0;
}

AFortPlayerState* AFortPlayerControllerAthena::GetSkydiveLeader() const
{
    static auto SkydiveLeader = GetClass()->FindPropertyByName(FName(TEXT("SkydiveLeader")));

    return *SkydiveLeader->ContainerPtrToValuePtr<AFortPlayerState*>(this);
}

void AFortPlayerControllerAthena::RespawnPlayerAfterDeath(bool bEnterSkydiving)
{
    static auto RespawnPlayerAfterDeath = Anchor::Function<void (*)(AFortPlayerControllerAthena*, bool)>(TEXT("AFortPlayerControllerAthena::RespawnPlayerAfterDeath"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerControllerAthena:RespawnPlayerAfterDeath"))
        .Get();

    RespawnPlayerAfterDeath(this, bEnterSkydiving);
}
