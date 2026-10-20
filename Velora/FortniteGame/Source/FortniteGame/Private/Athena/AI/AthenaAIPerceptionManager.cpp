#include "pch.h"
#include "Athena/AI/AthenaAIPerceptionManager.h"
#include "Athena/FortGameModeAthena.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UAthenaAIPerceptionManager::OnAISettingsUpdated(UAthenaAISettings* AISettings)
{
    static auto OnAISettingsUpdated = Anchor::Function<void (*)(UAthenaAIPerceptionManager*, UAthenaAISettings*)>(TEXT("UAthenaAIPerceptionManager::OnAISettingsUpdated"))
        .FieldUser(TEXT("/Script/FortniteGame.Default__AthenaAISystem"), "mov rdx, [*+%x]", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings"))))
        .LogSlot(TEXT("UAthenaAISystem::StartPlay"))
        .FieldFind("mov rdx, [*+%x]", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings"))))
        .Find("call 0x*")
        .Target()
        .Get();

    OnAISettingsUpdated(this, AISettings);
}
