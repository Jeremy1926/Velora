#include "pch.h"
#include "FortGameFeaturePluginManager.h"
#include "FortGlobals.h"
#include "Scanner.h"

UFortGameFeaturePluginManager* UFortGameFeaturePluginManager::Get()
{
    auto Globals = UFortGlobals::Get();

    return Globals ? Globals->GetGameFeaturePluginManager() : nullptr;
}

void UFortGameFeaturePluginManager::GetGameFeatureDataForActivePlugins(TArray<UFortGameFeatureData*>& OutGameFeatureDatas) const
{
    static auto GetGameFeatureDataForActivePlugins = Anchor::Function<void (*)(const UFortGameFeaturePluginManager*, TArray<UFortGameFeatureData*>&)>(TEXT("UFortGameFeaturePluginManager::GetGameFeatureDataForActivePlugins"))
        .String(TEXT(" - Adding preload object: '%s'"))
        .Start()
        .Find("call 0x*; mov rcx, rax; lea rdx, [*]; call 0x*; lea rdx, [*]; mov rcx, *; call 0x*")
        .Call(2)
        .Optional()
        .Get();

    if (GetGameFeatureDataForActivePlugins)
        GetGameFeatureDataForActivePlugins(this, OutGameFeatureDatas);
}
