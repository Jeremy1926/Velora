#include "pch.h"
#include "FortWorldManager.h"
#include "Scanner.h"

FReservedRandomValues& AFortWorldManager::GetFoundationBuildingActorsRRV()
{
    static auto ConditionalStreamIn = Scanner::CallTo(Scanner::String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!")).Start().Get()).Start();
    static auto FoundationBuildingActorsRRV = Scanner::CallTo(ConditionalStreamIn.Get()).Last("lea rdx, [*+0x*]").Disp(TEXT("AFortWorldManager::FoundationBuildingActorsRRV"));

    return *(FReservedRandomValues*)((uint8_t*)this + FoundationBuildingActorsRRV);
}
