#include "pch.h"
#include "Components/FortControllerComponent_Aircraft.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortControllerComponent_Aircraft::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_Aircraft"));

    return Class;
}

bool UFortControllerComponent_Aircraft::IsInAircraft() const
{
    static auto InAircraft = Anchor::Offset(TEXT("UFortControllerComponent_Aircraft::IsInAircraft"))
        .Exec(TEXT("/Script/FortniteGame.FortControllerComponent_Aircraft:IsInAircraft"))
        .FirstOf("movzx eax, byte ptr [rcx+0x*]", "mov al, [rcx+0x*]")
        .Disp();

    return *((uint8_t*)this + InAircraft);
}

bool& UFortControllerComponent_Aircraft::GetIsAttemptingAircraftJump()
{
    static auto bIsAttemptingAircraftJump = Anchor::Offset(TEXT("UFortControllerComponent_Aircraft::bIsAttemptingAircraftJump"))
        .String(TEXT("FortGameModeAthena::FinishRestartPlayer -- Found potential teleport spot. Retrying."))
        .Start()
        .Find("cmp [rax+0x*], *")
        .Disp();

    return *(bool*)((uint8_t*)this + bIsAttemptingAircraftJump);
}

void UFortControllerComponent_Aircraft::ServerAttemptAircraftJump_Implementation(FRotator ClientRotation)
{
    DETOUR(UFortControllerComponent_Aircraft, ServerAttemptAircraftJump_Implementation, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortControllerComponent_Aircraft:ServerAttemptAircraftJump"))
        .Find("call [*+0x*]", 2)
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_Aircraft"), "ret*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortControllerComponent_Aircraft:ServerAttemptAircraftJump"))
        .Find("call r8; test al, al")
        .Find("call [*+0x*]")
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortControllerComponent_Aircraft"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__FortControllerComponent_Aircraft")));

    auto PlayerController = Cast<AFortPlayerController>(GetOwner());

    if (!PlayerController)
        return;

    GetIsAttemptingAircraftJump() = true;

    if (IsInAircraft())
    {
        PlayerController->GetStateName() = NAME_Inactive;
        PlayerController->GetAirSpawnControlRotation() = ClientRotation;
        PlayerController->ServerRestartPlayer_Implementation();
        PlayerController->SetControlRotation(ClientRotation);
    }

    GetIsAttemptingAircraftJump() = false;

    auto PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerController);
    auto PlayerState = PlayerControllerAthena ? Cast<AFortPlayerStateAthena>(PlayerControllerAthena->GetPlayerState()) : nullptr;

    if (!PlayerState || PlayerControllerAthena->GetSkydiveLeader() != PlayerState)
        return;

    for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
    {
        auto SquadMember = Cast<AFortPlayerStateAthena>(OtherPlayerState);

        if (!SquadMember || SquadMember == PlayerState || SquadMember->GetSquadId() != PlayerState->GetSquadId())
            continue;

        if (auto OtherPlayerController = Cast<AFortPlayerControllerAthena>(SquadMember->GetOwner()))
        {
            if (auto AircraftComponent = OtherPlayerController->FindComponentByClass<UFortControllerComponent_Aircraft>())
                AircraftComponent->ServerAttemptAircraftJump_Implementation(ClientRotation);
        }
    }
}
