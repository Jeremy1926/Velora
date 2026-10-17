#include "pch.h"
#include "Engine/GameInstance.h"
#include "Scanner.h"

bool UGameInstance::RemoveLocalPlayer(ULocalPlayer* ExistingPlayer)
{
    static auto RemoveLocalPlayer = (bool (*)(UGameInstance*, ULocalPlayer*))(Scanner::Base() + Scanner::FindFunction(Scanner::FindReference(Scanner::FindString(TEXT("UGameInstance::RemovePlayer: Removed player %s with ControllerId %i at index %i (%i remaining players)"))))->BeginAddress);

    return RemoveLocalPlayer(this, ExistingPlayer);
}

ULocalPlayer* UGameInstance::GetFirstGamePlayer() const
{
    static auto GetFirstGamePlayer = (ULocalPlayer* (*)(const UGameInstance*))Scanner::FindPattern("83 79 40 00 7E ? 48 8B 41 38 48 8B 00 C3 33 C0 C3");

    return GetFirstGamePlayer(this);
}
