#include "pch.h"
#include "FortWorldManager.h"
#include "Scanner.h"

FReservedRandomValues& AFortWorldManager::GetFoundationBuildingActorsRRV()
{
    static auto FoundationBuildingActorsRRV = Anchor::Offset(TEXT("AFortWorldManager::FoundationBuildingActorsRRV"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .CallSite()
        .Last("lea rdx, [*+0x*]")
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Caller()
        .CallSite()
        .Last("lea rdx, [*+0x*]")
        .Disp();

    return *(FReservedRandomValues*)((uint8_t*)this + FoundationBuildingActorsRRV);
}
