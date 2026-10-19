#include "pch.h"
#include "GameFramework/GameNetworkManager.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool AGameNetworkManager::IsInLowBandwidthMode()
{
    static auto IsInLowBandwidthMode = []
    {
        auto UpdateNetSpeeds = Scanner::String(TEXT("New Dynamic NetSpeed %i vs old %i")).Start().Get(TEXT("AGameNetworkManager::UpdateNetSpeeds"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__GameNetworkManager"));
        auto UpdateNetSpeedsSlot = Scanner::Slot(TEXT("AGameNetworkManager::UpdateNetSpeeds"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == UpdateNetSpeeds; });

        return Scanner::Slot(TEXT("AGameNetworkManager::IsInLowBandwidthMode"), UpdateNetSpeedsSlot ? UpdateNetSpeedsSlot + 2 * (int32)sizeof(void*) : 0);
    }();

    return Scanner::Virtual<bool (*)(AGameNetworkManager*)>(this, IsInLowBandwidthMode)(this);
}
