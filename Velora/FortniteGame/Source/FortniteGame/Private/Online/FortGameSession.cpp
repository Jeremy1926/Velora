#include "pch.h"
#include "Online/FortGameSession.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

static auto ValidatePlayer = []
{
    auto Reservation = Scanner::String(TEXT("No reservation in game.")).Last("jnz 0x*");
    std::array<const uint8_t*, 2> Branches = { Reservation.Start().Find(std::format("jnz 0x{:x}", (uintptr_t)Reservation.Target())).Get(), Scanner::At(Reservation.Target()).Find("call 0x*; test al, al; jnz 0x*").Find("jnz 0x*").Get() };

    for (auto Branch : Branches)
    {
        if (Branch && *Branch == 0x0F)
            Patch(TEXT("AFortGameSession::ValidatePlayer"), Branch, { 0x90, 0xE9 });
        else
            Patch(TEXT("AFortGameSession::ValidatePlayer"), Branch, { 0xEB });
    }

    return Branches;
}();

void AFortGameSession::Restart()
{
    if (!Config::bRestartAfterMatch)
        return;

    auto Options = *GetWorld()->GetAuthGameMode()->GetOptionsString();

    if (*Options == TEXT('?'))
        Options++;

    UGameplayStatics::OpenLevel(this, FName(Config::Map), true, FString(Options));
}
