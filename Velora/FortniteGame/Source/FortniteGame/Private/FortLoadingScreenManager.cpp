#include "pch.h"
#include "FortLoadingScreenManager.h"
#include "Scanner.h"
#include "Detour.h"

bool FFortLoadingScreenManager::ShouldShowLoadingScreen()
{
    DETOUR(FFortLoadingScreenManager, ShouldShowLoadingScreen, Anchor::Hook()
        .String(TEXT("Loading screen showing: %d. Reason: %s"))
        .Start()
        .Find("mov rdi, rcx; call 0x*; lea rsi, [*]; test al, al")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Loading screen showing: %d. Reason: %s"))
        .Start()
        .Find("mov rbx, rcx; call 0x*; xor r14d, r14d; lea rdi, [*]; test al, al")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Loading screen showing: %d. Reason: %s"))
        .Start()
        .Find("mov rdi, rcx; call 0x*; lea rsi, [*]; lea rbp, [*]; test al, al")
        .Find("call 0x*")
        .Target()
        .Optional()
        .Inline());

    return false;
}
