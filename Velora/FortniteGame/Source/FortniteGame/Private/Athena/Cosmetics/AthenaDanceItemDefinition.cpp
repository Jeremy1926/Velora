#include "pch.h"
#include "Athena/Cosmetics/AthenaDanceItemDefinition.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UAthenaDanceItemDefinition::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaDanceItemDefinition"));

    return Class;
}

bool UAthenaDanceItemDefinition::GetMovingEmote() const
{
    static auto bMovingEmote = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bMovingEmote")));

    return bMovingEmote && bMovingEmote->GetPropertyValue_InContainer(this);
}

bool UAthenaDanceItemDefinition::GetMoveForwardOnly() const
{
    static auto bMoveForwardOnly = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bMoveForwardOnly")));

    return bMoveForwardOnly && bMoveForwardOnly->GetPropertyValue_InContainer(this);
}
