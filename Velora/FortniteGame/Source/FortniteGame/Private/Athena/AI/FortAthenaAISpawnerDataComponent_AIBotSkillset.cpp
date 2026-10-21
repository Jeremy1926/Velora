#include "pch.h"
#include "Athena/AI/FortAthenaAISpawnerDataComponent_AIBotSkillset.h"
#include "AI/FortAthenaAIBotController.h"
#include "Athena/FortGameStateAthena.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

void UFortAthenaAISpawnerDataComponent_AIBotSkillset::OnSpawned(APawn* SpawnedPawn)
{
    DETOUR(UFortAthenaAISpawnerDataComponent_AIBotSkillset, OnSpawned, Anchor::Hook()
        .String(TEXT("UFortAthenaAISpawnerDataComponent_InventoryBase::OnSpawned Failed to initialize Inventory on Pawn=%s Controller=%s (Can't create/initialize inventory on AI)"))
        .Optional()
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_AIBotInventory"))
        .LogSlot(TEXT("UFortAthenaAISpawnerDataComponent::OnSpawned"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_AIBotSkillset"), "ret*")
        .InVTable(TEXT("/Script/FortniteGame.Default__FortAthenaAISpawnerDataComponent_AIBotSkillset")));

    auto BotController = SpawnedPawn ? Cast<AFortAthenaAIBotController>(SpawnedPawn->GetController()) : nullptr;

    if (!BotController)
        return;

    const TArray<UClass*> SkillSets = GetSkillSets();
    auto World = SpawnedPawn->GetWorld();
    auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
    auto Playlist = GameState ? GameState->GetCurrentPlaylist() : nullptr;

    static auto bUsePlayerRating = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))->FindPropertyByName(FName(TEXT("bUsePlayerRating")));

    float Skill = 0.0f;

    if (ShouldUseMatchMMRToOverrideSkillLevel() && Playlist && *bUsePlayerRating->ContainerPtrToValuePtr<bool>(Playlist))
        Skill = (float)GameState->GetAvgPlayerMMR();
    else
        Skill = GetSkill();

    BotController->InitializeSkills(Skill != -1.0f ? &Skill : nullptr, &SkillSets);
}
