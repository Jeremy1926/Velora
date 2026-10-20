#include "pch.h"
#include "Online/FortGameSession.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "FortGlobals.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

PATCH(AFortGameSession, ValidatePlayer, Anchor::Patch()
    .String(TEXT("No reservation in game."))
    .Start()
    .FindTo("jnz", Scanner::String(TEXT("No reservation in game.")).Last("jnz 0x*").Target())
    .Jump(), Anchor::Patch()
    .String(TEXT("No reservation in game."))
    .Last("jnz 0x*")
    .Target()
    .Find("call 0x*; test al, al; jnz 0x*")
    .Find("jnz 0x*")
    .Jump())

void AFortGameSession::Restart()
{
    if (!Config::bRestartAfterMatch)
        return;

    auto Options = *GetWorld()->GetAuthGameMode()->GetOptionsString();

    if (*Options == TEXT('?'))
        Options++;

    auto BRMapFullName = UFortGlobals::Get()->GetBRMapFullName();

    UGameplayStatics::OpenLevel(this, FName(BRMapFullName ? **BRMapFullName : Config::Map), true, FString(Options));
}
