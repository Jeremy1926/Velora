#include "pch.h"
#include "GameFramework/GameNetworkManager.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool AGameNetworkManager::IsInLowBandwidthMode()
{
    static auto IsInLowBandwidthMode = []
    {
        auto UpdateNetSpeeds = Scanner::String(TEXT("New Dynamic NetSpeed %i vs old %i")).Start().Get();
        auto VTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__GameNetworkManager"));
        auto Index = 0;

        while (VTable[Index] != UpdateNetSpeeds)
            Index++;

        return (Index + 2) * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(AGameNetworkManager*))(VTable + IsInLowBandwidthMode))(this);
}
