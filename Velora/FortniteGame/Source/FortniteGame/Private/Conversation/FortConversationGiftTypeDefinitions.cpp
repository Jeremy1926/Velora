#include "pch.h"
#include "Conversation/FortConversationGiftTypeDefinitions.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

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
