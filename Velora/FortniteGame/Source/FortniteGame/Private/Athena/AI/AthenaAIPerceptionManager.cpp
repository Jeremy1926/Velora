#include "pch.h"
#include "Athena/AI/AthenaAIPerceptionManager.h"
#include "Athena/FortGameModeAthena.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UAthenaAIPerceptionManager::OnAISettingsUpdated(UAthenaAISettings* AISettings)
{
    static auto OnAISettingsUpdated = Scanner::Function<void (*)(UAthenaAIPerceptionManager*, UAthenaAISettings*)>([]
    {
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__AthenaAISystem"));
        auto ReadAISettings = std::format("mov rdx, [*+{:#x}]", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings")))->GetOffset_ForInternal());
        auto StartPlay = Scanner::Slot(TEXT("UAthenaAISystem::StartPlay"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Has(ReadAISettings); });

        return Scanner::At(Scanner::IsSlot(StartPlay) ? VTable[StartPlay / sizeof(void*)] : nullptr).Find(ReadAISettings).Find("call 0x*").Target(TEXT("UAthenaAIPerceptionManager::OnAISettingsUpdated"));
    }());

    OnAISettingsUpdated(this, AISettings);
}
