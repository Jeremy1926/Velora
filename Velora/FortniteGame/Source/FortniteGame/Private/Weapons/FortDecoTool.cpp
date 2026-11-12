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

UClass* AFortDecoTool::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDecoTool"));

    return Class;
}

void AFortDecoTool::SpawnDecoInternal(const TGameValue<FVector>& Location, const TGameValue<FRotator>& Rotation, ABuildingSMActor* AttachedActor, EBuildingAttachmentType AttachmentType, bool bFreeDeco)
{
    DETOUR(AFortDecoTool, SpawnDecoInternal, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortDecoTool"), "ret*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call r10")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .IsVirtual(TEXT("/Script/FortniteGame.Default__FortDecoTool"), "ret*")
        .Optional()
        .InChildVTables(StaticClass()));

    static auto SpawnDecoInternalSlot = Anchor::Slot(TEXT("AFortDecoTool::SpawnDecoInternal"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call r10")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .Get();

    static auto GetDecoItemDefinitionSlot = Anchor::Slot(TEXT("AFortDecoTool::GetDecoItemDefinition"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortDecoTool"), -0x18)
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call r10")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .SlotShift(TEXT("/Script/FortniteGame.Default__FortDecoTool"), -0x18)
        .Find("call [rax+0x*]")
        .Get();

    static auto GetBlueprintClass = Anchor::Optional<UClass** (*)(const UFortItemDefinition*, UClass**)>(TEXT("UFortDecoItemDefinition::GetBlueprintClass"))
        .Native(TEXT("/Script/FortniteGame.FortDecoItemDefinition:GetBlueprintClass"))
        .Is("push rbx; sub rsp, 0x20; mov rbx, rdx")
        .Get();

    static auto GetSubclassOf = Anchor::Optional<UClass** (*)(UClass**, const void*, bool)>(TEXT("FFortAssets::GetSubclassOf"))
        .If(!GetBlueprintClass)
        .Native(TEXT("/Script/FortniteGame.FortDecoItemDefinition:GetBlueprintClass"))
        .Get();

    static auto InitializationReason = (UEnum*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.EFortBuildingInitializationReason"));
    static auto TrapItemDefinition = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortTrapItemDefinition"));
    static auto TrapTool = (int32)InitializationReason->GetValueByName(FName(TEXT("EFortBuildingInitializationReason::TrapTool")));
    static auto PlacementTool = (int32)InitializationReason->GetValueByName(FName(TEXT("EFortBuildingInitializationReason::PlacementTool")));

    auto PlayerController = GetPlayerController();
    auto DecoItemDefinition = PlayerController ? GetDecoItemDefinition() : nullptr;

    if (!DecoItemDefinition)
        return;

    static auto BlueprintClassProperty = DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("BlueprintClass")));

    UClass* BlueprintClass = nullptr;

    if (GetBlueprintClass)
        GetBlueprintClass(DecoItemDefinition, &BlueprintClass);
    else if (GetSubclassOf && BlueprintClassProperty)
        GetSubclassOf(&BlueprintClass, BlueprintClassProperty->ContainerPtrToValuePtr<void>(DecoItemDefinition), true);

    if (!BlueprintClass || !BlueprintClass->IsChildOf(ABuildingActor::StaticClass()))
        return;

    auto ShouldAllowServerSpawnDeco = Scanner::Virtual<bool (*)(AFortDecoTool*, const TGameValue<FVector>*, const TGameValue<FRotator>*, ABuildingSMActor*, EBuildingAttachmentType)>(this, GetDecoItemDefinitionSlot, 0x10);

    if (!ShouldAllowServerSpawnDeco(this, &Location, &Rotation, AttachedActor, AttachmentType))
        return;

    auto SpawnDeco = Scanner::Virtual<AActor* (*)(AFortDecoTool*, UClass**, const TGameValue<FVector>*, const TGameValue<FRotator>*, ABuildingSMActor*, int32, int32)>(this, SpawnDecoInternalSlot, -0x18);

    static auto bBlueprintClassByValue = Anchor::Exists()
        .At(Scanner::Virtual<const void*>(StaticClass()->GetDefaultObject(false), SpawnDecoInternalSlot, -0x18))
        .Find("call [rax+0x*]")
        .Find("call 0x*")
        .Find("test rax, rax")
        .Is("test rax, rax; jz 0x*; test r*, r*; jz 0x*; test r*, r*; jz 0x*; call 0x*")
        .Get();

    const int32 InitializationReasonValue = TrapItemDefinition && DecoItemDefinition->IsA(TrapItemDefinition) ? TrapTool : PlacementTool;
    auto Deco = bBlueprintClassByValue ? ((AActor* (*)(AFortDecoTool*, UClass*, const TGameValue<FVector>*, const TGameValue<FRotator>*, ABuildingSMActor*, int32, int32))SpawnDeco)(this, BlueprintClass, &Location, &Rotation, AttachedActor, (int32)AttachmentType, InitializationReasonValue) : SpawnDeco(this, &BlueprintClass, &Location, &Rotation, AttachedActor, (int32)AttachmentType, InitializationReasonValue);

    if (!Deco)
        return;

    auto AdjustDecoAfterSpawn = Scanner::Virtual<void (*)(AFortDecoTool*, AActor*, ABuildingSMActor*, EBuildingAttachmentType)>(this, GetDecoItemDefinitionSlot, 0x30);

    AdjustDecoAfterSpawn(this, Deco, AttachedActor, AttachmentType);
    FinishedSpawnAction(bFreeDeco);
}

void AFortDecoTool::FinishedSpawnAction(bool bFreeDeco)
{
    static auto ClientForceCancelBuildingTool = Anchor::Function<void (*)(AFortPlayerController*)>(TEXT("AFortPlayerController::ClientForceCancelBuildingTool"))
        .Event(TEXT("ClientForceCancelBuildingTool"))
        .Get();

    auto PlayerController = GetPlayerController();
    auto DecoItemDefinition = PlayerController ? GetDecoItemDefinition() : nullptr;

    if (!DecoItemDefinition)
        return;

    static auto bConsumeWhenPlaced = (UBoolProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bConsumeWhenPlaced")));
    static auto bCancelToolWhenPlaced = (UBoolProperty*)DecoItemDefinition->GetClass()->FindPropertyByName(FName(TEXT("bCancelToolWhenPlaced")));

    if (!bFreeDeco && bConsumeWhenPlaced && bConsumeWhenPlaced->GetPropertyValue_InContainer(DecoItemDefinition))
        PlayerController->RemoveInventoryItem(GetItemEntryGuid(), 1, true);

    if (bCancelToolWhenPlaced && bCancelToolWhenPlaced->GetPropertyValue_InContainer(DecoItemDefinition))
        ClientForceCancelBuildingTool(PlayerController);
}

void AFortDecoTool::ServerCreateBuildingAndSpawnDeco_Implementation(const TGameValue<FVector>& GameBuildingLocation, const TGameValue<FRotator>& GameBuildingRotation, const TGameValue<FVector>& GameLocation, const TGameValue<FRotator>& GameRotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const TGameValue<FVector>& GameBuildingExtraPieceLocation)
{
    DETOUR(AFortDecoTool, ServerCreateBuildingAndSpawnDeco_Implementation, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortPlayerController, ServerCreateBuildingActorInternal))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Or()
        .If(!DETOUR_ORIGINAL(AFortPlayerController, ServerCreateBuildingActorInternal))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Or()
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
        .Is("add rcx, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
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
        .Find("mov r8b, 0x1; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Is("mov rax, rsp; mov [rax+0x8], rbx; mov [rax+0x10], rbp; mov [rax+0x18], rsi; mov [rax+0x20], rdi; push r14; sub rsp, 0x20; xor edi, edi; lea rbp, [rcx+0x10]; mov r14b, r8b")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("test byte ptr [*+0x*], 0x2")
        .Find("call 0x*")
        .Target()
        .Find("mov r8b, 0x1; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("test byte ptr [*+0x*], 0x2")
        .Find("call 0x*")
        .Target()
        .Is("add rcx, 0x*; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [r10+0x*]; add rsp, 0x*")
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call 0x*; mov esi, eax; cmp eax, 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call 0x*; mov esi, eax; cmp eax, 0x*")
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("mov esi, eax; cmp eax, 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("mov esi, eax; cmp eax, 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto TrapExtraPieceCostIgnore = Anchor::Offset(TEXT("FFortGlobalGameplayTags::TrapExtraPieceCostIgnore"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("lea rdx, [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("lea rdx, [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("lea rdx, [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [r10+0x*]; add rsp, 0x*")
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("lea rdx, [rax+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("lea rdx, [rax+0x*]")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [r10+0x*]; add rsp, 0x*")
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("mov [rsp+0x20], al")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("mov [rsp+0x20], al")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto SpawnDecoInternal = Anchor::VirtualFunction<void (*)(AFortDecoTool*, const TGameValue<FVector>*, const TGameValue<FRotator>*, ABuildingSMActor*, EBuildingAttachmentType, bool)>(TEXT("AFortDecoTool::SpawnDecoInternal"))
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call r10; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [r10+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
        .Find("call [r10+0x*]; add rsp, 0x*")
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

    static auto bKeepLoaded = Anchor::Exists()
        .At((const void*)GetSoftObject)
        .Find("lea r*, [rcx+0x*]; mov *, r8b")
        .Get();

    TArray<UObject*> AllowedShapes;
    auto& Shapes = *AutoCreateAttachmentBuildingShapes->ContainerPtrToValuePtr<FScriptArray>(DecoItemDefinition);
    const int32 ShapeSize = AutoCreateAttachmentBuildingShapes->GetInner()->GetSize();

    for (int32 Index = 0; Index < Shapes.Num(); Index++)
    {
        auto Shape = (const uint8*)Shapes.GetData() + Index * ShapeSize;

        if (auto Object = bKeepLoaded ? ((UObject* (*)(const void*, const void*, bool))GetSoftObject)(Shape, nullptr, true) : GetSoftObject(Shape))
            AllowedShapes.Add(Object);
    }

    if (!AllowedShapes.Num())
        return;

    const int32 BuildingType = GetBuildingTypeFromBuildingAttachmentType(InBuildingAttachmentType);

    static auto None = (int32)StaticEnum<EFortBuildingType>()->GetValueByName(FName(TEXT("EFortBuildingType::None")));

    if (BuildingType == None)
        return;

    const bool bIgnoreCostCheck = DoesItemDefinitionHaveGameplayTag(DecoItemDefinition, GameplayTags() + TrapExtraPieceCostIgnore);

    UClass* BuildingClass = nullptr;
    FindAvailableBuildingClassForBuildingTypeAndEditModePattern(PlayerController, &BuildingClass, BuildingType, &AllowedShapes, bIgnoreCostCheck, *AutoCreateAttachmentBuildingResourceType->ContainerPtrToValuePtr<uint8>(DecoItemDefinition));

    if (!BuildingClass || !BuildingClass->IsChildOf(ABuildingSMActor::StaticClass()))
        return;

    FBuildingClassData BuildingClassData{ BuildingClass, -1, 0 };
    FVector SnappedLocation = GameBuildingLocation.Get();
    SnappedLocation.Z = floorf(SnappedLocation.Z * (1.0f / 384.0f) + 0.5f) * 384.0f;

    auto Building = PlayerController->ServerCreateBuildingActorInternal(BuildingClassData, SnappedLocation, GameBuildingRotation, false, GetWorld()->GetTimeSeconds(), false, bIgnoreCostCheck, false);

    if (!Building)
        return;

    static auto bSpawnDecoOnExtraPieceParam = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco")))->FindPropertyByName(FName(TEXT("bSpawnDecoOnExtraPiece")));

    if (!bSpawnDecoOnExtraPieceParam)
    {
        static auto ServerSpawnDeco_Implementation = Anchor::VirtualFunction<void (*)(AFortDecoTool*, const TGameValue<FVector>*, const TGameValue<FRotator>*, ABuildingSMActor*, EBuildingAttachmentType)>(TEXT("AFortDecoTool::ServerSpawnDeco_Implementation"))
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
            .Find("call *r10*; test al, al")
            .Find("call [r10+0x*]")
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerSpawnDeco"))
            .Find("call [r10+0x*]; add rsp, 0x*")
            .Get();

        ServerSpawnDeco_Implementation(this, &GameLocation, &GameRotation, Building, InBuildingAttachmentType);

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
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
            .Find("call [r10+0x*]; add rsp, 0x*")
            .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap"))
            .Find("call [*+0x*]; test eax, eax; jz 0x*")
            .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
            .Find("call 0x*; test rax, rax; jz 0x*; mov r8, *; mov rdx, *; mov rcx, rax; call 0x*")
            .Target()
            .Get();

        static auto GetExistingBuildingInGridLocation = Anchor::Function<ABuildingSMActor* (*)(UObject*, const ABuildingSMActor*, const TGameValue<FVector>*)>(TEXT("UBuildingStructuralSupportSystem::GetExistingBuildingInGridLocation"))
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
            .Find("call *r10*; test al, al")
            .Find("call [r10+0x*]")
            .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool_ContextTrap"))
            .Find("call [*+0x*]; test eax, eax; jz 0x*")
            .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerControllerAthena"))
            .Find("mov r8, *; mov rdx, *; mov rcx, rax; call 0x*")
            .Find("call 0x*")
            .Target()
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
            .Find("call [r10+0x*]; add rsp, 0x*")
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

        if (auto ExistingBuilding = GetExistingBuildingInGridLocation(StructuralSupportSystem, Building, &GameBuildingExtraPieceLocation))
            SpawnDecoInternal(this, &GameBuildingExtraPieceLocation, &GameRotation, ExistingBuilding, InBuildingAttachmentType, false);

        if (bGiveFreeDecoOnUndergroundPiece && bGiveFreeDecoOnUndergroundPiece->GetPropertyValue_InContainer(DecoItemDefinition))
            SpawnDecoInternal(this, &GameLocation, &GameRotation, Building, InBuildingAttachmentType, true);

        return;
    }

    SpawnDecoInternal(this, &GameLocation, &GameRotation, Building, InBuildingAttachmentType, false);
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [r10+0x*]; add rsp, 0x*")
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
        .Has("movzx r10d, r8b; test r8b, r8b")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Has("sub r10d, 0x1; jz 0x*; sub r10d, 0x1; jz 0x*")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call *r10*; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortDecoTool:ServerCreateBuildingAndSpawnDeco"))
        .Find("call [r10+0x*]; add rsp, 0x*")
        .Entry(TEXT("/Script/FortniteGame.Default__FortDecoTool"))
        .Find("call [*+0x*]")
        .Get();

    return GetDecoItemDefinition(this);
}
