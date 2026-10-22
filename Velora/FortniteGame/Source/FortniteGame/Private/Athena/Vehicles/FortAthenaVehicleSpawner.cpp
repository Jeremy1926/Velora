#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicleSpawner.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Pawns/FortPlayerPawn.h"
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

    AActor* SpawnedVehicle = nullptr;

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

                SpawnedVehicle = UGameplayStatics::FinishSpawningActor(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, VehicleClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn, nullptr), SpawnTransform);

                if (auto SpawnedVehicleProperty = GetClass()->FindPropertyByName(FName(TEXT("SpawnedVehicle"))))
                    *SpawnedVehicleProperty->ContainerPtrToValuePtr<AActor*>(this) = SpawnedVehicle;

                if (SpawnedVehicle && StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetForceSpawnAlways")))
                {
                    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));

                    if (auto VehicleInterface = (IFortVehicleInterface*)SpawnedVehicle->GetInterfaceAddress(FortVehicleInterface))
                        VehicleInterface->SetForcedToAlwaysSpawn(GetForceSpawnAlways());
                }

                AFortAthenaVehicle* Vehicle = Cast<AFortAthenaVehicle>(SpawnedVehicle);

                if (auto OnVehicleSpawnedEvent = Vehicle ? AFortPlayerPawn::GetOnVehicleSpawnedEvent() : nullptr)
                    OnVehicleSpawnedEvent->ProcessMulticastDelegate<UObject>(&Vehicle);
            }
        }
    }

    auto bShouldDestroyAfterVehicleSpawn = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bShouldDestroyAfterVehicleSpawn")));

    if (!SpawnedVehicle || !bShouldDestroyAfterVehicleSpawn || bShouldDestroyAfterVehicleSpawn->GetPropertyValue_InContainer(this))
        Destroy();
}

UClass* AFortAthenaVehicleSpawner::GetVehicleClass() const
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        static auto GetVehicleSpawnData = Anchor::Function<UObject* (*)(const AFortAthenaVehicleSpawner*)>(TEXT("AFortAthenaVehicleSpawner::GetVehicleClass spawn data"))
            .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Get();

        static auto VehicleClassOffset = Anchor::Offset(TEXT("AFortAthenaVehicleSpawner::GetVehicleClass vehicle class"))
            .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("lea rdx, [rax+0x*]")
            .Disp();

        static auto LoadSynchronous = Anchor::Function<UObject** (*)(UObject**, const void*)>(TEXT("AFortAthenaVehicleSpawner::GetVehicleClass load"))
            .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("lea rdx, [rax+0x*]")
            .Find("call 0x*")
            .Target()
            .Get();

        static auto ResultClass = Anchor::Function<UClass* (*)()>(TEXT("AFortAthenaVehicleSpawner::GetVehicleClass result class"))
            .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass"))
            .Find("setnz *")
            .Find("add [*+0x20], *")
            .Find("lea rdx, [rax+0x*]")
            .Find("call 0x*")
            .Find("test rbx, rbx; jz 0x*; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get();

        auto SpawnData = VehicleClassOffset ? GetVehicleSpawnData(this) : nullptr;

        if (!SpawnData)
            return nullptr;

        UObject* Loaded = nullptr;
        LoadSynchronous(&Loaded, (uint8*)SpawnData + VehicleClassOffset);

        return Loaded && Loaded->IsA(ResultClass()) ? (UClass*)Loaded : nullptr;
    }

    static auto GetVehicleClass = Anchor::Function<UClass* (*)(const AFortAthenaVehicleSpawner*)>(TEXT("AFortAthenaVehicleSpawner::GetVehicleClass"))
        .Native(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetVehicleClass"))
        .Get();

    return GetVehicleClass(this);
}

bool AFortAthenaVehicleSpawner::GetForceSpawnAlways() const
{
    static auto GetForceSpawnAlways = Anchor::Function<bool (*)(const AFortAthenaVehicleSpawner*)>(TEXT("AFortAthenaVehicleSpawner::GetForceSpawnAlways"))
        .Native(TEXT("/Script/FortniteGame.FortAthenaVehicleSpawner:GetForceSpawnAlways"))
        .Get();

    return GetForceSpawnAlways(this);
}

UFortVehicleItemDefinition* AFortAthenaVehicleSpawner::GetFortVehicleItemDef() const
{
    static auto CachedFortVehicleItemDef = StaticClass()->FindPropertyByName(FName(TEXT("CachedFortVehicleItemDef")));

    if (CachedFortVehicleItemDef)
    {
        GetVehicleClass();

        return *CachedFortVehicleItemDef->ContainerPtrToValuePtr<UFortVehicleItemDefinition*>(this);
    }

    static auto FortVehicleItemDef = GetClass()->FindPropertyByName(FName(TEXT("FortVehicleItemDef")));

    return FortVehicleItemDef->ContainerPtrToValuePtr<TAssetPtr<UFortVehicleItemDefinition>>(this)->LoadSynchronous();
}

void AFortAthenaVehicleSpawner::SetFortVehicleItemDef(UFortVehicleItemDefinition* InFortVehicleItemDef)
{
    static auto CachedFortVehicleItemDef = StaticClass()->FindPropertyByName(FName(TEXT("CachedFortVehicleItemDef")));
    static auto bIsVehicleItemDefCached = StaticClass()->FindPropertyByName(FName(TEXT("bIsVehicleItemDefCached")));

    if (!CachedFortVehicleItemDef || !bIsVehicleItemDefCached)
        return;

    *CachedFortVehicleItemDef->ContainerPtrToValuePtr<UFortVehicleItemDefinition*>(this) = InFortVehicleItemDef;
    *bIsVehicleItemDefCached->ContainerPtrToValuePtr<bool>(this) = true;
}

AActor* AFortAthenaVehicleSpawner::GetSpawnedVehicle() const
{
    auto SpawnedVehicle = GetClass()->FindPropertyByName(FName(TEXT("SpawnedVehicle")));

    return SpawnedVehicle ? *SpawnedVehicle->ContainerPtrToValuePtr<AActor*>(this) : nullptr;
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
