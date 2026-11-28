#include "pch.h"
#include "Online/Iris/FortIrisHelper.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Actor.h"
#include "Player/FortPlayerController.h"
#include "UObject/Class.h"
#include "UObject/UObjectArray.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Logging/LogMacros.h"
#include "Scanner.h"
#include "Config.h"

UClass* UFortIrisHelper::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortIrisHelper"));

    return Class;
}

UFortIrisHelper* UFortIrisHelper::Get()
{
    static TWeakObjectPtr<UFortIrisHelper> IrisHelper;

    if (IrisHelper.IsValid() || !StaticClass())
        return IrisHelper.Get();

    for (int32 Index = 0; Index < GUObjectArray.GetObjectArrayNum(); Index++)
    {
        auto ObjectItem = GUObjectArray.IndexToObject(Index);
        auto Object = ObjectItem ? (UObject*)ObjectItem->GetObject() : nullptr;

        if (Object && Object->GetClass() == StaticClass() && !(Object->GetFlags() & RF_ClassDefaultObject))
        {
            IrisHelper = (UFortIrisHelper*)Object;
            break;
        }
    }

    return IrisHelper.Get();
}

void UFortIrisHelper::OnPlayerControllerInventorySwitched(AFortPlayerController* PC)
{
    static auto GetConnectionContext = Anchor::Optional<void (*)(UFortIrisHelper*, void**, AFortPlayerController*)>(TEXT("UFortIrisHelper::OnPlayerControllerInventorySwitched connection context"))
        .Exec(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerInventorySwitched"))
        .Find("movdqu [rbp-0x*], xmm1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerInventorySwitched"))
        .Find("movdqu [rsp+0x*], xmm1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto UpdateConnectionFilters = Anchor::Optional<void (*)(UFortIrisHelper*, void**)>(TEXT("UFortIrisHelper::OnPlayerControllerInventorySwitched filter update"))
        .Exec(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerInventorySwitched"))
        .Find("lea rdx, [rbp-0x*]; call 0x*; test rbx, rbx")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerInventorySwitched"))
        .Find("lea rdx, [rsp+0x*]; call 0x*; test rbx, rbx")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!GetConnectionContext || !UpdateConnectionFilters)
        return;

    void* Context[4] = {};

    GetConnectionContext(this, Context, PC);

    if (Config::bLogReplication)
    {
        UE_LOG(LogVelora, Log, TEXT("Iris inventory switched for %s, connection info %d, replication system %d, bridge %d"), *PC->GetName(), (int32)(Context[1] != nullptr), (int32)(Context[2] != nullptr), (int32)(Context[3] != nullptr));
    }

    if (Context[0] && Context[1] && Context[2] && Context[3])
        UpdateConnectionFilters(this, Context);
}

void UFortIrisHelper::OnPlayerControllerViewTargetChanged(AFortPlayerController* PC, AActor* OldViewTarget, AActor* NewViewTarget)
{
    static auto OnPlayerControllerViewTargetChanged = Anchor::Optional<void (*)(UFortIrisHelper*, AFortPlayerController*, AActor*, AActor*)>(TEXT("UFortIrisHelper::OnPlayerControllerViewTargetChanged"))
        .Exec(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerViewTargetChanged"))
        .Find("setnz *; add [*+0x20], *; call 0x*; mov rbx, [rsp+0x*]; add rsp, 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (OnPlayerControllerViewTargetChanged)
    {
        OnPlayerControllerViewTargetChanged(this, PC, OldViewTarget, NewViewTarget);
        return;
    }

    static auto GetConnectionContext = Anchor::Optional<void (*)(UFortIrisHelper*, void**, AFortPlayerController*)>(TEXT("UFortIrisHelper::OnPlayerControllerViewTargetChanged connection context"))
        .Exec(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerViewTargetChanged"))
        .Find("movdqu [rbp-0x*], xmm1; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto UpdateConnectionFilters = Anchor::Optional<void (*)(UFortIrisHelper*, void**)>(TEXT("UFortIrisHelper::OnPlayerControllerViewTargetChanged filter update"))
        .Exec(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerViewTargetChanged"))
        .Find("lea rdx, [rbp-0x*]; call 0x*; mov r9, *; lea rdx, [rbp-0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Spectating_On_%s"))
        .Last("lea rdx, [rbp-0x*]; call 0x*; mov r*, [rbp-0x*]; mov r*, [rbp-0x*]")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!GetConnectionContext || !UpdateConnectionFilters)
        return;

    void* Context[4] = {};

    GetConnectionContext(this, Context, PC);

    if (Context[0] && Context[1] && Context[2] && Context[3])
    {
        UpdateConnectionFilters(this, Context);
        UpdateSpectatingGroup(Context, OldViewTarget, NewViewTarget);
    }
}

void UFortIrisHelper::UpdateSpectatingGroup(void** Context, AActor* OldViewTarget, AActor* NewViewTarget)
{
    static auto UpdateSpectatingGroup = Anchor::Optional<void (*)(UFortIrisHelper*, void**, AActor*, AActor*)>(TEXT("UFortIrisHelper::OnPlayerControllerViewTargetChanged spectating group"))
        .Exec(TEXT("/Script/FortniteGame.FortIrisHelper:OnPlayerControllerViewTargetChanged"))
        .Find("lea rdx, [rbp-0x*]; call 0x*; mov r9, *; lea rdx, [rbp-0x*]; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Get();

    if (UpdateSpectatingGroup)
    {
        UpdateSpectatingGroup(this, Context, OldViewTarget, NewViewTarget);
        return;
    }

    static auto SpectatingGroup = Anchor::Offset(TEXT("UFortIrisHelper::FConnectionInfo::SpectatingGroup"))
        .String(TEXT("Spectating_On_%s"))
        .Last("cmp [r*+0x*], r*; jz 0x*; cmp [r*+0x*], r*; jz 0x*")
        .Disp();

    static auto SpectatedHandle = Anchor::Offset(TEXT("UFortIrisHelper::FConnectionInfo::SpectatedHandle"))
        .String(TEXT("Spectating_On_%s"))
        .Last("cmp [r*+0x*], r*; jz 0x*; cmp [r*+0x*], r*; jz 0x*")
        .Find("cmp [r*+0x*], r*")
        .Disp();

    static auto ConnectionId = Anchor::Offset(TEXT("UFortIrisHelper::FConnectionInfo::ConnectionId"))
        .String(TEXT("Spectating_On_%s"))
        .Find("mov r8d, [r*+0x*]")
        .Disp();

    if (SpectatingGroup <= 0 || SpectatedHandle <= 0 || ConnectionId <= 0)
        return;

    auto ConnectionInfo = (uint8*)Context[1];
    auto ReplicationSystem = (UReplicationSystem*)Context[2];
    auto ReplicationBridge = (UObjectReplicationBridge*)Context[3];
    auto& GroupHandle = *(uint64*)(ConnectionInfo + SpectatingGroup);
    auto& SpectatedRefHandle = *(uint64*)(ConnectionInfo + SpectatedHandle);

    if (GroupHandle && SpectatedRefHandle)
    {
        ReplicationSystem->RemoveFromGroup(GroupHandle, SpectatedRefHandle);
        SpectatedRefHandle = 0;
    }

    if (!NewViewTarget || NewViewTarget->GetOnlyRelevantToOwner())
        return;

    if (!GroupHandle)
    {
        FString GroupName(TEXT("Spectating_On_"));
        GroupName += ((UObject*)Context[0])->GetName();

        GroupHandle = ReplicationSystem->CreateGroup(FName(*GroupName));
        ReplicationSystem->AddInclusionFilterGroup(GroupHandle);
        ReplicationSystem->SetGroupFilterStatus(GroupHandle, *(uint32*)(ConnectionInfo + ConnectionId), UE::Net::ENetFilterStatus::Allow);
    }

    const uint64 NetRefHandle = ReplicationBridge->GetReplicatedRefHandle(NewViewTarget);

    ReplicationSystem->AddToGroup(GroupHandle, NetRefHandle);
    SpectatedRefHandle = NetRefHandle;
}
