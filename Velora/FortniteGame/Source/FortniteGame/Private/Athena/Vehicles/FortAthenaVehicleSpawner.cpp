#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortAthenaMapInfo.h"
#include "FortKismetLibrary.h"
#include "Items/FortVehicleItemDefinition.h"
#include "GameplayEffectTypes.h"
#include "Math/UnrealMathUtility.h"
#include "UObject/AssetPtr.h"
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


UClass* AFortAthenaVehicleSpawner::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner"));

    return Class;
}

void AFortAthenaVehicleSpawner::BeginPlay()
{
    DETOUR(AFortAthenaVehicleSpawner, BeginPlay, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:SpawnVehicle")))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Log(TEXT("ENGINE_ReceiveBeginPlay"))
        .Reference("mov rdx, *")
        .Start()
        .Log(TEXT("AActor::ReceiveBeginPlay"))
        .Caller()
        .Log(TEXT("AActor::BeginPlay"))
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .LogSlot(TEXT("AActor::BeginPlay"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortAthenaVehicleSpawner")));

    Original();

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

        static auto VehicleClassDetails = AFortAthenaMapInfo::StaticClass()->FindPropertyByName(FName(TEXT("VehicleClassDetails")));

        if (VehicleClassDetails ? GameMode->ShouldSpawnVehicle(VehicleClass) : ShouldSelfSpawnVehicle())
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
    static auto GetVehicleClass = Anchor::Function<UClass* (*)(const AFortAthenaVehicleSpawner*)>(TEXT("AFortAthenaVehicleSpawner::GetVehicleClass"))
        .Native(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass"))
        .Get();

    return GetVehicleClass(this);
}

UFortVehicleItemDefinition* AFortAthenaVehicleSpawner::GetFortVehicleItemDef() const
{
    static auto FortVehicleItemDef = GetClass()->FindPropertyByName(FName(TEXT("FortVehicleItemDef")));

    return FortVehicleItemDef->ContainerPtrToValuePtr<TAssetPtr<UFortVehicleItemDefinition>>(this)->LoadSynchronous();
}

bool AFortAthenaVehicleSpawner::ShouldSelfSpawnVehicle() const
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

    if (!GameState || !GameState->GetPlaylistDataIsLoaded())
        return false;

    auto FortVehicleItemDef = GetFortVehicleItemDef();

    if (!FortVehicleItemDef)
        return false;

    const float MinSpawnPercent = FMath::Clamp(FortVehicleItemDef->GetVehicleMinSpawnPercent().GetValueAtLevel(0.0f) * 0.01f, 0.0f, 1.0f);
    const float MaxSpawnPercent = FMath::Clamp(FortVehicleItemDef->GetVehicleMaxSpawnPercent().GetValueAtLevel(0.0f) * 0.01f, 0.0f, 1.0f);
    const float SpawnPercent = FMath::Lerp(MinSpawnPercent, MaxSpawnPercent, FMath::FRand());

    return SpawnPercent >= FMath::FRand();
}
