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
    static auto HasCompletedTransientQuest = Anchor::Optional<bool (*)(const UFortQuestManager*, const UFortQuestItemDefinition*, bool)>(TEXT("UFortQuestManager::HasCompletedTransientQuest"))
        .Native(TEXT("/Script/FortniteGame.FortQuestManager:HasCompletedTransientQuest"))
        .Get();

    static auto GetQuestManagerComponentAthena = Anchor::Optional<const UObject* (*)(const UFortQuestManager*)>(TEXT("UFortQuestManager::GetComponent<UFortQuestManagerComponent_Athena>"))
        .If(!HasCompletedTransientQuest)
        .Exec(TEXT("/Script/FortniteGame.FortControllerRequirement:IsRequirementMetInternal"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortControllerRequirement_HasCompletedQuest"))
        .Find("cmp byte ptr [rbx+0x*], 0xe; mov rcx, rax; jnz 0x*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto CompletedQuests = Anchor::Offset(TEXT("UFortQuestManagerComponent_Athena::CompletedQuests"))
        .If(GetQuestManagerComponentAthena != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortControllerRequirement:IsRequirementMetInternal"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortControllerRequirement_HasCompletedQuest"))
        .Find("cmp byte ptr [rbx+0x*], 0xe; mov rcx, rax; jnz 0x*; call 0x*")
        .Find("mov rcx, [rax+0x*]; movsxd rax, dword ptr [rax+0x*]; lea r9, [rcx+rax*8]")
        .Optional()
        .Disp();

    static auto ItemDefinition = Anchor::Offset(TEXT("UFortItem::ItemDefinition"))
        .If(GetQuestManagerComponentAthena != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortControllerRequirement:IsRequirementMetInternal"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortControllerRequirement_HasCompletedQuest"))
        .Find("cmp byte ptr [rbx+0x*], 0xe; mov rcx, rax; jnz 0x*; call 0x*")
        .Find("mov r8, [rax+0x*]; test r8, r8")
        .Optional()
        .Disp();

    if (HasCompletedTransientQuest)
        return HasCompletedTransientQuest(this, Definition, bMineOnly);

    auto QuestManagerComponent = GetQuestManagerComponentAthena && CompletedQuests > 0 && ItemDefinition > 0 ? GetQuestManagerComponentAthena(this) : nullptr;

    if (!QuestManagerComponent)
        return false;

    for (auto CompletedQuest : *(const TArray<UFortQuestItem*>*)((const uint8*)QuestManagerComponent + CompletedQuests))
    {
        if (CompletedQuest && *(const UFortQuestItemDefinition**)((const uint8*)CompletedQuest + ItemDefinition) == Definition)
            return true;
    }

    return false;
}

const TArray<UFortQuestItem*>& UFortQuestManager::GetActiveTransientQuests() const
{
    static auto ActiveTransientQuests = StaticClass()->FindPropertyByName(FName(TEXT("ActiveTransientQuests")));

    return *ActiveTransientQuests->ContainerPtrToValuePtr<TArray<UFortQuestItem*>>(this);
}
