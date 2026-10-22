#include "pch.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "AI/FortAthenaAIBotController.h"
#include "Quests/FortQuestItemDefinition.h"
#include "Engine/DataTable.h"
#include "FortGameFeatureData.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortNPCConversationParticipantComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortNPCConversationParticipantComponent"));

    return Class;
}

void UFortNPCConversationParticipantComponent::InitializeServerConversation(AFortAthenaAIBotController* BotController)
{
    if (!BotController)
        return;

    GetBotControllerOwner() = BotController;

    static auto OnAlertLevelChangedEventDelegate = AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("OnAlertLevelChangedEventDelegate")));
    static auto OnAgentDBNOStatusChangedEventDelegate = AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("OnAgentDBNOStatusChangedEventDelegate")));

    FScriptDelegate AlertLevelChangedDelegate;
    AlertLevelChangedDelegate.BindUFunction(this, FName(TEXT("OnBotControllerAlertLevelChanged")));
    OnAlertLevelChangedEventDelegate->ContainerPtrToValuePtr<FMulticastScriptDelegate>(BotController)->AddUnique(AlertLevelChangedDelegate);

    FScriptDelegate DBNOStatusChangedDelegate;
    DBNOStatusChangedDelegate.BindUFunction(this, FName(TEXT("OnBotControllerDBNOStatusChanged")));
    OnAgentDBNOStatusChangedEventDelegate->ContainerPtrToValuePtr<FMulticastScriptDelegate>(BotController)->AddUnique(DBNOStatusChangedDelegate);

    InitializeServices();
    UpdateCanStartConversation();
}

void UFortNPCConversationParticipantComponent::InitializeServices()
{
    static auto NPCDynamicServiceRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCDynamicServiceRow"));
    static auto NPC = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("NPC")));
    static auto Chance = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("Chance")));
    static auto ServiceTag = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("ServiceTag")));
    static auto Priority = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("Priority")));
    static auto MaxServices = StaticClass()->FindPropertyByName(FName(TEXT("MaxServices")));
    static auto NPCQuestRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCQuestRow"));
    static auto QuestNPC = NPCQuestRow->FindPropertyByName(FName(TEXT("NPC")));
    static auto Quest = NPCQuestRow->FindPropertyByName(FName(TEXT("Quest")));
    static auto Weight = NPCQuestRow->FindPropertyByName(FName(TEXT("Weight")));

    auto& Services = GetServices();

    if (!Services)
        Services = LoadObject<UDataTable>(nullptr, TEXT("/BattlepassS15/Balance/DataTables/NPCServices.NPCServices"));

    if (!GetSalesInventory())
        GetSalesInventory() = LoadObject<UDataTable>(nullptr, TEXT("/BattlepassS15/Balance/DataTables/NPCSales.NPCSales"));

    auto Quests = GetQuests();

    if (Quests && !*Quests)
        *Quests = LoadObject<UDataTable>(nullptr, TEXT("/BattlepassS15/Balance/DataTables/NPCQuests.NPCQuests"));

    auto ServicesTable = UFortGameFeatureData::GetPlaylistDataTable(this, Services);
    auto QuestsTable = Quests ? UFortGameFeatureData::GetPlaylistDataTable(this, *Quests) : UFortGameFeatureData::FindPlaylistDataTable(this, NPCQuestRow);

    if (!ServicesTable)
        return;

    const FGameplayTag NPCTag = GetNPCTag();
    std::vector<const uint8*> RolledServices;

    for (auto& RowPair : ServicesTable->RowMap)
    {
        const uint8* Row = RowPair.Value;

        if (*NPC->ContainerPtrToValuePtr<FGameplayTag>(Row) == NPCTag && FMath::FRand() < *Chance->ContainerPtrToValuePtr<float>(Row))
            RolledServices.push_back(Row);
    }

    std::stable_sort(RolledServices.begin(), RolledServices.end(), [](const uint8* A, const uint8* B)
    {
        return *Priority->ContainerPtrToValuePtr<int32>(A) > *Priority->ContainerPtrToValuePtr<int32>(B);
    });

    const int32 ServiceLimit = *MaxServices->ContainerPtrToValuePtr<int32>(this);

    for (size_t Index = 0; Index < RolledServices.size() && (ServiceLimit <= 0 || Index < (size_t)ServiceLimit); Index++)
        GetSupportedServices().AddTag(*ServiceTag->ContainerPtrToValuePtr<FGameplayTag>(RolledServices[Index]));

    if (!QuestsTable)
        return;

    std::vector<const uint8*> QuestRows;

    for (auto& RowPair : QuestsTable->RowMap)
    {
        if (*QuestNPC->ContainerPtrToValuePtr<FGameplayTag>(RowPair.Value) == NPCTag && *Weight->ContainerPtrToValuePtr<float>(RowPair.Value) > 0.0f)
            QuestRows.push_back(RowPair.Value);
    }

    while (!QuestRows.empty())
    {
        float TotalWeight = 0.0f;

        for (auto Row : QuestRows)
            TotalWeight += *Weight->ContainerPtrToValuePtr<float>(Row);

        float Roll = FMath::FRand() * TotalWeight;
        size_t Picked = QuestRows.size() - 1;

        for (size_t Index = 0; Index < QuestRows.size(); Index++)
        {
            Roll -= *Weight->ContainerPtrToValuePtr<float>(QuestRows[Index]);

            if (Roll <= 0.0f)
            {
                Picked = Index;
                break;
            }
        }

        GetSupportedQuests().Add(*Quest->ContainerPtrToValuePtr<TSoftObjectPtr<UFortQuestItemDefinition>>(QuestRows[Picked]));
        QuestRows.erase(QuestRows.begin() + Picked);
    }
}

void UFortNPCConversationParticipantComponent::UpdateCanStartConversation()
{
    static auto UpdateCanStartConversation = Anchor::Function<void (*)(UFortNPCConversationParticipantComponent*)>(TEXT("UFortNPCConversationParticipantComponent::UpdateCanStartConversation"))
        .Native(TEXT("/Script/FortniteGame.FortNPCConversationParticipantComponent:OnBotControllerDBNOStatusChanged"))
        .Get();

    UpdateCanStartConversation(this);
}

void UFortNPCConversationParticipantComponent::OnServerConversationStarted(UConversationInstance* Conversation, FGameplayTag AsParticipant)
{
    SetConversationModeActive(true);
}

void UFortNPCConversationParticipantComponent::OnServerConversationEnded(UConversationInstance* Conversation)
{
    if (!IsInActiveConversation())
        SetConversationModeActive(false);
}

FGameplayTag UFortNPCConversationParticipantComponent::GetNPCTag() const
{
    static auto CharacterData = StaticClass()->FindPropertyByName(FName(TEXT("CharacterData")));
    static auto GameplayTag = ((UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortTandemCharacterData")))->FindPropertyByName(FName(TEXT("GameplayTag")));

    auto TandemCharacterData = *CharacterData->ContainerPtrToValuePtr<UObject*>(this);

    return TandemCharacterData ? *GameplayTag->ContainerPtrToValuePtr<FGameplayTag>(TandemCharacterData) : FGameplayTag();
}

TArray<const uint8*> UFortNPCConversationParticipantComponent::GetSupportedSaleRows() const
{
    static auto NPCSaleInventoryRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCSaleInventoryRow"));
    static auto NPC = NPCSaleInventoryRow->FindPropertyByName(FName(TEXT("NPC")));
    static auto Priority = NPCSaleInventoryRow->FindPropertyByName(FName(TEXT("Priority")));

    TArray<const uint8*> SaleRows;
    auto SalesInventory = UFortGameFeatureData::GetPlaylistDataTable(this, GetSalesInventory());

    if (!SalesInventory)
        return SaleRows;

    const FGameplayTag NPCTag = GetNPCTag();
    std::vector<const uint8*> Rows;

    for (auto& RowPair : SalesInventory->RowMap)
    {
        if (*NPC->ContainerPtrToValuePtr<FGameplayTag>(RowPair.Value) == NPCTag)
            Rows.push_back(RowPair.Value);
    }

    std::stable_sort(Rows.begin(), Rows.end(), [](const uint8* A, const uint8* B)
    {
        return *Priority->ContainerPtrToValuePtr<int32>(A) > *Priority->ContainerPtrToValuePtr<int32>(B);
    });

    for (auto Row : Rows)
        SaleRows.Add(Row);

    return SaleRows;
}

TArray<const uint8*> UFortNPCConversationParticipantComponent::GetSupportedGiftRows() const
{
    static auto Gifts = StaticClass()->FindPropertyByName(FName(TEXT("Gifts")));
    static auto NPCConversationGiftRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCConversationGiftRow"));
    static auto NPC = NPCConversationGiftRow ? NPCConversationGiftRow->FindPropertyByName(FName(TEXT("NPC"))) : nullptr;

    TArray<const uint8*> GiftRows;
    auto GiftsTable = Gifts ? UFortGameFeatureData::GetPlaylistDataTable(this, *Gifts->ContainerPtrToValuePtr<UDataTable*>(this)) : nullptr;

    if (!GiftsTable || !NPC)
        return GiftRows;

    const FGameplayTag NPCTag = GetNPCTag();

    for (auto& RowPair : GiftsTable->RowMap)
    {
        const FGameplayTag& RowNPC = *NPC->ContainerPtrToValuePtr<FGameplayTag>(RowPair.Value);

        if (!RowNPC.IsValid() || RowNPC == NPCTag)
            GiftRows.Add(RowPair.Value);
    }

    return GiftRows;
}

TArray<const uint8*> UFortNPCConversationParticipantComponent::GetSupportedGiftItemRows() const
{
    static auto GiftItemInventory = StaticClass()->FindPropertyByName(FName(TEXT("GiftItemInventory")));
    static auto NPCGiftItemInventoryRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCGiftItemInventoryRow"));
    static auto NPC = NPCGiftItemInventoryRow ? NPCGiftItemInventoryRow->FindPropertyByName(FName(TEXT("NPC"))) : nullptr;

    TArray<const uint8*> GiftItemRows;
    auto GiftItemTable = GiftItemInventory ? UFortGameFeatureData::GetPlaylistDataTable(this, *GiftItemInventory->ContainerPtrToValuePtr<UDataTable*>(this)) : nullptr;

    if (!GiftItemTable || !NPC)
        return GiftItemRows;

    const FGameplayTag NPCTag = GetNPCTag();

    for (auto& RowPair : GiftItemTable->RowMap)
    {
        const FGameplayTag& RowNPC = *NPC->ContainerPtrToValuePtr<FGameplayTag>(RowPair.Value);

        if (!RowNPC.IsValid() || RowNPC == NPCTag)
            GiftItemRows.Add(RowPair.Value);
    }

    return GiftItemRows;
}

UFortConversationGiftTypeDefinitions* UFortNPCConversationParticipantComponent::GetGiftTypeDefinitions() const
{
    static auto GiftTypeDefinitions = StaticClass()->FindPropertyByName(FName(TEXT("GiftTypeDefinitions")));

    return GiftTypeDefinitions ? *GiftTypeDefinitions->ContainerPtrToValuePtr<UFortConversationGiftTypeDefinitions*>(this) : nullptr;
}

AFortPlayerPawn* UFortNPCConversationParticipantComponent::GetPlayerPawnOwner() const
{
    static auto PlayerPawnOwner = StaticClass()->FindPropertyByName(FName(TEXT("PlayerPawnOwner")));

    return *PlayerPawnOwner->ContainerPtrToValuePtr<AFortPlayerPawn*>(this);
}

AFortAthenaAIBotController*& UFortNPCConversationParticipantComponent::GetBotControllerOwner() const
{
    static auto BotControllerOwner = StaticClass()->FindPropertyByName(FName(TEXT("BotControllerOwner")));

    return *BotControllerOwner->ContainerPtrToValuePtr<AFortAthenaAIBotController*>((void*)this);
}

TArray<TSoftObjectPtr<UFortQuestItemDefinition>>& UFortNPCConversationParticipantComponent::GetSupportedQuests() const
{
    static auto SupportedQuests = StaticClass()->FindPropertyByName(FName(TEXT("SupportedQuests")));

    return *SupportedQuests->ContainerPtrToValuePtr<TArray<TSoftObjectPtr<UFortQuestItemDefinition>>>((void*)this);
}

FGameplayTagContainer& UFortNPCConversationParticipantComponent::GetSupportedServices() const
{
    static auto SupportedServices = StaticClass()->FindPropertyByName(FName(TEXT("SupportedServices")));

    return *SupportedServices->ContainerPtrToValuePtr<FGameplayTagContainer>((void*)this);
}

UDataTable*& UFortNPCConversationParticipantComponent::GetServices() const
{
    static auto Services = StaticClass()->FindPropertyByName(FName(TEXT("Services")));

    return *Services->ContainerPtrToValuePtr<UDataTable*>((void*)this);
}

UDataTable*& UFortNPCConversationParticipantComponent::GetSalesInventory() const
{
    static auto SalesInventory = StaticClass()->FindPropertyByName(FName(TEXT("SalesInventory")));

    return *SalesInventory->ContainerPtrToValuePtr<UDataTable*>((void*)this);
}

UDataTable** UFortNPCConversationParticipantComponent::GetQuests() const
{
    static auto Quests = StaticClass()->FindPropertyByName(FName(TEXT("Quests")));

    return Quests ? Quests->ContainerPtrToValuePtr<UDataTable*>((void*)this) : nullptr;
}

void UFortNPCConversationParticipantComponent::SetConversationModeActive(bool bInConversationModeActive)
{
    static auto bConversationModeActive = StaticClass()->FindPropertyByName(FName(TEXT("bConversationModeActive")));

    *bConversationModeActive->ContainerPtrToValuePtr<bool>(this) = bInConversationModeActive;
}
