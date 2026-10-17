#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"
#include "Templates/SubclassOf.h"

class AActor;

class UGameplayStatics : public UBlueprintFunctionLibrary
{
public:
    static void OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute = true, FString Options = FString(TEXT("")));

    static void GetAllActorsOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors);
};
