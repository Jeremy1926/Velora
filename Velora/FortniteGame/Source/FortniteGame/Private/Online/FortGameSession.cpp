#include "pch.h"
#include "Scanner.h"
#include "Detour.h"

static auto ValidatePlayer = []
{
    auto Reservation = Scanner::String(TEXT("No reservation in game.")).Last("jnz 0x*");
    std::array<const uint8_t*, 2> Branches = { Reservation.Get(), Scanner::At(Reservation.Target()).Find("call 0x*; test al, al; jnz 0x*").Find("jnz 0x*").Get() };

    for (auto Branch : Branches)
        Patch(TEXT("AFortGameSession::ValidatePlayer"), Branch, 0xEB);

    return Branches;
}();
