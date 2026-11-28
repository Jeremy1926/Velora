#include "pch.h"
#include "Player/FortPlayerControllerGameplay.h"
#include "FortKismetLibrary.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Items/FortItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "Misc/Guid.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FGhostModeRepData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.GhostModeRepData"));

    return Struct;
}

bool FGhostModeRepData::GetInGhostMode() const
{
    static auto bInGhostMode = StaticStruct()->FindPropertyByName(FName(TEXT("bInGhostMode")));

    return *bInGhostMode->ContainerPtrToValuePtr<bool>(this);
}

UFortWorldItemDefinition* FGhostModeRepData::GetGhostModeItemDef() const
{
    static auto GhostModeItemDef = StaticStruct()->FindPropertyByName(FName(TEXT("GhostModeItemDef")));

    return *GhostModeItemDef->ContainerPtrToValuePtr<UFortWorldItemDefinition*>(this);
}

FGhostModeRepData& AFortPlayerControllerGameplay::GetGhostModeRepData()
{
    static auto GhostModeRepData = GetClass()->FindPropertyByName(FName(TEXT("GhostModeRepData")));

    return *GhostModeRepData->ContainerPtrToValuePtr<FGhostModeRepData>(this);
}

void AFortPlayerControllerGameplay::StartGhostMode(UFortWorldItemDefinition* ItemProvidingGhostMode)
{
    DETOUR(AFortPlayerControllerGameplay, StartGhostMode, Anchor::Hook()
        .If(DETOUR_ORIGINAL(UFortKismetLibrary, GiveItemToInventoryOwnerWithVariant) != nullptr)
        .Native(TEXT("/Script/FortniteGame.FortPlayerControllerGameplay:StartGhostMode"))
        .Inline());

    auto PlayerPawn = Cast<AFortPlayerPawnAthena>(GetPawn());

    if (HasAuthority() && !GetGhostModeRepData().GetInGhostMode() && (!PlayerPawn || !PlayerPawn->GetIsDBNO()))
        UFortKismetLibrary::RemoveItemFromPlayer(this, FItemVariantHandle(ItemProvidingGhostMode, FGuid()), -1, true);

    Original(ItemProvidingGhostMode);
}
