#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "Containers/Array.h"
#include "Containers/EnumAsByte.h"

class AActor;
class AFortPlayerControllerAthena;
class UScriptStruct;
struct FFrame;
struct FGameplayTagContainer;
struct FGameplayTagQuery;
struct FIndicatedActorData;
struct FStenciledActorData;
enum class EFortTeamAffiliation : uint8;

struct FIndicatedActorDataWithFilter
{
    FIndicatedActorDataWithFilter(const FIndicatedActorDataWithFilter&) = delete;

    ~FIndicatedActorDataWithFilter();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    const FGameplayTagContainer& GetIndicatedActorTags() const;

    const FGameplayTagQuery* GetIndicatedActorTagQuery() const;

    const TArray<TEnumAsByte<EObjectTypeQuery>>& GetObjectTypes() const;

    UClass* GetActorClassFilter() const;

    const TArray<EFortTeamAffiliation>* GetWithAffiliation() const;

    const FIndicatedActorData& GetIndicatedData() const;

    float GetOverlapRadius() const;
};

template<>
class TArray<FIndicatedActorDataWithFilter, FDefaultAllocator> : public TRuntimeSizedArray<FIndicatedActorDataWithFilter>
{
};

class UFortIndicatedActorManagementLibrary : public UBlueprintFunctionLibrary
{
public:
    static void AddActorsInRadiusToIndicatedList(AFortPlayerControllerAthena* InstigatingController, const TArray<FIndicatedActorDataWithFilter>& IndicatedActorFilterDatas, const bool bAddAsUnique, AActor* InstigatingActorOverride);

    void execAddActorsInRadiusToIndicatedList(FFrame& Stack, void* const Z_Param__Result);

    static void AddActorsToIndicatedList(AFortPlayerControllerAthena* InstigatingController, TArray<AActor*> IndicatedActors, FIndicatedActorData IndicatedActorData, const bool bAddAsUnique);

    static void AddActorsToStenciledList(AFortPlayerControllerAthena* InstigatingController, TArray<AActor*> StenciledActors, FStenciledActorData StenciledActorData, const bool bAddAsUnique);

    void execAddActorsToIndicatedList(FFrame& Stack, void* const Z_Param__Result);

    void execAddActorsToStenciledList(FFrame& Stack, void* const Z_Param__Result);
};
