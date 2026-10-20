#include "pch.h"
#include "Engine/GameInstance.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

TArray<ULocalPlayer*>& UGameInstance::GetLocalPlayers()
{
    static auto LocalPlayers = GetClass()->FindPropertyByName(FName(TEXT("LocalPlayers")));

    return *LocalPlayers->ContainerPtrToValuePtr<TArray<ULocalPlayer*>>(this);
}

ULocalPlayer* UGameInstance::GetFirstGamePlayer() const
{
    static auto GetFirstGamePlayer = Anchor::Function<ULocalPlayer* (*)(const UGameInstance*)>(TEXT("UGameInstance::GetFirstGamePlayer"))
        .Pattern("83 79 40 00 7E ? 48 8B 41 38 48 8B 00 C3 33 C0 C3")
        .Get();

    return GetFirstGamePlayer(this);
}

FWorldContext* UGameInstance::GetWorldContext() const
{
    static auto WorldContext = Anchor::Offset(TEXT("UGameInstance::WorldContext"))
        .Pattern("48 8B 41 30 48 85 C0 74 ? 0F B6 80 ? ? 00 00 C3")
        .Disp();

    return *(FWorldContext**)((uint8_t*)this + WorldContext);
}
