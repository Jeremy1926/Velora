#include "pch.h"
#include "FortRuntimeOptions.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

UClass* UFortRuntimeOptions::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortRuntimeOptions"));

    return Class;
}

const UFortRuntimeOptions& UFortRuntimeOptions::Get()
{
    return *StaticClass()->GetDefaultObject<UFortRuntimeOptions>();
}

float UFortRuntimeOptions::GetBRServerMaxTickRate() const
{
    static auto BRServerMaxTickRate = StaticClass()->FindPropertyByName(FName(TEXT("BRServerMaxTickRate")));

    return *BRServerMaxTickRate->ContainerPtrToValuePtr<float>(this);
}

bool UFortRuntimeOptions::GetDisableReplicationGraph() const
{
    static auto bDisableReplicationGraph = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bDisableReplicationGraph")));

    return bDisableReplicationGraph->GetPropertyValue_InContainer(this);
}
