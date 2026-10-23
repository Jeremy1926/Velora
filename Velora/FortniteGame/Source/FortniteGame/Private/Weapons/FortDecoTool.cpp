#include "pch.h"
#include "Weapons/FortDecoTool.h"
#include "Player/FortPlayerController.h"
#include "Building/BuildingActor.h"
#include "Building/BuildingSMActor.h"
#include "Items/FortItemDefinition.h"
#include "Engine/World.h"
#include "Containers/Array.h"
#include "Containers/ScriptArray.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

void AFortDecoTool::ServerCreateBuildingAndSpawnDeco_Implementation(const FVector& BuildingLocation, const FRotator& BuildingRotation, const FVector& Location, const FRotator& Rotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const FVector& BuildingExtraPieceLocation)
{
    DETOUR(AFortDecoTool, ServerCreateBuildingAndSpawnDeco_Implementation, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortPlayerController, ServerCreateBuildingActorInternal))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .LogSlot(TEXT("AFortDecoTool::ServerCreateBuildingAndSpawnDeco_Implementation"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
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

    static auto GetBuildingTypeFromBuildingAttachmentType = Anchor::Function<int32 (*)(EBuildingAttachmentType)>(TEXT("AFortDecoHelper::GetBuildingTypeFromBuildingAttachmentType"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("cmp eax, 0xc")
        .Last("call 0x*")
        .Target()
        .Get();

    static auto GameplayTags = Anchor::Function<const uint8* (*)()>(TEXT("UFortGlobals::GameplayTags"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("cmp eax, 0xc")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto TrapExtraPieceCostIgnore = Anchor::Offset(TEXT("FFortGlobalGameplayTags::TrapExtraPieceCostIgnore"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("lea rdx, [rax+0x*]")
        .Disp();

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

    static auto SpawnDecoInternal = Anchor::VirtualFunction<void (*)(AFortDecoTool*, const FVector*, const FRotator*, ABuildingSMActor*, EBuildingAttachmentType, bool)>(TEXT("AFortDecoTool::SpawnDecoInternal"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call r10; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .Optional()
        .Get();

    auto PlayerController = GetPlayerController();
    auto DecoItemDefinition = PlayerController ? GetDecoItemDefinition() : nullptr;

    if (!DecoItemDefinition)
        return;

    static auto bAutoCreateAttachmentBuilding = (UBoolProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bAutoCreateAttachmentBuilding")));
    static auto bAllowAutoCreateExtraPiece = (UBoolProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bAllowAutoCreateExtraPiece")));
    static auto bGiveFreeDecoOnUndergroundPiece = (UBoolProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bGiveFreeDecoOnUndergroundPiece")));
    static auto AutoCreateAttachmentBuildingResourceType = DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("AutoCreateAttachmentBuildingResourceType")));
    static auto AutoCreateAttachmentBuildingShapes = (UArrayProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("AutoCreateAttachmentBuildingShapes")));

    if (!bAutoCreateAttachmentBuilding || !AutoCreateAttachmentBuildingShapes || !AutoCreateAttachmentBuildingResourceType || !bAutoCreateAttachmentBuilding->GetPropertyValue_InContainer(DecoItemDefinition))
        return;

    TArray<UObject*> AllowedShapes;
    auto& Shapes = *AutoCreateAttachmentBuildingShapes->ContainerPtrToValuePtr<FScriptArray>(DecoItemDefinition);
    const int32 ShapeSize = AutoCreateAttachmentBuildingShapes->GetInner()->GetSize();

    for (int32 Index = 0; Index < Shapes.Num(); Index++)
    {
        if (auto Shape = GetSoftObject((const uint8*)Shapes.GetData() + Index * ShapeSize))
            AllowedShapes.Add(Shape);
    }

    if (!AllowedShapes.Num())
        return;

    const int32 BuildingType = GetBuildingTypeFromBuildingAttachmentType(InBuildingAttachmentType);

    if (BuildingType == (int32)EFortBuildingType::None)
        return;

    const bool bIgnoreCostCheck = DoesItemDefinitionHaveGameplayTag(DecoItemDefinition, GameplayTags() + TrapExtraPieceCostIgnore);

    UClass* BuildingClass = nullptr;
    FindAvailableBuildingClassForBuildingTypeAndEditModePattern(PlayerController, &BuildingClass, BuildingType, &AllowedShapes, bIgnoreCostCheck, *AutoCreateAttachmentBuildingResourceType->ContainerPtrToValuePtr<uint8>(DecoItemDefinition));

    if (!BuildingClass || !BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return;

    FBuildingClassData BuildingClassData{ BuildingClass, -1, 0 };
    FVector SnappedLocation = BuildingLocation;
    SnappedLocation.Z = floorf(SnappedLocation.Z * (1.0f / 384.0f) + 0.5f) * 384.0f;

    auto Building = PlayerController->ServerCreateBuildingActorInternal(BuildingClassData, SnappedLocation, BuildingRotation, false, GetWorld()->GetTimeSeconds(), false, bIgnoreCostCheck, false);

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

AFortPlayerController* AFortDecoTool::GetPlayerController() const
{
    static auto GetPlayerController = Anchor::Function<AFortPlayerController* (*)(const AFortDecoTool*)>(TEXT("AFortDecoTool::GetPlayerController"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call 0x*")
        .Target()
        .Get();

    return GetPlayerController(this);
}

UFortItemDefinition* AFortDecoTool::GetDecoItemDefinition() const
{
    static auto GetDecoItemDefinition = Anchor::VirtualFunction<UFortItemDefinition* (*)(const AFortDecoTool*)>(TEXT("AFortDecoTool::GetDecoItemDefinition"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [*+0x*]")
        .Get();

    return GetDecoItemDefinition(this);
}
