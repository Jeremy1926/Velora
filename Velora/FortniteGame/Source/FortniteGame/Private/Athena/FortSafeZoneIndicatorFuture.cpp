#include "pch.h"
#include "Athena/FortSafeZoneIndicatorFuture.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "GameFramework/PlayerController.h"
#include "UObject/Class.h"
#include "UObject/ObjectKey.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortSafeZoneIndicatorFuture::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneIndicatorFuture"));

    return Class;
}

void AFortSafeZoneIndicatorFuture::SetWhitelistedPlayer(const APlayerController* PC, bool bWhitelist)
{
    auto WhitelistedPlayers = GetWhitelistedPlayers();

    if (!WhitelistedPlayers)
        return;

    if (bWhitelist)
        WhitelistedPlayers->AddUnique(FObjectKey(PC));
    else
        WhitelistedPlayers->RemoveSingleSwap(FObjectKey(PC));
}

TArray<FObjectKey>* AFortSafeZoneIndicatorFuture::GetWhitelistedPlayers()
{
    static auto FutureReplicator = AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("FutureReplicator")));
    static auto WhitelistedPlayers = Anchor::Data<TArray<FObjectKey>*>(TEXT("AFortSafeZoneIndicatorFuture::WhitelistedPlayers"))
        .FieldUserIn(TEXT("/Script/FortniteGame.Default__FortReplicationGraphNode_AlwaysRelevant_ForConnection"), "mov *, [*+%x]; test *, *", FutureReplicator, "movsxd *, dword ptr [0x*]")
        .FieldFind("mov *, [*+%x]; test *, *", FutureReplicator)
        .Find("mov *, [0x*]; movsxd *, dword ptr [0x*]")
        .Target()
        .Optional()
        .Get();

    return WhitelistedPlayers;
}

FVector AFortSafeZoneIndicatorFuture::GetNextNextCenter()
{
    static auto NextNextCenter = GetClass()->FindPropertyByName(FName(TEXT("NextNextCenter")));

    return NextNextCenter->GetPropertyValue_InContainer<FVector>(this);
}

void AFortSafeZoneIndicatorFuture::SetNextNextCenter(const FVector& InNextNextCenter)
{
    static auto NextNextCenter = GetClass()->FindPropertyByName(FName(TEXT("NextNextCenter")));

    NextNextCenter->SetPropertyValue_InContainer<FVector>(this, InNextNextCenter);
}

float& AFortSafeZoneIndicatorFuture::GetNextNextRadius()
{
    static auto NextNextRadius = GetClass()->FindPropertyByName(FName(TEXT("NextNextRadius")));

    return *NextNextRadius->ContainerPtrToValuePtr<float>(this);
}
