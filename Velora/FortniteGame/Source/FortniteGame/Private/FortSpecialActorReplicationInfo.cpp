#include "pch.h"
#include "FortSpecialActorReplicationInfo.h"
#include "Scanner.h"

FSpecialActorInitData::FSpecialActorInitData()
{
    static auto Constructor = Anchor::Function<void (*)(FSpecialActorInitData*)>(TEXT("FSpecialActorInitData::FSpecialActorInitData"))
        .Pattern("C7 ? 18 00 00 00 3F C7 ? 1C CD CC CC 3D")
        .Start()
        .Get();

    Constructor(this);
}

FSpecialActorInitData::~FSpecialActorInitData()
{
    static auto Destructor = Anchor::Function<void (*)(FSpecialActorInitData*)>(TEXT("FSpecialActorInitData::~FSpecialActorInitData"))
        .Exec(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:RegisterSpecialActor"))
        .Find("call 0x*; cmp eax, 0xffffffff")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:RegisterSpecialActor"))
        .Find("call 0x*; cmp eax, 0xffffffff")
        .Find("call 0x*")
        .Target()
        .Get();

    Destructor(this);
}

int32 AFortSpecialActorReplicationInfo::AddActorToReplicationList(const FSpecialActorInitData& InitData)
{
    static auto AddActorToReplicationList = Anchor::Function<int32 (*)(AFortSpecialActorReplicationInfo*, const FSpecialActorInitData*)>(TEXT("AFortSpecialActorReplicationInfo::AddActorToReplicationList"))
        .Exec(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:RegisterSpecialActor"))
        .Find("call 0x*; cmp eax, 0xffffffff")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:RegisterSpecialActor"))
        .Find("call 0x*; cmp eax, 0xffffffff")
        .Target()
        .Get();

    return AddActorToReplicationList(this, &InitData);
}

void AFortSpecialActorReplicationInfo::RemoveActorFromReplicationList(FName SpecialActorID)
{
    static auto RemoveActorFromReplicationList = Anchor::Function<void (*)(AFortSpecialActorReplicationInfo*, FName)>(TEXT("AFortSpecialActorReplicationInfo::RemoveActorFromReplicationList"))
        .Exec(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:UnregisterSpecialActor"))
        .Find("mov edx, [r*+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:UnregisterSpecialActor"))
        .Find("jmp 0x*")
        .Target()
        .Find("mov edx, [r*+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    RemoveActorFromReplicationList(this, SpecialActorID);
}
