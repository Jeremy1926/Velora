#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/NetSerialization.h"

class AActor;
class UScriptStruct;

enum class EShareActorWith : uint8
{
    None,
    SquadOnTeam,
    AllTeam,
};

struct FIndicatedActorData
{
    FIndicatedActorData(const FIndicatedActorData&) = delete;

    ~FIndicatedActorData();

    static UScriptStruct* StaticStruct();

    float GetDuration() const;

    float GetStepTime() const;

    EShareActorWith GetShareActorWith() const;
};

struct FStenciledActorData
{
    FStenciledActorData(const FStenciledActorData&) = delete;

    ~FStenciledActorData();

    static UScriptStruct* StaticStruct();

    float GetDuration() const;

    float GetStepTime() const;

    EShareActorWith GetShareActorWith() const;
};

struct FIndicatedActorInfoEntry : public FFastArraySerializerItem
{
    FIndicatedActorInfoEntry(AActor* InActor, float InStartTime, float InEndTime, const FIndicatedActorData& InData);

    FIndicatedActorInfoEntry(const FIndicatedActorInfoEntry&) = delete;

    ~FIndicatedActorInfoEntry();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    AActor* GetActor() const;

    float GetStartTime() const;

    float GetEndTime() const;

    const FIndicatedActorData& GetData() const;
};

template<>
class TArray<FIndicatedActorInfoEntry, FDefaultAllocator> : public TRuntimeSizedArray<FIndicatedActorInfoEntry>
{
};

struct FIndicatedActorInfoArray : public FFastArraySerializer
{
    static UScriptStruct* StaticStruct();

    TArray<FIndicatedActorInfoEntry>& GetEntries();
};

struct FStenciledActorInfoEntry : public FFastArraySerializerItem
{
    FStenciledActorInfoEntry(AActor* InActor, float InStartTime, float InEndTime, const FStenciledActorData& InData);

    FStenciledActorInfoEntry(const FStenciledActorInfoEntry&) = delete;

    ~FStenciledActorInfoEntry();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    AActor* GetActor() const;

    float GetStartTime() const;

    float GetEndTime() const;

    const FStenciledActorData& GetData() const;
};

template<>
class TArray<FStenciledActorInfoEntry, FDefaultAllocator> : public TRuntimeSizedArray<FStenciledActorInfoEntry>
{
};

struct FStenciledActorInfoArray : public FFastArraySerializer
{
    static UScriptStruct* StaticStruct();

    TArray<FStenciledActorInfoEntry>& GetEntries();
};

class UFortControllerComponent_IndicatedActorManagement : public UActorComponent
{
public:
    static UClass* StaticClass();

    FIndicatedActorInfoArray& GetIndicatedActorList();

    TArray<FIndicatedActorInfoEntry>& GetDeferredIndicatedActorList();

    FStenciledActorInfoArray& GetStenciledActorList();

    TArray<FStenciledActorInfoEntry>& GetDeferredStenciledActorList();

    void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

    void AddActorsToIndicatedList(const TArray<AActor*>& IndicatedActors, const FIndicatedActorData& Data, const bool bAddAsUnique);

    void AddActorsToStenciledList(const TArray<AActor*>& StenciledActors, const FStenciledActorData& Data, const bool bAddAsUnique);
};
