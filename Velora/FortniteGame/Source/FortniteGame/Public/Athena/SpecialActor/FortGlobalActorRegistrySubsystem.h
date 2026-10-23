#pragma once

#include "Components/ActorComponent.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "UObject/ScriptDelegates.h"

struct FFrame;
class AActor;

class UActorRegisterData_GameplayTagBased : public UObject
{
public:
    static UClass* StaticClass();

    void RegisterActors(const TArray<AActor*>& InActors);

    void UnregisterActors(const TArray<AActor*>& InActors);

    void RegisteredActorDestroyed(AActor* DestroyedActor);

    void execRegisteredActorDestroyed(FFrame& Stack, void* const Z_Param__Result);

    FGameplayTag& GetGameplayTag();

    TArray<AActor*>& GetRegisteredActors();

    FMulticastScriptDelegate& GetOnActorRegistered();

    FMulticastScriptDelegate& GetOnActorUnregistered();
};

class UFortGlobalActorRegistrySubsystem : public UObject
{
public:
    struct FGameplayTagKeyFuncs : TDefaultMapKeyFuncs<FGameplayTag, UActorRegisterData_GameplayTagBased*>
    {
        static FORCEINLINE uint32 GetKeyHash(const FGameplayTag& Key)
        {
            return GetTypeHash(Key.GetTagName());
        }
    };

    static UClass* StaticClass();

    static UFortGlobalActorRegistrySubsystem* GetGlobalActorRegistrySubsystem(const UObject* WorldContextObject);

    UActorRegisterData_GameplayTagBased* GetActorRegistryDataForGameplayTag(const FGameplayTag& GameplayTag);

    void GetRegisteredActors(TArray<AActor*>& OutActors, const FGameplayTag& GameplayTag);

    TArray<FGameplayTag> GetRegistrationTagsForActor(const AActor* InActor);

    void RegisterActors(const FGameplayTag& GameplayTag, const TArray<AActor*>& InActors);

    void UnregisterActors(const FGameplayTag& GameplayTag, const TArray<AActor*>& InActors);

    void RegisterActorsWithMultipleGameplayTags(const TArray<FGameplayTag>& GameplayTags, const TArray<AActor*>& InActors);

    void UnregisterActorsWithMultipleGameplayTags(const TArray<FGameplayTag>& GameplayTags, const TArray<AActor*>& InActors);

    void UnregisterActorsFromAllGameplayTags(const TArray<AActor*>& InActors);

    void execGetActorRegistryDataForGameplayTag(FFrame& Stack, void* const Z_Param__Result);

    void execGetRegisteredActors(FFrame& Stack, void* const Z_Param__Result);

    void execGetRegistrationTagsForActor(FFrame& Stack, void* const Z_Param__Result);

    void execRegisterActor(FFrame& Stack, void* const Z_Param__Result);

    void execRegisterActors(FFrame& Stack, void* const Z_Param__Result);

    void execRegisterActorsWithMultipleGameplayTags(FFrame& Stack, void* const Z_Param__Result);

    void execUnregisterActor(FFrame& Stack, void* const Z_Param__Result);

    void execUnregisterActors(FFrame& Stack, void* const Z_Param__Result);

    void execUnregisterActorsFromAllGameplayTags(FFrame& Stack, void* const Z_Param__Result);

    void execUnregisterActorsWithMultipleGameplayTags(FFrame& Stack, void* const Z_Param__Result);

    TMap<FGameplayTag, UActorRegisterData_GameplayTagBased*, FGameplayTagKeyFuncs>& GetActorRegistrationDataByGameplayTagMap();
};

class UFortAutoRegisterActorComponent : public UActorComponent
{
public:
    static UClass* StaticClass();

    void BeginPlay();

    FGameplayTagContainer& GetActorRegistrationTags();
};
