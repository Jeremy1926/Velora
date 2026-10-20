#include "pch.h"
#include "GameFramework/GameNetworkManager.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool AGameNetworkManager::IsInLowBandwidthMode()
{
    static auto IsInLowBandwidthMode = []
    {
        auto UpdateNetSpeeds = Anchor::Slot(TEXT("AGameNetworkManager::UpdateNetSpeeds"))
            .String(TEXT("New Dynamic NetSpeed %i vs old %i"))
            .Start()
            .Log(TEXT("AGameNetworkManager::UpdateNetSpeeds"))
            .SlotIn(TEXT("/Script/Engine.Default__GameNetworkManager"))
            .Get();

        return Scanner::Slot(TEXT("AGameNetworkManager::IsInLowBandwidthMode"), UpdateNetSpeeds ? UpdateNetSpeeds + 2 * (int32)sizeof(void*) : 0);
    }();

    return Scanner::Virtual<bool (*)(AGameNetworkManager*)>(this, IsInLowBandwidthMode)(this);
}
