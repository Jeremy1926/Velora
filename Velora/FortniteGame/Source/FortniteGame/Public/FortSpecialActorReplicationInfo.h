#pragma once

#include "GameFramework/Info.h"
#include "UObject/NameTypes.h"

class AActor;

struct alignas(16) FSpecialActorInitData
{
    FSpecialActorInitData();

    ~FSpecialActorInitData();

    AActor* SpecialActor;

    uint8 Data[0x7F8];
};

class AFortSpecialActorReplicationInfo : public AInfo
{
public:
    int32 AddActorToReplicationList(const FSpecialActorInitData& InitData);

    void RemoveActorFromReplicationList(FName SpecialActorID);
};
