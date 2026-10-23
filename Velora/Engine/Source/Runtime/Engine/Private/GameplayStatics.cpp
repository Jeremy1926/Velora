#include "pch.h"
#include "Kismet/GameplayStatics.h"
#include "Math/TransformVectorized.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UGameplayStatics::OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute, FString Options)
{
    static auto OpenLevel = Anchor::Function<void (*)(const UObject*, FName, bool, FString)>(TEXT("UGameplayStatics::OpenLevel"))
        .String(TEXT("WARNING: The map '%s' does not exist."))
        .Start()
        .Get();

    OpenLevel(WorldContextObject, LevelName, bAbsolute, MoveTemp(Options));
}

void UGameplayStatics::GetAllActorsOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors)
{
    static auto GetAllActorsOfClass = Anchor::Function<void (*)(const UObject*, TSubclassOf<AActor>, TArray<AActor*>&)>(TEXT("UGameplayStatics::GetAllActorsOfClass"))
        .Native(TEXT("/Script/Engine.GameplayStatics:GetAllActorsOfClass"))
        .Get();

    GetAllActorsOfClass(WorldContextObject, ActorClass, OutActors);
}

AActor* UGameplayStatics::BeginDeferredActorSpawnFromClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, AActor* Owner)
{
    static auto BeginDeferredActorSpawnFromClass = Anchor::Function<AActor* (*)(const UObject*, TSubclassOf<AActor>, const TGameValue<FTransform>&, ESpawnActorCollisionHandlingMethod, AActor*)>(TEXT("UGameplayStatics::BeginDeferredActorSpawnFromClass"))
        .Native(TEXT("/Script/Engine.GameplayStatics:BeginDeferredActorSpawnFromClass"))
        .Get();

    return BeginDeferredActorSpawnFromClass(WorldContextObject, ActorClass, SpawnTransform, CollisionHandlingOverride, Owner);
}

AActor* UGameplayStatics::FinishSpawningActor(AActor* Actor, const FTransform& SpawnTransform)
{
    static auto FinishSpawningActor = Anchor::Function<AActor* (*)(AActor*, const TGameValue<FTransform>&)>(TEXT("UGameplayStatics::FinishSpawningActor"))
        .Native(TEXT("/Script/Engine.GameplayStatics:FinishSpawningActor"))
        .Get();

    return FinishSpawningActor(Actor, SpawnTransform);
}
