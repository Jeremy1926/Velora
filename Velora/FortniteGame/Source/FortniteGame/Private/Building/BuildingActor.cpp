#include "pch.h"
#include "Building/BuildingActor.h"
#include "FortGameStateZone.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static int32_t InitializeBuildingActorSlot()
{
    static auto InitializeBuildingActor = []
    {
        auto InitializeKismetSpawnedBuildingActor = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:InitializeKismetSpawnedBuildingActor")).Find("call [*+0x*]").Slot(TEXT("ABuildingActor::InitializeKismetSpawnedBuildingActor"));
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingSMActor"));
        auto KismetSpawned = Scanner::At(Scanner::Virtual<const void*>(DefaultObject, InitializeKismetSpawnedBuildingActor));
        auto Call = KismetSpawned.Find("call [*+0x*]");

        return (Call.Get() ? Call : KismetSpawned.Find("mov r10, [r8+0x*]")).Slot(TEXT("ABuildingActor::InitializeBuildingActor"));
    }();

    return InitializeBuildingActor;
}

static int32_t GetCurrentBuildingLevelSlot()
{
    static auto GetCurrentBuildingLevel = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:GetCurrentBuildingLevel")).Find("call [*+0x*]").Slot(TEXT("ABuildingActor::GetCurrentBuildingLevel"));

    return GetCurrentBuildingLevel;
}

UClass* ABuildingActor::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingActor"));

    return Class;
}

void ABuildingActor::BeginPlay()
{
    static auto BeginPlay = []
    {
        auto ReceiveBeginPlayName = Scanner::String(TEXT("ReceiveBeginPlay")).Find("lea *, [0x*]").Target(TEXT("ENGINE_ReceiveBeginPlay"));
        auto ReceiveBeginPlay = Scanner::Reference(ReceiveBeginPlayName, "mov rdx, *").Start().Get(TEXT("AActor::ReceiveBeginPlay"));
        auto ActorBeginPlay = Scanner::CallTo(ReceiveBeginPlay).Start().Get(TEXT("AActor::BeginPlay"));
        auto ActorVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));

        return Scanner::Slot(TEXT("AActor::BeginPlay"), ActorVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == ActorBeginPlay; });
    }();

    Scanner::Virtual<void (*)(ABuildingActor*)>(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingActor")), BeginPlay)(this);
}

void ABuildingActor::SilentDie(bool bPropagateSilentDeath)
{
    static auto SilentDie = []
    {
        auto SilentDie = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"));
        auto Jump = SilentDie.Find("jmp [*+0x*]");

        return (Jump.Get() ? Jump : SilentDie.Find("call [*+0x*]")).Slot(TEXT("ABuildingActor::SilentDie"));
    }();
    static auto bHasPropagateSilentDeath = ((UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingActor:SilentDie")))->FindPropertyByName(FName(TEXT("bPropagateSilentDeath"))) != nullptr;

    if (bHasPropagateSilentDeath)
    {
        Scanner::Virtual<void (*)(ABuildingActor*, bool)>(this, SilentDie)(this, bPropagateSilentDeath);
        return;
    }

    Scanner::Virtual<void (*)(ABuildingActor*)>(this, SilentDie)(this);
}

void ABuildingActor::Die(FGameplayTag DamageTag, AController* EventInstigator, AActor* DamageCauser, const FGameplayEffectContextHandle* EffectContext)
{
    static auto Die = []
    {
        auto SilentDie = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie"));
        auto Jump = SilentDie.Find("jmp [*+0x*]");

        return (Jump.Get() ? Jump : SilentDie.Find("call [*+0x*]")).Slot(TEXT("ABuildingActor::SilentDie")) - (int32_t)sizeof(void*);
    }();
    static auto bEffectContext = Scanner::At(Scanner::Virtual<const void*>(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingSMActor")), Die, sizeof(void*))).Has("mov qword ptr [rsp+0x20], 0x0");

    if (bEffectContext)
    {
        Scanner::Virtual<void (*)(ABuildingActor*, FGameplayTag, AController*, AActor*, const FGameplayEffectContextHandle*)>(this, Die)(this, DamageTag, EventInstigator, DamageCauser, EffectContext);
        return;
    }

    Scanner::Virtual<void (*)(ABuildingActor*, FGameplayTag, AController*, AActor*)>(this, Die)(this, DamageTag, EventInstigator, DamageCauser);
}

void ABuildingActor::InitializeBuildingActor(EFortBuildingInitializationReason InitializationReason, int32 InOwnerPersistentID, ABuildingActor* BuildingOwner, ABuildingActor* ReplacedBuilding, bool bFlag)
{
    Scanner::Virtual<void (*)(ABuildingActor*, EFortBuildingInitializationReason, int32, ABuildingActor*, ABuildingActor*, bool)>(this, InitializeBuildingActorSlot())(this, InitializationReason, InOwnerPersistentID, BuildingOwner, ReplacedBuilding, bFlag);
}

void ABuildingActor::PostInitializeSpawnedBuildingActor(EFortBuildingInitializationReason InitializationReason)
{
    Scanner::Virtual<void (*)(ABuildingActor*, EFortBuildingInitializationReason)>(this, InitializeBuildingActorSlot(), sizeof(void*))(this, InitializationReason);
}

int32 ABuildingActor::GetCurrentBuildingLevel() const
{
    return Scanner::Virtual<int32 (*)(const ABuildingActor*)>(this, GetCurrentBuildingLevelSlot())(this);
}

void ABuildingActor::SetCurrentBuildingLevel(int32 NewLevel)
{
    Scanner::Virtual<void (*)(ABuildingActor*, int32)>(this, GetCurrentBuildingLevelSlot(), sizeof(void*))(this, NewLevel);
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
    static auto GetMaxHealth = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:GetMaxHealth")).Find("call [*+0x*]").Slot(TEXT("ABuildingActor::GetMaxHealth"));

    return Scanner::Virtual<float (*)(const ABuildingActor*)>(this, GetMaxHealth)(this);
}

bool ABuildingActor::HasHealthLeft() const
{
    static auto HasHealthLeft = Scanner::Function<bool (*)(const ABuildingActor*)>(Scanner::Native(TEXT("/Script/FortniteGame.BuildingActor:HasHealthLeft")).Get(TEXT("ABuildingActor::HasHealthLeft")));

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

FBuildingClassData FCreateBuildingActorData::GetBuildingClassData(AFortGameStateZone* GameState)
{
    if (GameState && GameState->GetAllPlayerBuildableClasses().IsValidIndex(BuildingClassHandle))
        BuildingClassData.BuildingClass = GameState->GetAllPlayerBuildableClasses()[BuildingClassHandle];

    return BuildingClassData;
}
