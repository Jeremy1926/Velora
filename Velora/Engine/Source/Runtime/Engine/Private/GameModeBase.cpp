#include "pch.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* AGameModeBase::GetDefaultPawnClassForController(AController* InController)
{
    static auto GetDefaultPawnClassForController = (UClass* (*)(AGameModeBase*, AController*))Scanner::String(TEXT("SpawnDefaultPawnFor: Failed due to no start spot")).Call().Get(TEXT("AGameModeBase::GetDefaultPawnClassForController"));

    return GetDefaultPawnClassForController(this, InController);
}

APawn* AGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    static auto SpawnDefaultPawnFor = []
    {
        auto AthenaSpawnDefaultPawnFor = Scanner::String(TEXT("SpawnDefaultPawnFor: Failed due to no world")).Start().Get(TEXT("AFortGameModeAthena::SpawnDefaultPawnFor_Implementation"));
        auto AthenaVTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (AthenaVTable[Index] != AthenaSpawnDefaultPawnFor)
            Index++;

        auto VTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__GameModeBase"));

        return (APawn* (*)(AGameModeBase*, AController*, AActor*))VTable[Index];
    }();

    return SpawnDefaultPawnFor(this, NewPlayer, StartSpot);
}

FString& AGameModeBase::GetOptionsString()
{
    static auto OptionsString = GetClass()->FindPropertyByName(FName(TEXT("OptionsString")));

    return *OptionsString->ContainerPtrToValuePtr<FString>(this);
}
