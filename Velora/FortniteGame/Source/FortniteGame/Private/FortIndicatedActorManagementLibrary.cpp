#include "pch.h"
#include "FortIndicatedActorManagementLibrary.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Components/FortControllerComponent_IndicatedActorManagement.h"
#include "FortKismetLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

FIndicatedActorDataWithFilter::~FIndicatedActorDataWithFilter()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FIndicatedActorDataWithFilter::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.IndicatedActorDataWithFilter"));

    return Struct;
}

int32 FIndicatedActorDataWithFilter::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

const FGameplayTagContainer& FIndicatedActorDataWithFilter::GetIndicatedActorTags() const
{
    static auto IndicatedActorTags = StaticStruct()->FindPropertyByName(FName(TEXT("IndicatedActorTags")));

    return *IndicatedActorTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this);
}

const FGameplayTagQuery* FIndicatedActorDataWithFilter::GetIndicatedActorTagQuery() const
{
    static auto IndicatedActorTagQuery = StaticStruct()->FindPropertyByName(FName(TEXT("IndicatedActorTagQuery")));

    return IndicatedActorTagQuery ? IndicatedActorTagQuery->ContainerPtrToValuePtr<FGameplayTagQuery>(this) : nullptr;
}

const TArray<TEnumAsByte<EObjectTypeQuery>>& FIndicatedActorDataWithFilter::GetObjectTypes() const
{
    static auto ObjectTypes = StaticStruct()->FindPropertyByName(FName(TEXT("ObjectTypes")));

    return *ObjectTypes->ContainerPtrToValuePtr<TArray<TEnumAsByte<EObjectTypeQuery>>>(this);
}

UClass* FIndicatedActorDataWithFilter::GetActorClassFilter() const
{
    static auto ActorClassFilter = StaticStruct()->FindPropertyByName(FName(TEXT("ActorClassFilter")));

    if (ActorClassFilter->GetSize() == sizeof(UClass*))
        return *ActorClassFilter->ContainerPtrToValuePtr<UClass*>(this);

    return ActorClassFilter->ContainerPtrToValuePtr<TSoftClassPtr<UObject>>(this)->LoadSynchronous();
}

const TArray<EFortTeamAffiliation>* FIndicatedActorDataWithFilter::GetWithAffiliation() const
{
    static auto WithAffiliation = StaticStruct()->FindPropertyByName(FName(TEXT("WithAffiliation")));

    return WithAffiliation ? WithAffiliation->ContainerPtrToValuePtr<TArray<EFortTeamAffiliation>>(this) : nullptr;
}

const FIndicatedActorData& FIndicatedActorDataWithFilter::GetIndicatedData() const
{
    static auto IndicatedData = []
    {
        auto IndicatedData = StaticStruct()->FindPropertyByName(FName(TEXT("IndicatedData")));

        return IndicatedData ? IndicatedData : StaticStruct()->FindPropertyByName(FName(TEXT("IndicatedActorData")));
    }();

    return *IndicatedData->ContainerPtrToValuePtr<FIndicatedActorData>(this);
}

float FIndicatedActorDataWithFilter::GetOverlapRadius() const
{
    static auto OverlapRadius = StaticStruct()->FindPropertyByName(FName(TEXT("OverlapRadius")));

    return *OverlapRadius->ContainerPtrToValuePtr<float>(this);
}

void UFortIndicatedActorManagementLibrary::AddActorsInRadiusToIndicatedList(AFortPlayerControllerAthena* InstigatingController, const TArray<FIndicatedActorDataWithFilter>& IndicatedActorFilterDatas, const bool bAddAsUnique, AActor* InstigatingActorOverride)
{
    static auto GameplayTagAssetInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayTags.GameplayTagAssetInterface"));

    auto Component = InstigatingController ? InstigatingController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr;
    auto Pawn = InstigatingActorOverride ? InstigatingActorOverride : InstigatingController ? (AActor*)InstigatingController->GetPawn() : nullptr;

    if (!Component || !Pawn)
        return;

    for (auto& IndicatedActorFilterData : IndicatedActorFilterDatas)
    {
        auto& IndicatedActorTags = IndicatedActorFilterData.GetIndicatedActorTags().GetGameplayTagArray();
        TArray<AActor*> OverlappedActors;
        TArray<AActor*> IndicatedActors;

        UKismetSystemLibrary::SphereOverlapActors(InstigatingController, Pawn->GetActorLocation(), IndicatedActorFilterData.GetOverlapRadius(), IndicatedActorFilterData.GetObjectTypes(), IndicatedActorFilterData.GetActorClassFilter(), TArray<AActor*>(), OverlappedActors);

        for (auto Actor : OverlappedActors)
        {
            auto TagAssetInterface = (IGameplayTagAssetInterface*)Actor->GetInterfaceAddress(GameplayTagAssetInterface);
            auto bMatchesTags = IndicatedActorTags.Num() == 0;
            FGameplayTagContainer OwnedTags;

            if (TagAssetInterface)
                TagAssetInterface->GetOwnedGameplayTags(OwnedTags);

            for (auto& Tag : IndicatedActorTags)
                bMatchesTags |= OwnedTags.HasTag(Tag);

            if (auto IndicatedActorTagQuery = IndicatedActorFilterData.GetIndicatedActorTagQuery(); bMatchesTags && IndicatedActorTagQuery && !IndicatedActorTagQuery->IsEmpty())
                bMatchesTags = IndicatedActorTagQuery->Matches(OwnedTags);

            if (auto WithAffiliation = IndicatedActorFilterData.GetWithAffiliation(); bMatchesTags && WithAffiliation && WithAffiliation->Num() > 0)
                bMatchesTags = WithAffiliation->Contains(UFortKismetLibrary::GetTeamAffiliation(Pawn, Actor));

            if (bMatchesTags)
                IndicatedActors.Add(Actor);
        }

        Component->AddActorsToIndicatedList(IndicatedActors, IndicatedActorFilterData.GetIndicatedData(), bAddAsUnique);
    }
}

void UFortIndicatedActorManagementLibrary::execAddActorsInRadiusToIndicatedList(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortIndicatedActorManagementLibrary, execAddActorsInRadiusToIndicatedList, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsInRadiusToIndicatedList"))
        .Find("movzx r8d, *; call 0x*")
        .Call()
        .Is("mov rcx, rdx; jmp 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsInRadiusToIndicatedList"))
        .Find("mov rcx, [rsp+*]; lea rdx, [rsp+*]; call 0x*")
        .Call()
        .Is("mov [rsp+0x8], rbx; mov [rsp+0x10], rsi; push rdi; sub rsp, 0x20; mov edi, [rdx+0x8]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsInRadiusToIndicatedList"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsInRadiusToIndicatedList")));

    static auto Function = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsInRadiusToIndicatedList"));
    static auto bTakesAddAsUnique = Function->FindPropertyByName(FName(TEXT("bAddAsUnique"))) != nullptr;
    static auto bTakesReplaceExistingEntry = Function->FindPropertyByName(FName(TEXT("bReplaceExistingEntry"))) != nullptr;
    static auto bTakesRefreshExistingEntry = Function->FindPropertyByName(FName(TEXT("bRefreshExistingEntry"))) != nullptr;
    static auto bTakesInstigatingActorOverride = Function->FindPropertyByName(FName(TEXT("InstigatingActorOverride"))) != nullptr;

    AFortPlayerControllerAthena* InstigatingController = nullptr;
    TArray<FIndicatedActorDataWithFilter> IndicatedActorFilterDatas;
    uint32 bAddAsUnique = 0;
    uint32 bReplaceExistingEntry = 0;
    uint32 bRefreshExistingEntry = 0;
    AActor* InstigatingActorOverride = nullptr;

    Stack.StepCompiledIn(&InstigatingController);
    Stack.StepCompiledIn(&IndicatedActorFilterDatas);

    if (bTakesAddAsUnique)
        Stack.StepCompiledIn(&bAddAsUnique);

    if (bTakesReplaceExistingEntry)
        Stack.StepCompiledIn(&bReplaceExistingEntry);

    if (bTakesRefreshExistingEntry)
        Stack.StepCompiledIn(&bRefreshExistingEntry);

    if (bTakesInstigatingActorOverride)
        Stack.StepCompiledIn(&InstigatingActorOverride);

    Stack.Code += !!Stack.Code;

    AddActorsInRadiusToIndicatedList(InstigatingController, IndicatedActorFilterDatas, !!bAddAsUnique, InstigatingActorOverride);
}

void UFortIndicatedActorManagementLibrary::AddActorsToIndicatedList(AFortPlayerControllerAthena* InstigatingController, TArray<AActor*> IndicatedActors, FIndicatedActorData IndicatedActorData, const bool bAddAsUnique)
{
    DETOUR(UFortIndicatedActorManagementLibrary, AddActorsToIndicatedList, Anchor::Hook()
        .If(Anchor::Address()
            .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"))
            .FindEither("movzx r9d, *; lea rdx, [*]; call 0x*", "lea rdx, [*]; mov r8, rax; call 0x*")
            .Call()
            .Is("push rbx; sub rsp, 0x20; mov rcx, [rdx]")
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"))
            .Find("movzx r9d, *; mov [rsp+0x20], *; lea rdx, [*]; call 0x*")
            .Call()
            .Is("push rbx; sub rsp, 0x20; mov rcx, [rdx]")
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"))
            .Find("movzx r9d, *; mov [rsp+0x20], *; call 0x*")
            .Call()
            .Is("push rbx; sub rsp, 0x20; mov rcx, [rdx]")
            .Get())
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"))
        .FindEither("movzx r9d, *; lea rdx, [*]; call 0x*", "lea rdx, [*]; mov r8, rax; call 0x*")
        .Find("call 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"))
        .Find("movzx r9d, *; mov [rsp+0x20], *; lea rdx, [*]; call 0x*")
        .Find("call 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"))
        .Find("movzx r9d, *; mov [rsp+0x20], *; call 0x*")
        .Find("call 0x*")
        .AtCall());

    static auto bTakesAddAsUnique = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList")))->FindPropertyByName(FName(TEXT("bAddAsUnique"))) != nullptr;

    if (auto Component = InstigatingController ? InstigatingController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr)
        Component->AddActorsToIndicatedList(IndicatedActors, IndicatedActorData, bTakesAddAsUnique && bAddAsUnique);
}

void UFortIndicatedActorManagementLibrary::AddActorsToStenciledList(AFortPlayerControllerAthena* InstigatingController, TArray<AActor*> StenciledActors, FStenciledActorData StenciledActorData, const bool bAddAsUnique)
{
    DETOUR(UFortIndicatedActorManagementLibrary, AddActorsToStenciledList, Anchor::Hook()
        .If(Anchor::Address()
            .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToStenciledList"))
            .Find("movzx r9d, *; lea rdx, [*]; call 0x*")
            .Call()
            .Is("push rbx; sub rsp, 0x20; mov rcx, [rdx]")
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToStenciledList"))
            .Find("movzx r9d, *; mov [rsp+0x20], *; lea rdx, [*]; call 0x*")
            .Call()
            .Is("push rbx; sub rsp, 0x20; mov rcx, [rdx]")
            .Get())
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToStenciledList"))
        .Find("movzx r9d, *; lea rdx, [*]; call 0x*")
        .Find("call 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToStenciledList"))
        .Find("movzx r9d, *; mov [rsp+0x20], *; lea rdx, [*]; call 0x*")
        .Find("call 0x*")
        .AtCall());

    if (auto Component = InstigatingController ? InstigatingController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr)
        Component->AddActorsToStenciledList(StenciledActors, StenciledActorData, bAddAsUnique);
}

void UFortIndicatedActorManagementLibrary::execAddActorsToIndicatedList(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortIndicatedActorManagementLibrary, execAddActorsToIndicatedList, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(UFortIndicatedActorManagementLibrary, AddActorsToIndicatedList))
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList")));

    static auto Function = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToIndicatedList"));
    static auto bTakesAddAsUnique = Function->FindPropertyByName(FName(TEXT("bAddAsUnique"))) != nullptr;
    static auto bTakesAllowOwningPlayer = Function->FindPropertyByName(FName(TEXT("bAllowOwningPlayer"))) != nullptr;
    static auto bTakesReplaceExistingEntry = Function->FindPropertyByName(FName(TEXT("bReplaceExistingEntry"))) != nullptr;
    static auto bTakesRefreshExistingEntry = Function->FindPropertyByName(FName(TEXT("bRefreshExistingEntry"))) != nullptr;

    AFortPlayerControllerAthena* InstigatingController = nullptr;
    TArray<AActor*> IndicatedActors;
    TArray<uint8> IndicatedActorData;
    uint32 bAddAsUnique = 0;
    uint32 bAllowOwningPlayer = 0;
    uint32 bReplaceExistingEntry = 0;
    uint32 bRefreshExistingEntry = 0;

    IndicatedActorData.AddUninitialized(FIndicatedActorData::StaticStruct()->GetPropertiesSize());
    FIndicatedActorData::StaticStruct()->InitializeStruct(IndicatedActorData.GetData());

    Stack.StepCompiledIn(&InstigatingController);
    Stack.StepCompiledIn(&IndicatedActors);
    Stack.StepCompiledIn(IndicatedActorData.GetData());

    if (bTakesAddAsUnique)
        Stack.StepCompiledIn(&bAddAsUnique);

    if (bTakesAllowOwningPlayer)
        Stack.StepCompiledIn(&bAllowOwningPlayer);

    if (bTakesReplaceExistingEntry)
        Stack.StepCompiledIn(&bReplaceExistingEntry);

    if (bTakesRefreshExistingEntry)
        Stack.StepCompiledIn(&bRefreshExistingEntry);

    Stack.Code += !!Stack.Code;

    if (auto Component = InstigatingController ? InstigatingController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr)
        Component->AddActorsToIndicatedList(IndicatedActors, *(const FIndicatedActorData*)IndicatedActorData.GetData(), !!bAddAsUnique);

    FIndicatedActorData::StaticStruct()->DestroyStruct(IndicatedActorData.GetData());
}

void UFortIndicatedActorManagementLibrary::execAddActorsToStenciledList(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(UFortIndicatedActorManagementLibrary, execAddActorsToStenciledList, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(UFortIndicatedActorManagementLibrary, AddActorsToStenciledList))
        .Exec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToStenciledList"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToStenciledList")));

    static auto Function = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortIndicatedActorManagementLibrary:AddActorsToStenciledList"));
    static auto bTakesAddAsUnique = Function->FindPropertyByName(FName(TEXT("bAddAsUnique"))) != nullptr;
    static auto bTakesReplaceExistingEntry = Function->FindPropertyByName(FName(TEXT("bReplaceExistingEntry"))) != nullptr;
    static auto bTakesRefreshExistingEntry = Function->FindPropertyByName(FName(TEXT("bRefreshExistingEntry"))) != nullptr;

    AFortPlayerControllerAthena* InstigatingController = nullptr;
    TArray<AActor*> StenciledActors;
    TArray<uint8> StenciledActorData;
    uint32 bAddAsUnique = 0;
    uint32 bReplaceExistingEntry = 0;
    uint32 bRefreshExistingEntry = 0;

    StenciledActorData.AddUninitialized(FStenciledActorData::StaticStruct()->GetPropertiesSize());
    FStenciledActorData::StaticStruct()->InitializeStruct(StenciledActorData.GetData());

    Stack.StepCompiledIn(&InstigatingController);
    Stack.StepCompiledIn(&StenciledActors);
    Stack.StepCompiledIn(StenciledActorData.GetData());

    if (bTakesAddAsUnique)
        Stack.StepCompiledIn(&bAddAsUnique);

    if (bTakesReplaceExistingEntry)
        Stack.StepCompiledIn(&bReplaceExistingEntry);

    if (bTakesRefreshExistingEntry)
        Stack.StepCompiledIn(&bRefreshExistingEntry);

    Stack.Code += !!Stack.Code;

    if (auto Component = InstigatingController ? InstigatingController->FindComponentByClass<UFortControllerComponent_IndicatedActorManagement>() : nullptr)
        Component->AddActorsToStenciledList(StenciledActors, *(const FStenciledActorData*)StenciledActorData.GetData(), !!bAddAsUnique);

    FStenciledActorData::StaticStruct()->DestroyStruct(StenciledActorData.GetData());
}
