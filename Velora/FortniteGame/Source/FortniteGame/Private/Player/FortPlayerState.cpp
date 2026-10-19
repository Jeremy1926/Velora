#include "pch.h"
#include "Player/FortPlayerState.h"
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
    static auto ApplyCharacterCustomization = []
    {
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerState"));
        auto ServerSetShowHeroBackpack_Implementation = Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerState:ServerSetShowHeroBackpack")).Find("call [rax+*]", 2).Slot(TEXT("AFortPlayerState::ServerSetShowHeroBackpack_Implementation"));
        auto ApplyHeroPartDisplaySettings = Scanner::At(Scanner::Virtual<const void*>(DefaultObject, ServerSetShowHeroBackpack_Implementation)).Find("jmp [*+0x*]").Slot(TEXT("AFortPlayerState::ApplyHeroPartDisplaySettings"));
        auto ApplyCustomizationToCharacter = Scanner::At(Scanner::Virtual<const void*>(DefaultObject, ApplyHeroPartDisplaySettings)).Find("ret").Last("call 0x*").Call();

        auto TailCall = ApplyCustomizationToCharacter.Find("pop *; jmp 0x*");

        return (void (*)(AFortPlayerState*, APawn*))(TailCall.Get() ? TailCall.Find("jmp 0x*") : ApplyCustomizationToCharacter.Find("mov rdx, *; mov rcx, *; call 0x*; jmp 0x*").Find("call 0x*")).Target(TEXT("AFortPlayerState::ApplyCharacterCustomization"));
    }();

    ApplyCharacterCustomization(this, Pawn);
}

int32& AFortPlayerState::GetWorldPlayerId()
{
    static auto WorldPlayerId = GetClass()->FindPropertyByName(FName(TEXT("WorldPlayerId")));

    return *WorldPlayerId->ContainerPtrToValuePtr<int32>(this);
}

UFortAbilitySystemComponent*& AFortPlayerState::GetAbilitySystemComponent()
{
    static auto AbilitySystemComponent = GetClass()->FindPropertyByName(FName(TEXT("AbilitySystemComponent")));

    return *AbilitySystemComponent->ContainerPtrToValuePtr<UFortAbilitySystemComponent*>(this);
}
