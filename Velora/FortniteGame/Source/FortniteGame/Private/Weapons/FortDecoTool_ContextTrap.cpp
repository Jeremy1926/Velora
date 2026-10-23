#include "pch.h"
#include "Weapons/FortDecoTool_ContextTrap.h"
#include "Player/FortPlayerController.h"
#include "Building/BuildingActor.h"
#include "Building/BuildingSMActor.h"
#include "Items/FortItemDefinition.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Containers/Array.h"
#include "Containers/ScriptArray.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

void AFortDecoTool_ContextTrap::ServerCreateBuildingAndSpawnDeco_Implementation(const FVector& BuildingLocation, const FRotator& BuildingRotation, const FVector& Location, const FRotator& Rotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const FVector& BuildingExtraPieceLocation)
{
    DETOUR(AFortDecoTool_ContextTrap, ServerCreateBuildingAndSpawnDeco_Implementation, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortPlayerController, ServerCreateBuildingActorInternal))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap"))
        .Optional()
        .Inline());

    static auto GetSoftObject = Anchor::Function<UObject* (*)(const void*)>(TEXT("FSoftObjectPtr::Get"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("mov rcx, rsi; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("test byte ptr [*+0x*], 0x2")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    static auto DoesItemDefinitionHaveGameplayTag = Anchor::Function<bool (*)(const UFortItemDefinition*, const void*)>(TEXT("UFortKismetLibrary::DoesItemDefinitionHaveGameplayTag"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("lea rdx, [rax+0x*]")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto FindAvailableBuildingClassForBuildingTypeAndEditModePattern = Anchor::Function<UClass** (*)(AFortPlayerController*, UClass**, int32, const TArray<UObject*>*, bool, uint8)>(TEXT("AFortPlayerController::FindAvailableBuildingClassForBuildingTypeAndEditModePattern"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("mov [rsp+0x20], al")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto BuildExtraPieceForSupport = Anchor::Function<void (*)(AFortPlayerController*, UWorld*, UClass*, const FVector*, const FRotator*, bool, int32, EFortBuildPreviewMarkerOptionalAdjustment)>(TEXT("AFortPlayerController::BuildExtraPieceForSupport"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap"))
        .Find("call [*+0x*]; test eax, eax; jz 0x*")
        .Find("mov r8, rbx")
        .Find("call 0x*")
        .Target()
        .Get();

    auto ContextTrapItemDefinition = GetContextTrapItemDefinition();

    if (!ContextTrapItemDefinition)
    {
        AFortDecoTool::ServerCreateBuildingAndSpawnDeco_Implementation(BuildingLocation, BuildingRotation, Location, Rotation, InBuildingAttachmentType, bSpawnDecoOnExtraPiece, BuildingExtraPieceLocation);
        return;
    }

    auto PlayerController = GetPlayerController();
    auto DecoItemDefinition = PlayerController ? GetDecoItemDefinition() : nullptr;

    static auto FloorTrap = ContextTrapItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("FloorTrap")));
    static auto WallTrap = ContextTrapItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("WallTrap")));

    auto FloorTrapItemDefinition = *FloorTrap->ContainerPtrToValuePtr<UFortItemDefinition*>(ContextTrapItemDefinition);

    if (!DecoItemDefinition || !FloorTrapItemDefinition)
        return;

    static auto bAutoCreateAttachmentBuilding = (UBoolProperty*)FloorTrapItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bAutoCreateAttachmentBuilding")));

    if (!bAutoCreateAttachmentBuilding->GetPropertyValue_InContainer(FloorTrapItemDefinition))
        return;

    const uint8 AttachmentType = (uint8)InBuildingAttachmentType;
    auto BuildingType = EFortBuildingType::Floor;
    auto TrapItemDefinition = FloorTrapItemDefinition;

    if (AttachmentType > 7 || !((0xC5 >> AttachmentType) & 1))
    {
        if (AttachmentType != 1)
            return;

        BuildingType = EFortBuildingType::Wall;
        TrapItemDefinition = *WallTrap->ContainerPtrToValuePtr<UFortItemDefinition*>(ContextTrapItemDefinition);
    }

    if (!TrapItemDefinition)
        return;

    static auto AutoCreateAttachmentBuildingShapes = (UArrayProperty*)TrapItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("AutoCreateAttachmentBuildingShapes")));
    static auto AutoCreateAttachmentBuildingResourceType = DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("AutoCreateAttachmentBuildingResourceType")));

    TArray<UObject*> AllowedShapes;
    auto& Shapes = *AutoCreateAttachmentBuildingShapes->ContainerPtrToValuePtr<FScriptArray>(TrapItemDefinition);
    const int32 ShapeSize = AutoCreateAttachmentBuildingShapes->GetInner()->GetSize();

    for (int32 Index = 0; Index < Shapes.Num(); Index++)
    {
        if (auto Shape = GetSoftObject((const uint8*)Shapes.GetData() + Index * ShapeSize))
            AllowedShapes.Add(Shape);
    }

    if (!AllowedShapes.Num())
        return;

    static auto ExtraPieceCostIgnoreTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Trap.ExtraPiece.Cost.Ignore")));
    static auto IgnoreInteractBuildCheckTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Trap.IgnoreInteractBuildCheck")));
    static auto BuildingValidityIgnoreTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Trap.Building.Validity.Ignore")));
    static auto InteractBuildTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.InteractBuild")));

    UClass* BuildingClass = nullptr;
    FindAvailableBuildingClassForBuildingTypeAndEditModePattern(PlayerController, &BuildingClass, (int32)BuildingType, &AllowedShapes, DoesItemDefinitionHaveGameplayTag(ContextTrapItemDefinition, &ExtraPieceCostIgnoreTag), *AutoCreateAttachmentBuildingResourceType->ContainerPtrToValuePtr<uint8>(DecoItemDefinition));

    if (!BuildingClass || !BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return;

    FVector SnappedLocation = BuildingLocation;
    SnappedLocation.Z = floorf(SnappedLocation.Z * (1.0f / 384.0f) + 0.5f) * 384.0f;

    bool bForceOverrideInteractBuildCheck = false;

    if (!PlayerController->CanPerformNativeAction(InteractBuildTag))
        bForceOverrideInteractBuildCheck = DoesItemDefinitionHaveGameplayTag(ContextTrapItemDefinition, &IgnoreInteractBuildCheckTag);

    TArray<ABuildingActor*> ExistingBuildings;
    auto MarkerOptionalAdjustment = EFortBuildPreviewMarkerOptionalAdjustment::None;
    FRotator BuildRotation = BuildingRotation;

    if (PlayerController->CanPlaceBuildableClassInStructuralGrid(BuildingClass, SnappedLocation, BuildRotation, false, &ExistingBuildings, MarkerOptionalAdjustment) != EFortStructuralGridQueryResults::CanAdd)
        BuildExtraPieceForSupport(PlayerController, GetWorld(), BuildingClass, &SnappedLocation, &BuildRotation, false, 0, EFortBuildPreviewMarkerOptionalAdjustment::FreeWallPieceOnBottom);

    const bool bIgnoreCostCheck = DoesItemDefinitionHaveGameplayTag(ContextTrapItemDefinition, &ExtraPieceCostIgnoreTag);
    const bool bIgnoreBuildingValidityCheck = DoesItemDefinitionHaveGameplayTag(ContextTrapItemDefinition, &BuildingValidityIgnoreTag);

    FBuildingClassData BuildingClassData{ BuildingClass, -1, 0 };

    auto Building = PlayerController->ServerCreateBuildingActorInternal(BuildingClassData, SnappedLocation, BuildRotation, false, GetWorld()->GetTimeSeconds(), bForceOverrideInteractBuildCheck, bIgnoreCostCheck, bIgnoreBuildingValidityCheck);

    if (!Building)
        return;

    static auto bSpawnDecoOnExtraPieceParam = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco")))->FindPropertyByName(FName(TEXT("bSpawnDecoOnExtraPiece")));

    if (!bSpawnDecoOnExtraPieceParam)
    {
        static auto ServerSpawnDeco_Implementation = Anchor::VirtualFunction<void (*)(AFortDecoTool*, const FVector*, const FRotator*, ABuildingSMActor*, EBuildingAttachmentType)>(TEXT("AFortDecoTool::ServerSpawnDeco_Implementation"))
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
            .Find("call *r10*; test al, al")
            .Find("call [r10+0x*]")
            .Get();

        ServerSpawnDeco_Implementation(this, &Location, &Rotation, Building, InBuildingAttachmentType);

        return;
    }

    static auto SpawnDecoInternal = Anchor::VirtualFunction<void (*)(AFortDecoTool*, const FVector*, const FRotator*, ABuildingSMActor*, EBuildingAttachmentType, bool)>(TEXT("AFortDecoTool::SpawnDecoInternal"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call r10; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .Optional()
        .Get();

    static auto bAllowAutoCreateExtraPiece = (UBoolProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bAllowAutoCreateExtraPiece")));
    static auto bGiveFreeDecoOnUndergroundPiece = (UBoolProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bGiveFreeDecoOnUndergroundPiece")));

    if (bAllowAutoCreateExtraPiece && bAllowAutoCreateExtraPiece->GetPropertyValue_InContainer(DecoItemDefinition) && bSpawnDecoOnExtraPiece)
    {
        static auto GetCurrent = Anchor::Function<UObject* (*)(const UObject*)>(TEXT("UBuildingStructuralSupportSystem::GetCurrent"))
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
            .Find("call *r10*; test al, al")
            .Find("call [r10+0x*]")
            .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap"))
            .Find("call [*+0x*]; test eax, eax; jz 0x*")
            .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
            .Find("call 0x*; test rax, rax; jz 0x*; mov r8, *; mov rdx, *; mov rcx, rax; call 0x*")
            .Target()
            .Get();

        static auto GetExistingBuildingInGridLocation = Anchor::Function<ABuildingSMActor* (*)(UObject*, const ABuildingSMActor*, const FVector*)>(TEXT("UBuildingStructuralSupportSystem::GetExistingBuildingInGridLocation"))
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
            .Find("call *r10*; test al, al")
            .Find("call [r10+0x*]")
            .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap"))
            .Find("call [*+0x*]; test eax, eax; jz 0x*")
            .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
            .Find("mov r8, *; mov rdx, *; mov rcx, rax; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get();

        auto StructuralSupportSystem = GetCurrent(this);

        if (!StructuralSupportSystem)
            return;

        if (auto ExistingBuilding = GetExistingBuildingInGridLocation(StructuralSupportSystem, Building, &BuildingExtraPieceLocation))
            SpawnDecoInternal(this, &BuildingExtraPieceLocation, &Rotation, ExistingBuilding, InBuildingAttachmentType, false);

        if (bGiveFreeDecoOnUndergroundPiece && bGiveFreeDecoOnUndergroundPiece->GetPropertyValue_InContainer(DecoItemDefinition))
            SpawnDecoInternal(this, &Location, &Rotation, Building, InBuildingAttachmentType, true);

        return;
    }

    SpawnDecoInternal(this, &Location, &Rotation, Building, InBuildingAttachmentType, false);
}

UFortItemDefinition* AFortDecoTool_ContextTrap::GetContextTrapItemDefinition() const
{
    static auto ContextTrapItemDefinition = GetClass()->FindPropertyByName(FName(TEXT("ContextTrapItemDefinition")));

    return *ContextTrapItemDefinition->ContainerPtrToValuePtr<UFortItemDefinition*>(this);
}
