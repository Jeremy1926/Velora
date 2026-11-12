#include "pch.h"
#include "FortGameStateZone.h"
#include "Building/BuildingSMActor.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortGameStateZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameStateZone"));

    return Class;
}

void AFortGameStateZone::GetPlayerBuildableClasses(TArray<UClass*>& OutBuildingClasses, const FPlayerBuildableClassFilter& Filter) const
{
    OutBuildingClasses.Reset();

    const int32 FirstResourceType = (uint8)Filter.ResourceType < 4 ? (uint8)Filter.ResourceType : 0;
    const int32 LastResourceType = (uint8)Filter.ResourceType < 4 ? (uint8)Filter.ResourceType : 3;

    for (int32 ResourceType = FirstResourceType; ResourceType <= LastResourceType; ResourceType++)
    {
        auto& BuildingClasses = GetPlayerBuildableClasses()[ResourceType].BuildingClasses;

        if (Filter.BuildingLevel <= 0 && Filter.BuildingType == EFortBuildingType::None && !Filter.EditModePatternData)
        {
            OutBuildingClasses.Append(BuildingClasses);
            continue;
        }

        for (auto BuildingClass : BuildingClasses)
        {
            auto BuildingActor = (ABuildingSMActor*)(BuildingClass && BuildingClass->IsChildOf(ABuildingActor::StaticClass()) ? BuildingClass : nullptr)->GetDefaultObject();

            const bool bLevelMatches = Filter.BuildingLevel == -1 || Filter.BuildingLevel == BuildingActor->GetCurrentBuildingLevel();
            const bool bTypeMatches = Filter.BuildingType == EFortBuildingType::None || Filter.BuildingType == BuildingActor->GetBuildingType();
            const bool bPatternMatches = !Filter.EditModePatternData || Filter.EditModePatternData == BuildingActor->GetEditModePatternData();

            if (bLevelMatches && bTypeMatches && bPatternMatches)
                OutBuildingClasses.Add(BuildingClass);
        }
    }
}

const FPlayerBuildableClassContainer* AFortGameStateZone::GetPlayerBuildableClasses() const
{
    static auto PlayerBuildableClasses = GetClass()->FindPropertyByName(FName(TEXT("PlayerBuildableClasses")));

    return PlayerBuildableClasses->ContainerPtrToValuePtr<FPlayerBuildableClassContainer>(this);
}

const TArray<UClass*>& AFortGameStateZone::GetAllPlayerBuildableClasses() const
{
    static auto AllPlayerBuildableClasses = GetClass()->FindPropertyByName(FName(TEXT("AllPlayerBuildableClasses")));

    return *AllPlayerBuildableClasses->ContainerPtrToValuePtr<TArray<UClass*>>(this);
}

void AFortGameStateZone::OnAbilitySystemInterfaceReadyForPersistentGameplayEffects(IAbilitySystemInterface* AbilitySystemInterface)
{
    static auto OnAbilitySystemInterfaceReadyForPersistentGameplayEffects = Anchor::Function<void (*)(AFortGameStateZone*, IAbilitySystemInterface*)>(TEXT("AFortGameStateZone::OnAbilitySystemInterfaceReadyForPersistentGameplayEffects"))
        .String(TEXT("STAT_Fort_GameStateZoneApplyingPersistentEffects"), "lea rcx, [0x*]; lea rdx, [rax+0x*]; mov r8, rdi")
        .Start()
        .Is("test rdx, rdx; jz 0x*")
        .Or()
        .String(TEXT("STAT_Fort_GameStateZoneApplyingPersistentEffects"))
        .Start()
        .Get();

    OnAbilitySystemInterfaceReadyForPersistentGameplayEffects(this, AbilitySystemInterface);
}

bool AFortGameStateZone::IsRespawningAllowed(AFortPlayerState* PlayerState) const
{
    static auto IsRespawningAllowed = Anchor::VirtualFunction<bool (*)(const AFortGameStateZone*, AFortPlayerState*)>(TEXT("AFortGameStateZone::IsRespawningAllowed"))
        .Exec(TEXT("/Script/FortniteGame.FortGameStateZone:IsRespawningAllowed"))
        .Has("call [rax+0x80]; test al, al")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortGameStateZone:IsRespawningAllowed"))
        .Has("bt rax, 0x8")
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortGameStateZone:IsRespawningAllowed"))
        .Find("add [*+0x20], *")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortGameStateZone:IsRespawningAllowed"))
        .Find("setnz *; add r*, r*")
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortGameStateZone:IsRespawningAllowed"))
        .Find("call [rax+0x*]")
        .Get();

    return IsRespawningAllowed(this, PlayerState);
}

UBuildingStructuralSupportSystem* AFortGameStateZone::GetStructuralSupportSystem() const
{
    static auto StructuralSupportSystem = GetClass()->FindPropertyByName(FName(TEXT("StructuralSupportSystem")));

    return *StructuralSupportSystem->ContainerPtrToValuePtr<UBuildingStructuralSupportSystem*>(this);
}

UBuildingStructuralSupportSystem* UBuildingStructuralSupportSystem::GetCurrent(const UObject* WorldContextObject)
{
    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? Cast<AFortGameStateZone>(World->GetGameState()) : nullptr;

    return GameState ? GameState->GetStructuralSupportSystem() : nullptr;
}

bool UBuildingStructuralSupportSystem::IsGridInitialized() const
{
    static auto GridBoundsIsValid = Anchor::Offset(TEXT("UBuildingStructuralSupportSystem::GridBounds.IsValid"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieManager"), "push rbx; sub rsp, 0x80; mov rbx, rcx; call 0x*; mov [rsp+0x98], rax; test rax, rax; jz 0x*; movsd xmm1, qword ptr [rax+0x*]")
        .Find("movsd xmm1, qword ptr [rax+0x*]")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieManager"), "mov [rsp+0x8], rbx; mov [rsp+0x18], rdi; push rbp; mov rbp, rsp; sub rsp, 0x80; mov rbx, rcx; call 0x*; mov [rbp+0x18], rax; test rax, rax; jz 0x*; movsd xmm1, qword ptr [rax+0x*]")
        .Find("movsd xmm1, qword ptr [rax+0x*]")
        .Disp();

    return *((const uint8*)this + GridBoundsIsValid) != 0;
}

TMulticastDelegate<void>& UBuildingStructuralSupportSystem::GetOnGridInitialized()
{
    static auto OnGridInitialized = Anchor::Offset(TEXT("UBuildingStructuralSupportSystem::OnGridInitialized"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieManager"), "push rbx; sub rsp, 0x80; mov rbx, rcx; call 0x*; mov [rsp+0x98], rax; test rax, rax; jz 0x*; movsd xmm1, qword ptr [rax+0x*]")
        .Find("lea rcx, [rax+0x*]")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieManager"), "mov [rsp+0x8], rbx; mov [rsp+0x18], rdi; push rbp; mov rbp, rsp; sub rsp, 0x80; mov rbx, rcx; call 0x*; mov [rbp+0x18], rax; test rax, rax; jz 0x*; movsd xmm1, qword ptr [rax+0x*]")
        .Find("lea rdi, [rax+0x*]")
        .Disp();

    return *(TMulticastDelegate<void>*)((uint8*)this + OnGridInitialized);
}

void UFortCurieManager::NotifyAllCurieManagerComponentsRegistered()
{
    DETOUR(UFortCurieManager, NotifyAllCurieManagerComponentsRegistered, Anchor::Hook()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieManager"), "push rbx; sub rsp, 0x80; mov rbx, rcx; call 0x*; mov [rsp+0x98], rax; test rax, rax; jz 0x*; movsd xmm1, qword ptr [rax+0x*]")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieManager"), "mov [rsp+0x8], rbx; mov [rsp+0x18], rdi; push rbp; mov rbp, rsp; sub rsp, 0x80; mov rbx, rcx; call 0x*; mov [rbp+0x18], rax; test rax, rax; jz 0x*; movsd xmm1, qword ptr [rax+0x*]")
        .Optional()
        .LogSlot(TEXT("UFortCurieManager::NotifyAllCurieManagerComponentsRegistered"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortCurieManager")));

    if (auto StructuralSupportSystem = UBuildingStructuralSupportSystem::GetCurrent(this))
    {
        if (StructuralSupportSystem->IsGridInitialized())
            HandleBuildingGridInitialized();
        else
            StructuralSupportSystem->GetOnGridInitialized().AddUObject(this, &UFortCurieManager::HandleBuildingGridInitialized);
    }

    Original();
}

void UFortCurieManager::HandleBuildingGridInitialized()
{
    static auto NotifyBuildingGridInitialized = Anchor::VirtualFunction<void (*)(UObject*, UBuildingStructuralSupportSystem*)>(TEXT("UFortCurieManagerComponent::NotifyBuildingGridInitialized"))
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieSpatialManager"), "test rdx, rdx; jz 0x*; mov rax, rsp; push rbx; sub rsp, 0x*; movups xmm0, [rdx+0x*]")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortCurieSpatialManager"), "test rdx, rdx; jz 0x*; push rbx; sub rsp, 0x*; mov rax, [0x*]; xor rax, rsp; mov [rsp+0x*], rax; movups xmm0, [rdx+0x*]")
        .Get();

    static auto FortCurieManagerComponentClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortCurieManagerComponent"));

    auto StructuralSupportSystem = UBuildingStructuralSupportSystem::GetCurrent(this);

    for (auto CurieManagerComponent : GetCurieManagerComponents())
    {
        if (CurieManagerComponent && CurieManagerComponent->IsA(FortCurieManagerComponentClass))
            NotifyBuildingGridInitialized(CurieManagerComponent, StructuralSupportSystem);
    }
}

const TArray<UObject*>& UFortCurieManager::GetCurieManagerComponents() const
{
    static auto CurieManagerComponents = GetClass()->FindPropertyByName(FName(TEXT("CurieManagerComponents")));

    return *CurieManagerComponents->ContainerPtrToValuePtr<TArray<UObject*>>(this);
}
