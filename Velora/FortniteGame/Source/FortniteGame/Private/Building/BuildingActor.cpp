#include "pch.h"
#include "Building/BuildingActor.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static int32_t InitializeBuildingActorSlot()
{
    static auto InitializeBuildingActor = []
    {
        auto InitializeKismetSpawnedBuildingActor = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:InitializeKismetSpawnedBuildingActor")).Find("call [*+0x*]").Disp();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingSMActor"));

        return Scanner::At(VTable[InitializeKismetSpawnedBuildingActor / sizeof(void*)]).Find("call [*+0x*]").Disp();
    }();

    return InitializeBuildingActor;
}

static int32_t GetCurrentBuildingLevelSlot()
{
    static auto GetCurrentBuildingLevel = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:GetCurrentBuildingLevel")).Find("call [*+0x*]").Disp();

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
        auto ReceiveBeginPlayName = Scanner::String(TEXT("ReceiveBeginPlay")).Find("lea *, [0x*]").Target();
        auto ReceiveBeginPlay = Scanner::Reference(ReceiveBeginPlayName, "mov rdx, *").Start().Get();
        auto ActorBeginPlay = Scanner::CallTo(ReceiveBeginPlay).Start().Get();
        auto ActorVTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = 0;

        while (ActorVTable[Index] != ActorBeginPlay)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__BuildingActor"));

    (*(void (**)(ABuildingActor*))(VTable + BeginPlay))(this);
}

void ABuildingActor::SilentDie()
{
    static auto SilentDie = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie")).Find("jmp [*+0x*]").Disp();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(ABuildingActor*))(VTable + SilentDie))(this);
}

void ABuildingActor::Die(FGameplayTag DamageTag, AController* EventInstigator, AActor* DamageCauser)
{
    static auto Die = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:SilentDie")).Find("jmp [*+0x*]").Disp() - (int32_t)sizeof(void*);

    auto VTable = *(uint8_t**)this;

    (*(void (**)(ABuildingActor*, FGameplayTag, AController*, AActor*))(VTable + Die))(this, DamageTag, EventInstigator, DamageCauser);
}

void ABuildingActor::InitializeBuildingActor(EFortBuildingInitializationReason InitializationReason, int32 InOwnerPersistentID, ABuildingActor* BuildingOwner, ABuildingActor* ReplacedBuilding, bool bFlag)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(ABuildingActor*, EFortBuildingInitializationReason, int32, ABuildingActor*, ABuildingActor*, bool))(VTable + InitializeBuildingActorSlot()))(this, InitializationReason, InOwnerPersistentID, BuildingOwner, ReplacedBuilding, bFlag);
}

void ABuildingActor::PostInitializeSpawnedBuildingActor(EFortBuildingInitializationReason InitializationReason)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(ABuildingActor*, EFortBuildingInitializationReason))(VTable + InitializeBuildingActorSlot() + sizeof(void*)))(this, InitializationReason);
}

int32 ABuildingActor::GetCurrentBuildingLevel() const
{
    auto VTable = *(uint8_t**)this;

    return (*(int32 (**)(const ABuildingActor*))(VTable + GetCurrentBuildingLevelSlot()))(this);
}

void ABuildingActor::SetCurrentBuildingLevel(int32 NewLevel)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(ABuildingActor*, int32))(VTable + GetCurrentBuildingLevelSlot() + sizeof(void*)))(this, NewLevel);
}

bool ABuildingActor::GetDestroyOnPlayerBuildingPlacement() const
{
    static auto bDestroyOnPlayerBuildingPlacement = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bDestroyOnPlayerBuildingPlacement")));

    return bDestroyOnPlayerBuildingPlacement->GetPropertyValue_InContainer(this);
}

float ABuildingActor::GetCullDistance() const
{
    static auto CullDistance = ABuildingActor::StaticClass()->FindPropertyByName(FName(TEXT("CullDistance")));

    return *CullDistance->ContainerPtrToValuePtr<float>(this);
}

EFortBuildingType ABuildingActor::GetBuildingType() const
{
    static auto BuildingType = GetClass()->FindPropertyByName(FName(TEXT("BuildingType")));

    return *BuildingType->ContainerPtrToValuePtr<EFortBuildingType>(this);
}

float ABuildingActor::GetMaxHealth() const
{
    static auto GetMaxHealth = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingActor:GetMaxHealth")).Find("call [*+0x*]").Disp();

    auto VTable = *(uint8_t**)this;

    return (*(float (**)(const ABuildingActor*))(VTable + GetMaxHealth))(this);
}

bool ABuildingActor::HasHealthLeft() const
{
    static auto HasHealthLeft = (bool (*)(const ABuildingActor*))Scanner::Native(TEXT("/Script/FortniteGame.BuildingActor:HasHealthLeft")).Get();

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
