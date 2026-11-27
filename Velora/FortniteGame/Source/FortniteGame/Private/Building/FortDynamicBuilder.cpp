#include "pch.h"
#include "Building/FortDynamicBuilder.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortDynamicBuilder::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDynamicBuilder"));

    return Class;
}

const FServerOnlyDynamicBuilderClass* AFortDynamicBuilder::FindServerOnlyBuilderClass(const FFrame& Stack, UObject* WorldContextObject)
{
    static const FServerOnlyDynamicBuilderClass BuilderClasses[] =
    {
        { TEXT("/LaunchPadItemGameplay/Items/Projectiles/B_Prj_Athena_LaunchPadThrowable_Parent.B_Prj_Athena_LaunchPadThrowable_Parent_C"), TEXT("BuilderClass"), TEXT("FortDynamicBuilder_LaunchPadThrowable_A"), TEXT("/LaunchPadItemGameplay/Items/BuildingInstructions/Metal/FortBuildingInstructions_LaunchPadThrowable_A_M.FortBuildingInstructions_LaunchPadThrowable_A_M"), false, true, TEXT("EDynamicBuildingPlacementType::CountsTowardsBounds") },
        { TEXT("/LaunchPadItemGameplay/Items/Projectiles/B_Prj_Athena_LaunchPadThrowable_Parent.B_Prj_Athena_LaunchPadThrowable_Parent_C"), TEXT("BuilderClassBasement"), TEXT("FortDynamicBuilder_LaunchPadThrowable_A_Basement"), TEXT("/LaunchPadItemGameplay/Items/BuildingInstructions/Metal/FortBuildingInstructions_LaunchPadThrowable_A_BaseM.FortBuildingInstructions_LaunchPadThrowable_A_BaseM"), true, true, TEXT("EDynamicBuildingPlacementType::DestroyAnythingThatCollides") },
        { TEXT("/LaunchPadItemGameplay/Items/Projectiles/B_Prj_Athena_LaunchPadThrowable_Parent.B_Prj_Athena_LaunchPadThrowable_Parent_C"), TEXT("BuilderClassFloorless"), TEXT("FortDynamicBuilder_LaunchPadThrowable_Floorless"), TEXT("/LaunchPadItemGameplay/Items/BuildingInstructions/FortBuildingInstructions_LaunchPadThrowable_Floorless.FortBuildingInstructions_LaunchPadThrowable_Floorless"), true, true, TEXT("EDynamicBuildingPlacementType::DestroyAnythingThatCollides") }
    };

    for (auto& BuilderClass : BuilderClasses)
    {
        auto ProjectileClass = WorldContextObject ? (UClass*)StaticFindObject(nullptr, nullptr, BuilderClass.ProjectileClass) : nullptr;
        auto BuilderClassProperty = ProjectileClass && WorldContextObject->IsA(ProjectileClass) ? ProjectileClass->FindPropertyByName(FName(BuilderClass.BuilderClassProperty)) : nullptr;

        if (!BuilderClassProperty || *BuilderClassProperty->ContainerPtrToValuePtr<UClass*>(WorldContextObject))
            continue;

        for (auto Frame = Stack.GetPreviousFrame(); Frame && Frame->Object == WorldContextObject && Frame->Code; Frame = Frame->GetPreviousFrame())
        {
            for (auto Code = Frame->Code - sizeof(BuilderClassProperty); Code >= Frame->Code - 32; Code--)
            {
                if (*(UProperty* const*)Code != BuilderClassProperty)
                    continue;

                if (LoadObject<UObject>(nullptr, BuilderClass.BuildingInstructions))
                    return &BuilderClass;

                UE_LOG(LogVelora, Error, TEXT("Could not find %s"), BuilderClass.BuildingInstructions);

                return nullptr;
            }
        }
    }

    return nullptr;
}

void AFortDynamicBuilder::SetServerOnlyBuilderClassDefaults(const FServerOnlyDynamicBuilderClass& BuilderClass)
{
    static auto BuildingInstructions = StaticClass()->FindPropertyByName(FName(TEXT("BuildingInstructions")));
    static auto bSelectiveDestruction = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bSelectiveDestruction")));
    static auto bDestroyMatchingPiece = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bDestroyMatchingPiece")));
    static auto bNoCollisionFail = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bNoCollisionFail")));
    static auto bShrinkAndDestroyEffect = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bShrinkAndDestroyEffect")));
    static auto BuildingConstructionTime = StaticClass()->FindPropertyByName(FName(TEXT("BuildingConstructionTime")));
    static auto DynamicBuildingPlacementType = StaticClass()->FindPropertyByName(FName(TEXT("DynamicBuildingPlacementType")));
    static auto bDestroyFoliageWhenPlaced = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bDestroyFoliageWhenPlaced")));
    static auto DynamicBuildingPlacementTypes = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EDynamicBuildingPlacementType"));

    *BuildingInstructions->ContainerPtrToValuePtr<UObject*>(this) = LoadObject<UObject>(nullptr, BuilderClass.BuildingInstructions);
    bSelectiveDestruction->SetPropertyValue_InContainer(this, BuilderClass.bSelectiveDestruction);
    bDestroyMatchingPiece->SetPropertyValue_InContainer(this, BuilderClass.bDestroyMatchingPiece);
    bNoCollisionFail->SetPropertyValue_InContainer(this, true);
    bShrinkAndDestroyEffect->SetPropertyValue_InContainer(this, false);
    *BuildingConstructionTime->ContainerPtrToValuePtr<float>(this) = 4.0f;

    if (DynamicBuildingPlacementType && DynamicBuildingPlacementTypes)
        *DynamicBuildingPlacementType->ContainerPtrToValuePtr<uint8>(this) = (uint8)DynamicBuildingPlacementTypes->GetValueByName(FName(BuilderClass.DynamicBuildingPlacementType));

    if (bDestroyFoliageWhenPlaced)
        bDestroyFoliageWhenPlaced->SetPropertyValue_InContainer(this, true);
}
