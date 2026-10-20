#include "pch.h"
#include "Athena/AI/AthenaAIPerceptionManager.h"
#include "Athena/FortGameModeAthena.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UAthenaAIPerceptionManager::OnAISettingsUpdated(UAthenaAISettings* AISettings)
{
    static auto OnAISettingsUpdated = []
    {
        auto ReadAISettings = std::format("mov rdx, [*+{:#x}]", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings")))->GetOffset_ForInternal());

        return Anchor::Function<void (*)(UAthenaAIPerceptionManager*, UAthenaAISettings*)>(TEXT("UAthenaAIPerceptionManager::OnAISettingsUpdated"))
            .Virtual(TEXT("/Script/FortniteGame.Default__AthenaAISystem"), [&](const Anchor::FCode& Entry) { return Entry.Has(ReadAISettings); })
            .LogSlot(TEXT("UAthenaAISystem::StartPlay"))
            .Find(ReadAISettings)
            .Find("call 0x*")
            .Target()
            .Get();
    }();

    OnAISettingsUpdated(this, AISettings);
}
