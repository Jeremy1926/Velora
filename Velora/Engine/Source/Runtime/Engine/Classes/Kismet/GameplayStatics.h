#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Containers/UnrealString.h"
#include "Engine/EngineTypes.h"
#include "UObject/NameTypes.h"
#include "Templates/SubclassOf.h"

class AActor;
struct FTransform;

class UGameplayStatics : public UBlueprintFunctionLibrary
{
public:
    static void OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute = true, FString Options = FString(TEXT("")));

    static void GetAllActorsOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors);

    static AActor* BeginDeferredActorSpawnFromClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::Undefined, AActor* Owner = nullptr);

    static AActor* FinishSpawningActor(AActor* Actor, const FTransform& SpawnTransform);
};
