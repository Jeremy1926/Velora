#include "pch.h"
#include "Pawns/FortPlayerPawn.h"
#include "Scanner.h"

void AFortPlayerPawn::OnBlueprintPlace()
{
    static auto OnBlueprintPlace = []
    {
        auto ClientBuild = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerCreateBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();

        return (void (*)(AFortPlayerPawn*))ClientBuild.Find("mov rcx, [*+0x*]; test rcx, rcx; jz *; call 0x*").Call().Get(TEXT("AFortPlayerPawn::OnBlueprintPlace"));
    }();

    OnBlueprintPlace(this);
}
