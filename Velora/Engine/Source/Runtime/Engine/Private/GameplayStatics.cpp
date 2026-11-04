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

    static auto bClassByAddress = Anchor::Exists()
        .Exec(TEXT("/Script/Engine.GameplayStatics:GetAllActorsOfClass"))
        .Find("add [*+0x20], *")
        .Find("lea rdx, [*]; mov r8, *; call 0x*")
        .Get();

    if (bClassByAddress)
        return ((void (*)(const UObject*, const TSubclassOf<AActor>*, TArray<AActor*>&))GetAllActorsOfClass)(WorldContextObject, &ActorClass, OutActors);

    GetAllActorsOfClass(WorldContextObject, ActorClass, OutActors);
}

AActor* UGameplayStatics::BeginDeferredActorSpawnFromClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, AActor* Owner)
{
    static auto BeginDeferredActorSpawnFromClass = Anchor::Function<AActor* (*)(const UObject*, TSubclassOf<AActor>, const TGameValue<FTransform>&, ESpawnActorCollisionHandlingMethod, AActor*)>(TEXT("UGameplayStatics::BeginDeferredActorSpawnFromClass"))
        .Native(TEXT("/Script/Engine.GameplayStatics:BeginDeferredActorSpawnFromClass"))
        .Get();

    static auto bClassByAddress = Anchor::Exists()
        .Exec(TEXT("/Script/Engine.GameplayStatics:BeginDeferredActorSpawnFromClass"))
        .Find("add [*+0x20], *")
        .Find("lea rdx, [*]; mov [rsp+0x28], *")
        .Or()
        .Exec(TEXT("/Script/Engine.GameplayStatics:BeginDeferredActorSpawnFromClass"))
        .Find("add [*+0x20], *")
        .Find("mov rcx, [rbp-0x*]; mov [rsp+0x28], *; lea rdx, [rbp-0x*]")
        .Or()
        .Exec(TEXT("/Script/Engine.GameplayStatics:BeginDeferredActorSpawnFromClass"))
        .Find("add [*+0x20], *")
        .Find("lea rdx, [rbp-0x*]; mov rcx, [rbp-0x*]; mov [rsp+0x28], *")
        .Or()
        .Exec(TEXT("/Script/Engine.GameplayStatics:BeginDeferredActorSpawnFromClass"))
        .Find("add [*+0x20], *")
        .Find("mov rcx, [rbp-0x*]; lea rdx, [rbp-0x*]; mov r9b, *")
        .Get();

    if (bClassByAddress)
        return ((AActor* (*)(const UObject*, const TSubclassOf<AActor>*, const TGameValue<FTransform>&, ESpawnActorCollisionHandlingMethod, AActor*))BeginDeferredActorSpawnFromClass)(WorldContextObject, &ActorClass, SpawnTransform, CollisionHandlingOverride, Owner);

    return BeginDeferredActorSpawnFromClass(WorldContextObject, ActorClass, SpawnTransform, CollisionHandlingOverride, Owner);
}

AActor* UGameplayStatics::FinishSpawningActor(AActor* Actor, const FTransform& SpawnTransform)
{
    static auto FinishSpawningActor = Anchor::Function<AActor* (*)(AActor*, const TGameValue<FTransform>&)>(TEXT("UGameplayStatics::FinishSpawningActor"))
        .Native(TEXT("/Script/Engine.GameplayStatics:FinishSpawningActor"))
        .Get();

    return FinishSpawningActor(Actor, SpawnTransform);
}
