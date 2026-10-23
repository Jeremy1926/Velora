#include "pch.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* AGameModeBase::GetDefaultPawnClassForController(AController* InController)
{
    static auto GetDefaultPawnClassForController = Anchor::Function<UClass* (*)(AGameModeBase*, AController*)>(TEXT("AGameModeBase::GetDefaultPawnClassForController"))
        .String(TEXT("SpawnDefaultPawnFor: Failed due to no start spot"))
        .Is("call 0x*")
        .Find("mov rdx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("SpawnDefaultPawnFor: Failed due to no start spot"))
        .Call()
        .Get();

    return GetDefaultPawnClassForController(this, InController);
}

APawn* AGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    static auto SpawnDefaultPawnFor = Anchor::VirtualFunction<APawn* (*)(AGameModeBase*, AController*, AActor*)>(TEXT("AGameModeBase::SpawnDefaultPawnFor_Implementation"))
        .String(TEXT("SpawnDefaultPawnFor: Failed due to no world"))
        .Start()
        .Log(TEXT("AFortGameModeAthena::SpawnDefaultPawnFor_Implementation"))
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortGameModeAthena"))
        .Get(TEXT("/Script/Engine.Default__GameModeBase"));

    return SpawnDefaultPawnFor(this, NewPlayer, StartSpot);
}

AGameStateBase* AGameModeBase::GetGameState() const
{
    static auto GameState = GetClass()->FindPropertyByName(FName(TEXT("GameState")));

    return *GameState->ContainerPtrToValuePtr<AGameStateBase*>(this);
}

FString& AGameModeBase::GetOptionsString()
{
    static auto OptionsString = GetClass()->FindPropertyByName(FName(TEXT("OptionsString")));

    return *OptionsString->ContainerPtrToValuePtr<FString>(this);
}
