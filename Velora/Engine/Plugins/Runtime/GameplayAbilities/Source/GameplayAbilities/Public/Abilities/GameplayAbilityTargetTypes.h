#pragma once

#include "HAL/Platform.h"
#include "Containers/Array.h"
#include "Containers/ContainerAllocationPolicies.h"
#include "Templates/SharedPointer.h"
#include "Delegates/Delegate.h"
#include "Engine/NetSerialization.h"
#include "GameplayTagContainer.h"
#include "GameplayPrediction.h"

struct FGameplayAbilityTargetData;

struct FGameplayAbilityTargetDataHandle
{
    TArray<TSharedPtr<FGameplayAbilityTargetData>, TInlineAllocator<1>> Data;
};

struct FAbilityReplicatedDataCache
{
    void Reset();
};
