#include "pch.h"
#include "Athena/SpecialActor/FortGlobalActorRegistrySubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/SparseDelegate.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UActorRegisterData_GameplayTagBased::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ActorRegisterData_GameplayTagBased"));

    return Class;
}

void UActorRegisterData_GameplayTagBased::RegisterActors(const TArray<AActor*>& InActors)
{
    auto& RegisteredActors = GetRegisteredActors();

    for (auto Actor : InActors)
    {
        if (!Actor || RegisteredActors.Contains(Actor))
            continue;

        RegisteredActors.Add(Actor);

        FScriptDelegate RegisteredActorDestroyedDelegate;
        RegisteredActorDestroyedDelegate.BindUFunction(this, FName(TEXT("RegisteredActorDestroyed")));
        Actor->GetOnDestroyed().bIsBound |= FSparseDelegateStorage::Add(Actor, FName(TEXT("OnDestroyed")), RegisteredActorDestroyedDelegate);

        struct
        {
            FGameplayTag GameplayTag;
            AActor* RegisteredActor;
        } Parms{ GetGameplayTag(), Actor };

        GetOnActorRegistered().ProcessMulticastDelegate<UObject>(&Parms);
    }
}

void UActorRegisterData_GameplayTagBased::UnregisterActors(const TArray<AActor*>& InActors)
{
    auto& RegisteredActors = GetRegisteredActors();

    for (auto Actor : InActors)
    {
        if (!Actor || !RegisteredActors.RemoveSingle(Actor))
            continue;

        struct
        {
            FGameplayTag GameplayTag;
            AActor* UnregisteredActor;
        } Parms{ GetGameplayTag(), Actor };

        GetOnActorUnregistered().ProcessMulticastDelegate<UObject>(&Parms);
    }
}

void UActorRegisterData_GameplayTagBased::RegisteredActorDestroyed(AActor* DestroyedActor)
{
    TArray<AActor*> DestroyedActors;
    DestroyedActors.Add(DestroyedActor);

    UnregisterActors(DestroyedActors);
}

void UActorRegisterData_GameplayTagBased::execRegisteredActorDestroyed(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UActorRegisterData_GameplayTagBased, execRegisteredActorDestroyed, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.ActorRegisterData_GameplayTagBased:RegisteredActorDestroyed"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.ActorRegisterData_GameplayTagBased:RegisteredActorDestroyed")));

    AActor* DestroyedActor = nullptr;

    Stack.StepCompiledIn(&DestroyedActor);
    Stack.Code += !!Stack.Code;

    RegisteredActorDestroyed(DestroyedActor);
}

FGameplayTag& UActorRegisterData_GameplayTagBased::GetGameplayTag()
{
    static auto GameplayTag = Anchor::Offset(TEXT("UActorRegisterData_GameplayTagBased::GameplayTag"))
        .Exec(TEXT("/Script/FortniteGame.ActorRegisterData_GameplayTagBased:GetGameplayTag"))
        .Find("add [*+0x20], *")
        .Find("mov rax, [rcx+0x*]")
        .Disp();

    return *(FGameplayTag*)((uint8*)this + GameplayTag);
}

TArray<AActor*>& UActorRegisterData_GameplayTagBased::GetRegisteredActors()
{
    static auto RegisteredActors = GetClass()->FindPropertyByName(FName(TEXT("RegisteredActors")));

    return *RegisteredActors->ContainerPtrToValuePtr<TArray<AActor*>>(this);
}

FMulticastScriptDelegate& UActorRegisterData_GameplayTagBased::GetOnActorRegistered()
{
    static auto OnActorRegistered = GetClass()->FindPropertyByName(FName(TEXT("OnActorRegistered")));

    return *OnActorRegistered->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

FMulticastScriptDelegate& UActorRegisterData_GameplayTagBased::GetOnActorUnregistered()
{
    static auto OnActorUnregistered = GetClass()->FindPropertyByName(FName(TEXT("OnActorUnregistered")));

    return *OnActorUnregistered->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

UClass* UFortGlobalActorRegistrySubsystem::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem"));

    return Class;
}

UFortGlobalActorRegistrySubsystem* UFortGlobalActorRegistrySubsystem::GetGlobalActorRegistrySubsystem(const UObject* WorldContextObject)
{
    static auto SubsystemCollection = Anchor::Offset(TEXT("UWorld::SubsystemCollection"))
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; lea *, [rax+0x*]")
        .Find("lea *, [rax+0x*]")
        .Disp();
    static auto GetSubsystemInternal = Anchor::Function<UObject* (*)(void*, UClass*)>(TEXT("FSubsystemCollectionBase::GetSubsystemInternal"))
        .Exec(TEXT("/Script/Engine.SubsystemBlueprintLibrary:GetWorldSubsystem"))
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; lea *, [rax+0x*]")
        .Find("lea *, [rax+0x*]")
        .Find("call 0x*")
        .Target()
        .Get();

    auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

    return World && StaticClass() ? (UFortGlobalActorRegistrySubsystem*)GetSubsystemInternal((uint8*)World + SubsystemCollection, StaticClass()) : nullptr;
}

UActorRegisterData_GameplayTagBased* UFortGlobalActorRegistrySubsystem::GetActorRegistryDataForGameplayTag(const FGameplayTag& GameplayTag)
{
    if (!GameplayTag.IsValid())
        return nullptr;

    auto& ActorRegistrationDataByGameplayTagMap = GetActorRegistrationDataByGameplayTagMap();

    if (auto RegisterData = ActorRegistrationDataByGameplayTagMap.Find(GameplayTag))
        return *RegisterData;

    auto RegisterData = NewObject<UActorRegisterData_GameplayTagBased>(this, UActorRegisterData_GameplayTagBased::StaticClass());

    if (!RegisterData)
        return nullptr;

    RegisterData->GetGameplayTag() = GameplayTag;

    return ActorRegistrationDataByGameplayTagMap.Add(GameplayTag, RegisterData);
}

void UFortGlobalActorRegistrySubsystem::GetRegisteredActors(TArray<AActor*>& OutActors, const FGameplayTag& GameplayTag)
{
    OutActors.Empty();

    auto RegisterData = GetActorRegistrationDataByGameplayTagMap().Find(GameplayTag);

    if (RegisterData && *RegisterData)
        OutActors = (*RegisterData)->GetRegisteredActors();
}

TArray<FGameplayTag> UFortGlobalActorRegistrySubsystem::GetRegistrationTagsForActor(const AActor* InActor)
{
    TArray<FGameplayTag> RegistrationTags;

    for (auto& Pair : GetActorRegistrationDataByGameplayTagMap())
    {
        if (Pair.Value && Pair.Value->GetRegisteredActors().Contains((AActor*)InActor))
            RegistrationTags.Add(Pair.Key);
    }

    return RegistrationTags;
}

void UFortGlobalActorRegistrySubsystem::RegisterActors(const FGameplayTag& GameplayTag, const TArray<AActor*>& InActors)
{
    if (auto RegisterData = GetActorRegistryDataForGameplayTag(GameplayTag))
        RegisterData->RegisterActors(InActors);
}

void UFortGlobalActorRegistrySubsystem::UnregisterActors(const FGameplayTag& GameplayTag, const TArray<AActor*>& InActors)
{
    auto RegisterData = GetActorRegistrationDataByGameplayTagMap().Find(GameplayTag);

    if (RegisterData && *RegisterData)
        (*RegisterData)->UnregisterActors(InActors);
}

void UFortGlobalActorRegistrySubsystem::RegisterActorsWithMultipleGameplayTags(const TArray<FGameplayTag>& GameplayTags, const TArray<AActor*>& InActors)
{
    for (auto& GameplayTag : GameplayTags)
        RegisterActors(GameplayTag, InActors);
}

void UFortGlobalActorRegistrySubsystem::UnregisterActorsWithMultipleGameplayTags(const TArray<FGameplayTag>& GameplayTags, const TArray<AActor*>& InActors)
{
    for (auto& GameplayTag : GameplayTags)
        UnregisterActors(GameplayTag, InActors);
}

void UFortGlobalActorRegistrySubsystem::UnregisterActorsFromAllGameplayTags(const TArray<AActor*>& InActors)
{
    for (auto& Pair : GetActorRegistrationDataByGameplayTagMap())
    {
        if (Pair.Value)
            Pair.Value->UnregisterActors(InActors);
    }
}

void UFortGlobalActorRegistrySubsystem::execGetActorRegistryDataForGameplayTag(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execGetActorRegistryDataForGameplayTag, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:GetActorRegistryDataForGameplayTag")));

    FGameplayTag TemporaryGameplayTag;
    auto& GameplayTag = Stack.StepCompiledInRef<FGameplayTag>(&TemporaryGameplayTag);
    Stack.Code += !!Stack.Code;

    *(UActorRegisterData_GameplayTagBased**)Z_Param__Result = GetActorRegistryDataForGameplayTag(GameplayTag);
}

void UFortGlobalActorRegistrySubsystem::execGetRegisteredActors(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execGetRegisteredActors, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:GetRegisteredActors")));

    TArray<AActor*> TemporaryOutActors;
    auto& OutActors = Stack.StepCompiledInRef<TArray<AActor*>>(&TemporaryOutActors);
    FGameplayTag TemporaryGameplayTag;
    auto& GameplayTag = Stack.StepCompiledInRef<FGameplayTag>(&TemporaryGameplayTag);
    Stack.Code += !!Stack.Code;

    GetRegisteredActors(OutActors, GameplayTag);
}

void UFortGlobalActorRegistrySubsystem::execGetRegistrationTagsForActor(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execGetRegistrationTagsForActor, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:GetRegistrationTagsForActor")));

    AActor* InActor = nullptr;

    Stack.StepCompiledIn(&InActor);
    Stack.Code += !!Stack.Code;

    *(TArray<FGameplayTag>*)Z_Param__Result = GetRegistrationTagsForActor(InActor);
}

void UFortGlobalActorRegistrySubsystem::execRegisterActor(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execRegisterActor, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor")));

    FGameplayTag TemporaryGameplayTag;
    auto& GameplayTag = Stack.StepCompiledInRef<FGameplayTag>(&TemporaryGameplayTag);
    AActor* InActor = nullptr;

    Stack.StepCompiledIn(&InActor);
    Stack.Code += !!Stack.Code;

    TArray<AActor*> InActors;
    InActors.Add(InActor);

    RegisterActors(GameplayTag, InActors);
}

void UFortGlobalActorRegistrySubsystem::execRegisterActors(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execRegisterActors, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActors")));

    FGameplayTag TemporaryGameplayTag;
    auto& GameplayTag = Stack.StepCompiledInRef<FGameplayTag>(&TemporaryGameplayTag);
    TArray<AActor*> TemporaryInActors;
    auto& InActors = Stack.StepCompiledInRef<TArray<AActor*>>(&TemporaryInActors);
    Stack.Code += !!Stack.Code;

    RegisterActors(GameplayTag, InActors);
}

void UFortGlobalActorRegistrySubsystem::execRegisterActorsWithMultipleGameplayTags(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execRegisterActorsWithMultipleGameplayTags, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActorsWithMultipleGameplayTags")));

    TArray<FGameplayTag> TemporaryGameplayTags;
    auto& GameplayTags = Stack.StepCompiledInRef<TArray<FGameplayTag>>(&TemporaryGameplayTags);
    TArray<AActor*> TemporaryInActors;
    auto& InActors = Stack.StepCompiledInRef<TArray<AActor*>>(&TemporaryInActors);
    Stack.Code += !!Stack.Code;

    RegisterActorsWithMultipleGameplayTags(GameplayTags, InActors);
}

void UFortGlobalActorRegistrySubsystem::execUnregisterActor(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execUnregisterActor, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:UnregisterActor")));

    FGameplayTag TemporaryGameplayTag;
    auto& GameplayTag = Stack.StepCompiledInRef<FGameplayTag>(&TemporaryGameplayTag);
    AActor* InActor = nullptr;

    Stack.StepCompiledIn(&InActor);
    Stack.Code += !!Stack.Code;

    TArray<AActor*> InActors;
    InActors.Add(InActor);

    UnregisterActors(GameplayTag, InActors);
}

void UFortGlobalActorRegistrySubsystem::execUnregisterActors(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execUnregisterActors, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:UnregisterActors")));

    FGameplayTag TemporaryGameplayTag;
    auto& GameplayTag = Stack.StepCompiledInRef<FGameplayTag>(&TemporaryGameplayTag);
    TArray<AActor*> TemporaryInActors;
    auto& InActors = Stack.StepCompiledInRef<TArray<AActor*>>(&TemporaryInActors);
    Stack.Code += !!Stack.Code;

    UnregisterActors(GameplayTag, InActors);
}

void UFortGlobalActorRegistrySubsystem::execUnregisterActorsFromAllGameplayTags(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execUnregisterActorsFromAllGameplayTags, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:UnregisterActorsFromAllGameplayTags")));

    TArray<AActor*> TemporaryInActors;
    auto& InActors = Stack.StepCompiledInRef<TArray<AActor*>>(&TemporaryInActors);
    Stack.Code += !!Stack.Code;

    UnregisterActorsFromAllGameplayTags(InActors);
}

void UFortGlobalActorRegistrySubsystem::execUnregisterActorsWithMultipleGameplayTags(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortGlobalActorRegistrySubsystem, execUnregisterActorsWithMultipleGameplayTags, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:UnregisterActorsWithMultipleGameplayTags")));

    TArray<FGameplayTag> TemporaryGameplayTags;
    auto& GameplayTags = Stack.StepCompiledInRef<TArray<FGameplayTag>>(&TemporaryGameplayTags);
    TArray<AActor*> TemporaryInActors;
    auto& InActors = Stack.StepCompiledInRef<TArray<AActor*>>(&TemporaryInActors);
    Stack.Code += !!Stack.Code;

    UnregisterActorsWithMultipleGameplayTags(GameplayTags, InActors);
}

TMap<FGameplayTag, UActorRegisterData_GameplayTagBased*, UFortGlobalActorRegistrySubsystem::FGameplayTagKeyFuncs>& UFortGlobalActorRegistrySubsystem::GetActorRegistrationDataByGameplayTagMap()
{
    static auto ActorRegistrationDataByGameplayTagMap = GetClass()->FindPropertyByName(FName(TEXT("ActorRegistrationDataByGameplayTagMap")));

    return *ActorRegistrationDataByGameplayTagMap->ContainerPtrToValuePtr<TMap<FGameplayTag, UActorRegisterData_GameplayTagBased*, FGameplayTagKeyFuncs>>(this);
}

UClass* UFortAutoRegisterActorComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAutoRegisterActorComponent"));

    return Class;
}

void UFortAutoRegisterActorComponent::BeginPlay()
{
    DETOUR(UFortAutoRegisterActorComponent, BeginPlay, Anchor::Hook()
        .If(StaticClass())
        .StubExec(TEXT("/Script/FortniteGame.FortGlobalActorRegistrySubsystem:RegisterActor"))
        .VirtualMatch(TEXT("/Script/Engine.Default__ActorComponent"), "mov [rsp+0x8], rbx; push rdi; sub rsp, 0x20; mov rax, [rcx+0x10]; mov rdi, rcx; mov ecx, [rax+0x*]; bt ecx, 0x12")
        .LogSlot(TEXT("UActorComponent::BeginPlay"))
        .InVTable(TEXT("/Script/FortniteGame.Default__FortAutoRegisterActorComponent")));

    Original();

    auto Owner = GetOwner();
    auto ActorRegistrySubsystem = Owner && Owner->HasAuthority() ? UFortGlobalActorRegistrySubsystem::GetGlobalActorRegistrySubsystem(Owner) : nullptr;

    if (!ActorRegistrySubsystem)
        return;

    TArray<AActor*> InActors;
    InActors.Add(Owner);

    ActorRegistrySubsystem->RegisterActorsWithMultipleGameplayTags(GetActorRegistrationTags().GetGameplayTagArray(), InActors);
}

FGameplayTagContainer& UFortAutoRegisterActorComponent::GetActorRegistrationTags()
{
    static auto ActorRegistrationTags = GetClass()->FindPropertyByName(FName(TEXT("ActorRegistrationTags")));

    return *ActorRegistrationTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}
