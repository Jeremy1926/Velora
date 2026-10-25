#include "pch.h"
#include "Conversation/FortNonPlayerConversationParticipantComponent.h"
#include "Conversation/FortNPCConversationParticipantComponent.h"
#include "Engine/DataTable.h"
#include "FortGameFeatureData.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortNonPlayerConversationParticipantComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortNonPlayerConversationParticipantComponent"));

    return Class;
}

void UFortNonPlayerConversationParticipantComponent::InitializeServices()
{
    static auto NPCDynamicServiceRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCDynamicServiceRow"));
    static auto NPC = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("NPC")));
    static auto Chance = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("Chance")));
    static auto ServiceTag = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("ServiceTag")));
    static auto Priority = NPCDynamicServiceRow->FindPropertyByName(FName(TEXT("Priority")));
    static auto MaxServices = UFortNPCConversationParticipantComponent::StaticClass()->FindPropertyByName(FName(TEXT("MaxServices")));
    static auto bServiceDataInitialized = StaticClass()->FindPropertyByName(FName(TEXT("bServiceDataInitialized")));

    if (!HasServiceData())
        return;

    if (bServiceDataInitialized)
    {
        if (*bServiceDataInitialized->ContainerPtrToValuePtr<bool>(this))
            return;

        *bServiceDataInitialized->ContainerPtrToValuePtr<bool>(this) = true;
    }

    auto& Services = GetServices();

    if (!Services)
        Services = LoadObject<UDataTable>(nullptr, TEXT("/BattlepassS15/Balance/DataTables/NPCServices.NPCServices"));

    if (!GetSalesInventory())
        GetSalesInventory() = LoadObject<UDataTable>(nullptr, TEXT("/BattlepassS15/Balance/DataTables/NPCSales.NPCSales"));

    auto ServicesTable = UFortGameFeatureData::GetPlaylistDataTable(this, Services);

    if (!ServicesTable)
        return;

    const FGameplayTag ProviderTag = GetServiceProviderTag();
    std::vector<const uint8*> RolledServices;

    for (auto& RowPair : ServicesTable->RowMap)
    {
        const uint8* Row = RowPair.Value;

        if (NPC->GetPropertyValue_InContainer<FGameplayTag>(Row) == ProviderTag && FMath::FRand() < *Chance->ContainerPtrToValuePtr<float>(Row))
            RolledServices.push_back(Row);
    }

    std::stable_sort(RolledServices.begin(), RolledServices.end(), [](const uint8* A, const uint8* B)
    {
        return *Priority->ContainerPtrToValuePtr<int32>(A) > *Priority->ContainerPtrToValuePtr<int32>(B);
    });

    const int32 ServiceLimit = *MaxServices->ContainerPtrToValuePtr<int32>(this);

    for (size_t Index = 0; Index < RolledServices.size() && (ServiceLimit <= 0 || Index < (size_t)ServiceLimit); Index++)
        GetSupportedServices().AddTag(ServiceTag->GetPropertyValue_InContainer<FGameplayTag>(RolledServices[Index]));
}

bool UFortNonPlayerConversationParticipantComponent::HasServiceData() const
{
    static auto SupportedServices = StaticClass()->FindPropertyByName(FName(TEXT("SupportedServices")));

    return SupportedServices || Cast<UFortNPCConversationParticipantComponent>(this);
}

FGameplayTag UFortNonPlayerConversationParticipantComponent::GetServiceProviderTag() const
{
    if (auto NPCConversationParticipantComponent = Cast<UFortNPCConversationParticipantComponent>(this))
        return NPCConversationParticipantComponent->GetNPCTag();

    return GetConversationEntryTag();
}

TArray<const uint8*> UFortNonPlayerConversationParticipantComponent::GetSupportedSaleRows() const
{
    static auto NPCSaleInventoryRow = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NPCSaleInventoryRow"));
    static auto NPC = NPCSaleInventoryRow->FindPropertyByName(FName(TEXT("NPC")));
    static auto Priority = NPCSaleInventoryRow->FindPropertyByName(FName(TEXT("Priority")));

    TArray<const uint8*> SaleRows;
    auto SalesInventory = HasServiceData() ? UFortGameFeatureData::GetPlaylistDataTable(this, GetSalesInventory()) : nullptr;

    if (!SalesInventory)
        return SaleRows;

    const FGameplayTag ProviderTag = GetServiceProviderTag();
    std::vector<const uint8*> Rows;

    for (auto& RowPair : SalesInventory->RowMap)
    {
        if (NPC->GetPropertyValue_InContainer<FGameplayTag>(RowPair.Value) == ProviderTag)
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

FGameplayTag UFortNonPlayerConversationParticipantComponent::GetConversationEntryTag() const
{
    static auto ConversationEntryTag = StaticClass()->FindPropertyByName(FName(TEXT("ConversationEntryTag")));

    return ConversationEntryTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

FGameplayTag UFortNonPlayerConversationParticipantComponent::GetInteractorParticipantTag() const
{
    static auto InteractorParticipantTag = StaticClass()->FindPropertyByName(FName(TEXT("InteractorParticipantTag")));

    return InteractorParticipantTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

FGameplayTag UFortNonPlayerConversationParticipantComponent::GetSelfParticipantTag() const
{
    static auto SelfParticipantTag = StaticClass()->FindPropertyByName(FName(TEXT("SelfParticipantTag")));

    return SelfParticipantTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

FGameplayTagContainer& UFortNonPlayerConversationParticipantComponent::GetSupportedServices() const
{
    static auto SupportedServices = UFortNPCConversationParticipantComponent::StaticClass()->FindPropertyByName(FName(TEXT("SupportedServices")));

    return *SupportedServices->ContainerPtrToValuePtr<FGameplayTagContainer>((void*)this);
}

UDataTable*& UFortNonPlayerConversationParticipantComponent::GetServices() const
{
    static auto Services = UFortNPCConversationParticipantComponent::StaticClass()->FindPropertyByName(FName(TEXT("Services")));

    return *Services->ContainerPtrToValuePtr<UDataTable*>((void*)this);
}

UDataTable*& UFortNonPlayerConversationParticipantComponent::GetSalesInventory() const
{
    static auto SalesInventory = UFortNPCConversationParticipantComponent::StaticClass()->FindPropertyByName(FName(TEXT("SalesInventory")));

    return *SalesInventory->ContainerPtrToValuePtr<UDataTable*>((void*)this);
}
