#include "pch.h"
#include "Player/FortPlayerState.h"
#include "Pawns/FortPlayerPawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortPlayerState::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerState"));

    return Class;
}

void AFortPlayerState::ApplyCharacterCustomization(APawn* Pawn)
{
    static auto ApplyCharacterCustomization = Anchor::Function<void (*)(AFortPlayerState*, APawn*)>(TEXT("AFortPlayerState::ApplyCharacterCustomization"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerState:ServerSetShowHeroBackpack"))
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortPlayerState::ServerSetShowHeroBackpack_Implementation"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerState"))
        .Find("jmp [*+0x*]")
        .LogSlot(TEXT("AFortPlayerState::ApplyHeroPartDisplaySettings"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerState"))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Find("pop *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerState:ServerSetShowHeroBackpack"))
        .Find("call [rax+*]", 2)
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerState"))
        .Find("jmp [*+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerState"))
        .Find("ret")
        .Last("call 0x*")
        .Call()
        .Find("mov rdx, *; mov rcx, *; call 0x*; jmp 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerState:ServerSetShowHeroBackpack"))
        .Find("call [rax+*]", 2)
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerState"))
        .Find("jmp [*+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerState"))
        .Find("mov rdx, *; mov rcx, *; call 0x*; jmp 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("AFortPlayerState::ApplyCharacterCustomization - Failed initialization, using default parts. Player Controller: %s PlayerState: %s, HeroId: %s"))
        .Start()
        .Get();

    ApplyCharacterCustomization(this, Pawn);
}

APawn* AFortPlayerState::GetCurrentPawn() const
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerState:GetCurrentPawn"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto PawnPrivate = GetClass()->FindPropertyByName(FName(TEXT("PawnPrivate")));

        return PawnPrivate ? Cast<AFortPlayerPawn>(*PawnPrivate->ContainerPtrToValuePtr<APawn*>(this)) : nullptr;
    }

    static auto GetCurrentPawn = Anchor::Function<APawn* (*)(const AFortPlayerState*)>(TEXT("AFortPlayerState::GetCurrentPawn"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerState:GetCurrentPawn"))
        .Get();

    return GetCurrentPawn(this);
}

int32 AFortPlayerState::GetWorldPlayerId() const
{
    static auto WorldPlayerId = GetClass()->FindPropertyByName(FName(TEXT("WorldPlayerId")));

    return WorldPlayerId->GetSize() == sizeof(int16) ? *WorldPlayerId->ContainerPtrToValuePtr<int16>(this) : *WorldPlayerId->ContainerPtrToValuePtr<int32>(this);
}

void AFortPlayerState::SetWorldPlayerId(int32 InWorldPlayerId)
{
    static auto WorldPlayerId = GetClass()->FindPropertyByName(FName(TEXT("WorldPlayerId")));

    if (WorldPlayerId->GetSize() == sizeof(int16))
        *WorldPlayerId->ContainerPtrToValuePtr<int16>(this) = (int16)InWorldPlayerId;
    else
        *WorldPlayerId->ContainerPtrToValuePtr<int32>(this) = InWorldPlayerId;
}

UFortAbilitySystemComponent*& AFortPlayerState::GetAbilitySystemComponent()
{
    static auto AbilitySystemComponent = GetClass()->FindPropertyByName(FName(TEXT("AbilitySystemComponent")));

    return *AbilitySystemComponent->ContainerPtrToValuePtr<UFortAbilitySystemComponent*>(this);
}
