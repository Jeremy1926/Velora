#include "pch.h"
#include "Athena/FortSafeZoneIndicator.h"
#include "Athena/FortAthenaMapInfo.h"
#include "Athena/FortGameModeAthena.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/GameStateComponents/FortGameStateComponent_BattleRoyaleGamePhaseLogic.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayEffectTypes.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"
#include "Templates/UnrealTemplate.h"
#include "UObject/Class.h"
#include "UObject/ScriptDelegates.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FFortSafeZonePhaseInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZonePhaseInfo"));

    return Struct;
}

int32 FFortSafeZonePhaseInfo::GetTypeSize()
{
    static auto TypeSize = StaticStruct()->GetPropertiesSize();

    return TypeSize;
}

int32 FFortSafeZonePhaseInfo::GetStormDamageLevel() const
{
    static auto StormDamageLevel = StaticStruct()->FindPropertyByName(FName(TEXT("StormDamageLevel")));

    return StormDamageLevel ? *StormDamageLevel->ContainerPtrToValuePtr<int32>(this) : -1;
}

void FFortSafeZonePhaseInfo::SetStormDamageLevel(int32 InStormDamageLevel)
{
    static auto StormDamageLevel = StaticStruct()->FindPropertyByName(FName(TEXT("StormDamageLevel")));

    if (StormDamageLevel)
        *StormDamageLevel->ContainerPtrToValuePtr<int32>(this) = InStormDamageLevel;
}

void FFortSafeZonePhaseInfo::SetDamageInfo(float InDamage, bool bInPercentageBasedDamage)
{
    static auto DamageInfo = StaticStruct()->FindPropertyByName(FName(TEXT("DamageInfo")));
    static auto DamageInfoStruct = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneDamageInfo"));
    static auto Damage = DamageInfoStruct ? DamageInfoStruct->FindPropertyByName(FName(TEXT("Damage"))) : nullptr;
    static auto bPercentageBasedDamage = DamageInfoStruct ? (UBoolProperty*)DamageInfoStruct->FindPropertyByName(FName(TEXT("bPercentageBasedDamage"))) : nullptr;

    if (!DamageInfo || !Damage || !bPercentageBasedDamage)
        return;

    auto Info = DamageInfo->ContainerPtrToValuePtr<void>(this);

    *Damage->ContainerPtrToValuePtr<float>(Info) = InDamage;
    bPercentageBasedDamage->SetPropertyValue_InContainer(Info, bInPercentageBasedDamage);
}

int32& FFortSafeZonePhaseInfo::GetMegaStormGridCellThickness()
{
    static auto MegaStormGridCellThickness = StaticStruct()->FindPropertyByName(FName(TEXT("MegaStormGridCellThickness")));

    return *MegaStormGridCellThickness->ContainerPtrToValuePtr<int32>(this);
}

UClass* AFortSafeZoneIndicator::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneIndicator"));

    return Class;
}

void AFortSafeZoneIndicator::InitializeWithMapDefaultPhases()
{
    static auto Count = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneDefinition")))->FindPropertyByName(FName(TEXT("Count")))->GetOffset_ForInternal();

    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;
    auto Definition = MapInfo ? MapInfo->GetSafeZoneDefinition() : nullptr;
    auto& SafeZonePhases = GetSafeZonePhases();

    SafeZonePhases.RemoveAt(0, SafeZonePhases.Num());

    if (!Definition)
        return;

    const float PhaseCount = ((const FScalableFloat*)(Definition + Count))->GetValueAtLevel(0.0f);

    for (int32 PhaseIndex = 0; (float)PhaseIndex < PhaseCount; PhaseIndex++)
    {
        if (!ConstructMapDefaultPhase(PhaseIndex, SafeZonePhases[SafeZonePhases.AddUninitialized()]))
        {
            SafeZonePhases.RemoveAt(PhaseIndex);
            break;
        }
    }

    SetPhaseCount(SafeZonePhases.Num());
}

void AFortSafeZoneIndicator::StartPhase(int32 InPhaseIndex)
{
    static auto PreviousCenter = GetClass()->FindPropertyByName(FName(TEXT("PreviousCenter")));
    static auto PreviousRadius = GetClass()->FindPropertyByName(FName(TEXT("PreviousRadius")));
    static auto NextRadius = GetClass()->FindPropertyByName(FName(TEXT("NextRadius")));
    static auto NextMegaStormGridCellThickness = GetClass()->FindPropertyByName(FName(TEXT("NextMegaStormGridCellThickness")));
    static auto bSafezoneEventDriven = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bSafezoneEventDriven")));
    static auto OnSafeZonePhaseChanged = GetClass()->FindPropertyByName(FName(TEXT("OnSafeZonePhaseChanged")));

    auto& SafeZonePhases = GetSafeZonePhases();

    if (InPhaseIndex < 0 || InPhaseIndex >= SafeZonePhases.Num())
        return;

    auto& SafeZonePhase = SafeZonePhases[InPhaseIndex];
    const FVector Center = (FVector)SafeZonePhase.Center;

    PreviousCenter->SetPropertyValue_InContainer<FVector>(this, InPhaseIndex ? GetNextCenter() : Center);
    *PreviousRadius->ContainerPtrToValuePtr<float>(this) = InPhaseIndex ? GetRadius() : SafeZonePhase.Radius;
    SetNextCenter(Center);
    *NextRadius->ContainerPtrToValuePtr<float>(this) = SafeZonePhase.Radius;
    *NextMegaStormGridCellThickness->ContainerPtrToValuePtr<int32>(this) = SafeZonePhase.GetMegaStormGridCellThickness();
    SetCurrentPhase(InPhaseIndex);
    SetCurrentDamageInfo(SafeZonePhase);
    bSafezoneEventDriven->SetPropertyValue_InContainer(this, SafeZonePhase.WaitTime < 0.0f);

    if (SafeZonePhase.WaitTime >= 0.0f)
    {
        OffsetShrinkTimesFromServerTime(SafeZonePhase.WaitTime);
        SetShrinkDuration(SafeZonePhase.ShrinkTime);
    }

    if (!InPhaseIndex)
        SetSafeZoneRadiusAndCenter(SafeZonePhase.Radius, Center);

    ForceNetUpdate();
    OnSafeZonePhaseChanged->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this)->ProcessMulticastDelegate<UObject>(nullptr);
}

bool AFortSafeZoneIndicator::ConstructMapDefaultPhase(int32 PhaseToConstruct, FFortSafeZonePhaseInfo& OutConstructedPhase) const
{
    static auto SafeZoneDefinition = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortSafeZoneDefinition"));
    static auto Radius = SafeZoneDefinition->FindPropertyByName(FName(TEXT("Radius")))->GetOffset_ForInternal();
    static auto WaitTime = SafeZoneDefinition->FindPropertyByName(FName(TEXT("WaitTime")))->GetOffset_ForInternal();
    static auto ShrinkTime = SafeZoneDefinition->FindPropertyByName(FName(TEXT("ShrinkTime")))->GetOffset_ForInternal();
    static auto MegaStormGridCellThickness = SafeZoneDefinition->FindPropertyByName(FName(TEXT("MegaStormGridCellThickness")))->GetOffset_ForInternal();
    static auto DamageInfo = FFortSafeZonePhaseInfo::StaticStruct()->FindPropertyByName(FName(TEXT("DamageInfo")));

    auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetAuthGameMode());
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    auto MapInfo = GameState ? GameState->GetMapInfo() : nullptr;
    auto Definition = MapInfo ? MapInfo->GetSafeZoneDefinition() : nullptr;

    if (!GameMode || !Definition || PhaseToConstruct < 0 || PhaseToConstruct >= GameMode->GetSafeZoneLocations().Num())
        return false;

    auto GetValueAtPhase = [&](int32 Offset)
    {
        return ((const FScalableFloat*)(Definition + Offset))->GetValueAtLevel((float)PhaseToConstruct);
    };

    memset(&OutConstructedPhase, 0, FFortSafeZonePhaseInfo::GetTypeSize());
    OutConstructedPhase.Center = FVector3d(TGameLayout<FVector>::Load((uint8*)GameMode->GetSafeZoneLocations().GetData() + PhaseToConstruct * TGameLayout<FVector>::Size()));
    OutConstructedPhase.Radius = PhaseToConstruct ? GetValueAtPhase(Radius) : FMath::Max(MapInfo->GetSafeZoneStartingRadius().GetValueAtLevel(0.0f), GetValueAtPhase(Radius));
    OutConstructedPhase.WaitTime = GetValueAtPhase(WaitTime);
    OutConstructedPhase.ShrinkTime = GetValueAtPhase(ShrinkTime);
    OutConstructedPhase.SetStormDamageLevel(PhaseToConstruct);

    if (auto SafeZoneDamageAttribute = DamageInfo ? GameState->GetSafeZoneDamageAttribute() : nullptr)
        OutConstructedPhase.SetDamageInfo(SafeZoneDamageAttribute->GetValueAtLevel((float)PhaseToConstruct), true);

    OutConstructedPhase.GetMegaStormGridCellThickness() = FMath::RoundToInt(GetValueAtPhase(MegaStormGridCellThickness));

    return true;
}

int32 AFortSafeZoneIndicator::GetPhaseDamageLevel() const
{
    DETOUR(AFortSafeZoneIndicator, GetPhaseDamageLevel, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortSafeZoneIndicator:GetPhaseDamageLevel"))
        .Optional()
        .Find("call 0x*")
        .Target()
        .Inline());

    auto& SafeZonePhases = GetSafeZonePhases();
    const int32 CurrentPhase = GetCurrentPhase();

    return CurrentPhase >= 0 && CurrentPhase < SafeZonePhases.Num() ? SafeZonePhases[CurrentPhase].GetStormDamageLevel() : -1;
}

void AFortSafeZoneIndicator::SetSafeZoneRadiusAndCenter(float InRadius, const FVector& InLocation)
{
    static auto SetSafeZoneRadiusAndCenter = Anchor::Function<void (*)(AFortSafeZoneIndicator*, float, const TGameValue<FVector>*)>(TEXT("AFortSafeZoneIndicator::SetSafeZoneRadiusAndCenter"))
        .Exec(TEXT("/Script/FortniteGame.FortSafeZoneIndicator:SetSafeZoneRadiusAndCenter"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortSafeZoneIndicator:SetSafeZoneRadiusAndCenter"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Get();

    const TGameValue<FVector> Location(InLocation);

    SetSafeZoneRadiusAndCenter(this, InRadius, &Location);
}

void AFortSafeZoneIndicator::OffsetShrinkTimesFromServerTime(float Offset)
{
    DETOUR(AFortSafeZoneIndicator, OffsetShrinkTimesFromServerTime, Anchor::Hook()
        .If(AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("call 0x*; mov rcx, [*]; mov *, [rcx+0x*]")
        .Target()
        .Log(TEXT("AFortGameModeAthena::SetSafeZoneIndicatorShrinkTimes"))
        .Find("mov byte ptr [*], 0x0; mov rcx, [*]; call 0x*")
        .Find("call 0x*")
        .IsTarget("ret*")
        .Optional()
        .AtCall());

    auto GameState = GetWorld()->GetGameState();

    if (!GameState)
        return;

    const float ShrinkDuration = GetShrinkDuration();

    GetSafeZoneStartShrinkTime() = GameState->GetServerWorldTimeSeconds() + Offset;
    GetSafeZoneFinishShrinkTime() = GetSafeZoneStartShrinkTime() + ShrinkDuration;
}

void AFortSafeZoneIndicator::SetShrinkDuration(float ShrinkDuration)
{
    DETOUR(AFortSafeZoneIndicator, SetShrinkDuration, Anchor::Hook()
        .If(AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("FortGameModeAthena: Starting MegaStorm on SafeZone[%d].  NumBuildingGridCellsThick=%d."))
        .Last("call 0x*; mov rcx, [*]; mov *, [rcx+0x*]")
        .Target()
        .Find("mov rcx, [*]; movaps xmm1, *; call 0x*")
        .Find("call 0x*")
        .IsTarget("ret*")
        .Optional()
        .AtCall());

    GetSafeZoneFinishShrinkTime() = GetSafeZoneStartShrinkTime() + ShrinkDuration;
}

float AFortSafeZoneIndicator::GetShrinkDuration()
{
    return FMath::Max(GetSafeZoneFinishShrinkTime() - GetSafeZoneStartShrinkTime(), 0.0f);
}

void AFortSafeZoneIndicator::OnSafeZoneTimerPauseChanged(bool bIsPaused)
{
    DETOUR(AFortSafeZoneIndicator, OnSafeZoneTimerPauseChanged, Anchor::Hook()
        .If(!UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass() && AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("PAUSESAFEZONE"))
        .Find("setz dl; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movzx edx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("PAUSESAFEZONE"))
        .Find("movzx edx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Optional()
        .AtStubBranches(), Anchor::Hook()
        .If(!UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass() && AFortSafeZoneIndicator::StaticClass() && AFortSafeZoneIndicator::StaticClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        .String(TEXT("AFortGameStateAthena::StartPlaylistTimer(%f) Set PlaylistSecondsRemainingCount to: %d"))
        .AtStubBranches());

    auto GameState = GetWorld()->GetGameState();

    if (!GameState || !GetClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused"))))
        return;

    auto& TimeRemainingWhenPhasePaused = GetTimeRemainingWhenPhasePaused();

    if (bIsPaused)
    {
        if (TimeRemainingWhenPhasePaused < 0.0f)
            TimeRemainingWhenPhasePaused = GetSafeZoneFinishShrinkTime() - GameState->GetServerWorldTimeSeconds();

        return;
    }

    if (TimeRemainingWhenPhasePaused >= 0.0f)
    {
        const float ShrinkDuration = GetShrinkDuration();

        GetSafeZoneFinishShrinkTime() = GameState->GetServerWorldTimeSeconds() + TimeRemainingWhenPhasePaused;
        GetSafeZoneStartShrinkTime() = GetSafeZoneFinishShrinkTime() - ShrinkDuration;
    }

    TimeRemainingWhenPhasePaused = -1.0f;
}

float& AFortSafeZoneIndicator::GetSafeZoneStartShrinkTime()
{
    static auto SafeZoneStartShrinkTime = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneStartShrinkTime")));

    return *SafeZoneStartShrinkTime->ContainerPtrToValuePtr<float>(this);
}

float& AFortSafeZoneIndicator::GetSafeZoneFinishShrinkTime()
{
    static auto SafeZoneFinishShrinkTime = GetClass()->FindPropertyByName(FName(TEXT("SafeZoneFinishShrinkTime")));

    return *SafeZoneFinishShrinkTime->ContainerPtrToValuePtr<float>(this);
}

float& AFortSafeZoneIndicator::GetTimeRemainingWhenPhasePaused()
{
    static auto TimeRemainingWhenPhasePaused = GetClass()->FindPropertyByName(FName(TEXT("TimeRemainingWhenPhasePaused")));

    return *TimeRemainingWhenPhasePaused->ContainerPtrToValuePtr<float>(this);
}

float& AFortSafeZoneIndicator::GetNextNextRadius()
{
    static auto NextNextRadius = GetClass()->FindPropertyByName(FName(TEXT("NextNextRadius")));

    return *NextNextRadius->ContainerPtrToValuePtr<float>(this);
}

FVector AFortSafeZoneIndicator::GetNextNextCenter()
{
    static auto NextNextCenter = GetClass()->FindPropertyByName(FName(TEXT("NextNextCenter")));

    return NextNextCenter->GetPropertyValue_InContainer<FVector>(this);
}

void AFortSafeZoneIndicator::SetNextNextCenter(const FVector& InNextNextCenter)
{
    static auto NextNextCenter = GetClass()->FindPropertyByName(FName(TEXT("NextNextCenter")));

    NextNextCenter->SetPropertyValue_InContainer<FVector>(this, InNextNextCenter);
}

void AFortSafeZoneIndicator::SetNextCenter(const FVector& InNextCenter)
{
    static auto NextCenter = GetClass()->FindPropertyByName(FName(TEXT("NextCenter")));

    if (NextCenter)
        NextCenter->SetPropertyValue_InContainer<FVector>(this, InNextCenter);
}

void AFortSafeZoneIndicator::SetRadius(float InRadius)
{
    static auto Radius = GetClass()->FindPropertyByName(FName(TEXT("Radius")));

    if (Radius)
        *Radius->ContainerPtrToValuePtr<float>(this) = InRadius;
}

float AFortSafeZoneIndicator::GetRadius()
{
    static auto Radius = GetClass()->FindPropertyByName(FName(TEXT("Radius")));

    return *Radius->ContainerPtrToValuePtr<float>(this);
}

FVector AFortSafeZoneIndicator::GetNextCenter()
{
    static auto NextCenter = GetClass()->FindPropertyByName(FName(TEXT("NextCenter")));

    return NextCenter->GetPropertyValue_InContainer<FVector>(this);
}

int32 AFortSafeZoneIndicator::GetCurrentPhase() const
{
    static auto CurrentPhase = GetClass()->FindPropertyByName(FName(TEXT("CurrentPhase")));

    return *CurrentPhase->ContainerPtrToValuePtr<int32>(this);
}

void AFortSafeZoneIndicator::SetCurrentPhase(int32 InCurrentPhase)
{
    static auto CurrentPhase = GetClass()->FindPropertyByName(FName(TEXT("CurrentPhase")));

    *CurrentPhase->ContainerPtrToValuePtr<int32>(this) = InCurrentPhase;
}

void AFortSafeZoneIndicator::SetPhaseCount(int32 InPhaseCount)
{
    static auto PhaseCount = GetClass()->FindPropertyByName(FName(TEXT("PhaseCount")));

    if (PhaseCount)
        *PhaseCount->ContainerPtrToValuePtr<int32>(this) = InPhaseCount;
}

void AFortSafeZoneIndicator::SetCurrentDamageInfo(const FFortSafeZonePhaseInfo& InPhase)
{
    static auto CurrentDamageInfo = GetClass()->FindPropertyByName(FName(TEXT("CurrentDamageInfo")));
    static auto DamageInfo = FFortSafeZonePhaseInfo::StaticStruct()->FindPropertyByName(FName(TEXT("DamageInfo")));

    if (CurrentDamageInfo && DamageInfo)
        memcpy(CurrentDamageInfo->ContainerPtrToValuePtr<void>(this), DamageInfo->ContainerPtrToValuePtr<void>(&InPhase), CurrentDamageInfo->GetSize());
}

TArray<FFortSafeZonePhaseInfo>& AFortSafeZoneIndicator::GetSafeZonePhases() const
{
    static auto SafeZonePhases = Anchor::Offset(TEXT("AFortSafeZoneIndicator::SafeZonePhases"))
        .Exec(TEXT("/Script/FortniteGame.FortSafeZoneIndicator:GetSafeZonePhases"))
        .Find("add rcx, 0x*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortSafeZoneIndicator:GetSafeZonePhases"))
        .Find("add [*+0x20], *")
        .Find("lea rdx, [rcx+0x*]")
        .Get();

    return *(TArray<FFortSafeZonePhaseInfo>*)((uint8*)this + SafeZonePhases);
}

AFortSafeZoneIndicatorFuture* AFortSafeZoneIndicator::GetFutureReplicator() const
{
    static auto FutureReplicator = GetClass()->FindPropertyByName(FName(TEXT("FutureReplicator")));

    return FutureReplicator ? *FutureReplicator->ContainerPtrToValuePtr<AFortSafeZoneIndicatorFuture*>(this) : nullptr;
}
