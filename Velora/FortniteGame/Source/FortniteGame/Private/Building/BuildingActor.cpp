#include "pch.h"
#include "Building/BuildingActor.h"
#include "FortGameStateZone.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* ABuildingActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingActor"));

    return Class;
}

TMulticastDelegate<void, ABuildingActor*, float>& ABuildingActor::GetNotifyBuildingNetCullDistChange()
{
    static auto NotifyBuildingNetCullDistChange = Anchor::Data<TMulticastDelegate<void, ABuildingActor*, float>*>(TEXT("ABuildingActor::NotifyBuildingNetCullDistChange"))
        .Pattern("0F 57 D2 F3 0F 11 89 ? ? ? ? F3 0F 51 D1 48 8D 91 ? ? ? ? E8")
        .Find("call 0x*")
        .Target()
        .Find("mov rax, [0x*]")
        .Target()
        .Or()
        .Pattern("0F 57 D2 F3 0F 11 89 ? ? ? ? F3 0F 51 D1 48 8D 91 ? ? ? ? E8")
        .Find("call 0x*")
        .Target()
        .Find("movsxd rdi, eax; mov rbp, rdx; jns 0x*")
        .Find("jns 0x*")
        .Target()
        .Find("mov rax, [0x*]")
        .Target()
        .Get();

    return *NotifyBuildingNetCullDistChange;
}

void ABuildingActor::BeginPlay()
{
    static auto BeginPlay = Anchor::VirtualFunction<void (*)(ABuildingActor*)>(TEXT("AActor::BeginPlay"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Log(TEXT("ENGINE_ReceiveBeginPlay"))
        .Reference("mov ?dx, *")
        .Start()
        .Log(TEXT("AActor::ReceiveBeginPlay"))
        .Caller()
        .Log(TEXT("AActor::BeginPlay"))
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Get(TEXT("/Script/FortniteGame.Default__BuildingActor"));

    BeginPlay(this);
}

void ABuildingActor::SilentDie(bool bPropagateSilentDeath)
{
    static auto SilentDie = Anchor::VirtualFunction<void (*)(ABuildingActor*, bool)>(TEXT("ABuildingActor::SilentDie"))
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"))
        .Find("jmp [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"))
        .Find("call [*+0x*]")
        .Get();
    static auto bHasPropagateSilentDeath = ((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingActor:SilentDie")))->FindPropertyByName(FName(TEXT("bPropagateSilentDeath"))) != nullptr;

    if (bHasPropagateSilentDeath)
    {
        SilentDie(this, bPropagateSilentDeath);
        return;
    }

    SilentDie.As<void (*)(ABuildingActor*)>()(this);
}

void ABuildingActor::Die(FGameplayTag DamageTag, AController* EventInstigator, AActor* DamageCauser, const FGameplayEffectContextHandle* EffectContext)
{
    static auto Die = Anchor::VirtualFunction<void (*)(ABuildingActor*, FGameplayTag, AController*, AActor*, const FGameplayEffectContextHandle*)>(TEXT("ABuildingActor::SilentDie"))
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"))
        .Find("jmp [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"))
        .Find("call [*+0x*]")
        .Get(-(int64_t)sizeof(void*));

    static auto bEffectContext = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"))
        .FindEither("jmp [*+0x*]", "call [*+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingSMActor"))
        .Has("mov qword ptr [rsp+0x20], 0x0")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"))
        .FindEither("jmp [*+0x*]", "call [*+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingSMActor"))
        .Has("and qword ptr [rsp+0x20], 0x0*")
        .Get();

    if (bEffectContext)
    {
        Die(this, DamageTag, EventInstigator, DamageCauser, EffectContext);
        return;
    }

    Die.As<void (*)(ABuildingActor*, FGameplayTag, AController*, AActor*)>()(this, DamageTag, EventInstigator, DamageCauser);
}

void ABuildingActor::InitializeBuildingActor(EFortBuildingInitializationReason InitializationReason, int32 InOwnerPersistentID, ABuildingActor* BuildingOwner, ABuildingActor* ReplacedBuilding, bool bFlag)
{
    static auto InitializeBuildingActor = Anchor::VirtualFunction<void (*)(ABuildingActor*, EFortBuildingInitializationReason, int32, ABuildingActor*, ABuildingActor*, bool)>(TEXT("ABuildingActor::InitializeBuildingActor"))
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:InitializeKismetSpawnedBuildingActor"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("ABuildingActor::InitializeKismetSpawnedBuildingActor"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingSMActor"))
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:InitializeKismetSpawnedBuildingActor"))
        .Find("call [*+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingSMActor"))
        .Find("mov r10, [r8+0x*]")
        .Get();

    InitializeBuildingActor(this, InitializationReason, InOwnerPersistentID, BuildingOwner, ReplacedBuilding, bFlag);
}

void ABuildingActor::PostInitializeSpawnedBuildingActor(EFortBuildingInitializationReason InitializationReason)
{
    static auto PostInitializeSpawnedBuildingActor = Anchor::VirtualFunction<void (*)(ABuildingActor*, EFortBuildingInitializationReason)>(TEXT("ABuildingActor::InitializeBuildingActor"))
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:InitializeKismetSpawnedBuildingActor"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("ABuildingActor::InitializeKismetSpawnedBuildingActor"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingSMActor"))
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:InitializeKismetSpawnedBuildingActor"))
        .Find("call [*+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingSMActor"))
        .Find("mov r10, [r8+0x*]")
        .Get(sizeof(void*));

    PostInitializeSpawnedBuildingActor(this, InitializationReason);
}

int32 ABuildingActor::GetCurrentBuildingLevel() const
{
    static auto GetCurrentBuildingLevel = Anchor::VirtualFunction<int32 (*)(const ABuildingActor*)>(TEXT("ABuildingActor::GetCurrentBuildingLevel"))
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:GetCurrentBuildingLevel"))
        .Find("call [*+0x*]")
        .Get();

    return GetCurrentBuildingLevel(this);
}

void ABuildingActor::SetCurrentBuildingLevel(int32 NewLevel)
{
    static auto SetCurrentBuildingLevel = Anchor::VirtualFunction<void (*)(ABuildingActor*, int32)>(TEXT("ABuildingActor::GetCurrentBuildingLevel"))
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:GetCurrentBuildingLevel"))
        .Find("call [*+0x*]")
        .Get(sizeof(void*));

    SetCurrentBuildingLevel(this, NewLevel);
}

bool ABuildingActor::GetDestroyOnPlayerBuildingPlacement() const
{
    static auto bDestroyOnPlayerBuildingPlacement = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDestroyOnPlayerBuildingPlacement")));

    return bDestroyOnPlayerBuildingPlacement->GetPropertyValue_InContainer(this);
}

EFortBuildingType ABuildingActor::GetBuildingType() const
{
    static auto BuildingType = GetClass()->FindPropertyByName(FName(TEXT("BuildingType")));

    return *BuildingType->ContainerPtrToValuePtr<EFortBuildingType>(this);
}

float ABuildingActor::GetMaxHealth() const
{
    static auto GetMaxHealth = Anchor::VirtualFunction<float (*)(const ABuildingActor*)>(TEXT("ABuildingActor::GetMaxHealth"))
        .Exec(TEXT("/Script/FortniteGame.BuildingActor:GetMaxHealth"))
        .Find("call [*+0x*]")
        .Get();

    return GetMaxHealth(this);
}

bool ABuildingActor::HasHealthLeft() const
{
    static auto HasHealthLeft = Anchor::Function<bool (*)(const ABuildingActor*)>(TEXT("ABuildingActor::HasHealthLeft"))
        .Native(TEXT("/Script/FortniteGame.BuildingActor:HasHealthLeft"))
        .Get();

    return HasHealthLeft(this);
}

const FGameplayTagContainer& ABuildingActor::GetStaticGameplayTags() const
{
    static auto StaticGameplayTags = GetClass()->FindPropertyByName(FName(TEXT("StaticGameplayTags")));

    return *StaticGameplayTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}

bool ABuildingActor::GetDestroyed() const
{
    static auto bDestroyed = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDestroyed")));

    return bDestroyed->GetPropertyValue_InContainer(this);
}

UScriptStruct* FCreateBuildingActorData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.CreateBuildingActorData"));

    return Struct;
}

FVector FCreateBuildingActorData::GetBuildLoc() const
{
    static auto BuildLoc = StaticStruct()->FindPropertyByName(FName(TEXT("BuildLoc")));

    return BuildLoc->GetPropertyValue_InContainer<FVector>(this);
}

FRotator FCreateBuildingActorData::GetBuildRot() const
{
    static auto BuildRot = StaticStruct()->FindPropertyByName(FName(TEXT("BuildRot")));

    return BuildRot->GetPropertyValue_InContainer<FRotator>(this);
}

bool FCreateBuildingActorData::GetMirrored() const
{
    static auto bMirrored = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bMirrored")));

    return bMirrored->GetPropertyValue_InContainer(this);
}

FBuildingClassData FCreateBuildingActorData::GetBuildingClassData(AFortGameStateZone* GameState) const
{
    static auto BuildingClassHandle = StaticStruct()->FindPropertyByName(FName(TEXT("BuildingClassHandle")));
    static auto BuildingClassData = StaticStruct()->FindPropertyByName(FName(TEXT("BuildingClassData")));

    auto Result = *BuildingClassData->ContainerPtrToValuePtr<FBuildingClassData>(this);
    const auto Handle = *BuildingClassHandle->ContainerPtrToValuePtr<uint32>(this);

    if (GameState && GameState->GetAllPlayerBuildableClasses().IsValidIndex(Handle))
        Result.BuildingClass = GameState->GetAllPlayerBuildableClasses()[Handle];

    return Result;
}
