#include "pch.h"
#include "Athena/SpecialActor/AthenaSpecialActorComponent.h"
#include "Athena/FortGameStateAthena.h"
#include "FortSpecialActorReplicationInfo.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Misc/CoreMiscDefines.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "Config.h"

UClass* UAthenaSpecialActorComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaSpecialActorComponent"));

    return Class;
}

void UAthenaSpecialActorComponent::Activate(bool bReset)
{
    DETOUR(UAthenaSpecialActorComponent, Activate, Anchor::Hook()
        .If(StaticClass())
        .If(::Anchor::Exists().Pattern("C7 ? 18 00 00 00 3F C7 ? 1C CD CC CC 3D").Get())
        .Exec(TEXT("/Script/Engine.ActorComponent:Activate"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("UActorComponent::Activate"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__AthenaSpecialActorComponent"), "jmp 0x*")
        .Or()
        .Exec(TEXT("/Script/Engine.ActorComponent:Activate"))
        .Find("mov [*+0x20], r*; mov rax, [r*]")
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("UActorComponent::Activate"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__AthenaSpecialActorComponent"), "jmp 0x*")
        .InVTable(TEXT("/Script/FortniteGame.Default__AthenaSpecialActorComponent")));

    Original(bReset);

    RegisterSpecialActor();
}

void UAthenaSpecialActorComponent::Deactivate()
{
    DETOUR(UAthenaSpecialActorComponent, Deactivate, Anchor::Hook()
        .If(StaticClass())
        .If(::Anchor::Exists().Pattern("C7 ? 18 00 00 00 3F C7 ? 1C CD CC CC 3D").Get())
        .Exec(TEXT("/Script/Engine.ActorComponent:Deactivate"))
        .Find("jmp [rax+0x*]")
        .LogSlot(TEXT("UActorComponent::Deactivate"))
        .IsVirtual(TEXT("/Script/FortniteGame.Default__AthenaSpecialActorComponent"), "jmp 0x*")
        .InVTable(TEXT("/Script/FortniteGame.Default__AthenaSpecialActorComponent")));

    Original();

    UnregisterSpecialActor();
}

void UAthenaSpecialActorComponent::RegisterSpecialActor()
{
    auto Owner = GetOwner();
    auto World = Owner && Owner->HasAuthority() ? Owner->GetWorld() : nullptr;
    auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
    auto SpecialActorData = GameState ? GameState->GetSpecialActorData() : nullptr;
    auto SpecialActorTag = GetSpecialActorTag();

    if (!SpecialActorData || !SpecialActorTag.IsValid() || GetSpecialActorID() != NAME_None)
        return;

    static auto RepDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.SpecialActorRepData"));
    static auto SlateBrushStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/SlateCore.SlateBrush"));
    static auto Vector2DStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.Vector2D"));
    static auto GameplayTagContainerStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayTags.GameplayTagContainer"));
    static auto ElevationArrowDataStruct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ElevationArrowData"));

    static auto RepDataConstructor = Anchor::Address()
        .Exec(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:RegisterSpecialActor"))
        .Find("call 0x*; cmp eax, 0xffffffff")
        .Target()
        .Find("mov r8, r*; lea rcx, [rsp+0x*]; mov rdx, r*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.BuildingProp_CaptureItemSpawner:RegisterSpecialActor"))
        .Find("call 0x*; cmp eax, 0xffffffff")
        .Target()
        .Find("mov r8, r*; lea rcx, [rsp+0x*]; mov rdx, r*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto InitDataConstructor = Anchor::Address()
        .Pattern("C7 ? 18 00 00 00 3F C7 ? 1C CD CC CC 3D")
        .Start()
        .Get();

    if (!RepDataStruct || !SlateBrushStruct || !Vector2DStruct || !GameplayTagContainerStruct || !ElevationArrowDataStruct || !RepDataConstructor || !InitDataConstructor)
        return;

    static auto SpecialActorID = Anchor::Offset(TEXT("FSpecialActorInitData::SpecialActorID"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("SpecialActorID"))))
        .Last("mov eax, [r*+0x*]")
        .Disp();

    static auto CategoryTag = Anchor::Offset(TEXT("FSpecialActorInitData::CategoryTag"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("CategoryTag"))))
        .Last("mov eax, [r*+0x*]")
        .Disp();

    static auto MinimapIconBrush = Anchor::Offset(TEXT("FSpecialActorInitData::MinimapIconBrush"))
        .At(InitDataConstructor)
        .Find("add rcx, 0x*; call 0x*")
        .Imm();

    static auto MinimapIconScale = Anchor::Offset(TEXT("FSpecialActorInitData::MinimapIconScale"))
        .At(InitDataConstructor)
        .Find("add rcx, 0x*; call 0x*")
        .Find("call 0x*")
        .Find("mov [r*+0x*], r*")
        .Disp();

    static auto FullScreenMapIconBrush = Anchor::Offset(TEXT("FSpecialActorInitData::FullScreenMapIconBrush"))
        .At(InitDataConstructor)
        .Find("call 0x*", 2)
        .Last("lea rcx, [r*+0x*]")
        .Disp();

    static auto FullScreenMapIconScale = Anchor::Offset(TEXT("FSpecialActorInitData::FullScreenMapIconScale"))
        .At(InitDataConstructor)
        .Find("call 0x*", 2)
        .Find("mov [r*+0x*], r*")
        .Disp();

    static auto CompassIconBrush = Anchor::Offset(TEXT("FSpecialActorInitData::CompassIconBrush"))
        .At(InitDataConstructor)
        .Find("call 0x*", 3)
        .Last("lea rcx, [r*+0x*]")
        .Disp();

    static auto CompassIconScale = Anchor::Offset(TEXT("FSpecialActorInitData::CompassIconScale"))
        .At(InitDataConstructor)
        .Find("call 0x*", 3)
        .Find("mov [r*+0x*], r*")
        .Disp();

    static auto bDrawCompassIcon = Anchor::Offset(TEXT("FSpecialActorInitData::bDrawCompassIcon"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], al", RepDataStruct->FindPropertyByName(FName(TEXT("bDrawCompassIcon"))))
        .Last("mov* *, *[r*+0x*]")
        .Disp();

    static auto bDrawCompassDistanceText = Anchor::Offset(TEXT("FSpecialActorInitData::bDrawCompassDistanceText"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], al", RepDataStruct->FindPropertyByName(FName(TEXT("bDrawCompassDistanceText"))))
        .Last("mov* *, *[r*+0x*]")
        .Disp();

    static auto bDrawRotated = Anchor::Offset(TEXT("FSpecialActorInitData::bDrawRotated"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], al", RepDataStruct->FindPropertyByName(FName(TEXT("bDrawRotated"))))
        .Last("mov* *, *[r*+0x*]")
        .Optional()
        .Disp();

    static auto YawOffset = Anchor::Offset(TEXT("FSpecialActorInitData::YawOffset"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("YawOffset"))))
        .Last("mov eax, [r*+0x*]")
        .Optional()
        .Disp();

    static auto ElevationArrowData = Anchor::Offset(TEXT("FSpecialActorInitData::ElevationArrowData"))
        .At(RepDataConstructor)
        .FieldFind("movups [r*+%x], xmm0", RepDataStruct->FindPropertyByName(FName(TEXT("ElevationArrowData"))))
        .Last("movups xmm0, [r*+0x*]")
        .Disp();

    static auto RenderOnMapDuringPhases = Anchor::Offset(TEXT("FSpecialActorInitData::RenderOnMapDuringPhases"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], al", RepDataStruct->FindPropertyByName(FName(TEXT("RenderOnMapDuringPhases"))))
        .Last("mov* *, *[r*+0x*]")
        .Disp();

    static auto RenderDistance = Anchor::Offset(TEXT("FSpecialActorInitData::RenderDistance"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("RenderDistance"))))
        .Last("mov eax, [r*+0x*]")
        .Disp();

    static auto RenderTags = Anchor::Offset(TEXT("FSpecialActorInitData::RenderTags"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("RenderDistance"))))
        .Find("call 0x*")
        .Last("lea rdx, [r*+0x*]")
        .Disp();

    static auto DrawLayer = Anchor::Offset(TEXT("FSpecialActorInitData::DrawLayer"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], al", RepDataStruct->FindPropertyByName(FName(TEXT("DrawLayer"))))
        .Last("mov* *, *[r*+0x*]")
        .Optional()
        .Disp();

    static auto bDrawUnderStorm = Anchor::Offset(TEXT("FSpecialActorInitData::bDrawUnderStorm"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], al", RepDataStruct->FindPropertyByName(FName(TEXT("bDrawUnderStorm"))))
        .Last("mov* *, *[r*+0x*]")
        .Optional()
        .Disp();

    static auto SeasonItemTag = Anchor::Offset(TEXT("FSpecialActorInitData::SeasonItemTag"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("SeasonItemTag"))))
        .Last("mov eax, [r*+0x*]")
        .Optional()
        .Disp();

    static auto SeasonItemInteractedBrush = Anchor::Offset(TEXT("FSpecialActorInitData::SeasonItemInteractedBrush"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("SeasonItemTag"))))
        .Find("call 0x*")
        .Last("lea rdx, [r*+0x*]")
        .Optional()
        .Disp();

    static auto Priority = Anchor::Offset(TEXT("FSpecialActorInitData::Priority"))
        .At(RepDataConstructor)
        .FieldFind("mov [r*+%x], eax", RepDataStruct->FindPropertyByName(FName(TEXT("Priority"))))
        .Last("mov eax, [r*+0x*]")
        .Optional()
        .Disp();

    if (!SpecialActorID || !CategoryTag || !MinimapIconBrush)
        return;

    static auto SpecialActorMinimapIconBrush = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorMinimapIconBrush")));
    static auto SpecialActorMinimapIconScale = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorMinimapIconScale")));
    static auto SpecialActorFullScreenMapIconBrush = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorFullScreenMapIconBrush")));
    static auto SpecialActorFullScreenMapIconScale = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorFullScreenMapIconScale")));
    static auto SpecialActorCompassIconBrush = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorCompassIconBrush")));
    static auto SpecialActorCompassIconScale = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorCompassIconScale")));
    static auto bUseCompassIcon = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bUseCompassIcon")));
    static auto bComponentDrawCompassDistanceText = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDrawCompassDistanceText")));
    static auto bComponentDrawRotated = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDrawRotated")));
    static auto ComponentYawOffset = GetClass()->FindPropertyByName(FName(TEXT("YawOffset")));
    static auto ComponentElevationArrowData = GetClass()->FindPropertyByName(FName(TEXT("ElevationArrowData")));
    static auto ComponentRenderOnMapDuringPhases = GetClass()->FindPropertyByName(FName(TEXT("RenderOnMapDuringPhases")));
    static auto ComponentRenderDistance = GetClass()->FindPropertyByName(FName(TEXT("RenderDistance")));
    static auto ComponentRenderTags = GetClass()->FindPropertyByName(FName(TEXT("RenderTags")));
    static auto ComponentDrawLayer = GetClass()->FindPropertyByName(FName(TEXT("DrawLayer")));
    static auto bDrawMapIconUnderStorm = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDrawMapIconUnderStorm")));
    static auto ComponentSeasonItemTag = GetClass()->FindPropertyByName(FName(TEXT("SeasonItemTag")));
    static auto ComponentSeasonItemInteractedBrush = GetClass()->FindPropertyByName(FName(TEXT("SeasonItemInteractedBrush")));
    static auto ComponentPriority = GetClass()->FindPropertyByName(FName(TEXT("Priority")));

    FSpecialActorInitData InitData;
    auto Data = (uint8*)&InitData;
    auto Name = SpecialActorTag.ToString();

    Name += TEXT("_");
    Name += Owner->GetName();

    InitData.SpecialActor = Owner;
    TGameLayout<FName>::Store(Data + SpecialActorID, FName(*Name));
    TGameLayout<FGameplayTag>::Store(Data + CategoryTag, SpecialActorTag);

    if (SpecialActorMinimapIconBrush)
        SlateBrushStruct->CopyScriptStruct(Data + MinimapIconBrush, SpecialActorMinimapIconBrush->ContainerPtrToValuePtr<void>(this));

    if (MinimapIconScale && SpecialActorMinimapIconScale)
        Vector2DStruct->CopyScriptStruct(Data + MinimapIconScale, SpecialActorMinimapIconScale->ContainerPtrToValuePtr<void>(this));

    if (FullScreenMapIconBrush && SpecialActorFullScreenMapIconBrush)
        SlateBrushStruct->CopyScriptStruct(Data + FullScreenMapIconBrush, SpecialActorFullScreenMapIconBrush->ContainerPtrToValuePtr<void>(this));

    if (FullScreenMapIconScale && SpecialActorFullScreenMapIconScale)
        Vector2DStruct->CopyScriptStruct(Data + FullScreenMapIconScale, SpecialActorFullScreenMapIconScale->ContainerPtrToValuePtr<void>(this));

    if (CompassIconBrush && SpecialActorCompassIconBrush)
        SlateBrushStruct->CopyScriptStruct(Data + CompassIconBrush, SpecialActorCompassIconBrush->ContainerPtrToValuePtr<void>(this));

    if (CompassIconScale && SpecialActorCompassIconScale)
        Vector2DStruct->CopyScriptStruct(Data + CompassIconScale, SpecialActorCompassIconScale->ContainerPtrToValuePtr<void>(this));

    if (bDrawCompassIcon && bUseCompassIcon)
        *(bool*)(Data + bDrawCompassIcon) = bUseCompassIcon->GetPropertyValue_InContainer(this);

    if (bDrawCompassDistanceText && bComponentDrawCompassDistanceText)
        *(bool*)(Data + bDrawCompassDistanceText) = bComponentDrawCompassDistanceText->GetPropertyValue_InContainer(this);

    if (bDrawRotated && bComponentDrawRotated)
        *(bool*)(Data + bDrawRotated) = bComponentDrawRotated->GetPropertyValue_InContainer(this);

    if (YawOffset && ComponentYawOffset)
        *(float*)(Data + YawOffset) = *ComponentYawOffset->ContainerPtrToValuePtr<float>(this);

    if (ElevationArrowData && ComponentElevationArrowData)
        ElevationArrowDataStruct->CopyScriptStruct(Data + ElevationArrowData, ComponentElevationArrowData->ContainerPtrToValuePtr<void>(this));

    if (RenderOnMapDuringPhases && ComponentRenderOnMapDuringPhases)
        *(Data + RenderOnMapDuringPhases) = *ComponentRenderOnMapDuringPhases->ContainerPtrToValuePtr<uint8>(this);

    if (RenderDistance && ComponentRenderDistance)
        *(float*)(Data + RenderDistance) = ComponentRenderDistance->ContainerPtrToValuePtr<FScalableFloat>(this)->GetValueAtLevel(0.0f);

    if (RenderTags && ComponentRenderTags)
        GameplayTagContainerStruct->CopyScriptStruct(Data + RenderTags, ComponentRenderTags->ContainerPtrToValuePtr<void>(this));

    if (DrawLayer && ComponentDrawLayer)
        *(Data + DrawLayer) = *ComponentDrawLayer->ContainerPtrToValuePtr<uint8>(this);

    if (bDrawUnderStorm && bDrawMapIconUnderStorm)
        *(bool*)(Data + bDrawUnderStorm) = bDrawMapIconUnderStorm->GetPropertyValue_InContainer(this);

    if (SeasonItemTag && ComponentSeasonItemTag)
        TGameLayout<FGameplayTag>::Store(Data + SeasonItemTag, ComponentSeasonItemTag->GetPropertyValue_InContainer<FGameplayTag>(this));

    if (SeasonItemInteractedBrush && ComponentSeasonItemInteractedBrush)
        SlateBrushStruct->CopyScriptStruct(Data + SeasonItemInteractedBrush, ComponentSeasonItemInteractedBrush->ContainerPtrToValuePtr<void>(this));

    if (Priority && ComponentPriority)
        *(int32*)(Data + Priority) = *ComponentPriority->ContainerPtrToValuePtr<int32>(this);

    auto Index = SpecialActorData->AddActorToReplicationList(InitData);

    if (Index != INDEX_NONE)
        SetSpecialActorID(TGameLayout<FName>::Load(Data + SpecialActorID));

    if (Config::bLogReplication)
    {
        UE_LOG(LogVelora, Log, TEXT("SpecialActor %s registered as %s, index %d"), *Owner->GetName(), *Name, Index);
    }
}

void UAthenaSpecialActorComponent::UnregisterSpecialActor()
{
    auto Owner = GetOwner();
    auto SpecialActorID = GetSpecialActorID();
    auto World = Owner && Owner->HasAuthority() && SpecialActorID != NAME_None ? Owner->GetWorld() : nullptr;
    auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
    auto SpecialActorData = GameState ? GameState->GetSpecialActorData() : nullptr;

    if (!SpecialActorData)
        return;

    SpecialActorData->RemoveActorFromReplicationList(SpecialActorID);
    SetSpecialActorID(NAME_None);

    if (Config::bLogReplication)
    {
        UE_LOG(LogVelora, Log, TEXT("SpecialActor %s unregistered %s"), *Owner->GetName(), *SpecialActorID.ToString());
    }
}

FGameplayTag UAthenaSpecialActorComponent::GetSpecialActorTag() const
{
    static auto SpecialActorTag = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorTag")));

    return SpecialActorTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

FName UAthenaSpecialActorComponent::GetSpecialActorID() const
{
    static auto SpecialActorID = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorID")));

    return SpecialActorID->GetPropertyValue_InContainer<FName>(this);
}

void UAthenaSpecialActorComponent::SetSpecialActorID(FName InSpecialActorID)
{
    static auto SpecialActorID = GetClass()->FindPropertyByName(FName(TEXT("SpecialActorID")));

    SpecialActorID->SetPropertyValue_InContainer<FName>(this, InSpecialActorID);
}
