#include "pch.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UClass* AGameModeBase::GetDefaultPawnClassForController(AController* InController)
{
    static auto GetDefaultPawnClassForController = Scanner::Function<UClass* (*)(AGameModeBase*, AController*)>(Scanner::String(TEXT("SpawnDefaultPawnFor: Failed due to no start spot")).Call().Get(TEXT("AGameModeBase::GetDefaultPawnClassForController")));

    return GetDefaultPawnClassForController(this, InController);
}

APawn* AGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
    static auto SpawnDefaultPawnFor = []
    {
        auto AthenaSpawnDefaultPawnFor = Scanner::String(TEXT("SpawnDefaultPawnFor: Failed due to no world")).Start().Get(TEXT("AFortGameModeAthena::SpawnDefaultPawnFor_Implementation"));
        auto AthenaVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto SpawnDefaultPawnFor = Scanner::Slot(TEXT("AGameModeBase::SpawnDefaultPawnFor_Implementation"), AthenaVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == AthenaSpawnDefaultPawnFor; });

        return Scanner::Virtual<APawn* (*)(AGameModeBase*, AController*, AActor*)>(StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__GameModeBase")), SpawnDefaultPawnFor);
    }();

    return SpawnDefaultPawnFor(this, NewPlayer, StartSpot);
}

FString& AGameModeBase::GetOptionsString()
{
    static auto OptionsString = GetClass()->FindPropertyByName(FName(TEXT("OptionsString")));

    return *OptionsString->ContainerPtrToValuePtr<FString>(this);
}
