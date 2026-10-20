#include "pch.h"
#include "GameFramework/GameNetworkManager.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool AGameNetworkManager::IsInLowBandwidthMode()
{
    static auto IsInLowBandwidthMode = Anchor::VirtualFunction<bool (*)(AGameNetworkManager*)>(TEXT("AGameNetworkManager::UpdateNetSpeeds"))
        .String(TEXT("New Dynamic NetSpeed %i vs old %i"))
        .Start()
        .Log(TEXT("AGameNetworkManager::UpdateNetSpeeds"))
        .SlotIn(TEXT("/Script/Engine.Default__GameNetworkManager"))
        .LogSlot(TEXT("AGameNetworkManager::IsInLowBandwidthMode"), 2 * sizeof(void*))
        .Get(2 * sizeof(void*));

    return IsInLowBandwidthMode(this);
}
