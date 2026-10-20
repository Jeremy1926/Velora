#include "pch.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto ServerRestartPlayer = []
{
    auto ServerRestartPlayer = Scanner::Exec(TEXT("/Script/Engine.PlayerController:ServerRestartPlayer")).Find("jmp [rax+*]");
    auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"));

    return ServerRestartPlayer.Get() && Scanner::At(Scanner::Virtual<const void*>(DefaultObject, ServerRestartPlayer.Disp())).Is("ret*") ? DetourVTable(TEXT("AFortPlayerControllerAthena::ServerRestartPlayer_Implementation"), *(void***)DefaultObject, ServerRestartPlayer.Disp(), &AFortPlayerControllerAthena::ServerRestartPlayer_Implementation) : nullptr;
}();

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
    Super::ServerRestartPlayer_Implementation();
}

FFortAthenaLoadout* AFortPlayerControllerAthena::GetCustomizationLoadout()
{
    static auto Property = GetClass()->FindPropertyByName(FName(TEXT("CustomizationLoadout")));
    static auto CustomizationLoadout = Property ? Property : GetClass()->FindPropertyByName(FName(TEXT("CosmeticLoadoutPC")));

    return CustomizationLoadout ? CustomizationLoadout->ContainerPtrToValuePtr<FFortAthenaLoadout>(this) : nullptr;
}

void AFortPlayerControllerAthena::ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution)
{
    static auto ClientEquipItem = Scanner::Function<void (*)(AFortPlayerControllerAthena*, const FGuid&, bool)>(Scanner::Event(TEXT("ClientEquipItem")).Get(TEXT("AFortPlayerControllerAthena::ClientEquipItem")));

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
    static auto RespawnPlayerAfterDeath = Scanner::Function<void (*)(AFortPlayerControllerAthena*, bool)>(Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerControllerAthena:RespawnPlayerAfterDeath")).Get(TEXT("AFortPlayerControllerAthena::RespawnPlayerAfterDeath")));

    RespawnPlayerAfterDeath(this, bEnterSkydiving);
}
