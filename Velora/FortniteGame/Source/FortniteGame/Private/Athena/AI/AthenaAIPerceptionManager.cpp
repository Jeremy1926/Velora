#include "pch.h"
#include "Athena/AI/AthenaAIPerceptionManager.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/AI/AthenaAISystem.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UAthenaAIPerceptionManager::OnAISettingsUpdated(UAthenaAISettings* AISettings)
{
    static auto OnAISettingsUpdated = Anchor::Optional<void (*)(UAthenaAIPerceptionManager*, UAthenaAISettings*)>(TEXT("UAthenaAIPerceptionManager::OnAISettingsUpdated"))
        .FieldUser(TEXT("/Script/FortniteGame.Default__AthenaAISystem"), "mov rdx, [*+%x]", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings"))))
        .LogSlot(TEXT("UAthenaAISystem::StartPlay"))
        .FieldFind("mov rdx, [*+%x]", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings"))))
        .Find("call 0x*")
        .Target()
        .Or()
        .FieldPattern("48 8B ? %o 48 85 D2 74 ? 48 8B ? E8", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings"))))
        .Find("call 0x*")
        .Target()
        .Get();

    if (OnAISettingsUpdated)
        return OnAISettingsUpdated(this, AISettings);

    StartPlay();
}

void UAthenaAIPerceptionManager::StartPlay()
{
    static auto StartPlay = Anchor::Function<void (*)(UAthenaAIPerceptionManager*)>(TEXT("UAthenaAIPerceptionManager::StartPlay"))
        .FieldUser(TEXT("/Script/FortniteGame.Default__AthenaAISystem"), "mov rcx, [*+%x]; add rsp, 0x*; pop *; jmp 0x*", UAthenaAISystem::StaticClass()->FindPropertyByName(FName(TEXT("PerceptionManager"))))
        .LogSlot(TEXT("UAthenaAISystem::StartPlay"))
        .Find("jmp 0x*")
        .Target()
        .Or()
        .FieldPattern("48 8B ? %o 48 85 D2 74 ? 48 8B ? E8", AFortGameModeAthena::StaticClass()->FindPropertyByName(FName(TEXT("AISettings"))))
        .Start()
        .Get();

    StartPlay(this);
}
