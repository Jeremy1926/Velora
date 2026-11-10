#pragma once

#include "Player/FortPlayerState.h"
#include "AttributeSet.h"
#include "Engine/NetSerialization.h"

class UWorld;

struct FSimulatedAttributeEntry : public FFastArraySerializerItem
{
    FGameplayAttribute Attribute;
    float CurrentValue;
};

class AFortPlayerStateZone : public AFortPlayerState
{
public:
    static UClass* StaticClass();

    static void TickSimulatedAttributes(UWorld* World);

    void UpdateSimulatedAttributes();
};
