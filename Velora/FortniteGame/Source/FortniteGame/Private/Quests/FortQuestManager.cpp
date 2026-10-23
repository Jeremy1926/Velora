#include "pch.h"
#include "Quests/FortQuestManager.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortQuestManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortQuestManager"));

    return Class;
}

bool UFortQuestManager::HasCompletedTransientQuest(const UFortQuestItemDefinition* Definition, bool bMineOnly) const
{
    static auto HasCompletedTransientQuest = Anchor::Function<bool (*)(const UFortQuestManager*, const UFortQuestItemDefinition*, bool)>(TEXT("UFortQuestManager::HasCompletedTransientQuest"))
        .Native(TEXT("/Script/FortniteGame.FortQuestManager:HasCompletedTransientQuest"))
        .Get();

    return HasCompletedTransientQuest(this, Definition, bMineOnly);
}

const TArray<UFortQuestItem*>& UFortQuestManager::GetActiveTransientQuests() const
{
    static auto ActiveTransientQuests = StaticClass()->FindPropertyByName(FName(TEXT("ActiveTransientQuests")));

    return *ActiveTransientQuests->ContainerPtrToValuePtr<TArray<UFortQuestItem*>>(this);
}
