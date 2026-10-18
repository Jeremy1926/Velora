#include "pch.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UGameplayStatics::OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute, FString Options)
{
    static auto OpenLevel = Scanner::Function<void (*)(const UObject*, FName, bool, FString)>(Scanner::String(TEXT("WARNING: The map '%s' does not exist.")).Start().Get(TEXT("UGameplayStatics::OpenLevel")));

    OpenLevel(WorldContextObject, LevelName, bAbsolute, MoveTemp(Options));
}

void UGameplayStatics::GetAllActorsOfClass(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors)
{
    static auto GetAllActorsOfClass = Scanner::Function<void (*)(const UObject*, TSubclassOf<AActor>, TArray<AActor*>&)>(Scanner::Native(TEXT("/Script/Engine.GameplayStatics:GetAllActorsOfClass")).Get(TEXT("UGameplayStatics::GetAllActorsOfClass")));

    GetAllActorsOfClass(WorldContextObject, ActorClass, OutActors);
}
