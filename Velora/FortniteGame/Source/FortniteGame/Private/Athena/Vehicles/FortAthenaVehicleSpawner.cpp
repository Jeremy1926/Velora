#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "FortKismetLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Math/TransformVectorized.h"
#include "Misc/Timespan.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto BeginPlay = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:SpawnVehicle")) ? []
{
    auto ReceiveBeginPlayName = Scanner::String(TEXT("ReceiveBeginPlay")).Find("lea *, [0x*]").Target(TEXT("ENGINE_ReceiveBeginPlay"));
    auto ReceiveBeginPlay = Scanner::Reference(ReceiveBeginPlayName, "mov rdx, *").Start().Get(TEXT("AActor::ReceiveBeginPlay"));
    auto ActorBeginPlay = Scanner::CallTo(ReceiveBeginPlay).Start().Get(TEXT("AActor::BeginPlay"));
    auto ActorVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
    auto BeginPlay = Scanner::Slot(TEXT("AActor::BeginPlay"), ActorVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == ActorBeginPlay; });
    auto VTable = *(void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortAthenaVehicleSpawner"));

    return DetourVTable(TEXT("AFortAthenaVehicleSpawner::BeginPlay"), VTable, BeginPlay, &AFortAthenaVehicleSpawner::BeginPlay);
}() : nullptr;

UClass* AFortAthenaVehicleSpawner::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner"));

    return Class;
}

void AFortAthenaVehicleSpawner::BeginPlay()
{
    (this->*::BeginPlay)();

    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode());
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (GameMode && GameMode->IsMapInfoInitialized() && GameState && GameState->GetPlaylistDataIsLoaded())
        SpawnVehicle();
}

void AFortAthenaVehicleSpawner::SpawnVehicle()
{
    auto YAxisForward = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("YAxisForward")));
    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode());

    if (!YAxisForward)
        return;

    if (HasAuthority() && UFortKismetLibrary::AreAthenaVehiclesEnabled() && GameMode)
    {
        UClass* VehicleClass = GetVehicleClass();

        if (VehicleClass && !VehicleClass->IsChildOf(AActor::StaticClass()))
            VehicleClass = nullptr;

        UClass* VehicleClassOverride = GameMode->GetVehicleClassOverride(VehicleClass);

        if (VehicleClassOverride && !VehicleClassOverride->IsPendingKill())
            VehicleClass = VehicleClassOverride;

        if (GameMode->ShouldSpawnVehicle(VehicleClass))
        {
            bool bIsEventActive = false;
            FTimespan TimeSinceBegin;
            FTimespan TimeUntilEnd;
            float TimespanRatio = 0.0f;

            UFortKismetLibrary::GetCalendarEventInformation(this, FString(TEXT("NoVehicle")), bIsEventActive, TimeSinceBegin, TimeUntilEnd, TimespanRatio);

            if (!bIsEventActive)
            {
                const FTransform SpawnTransform = YAxisForward->GetPropertyValue_InContainer(this) ? UKismetMathLibrary::ComposeTransforms(FTransform(FRotator{ 0.0f, 90.0f, 0.0f }, FVector(0.0f, 0.0f, 0.0f), FVector(1.0f, 1.0f, 1.0f)), GetTransform()) : GetTransform();

                UGameplayStatics::FinishSpawningActor(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, VehicleClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, nullptr), SpawnTransform);
            }
        }
    }

    Destroy();
}

UClass* AFortAthenaVehicleSpawner::GetVehicleClass() const
{
    static auto GetVehicleClass = Scanner::Function<UClass* (*)(const AFortAthenaVehicleSpawner*)>(Scanner::Native(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass")).Get(TEXT("AFortAthenaVehicleSpawner::GetVehicleClass")));

    return GetVehicleClass(this);
}
