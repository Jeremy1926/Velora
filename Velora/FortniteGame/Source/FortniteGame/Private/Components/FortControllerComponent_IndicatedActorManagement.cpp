#include "pch.h"
#include "Components/FortControllerComponent_IndicatedActorManagement.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "FortKismetLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

FIndicatedActorData::~FIndicatedActorData()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FIndicatedActorData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.IndicatedActorData"));

    return Struct;
}

float FIndicatedActorData::GetDuration() const
{
    static auto Duration = StaticStruct()->FindPropertyByName(FName(TEXT("Duration")));

    return *Duration->ContainerPtrToValuePtr<float>(this);
}

float FIndicatedActorData::GetStepTime() const
{
    static auto StepTime = StaticStruct()->FindPropertyByName(FName(TEXT("StepTime")));

    return *StepTime->ContainerPtrToValuePtr<float>(this);
}

EShareActorWith FIndicatedActorData::GetShareActorWith() const
{
    static auto ShareActorWith = StaticStruct()->FindPropertyByName(FName(TEXT("ShareActorWith")));
    static auto bIncludeSquad = StaticStruct()->FindPropertyByName(FName(TEXT("bIncludeSquad")));

    if (!ShareActorWith)
        return *bIncludeSquad->ContainerPtrToValuePtr<bool>(this) ? EShareActorWith::SquadOnTeam : EShareActorWith::None;

    return *ShareActorWith->ContainerPtrToValuePtr<EShareActorWith>(this);
}

FStenciledActorData::~FStenciledActorData()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FStenciledActorData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.StenciledActorData"));

    return Struct;
}

float FStenciledActorData::GetDuration() const
{
    static auto Duration = StaticStruct()->FindPropertyByName(FName(TEXT("Duration")));

    return *Duration->ContainerPtrToValuePtr<float>(this);
}

float FStenciledActorData::GetStepTime() const
{
    static auto StepTime = StaticStruct()->FindPropertyByName(FName(TEXT("StepTime")));

    return *StepTime->ContainerPtrToValuePtr<float>(this);
}

EShareActorWith FStenciledActorData::GetShareActorWith() const
{
    static auto ShareActorWith = StaticStruct()->FindPropertyByName(FName(TEXT("ShareActorWith")));

    return *ShareActorWith->ContainerPtrToValuePtr<EShareActorWith>(this);
}

FIndicatedActorInfoEntry::FIndicatedActorInfoEntry(AActor* InActor, float InStartTime, float InEndTime, const FIndicatedActorData& InData)
{
    static auto Actor = StaticStruct()->FindPropertyByName(FName(TEXT("Actor")));
    static auto StartTime = StaticStruct()->FindPropertyByName(FName(TEXT("StartTime")));
    static auto EndTime = StaticStruct()->FindPropertyByName(FName(TEXT("EndTime")));
    static auto Data = []
    {
        auto Data = StaticStruct()->FindPropertyByName(FName(TEXT("Data")));

        return Data ? Data : StaticStruct()->FindPropertyByName(FName(TEXT("IndicatedActorData")));
    }();

    StaticStruct()->InitializeStruct(this);

    *Actor->ContainerPtrToValuePtr<AActor*>(this) = InActor;
    *StartTime->ContainerPtrToValuePtr<float>(this) = InStartTime;
    *EndTime->ContainerPtrToValuePtr<float>(this) = InEndTime;

    FIndicatedActorData::StaticStruct()->CopyScriptStruct(Data->ContainerPtrToValuePtr<FIndicatedActorData>(this), &InData);
}

FIndicatedActorInfoEntry::~FIndicatedActorInfoEntry()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FIndicatedActorInfoEntry::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.IndicatedActorInfoEntry"));

    return Struct;
}

int32 FIndicatedActorInfoEntry::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

AActor* FIndicatedActorInfoEntry::GetActor() const
{
    static auto Actor = StaticStruct()->FindPropertyByName(FName(TEXT("Actor")));

    return *Actor->ContainerPtrToValuePtr<AActor*>(this);
}

float FIndicatedActorInfoEntry::GetStartTime() const
{
    static auto StartTime = StaticStruct()->FindPropertyByName(FName(TEXT("StartTime")));

    return *StartTime->ContainerPtrToValuePtr<float>(this);
}

float FIndicatedActorInfoEntry::GetEndTime() const
{
    static auto EndTime = StaticStruct()->FindPropertyByName(FName(TEXT("EndTime")));

    return *EndTime->ContainerPtrToValuePtr<float>(this);
}

const FIndicatedActorData& FIndicatedActorInfoEntry::GetData() const
{
    static auto Data = []
    {
        auto Data = StaticStruct()->FindPropertyByName(FName(TEXT("Data")));

        return Data ? Data : StaticStruct()->FindPropertyByName(FName(TEXT("IndicatedActorData")));
    }();

    return *Data->ContainerPtrToValuePtr<FIndicatedActorData>(this);
}

UScriptStruct* FIndicatedActorInfoArray::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.IndicatedActorInfoArray"));

    return Struct;
}

TArray<FIndicatedActorInfoEntry>& FIndicatedActorInfoArray::GetEntries()
{
    static auto Entries = StaticStruct()->FindPropertyByName(FName(TEXT("Entries")));

    return *Entries->ContainerPtrToValuePtr<TArray<FIndicatedActorInfoEntry>>(this);
}

FStenciledActorInfoEntry::FStenciledActorInfoEntry(AActor* InActor, float InStartTime, float InEndTime, const FStenciledActorData& InData)
{
    static auto Actor = StaticStruct()->FindPropertyByName(FName(TEXT("Actor")));
    static auto StartTime = StaticStruct()->FindPropertyByName(FName(TEXT("StartTime")));
    static auto EndTime = StaticStruct()->FindPropertyByName(FName(TEXT("EndTime")));
    static auto Data = StaticStruct()->FindPropertyByName(FName(TEXT("Data")));

    StaticStruct()->InitializeStruct(this);

    *Actor->ContainerPtrToValuePtr<AActor*>(this) = InActor;
    *StartTime->ContainerPtrToValuePtr<float>(this) = InStartTime;
    *EndTime->ContainerPtrToValuePtr<float>(this) = InEndTime;

    FStenciledActorData::StaticStruct()->CopyScriptStruct(Data->ContainerPtrToValuePtr<FStenciledActorData>(this), &InData);
}

FStenciledActorInfoEntry::~FStenciledActorInfoEntry()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FStenciledActorInfoEntry::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.StenciledActorInfoEntry"));

    return Struct;
}

int32 FStenciledActorInfoEntry::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

AActor* FStenciledActorInfoEntry::GetActor() const
{
    static auto Actor = StaticStruct()->FindPropertyByName(FName(TEXT("Actor")));

    return *Actor->ContainerPtrToValuePtr<AActor*>(this);
}

float FStenciledActorInfoEntry::GetStartTime() const
{
    static auto StartTime = StaticStruct()->FindPropertyByName(FName(TEXT("StartTime")));

    return *StartTime->ContainerPtrToValuePtr<float>(this);
}

float FStenciledActorInfoEntry::GetEndTime() const
{
    static auto EndTime = StaticStruct()->FindPropertyByName(FName(TEXT("EndTime")));

    return *EndTime->ContainerPtrToValuePtr<float>(this);
}

const FStenciledActorData& FStenciledActorInfoEntry::GetData() const
{
    static auto Data = StaticStruct()->FindPropertyByName(FName(TEXT("Data")));

    return *Data->ContainerPtrToValuePtr<FStenciledActorData>(this);
}

UScriptStruct* FStenciledActorInfoArray::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.StenciledActorInfoArray"));

    return Struct;
}

TArray<FStenciledActorInfoEntry>& FStenciledActorInfoArray::GetEntries()
{
    static auto Entries = StaticStruct()->FindPropertyByName(FName(TEXT("Entries")));

    return *Entries->ContainerPtrToValuePtr<TArray<FStenciledActorInfoEntry>>(this);
}

UClass* UFortControllerComponent_IndicatedActorManagement::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_IndicatedActorManagement"));

    return Class;
}

FIndicatedActorInfoArray& UFortControllerComponent_IndicatedActorManagement::GetIndicatedActorList()
{
    static auto IndicatedActorList = GetClass()->FindPropertyByName(FName(TEXT("IndicatedActorList")));

    return *IndicatedActorList->ContainerPtrToValuePtr<FIndicatedActorInfoArray>(this);
}

TArray<FIndicatedActorInfoEntry>& UFortControllerComponent_IndicatedActorManagement::GetDeferredIndicatedActorList()
{
    static auto IndicatedActorList = GetClass()->FindPropertyByName(FName(TEXT("IndicatedActorList")));
    static auto DeferredIndicatedActorList = GetClass()->FindPropertyByName(FName(TEXT("DeferredIndicatedActorList")));

    if (DeferredIndicatedActorList)
        return *DeferredIndicatedActorList->ContainerPtrToValuePtr<TArray<FIndicatedActorInfoEntry>>(this);

    return ((FIndicatedActorInfoArray*)((uint8*)this + IndicatedActorList->GetOffset_ForInternal() + IndicatedActorList->GetSize()))->GetEntries();
}

FStenciledActorInfoArray& UFortControllerComponent_IndicatedActorManagement::GetStenciledActorList()
{
    static auto StenciledActorList = GetClass()->FindPropertyByName(FName(TEXT("StenciledActorList")));

    return *StenciledActorList->ContainerPtrToValuePtr<FStenciledActorInfoArray>(this);
}

TArray<FStenciledActorInfoEntry>& UFortControllerComponent_IndicatedActorManagement::GetDeferredStenciledActorList()
{
    static auto StenciledActorList = GetClass()->FindPropertyByName(FName(TEXT("StenciledActorList")));
    static auto DeferredStenciledActorList = GetClass()->FindPropertyByName(FName(TEXT("DeferredStenciledActorList")));

    if (DeferredStenciledActorList)
        return *DeferredStenciledActorList->ContainerPtrToValuePtr<TArray<FStenciledActorInfoEntry>>(this);

    return ((FStenciledActorInfoArray*)((uint8*)this + StenciledActorList->GetOffset_ForInternal() + StenciledActorList->GetSize()))->GetEntries();
}

void UFortControllerComponent_IndicatedActorManagement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    DETOUR(UFortControllerComponent_IndicatedActorManagement, TickComponent, Anchor::Hook()
        .If(!::Anchor::Exists()
            .String(TEXT("STAT_FortControllerComponent_IndicatedActorManagementTick"))
            .Start()
            .Find("cmp dword ptr [*], 0xffffffff; jnz 0x*; inc [*+0x50]")
            .Get())
        .String(TEXT("STAT_FortControllerComponent_IndicatedActorManagementTick"))
        .Start()
        .Optional()
        .Inline());

    const float ServerWorldTimeSeconds = UFortKismetLibrary::GetServerWorldTimeSeconds(this);
    auto& DeferredIndicatedActorList = GetDeferredIndicatedActorList();

    for (int32 Index = DeferredIndicatedActorList.Num() - 1; Index >= 0; Index--)
    {
        auto& Entry = DeferredIndicatedActorList[Index];

        if (Entry.GetStartTime() <= 0.0f || Entry.GetStartTime() >= ServerWorldTimeSeconds)
            continue;

        auto& IndicatedActorList = GetIndicatedActorList();
        auto NewEntry = new(IndicatedActorList.GetEntries()) FIndicatedActorInfoEntry(Entry.GetActor(), Entry.GetStartTime(), Entry.GetEndTime(), Entry.GetData());

        IndicatedActorList.MarkItemDirty(*NewEntry);
        GetOwner()->ForceNetUpdate();
    }

    auto& DeferredStenciledActorList = GetDeferredStenciledActorList();

    for (int32 Index = DeferredStenciledActorList.Num() - 1; Index >= 0; Index--)
    {
        auto& Entry = DeferredStenciledActorList[Index];

        if (Entry.GetStartTime() <= 0.0f || Entry.GetStartTime() >= ServerWorldTimeSeconds)
            continue;

        auto& StenciledActorList = GetStenciledActorList();
        auto NewEntry = new(StenciledActorList.GetEntries()) FStenciledActorInfoEntry(Entry.GetActor(), Entry.GetStartTime(), Entry.GetEndTime(), Entry.GetData());

        StenciledActorList.MarkItemDirty(*NewEntry);
        GetOwner()->ForceNetUpdate();
    }

    Original(DeltaTime, TickType, ThisTickFunction);
}

void UFortControllerComponent_IndicatedActorManagement::AddActorsToIndicatedList(const TArray<AActor*>& IndicatedActors, const FIndicatedActorData& Data, const bool bAddAsUnique)
{
    TArray<UFortControllerComponent_IndicatedActorManagement*> Components;
    Components.Add(this);

    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetOwner());
    auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;

    if (PlayerState && Data.GetShareActorWith() != EShareActorWith::None)
    {
        for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
        {
            auto SquadMember = Cast<AFortPlayerStateAthena>(OtherPlayerState);

            if (!SquadMember || SquadMember == PlayerState || (Data.GetShareActorWith() == EShareActorWith::AllTeam ? SquadMember->GetTeamIndex() != PlayerState->GetTeamIndex() : SquadMember->GetSquadId() != PlayerState->GetSquadId()))
                continue;

            auto OtherPlayerController = Cast<AFortPlayerControllerAthena>(SquadMember->GetOwner());

            if (auto Component = OtherPlayerController ? OtherPlayerController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr)
                Components.Add(Component);
        }
    }

    const float ServerWorldTimeSeconds = UFortKismetLibrary::GetServerWorldTimeSeconds(this);
    int32 Step = 0;

    for (auto Actor : IndicatedActors)
    {
        if (!Actor)
            continue;

        const float StartTime = ServerWorldTimeSeconds + Data.GetStepTime() * Step++;
        const float EndTime = Data.GetDuration() > 0.0f ? StartTime + Data.GetDuration() : 0.0f;

        for (auto Component : Components)
        {
            auto bIsListed = false;

            if (bAddAsUnique)
            {
                for (auto& Entry : Component->GetIndicatedActorList().GetEntries())
                    bIsListed |= Entry.GetActor() == Actor;

                for (auto& Entry : Component->GetDeferredIndicatedActorList())
                    bIsListed |= Entry.GetActor() == Actor;
            }

            if (!bIsListed)
                new(Component->GetDeferredIndicatedActorList()) FIndicatedActorInfoEntry(Actor, StartTime, EndTime, Data);
        }
    }
}

void UFortControllerComponent_IndicatedActorManagement::AddActorsToStenciledList(const TArray<AActor*>& StenciledActors, const FStenciledActorData& Data, const bool bAddAsUnique)
{
    TArray<UFortControllerComponent_IndicatedActorManagement*> Components;
    Components.Add(this);

    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetOwner());
    auto PlayerState = PlayerController ? Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState()) : nullptr;

    if (PlayerState && Data.GetShareActorWith() != EShareActorWith::None)
    {
        for (auto OtherPlayerState : GetWorld()->GetGameState()->GetPlayerArray())
        {
            auto SquadMember = Cast<AFortPlayerStateAthena>(OtherPlayerState);

            if (!SquadMember || SquadMember == PlayerState || (Data.GetShareActorWith() == EShareActorWith::AllTeam ? SquadMember->GetTeamIndex() != PlayerState->GetTeamIndex() : SquadMember->GetSquadId() != PlayerState->GetSquadId()))
                continue;

            auto OtherPlayerController = Cast<AFortPlayerControllerAthena>(SquadMember->GetOwner());

            if (auto Component = OtherPlayerController ? OtherPlayerController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr)
                Components.Add(Component);
        }
    }

    const float ServerWorldTimeSeconds = UFortKismetLibrary::GetServerWorldTimeSeconds(this);
    int32 Step = 0;

    for (auto Actor : StenciledActors)
    {
        if (!Actor)
            continue;

        const float StartTime = ServerWorldTimeSeconds + Data.GetStepTime() * Step++;
        const float EndTime = Data.GetDuration() > 0.0f ? StartTime + Data.GetDuration() : 0.0f;

        for (auto Component : Components)
        {
            auto bIsListed = false;

            if (bAddAsUnique)
            {
                for (auto& Entry : Component->GetStenciledActorList().GetEntries())
                    bIsListed |= Entry.GetActor() == Actor;

                for (auto& Entry : Component->GetDeferredStenciledActorList())
                    bIsListed |= Entry.GetActor() == Actor;
            }

            if (!bIsListed)
                new(Component->GetDeferredStenciledActorList()) FStenciledActorInfoEntry(Actor, StartTime, EndTime, Data);
        }
    }
}
