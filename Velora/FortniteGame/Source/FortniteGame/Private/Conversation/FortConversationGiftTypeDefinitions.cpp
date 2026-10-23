#include "pch.h"
#include "Conversation/FortConversationGiftTypeDefinitions.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UScriptStruct* FConversationGiftTypeDefinition::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ConversationGiftTypeDefinition"));

    return Struct;
}

uint8 FConversationGiftTypeDefinition::GetRequirementMatchPolicy() const
{
    static auto RequirementMatchPolicy = StaticStruct()->FindPropertyByName(FName(TEXT("RequirementMatchPolicy")));

    return *RequirementMatchPolicy->ContainerPtrToValuePtr<uint8>(this);
}

const TArray<UFortControllerRequirement*>& FConversationGiftTypeDefinition::GetRequirements() const
{
    static auto Requirements = StaticStruct()->FindPropertyByName(FName(TEXT("Requirements")));

    return *Requirements->ContainerPtrToValuePtr<TArray<UFortControllerRequirement*>>(this);
}

const TArray<UFortControllerEffect*>& FConversationGiftTypeDefinition::GetEffects() const
{
    static auto Effects = StaticStruct()->FindPropertyByName(FName(TEXT("Effects")));

    return *Effects->ContainerPtrToValuePtr<TArray<UFortControllerEffect*>>(this);
}

const FConversationGiftTypeDefinition* TMap<FGameplayTag, FConversationGiftTypeDefinition>::Find(const FGameplayTag& Key) const
{
    static auto ElementSize = FConversationGiftTypeDefinition::StaticStruct()->GetPropertiesSize() + 0x10;

    for (int32 Index = 0; Index < Elements.Num(); Index++)
    {
        if (AllocationFlags[Index] && TGameLayout<FGameplayTag>::Load(Elements.GetData() + Index * ElementSize) == Key)
            return (const FConversationGiftTypeDefinition*)(Elements.GetData() + Index * ElementSize + 8);
    }

    return nullptr;
}

UClass* UFortConversationGiftTypeDefinitions::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortConversationGiftTypeDefinitions"));

    return Class;
}

const TMap<FGameplayTag, FConversationGiftTypeDefinition>& UFortConversationGiftTypeDefinitions::GetGiftDefinitionsByGiftTag() const
{
    static auto GiftDefinitionsByGiftTag = StaticClass()->FindPropertyByName(FName(TEXT("GiftDefinitionsByGiftTag")));

    return *GiftDefinitionsByGiftTag->ContainerPtrToValuePtr<TMap<FGameplayTag, FConversationGiftTypeDefinition>>(this);
}
