#include "pch.h"
#include "Athena/FortPlayerStateAthena.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

FDeathInfo::FDeathInfo()
{
    FMemory::Memzero(this, sizeof(FDeathInfo));

    StaticStruct()->InitializeStruct(this);
}

FDeathInfo::~FDeathInfo()
{
    StaticStruct()->DestroyStruct(this);
}

UScriptStruct* FDeathInfo::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.DeathInfo"));

    return Struct;
}

AActor*& FDeathInfo::GetFinisherOrDowner()
{
    static auto FinisherOrDowner = StaticStruct()->FindPropertyByName(FName(TEXT("FinisherOrDowner")));

    return *FinisherOrDowner->ContainerPtrToValuePtr<AActor*>(this);
}

AActor** FDeathInfo::GetDowner()
{
    static auto Downer = StaticStruct()->FindPropertyByName(FName(TEXT("Downer")));

    return Downer ? Downer->ContainerPtrToValuePtr<AActor*>(this) : nullptr;
}

void FDeathInfo::SetDBNO(bool bInDBNO)
{
    static auto bDBNO = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bDBNO")));

    bDBNO->SetPropertyValue_InContainer(this, bInDBNO);
}

EDeathCause& FDeathInfo::GetDeathCause()
{
    static auto DeathCause = StaticStruct()->FindPropertyByName(FName(TEXT("DeathCause")));

    return *DeathCause->ContainerPtrToValuePtr<EDeathCause>(this);
}

float* FDeathInfo::GetDistance()
{
    static auto Distance = StaticStruct()->FindPropertyByName(FName(TEXT("Distance")));

    return Distance ? Distance->ContainerPtrToValuePtr<float>(this) : nullptr;
}

FVector* FDeathInfo::GetDeathLocation()
{
    static auto DeathLocation = StaticStruct()->FindPropertyByName(FName(TEXT("DeathLocation")));

    return DeathLocation ? DeathLocation->ContainerPtrToValuePtr<FVector>(this) : nullptr;
}

void FDeathInfo::SetInitialized(bool bInInitialized)
{
    static auto bInitialized = (UBoolProperty*)StaticStruct()->FindPropertyByName(FName(TEXT("bInitialized")));

    if (bInitialized)
        bInitialized->SetPropertyValue_InContainer(this, bInInitialized);
}

int32* FDeathInfo::GetDeathCircumstance()
{
    static auto DeathCircumstance = StaticStruct()->FindPropertyByName(FName(TEXT("DeathCircumstance")));

    return DeathCircumstance ? DeathCircumstance->ContainerPtrToValuePtr<int32>(this) : nullptr;
}

FGameplayTagContainer* FDeathInfo::GetDeathTags()
{
    static auto DeathTags = StaticStruct()->FindPropertyByName(FName(TEXT("DeathTags")));

    return DeathTags ? DeathTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this) : nullptr;
}

FGameplayTagContainer* FDeathInfo::GetFinisherOrDownerTags()
{
    static auto FinisherOrDownerTags = StaticStruct()->FindPropertyByName(FName(TEXT("FinisherOrDownerTags")));

    return FinisherOrDownerTags ? FinisherOrDownerTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this) : nullptr;
}

FGameplayTagContainer* FDeathInfo::GetVictimTags()
{
    static auto VictimTags = StaticStruct()->FindPropertyByName(FName(TEXT("VictimTags")));

    return VictimTags ? VictimTags->ContainerPtrToValuePtr<FGameplayTagContainer>(this) : nullptr;
}

UClass* AFortPlayerStateAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerStateAthena"));

    return Class;
}

EDeathCause AFortPlayerStateAthena::ToDeathCause(const FGameplayTagContainer& InTags, bool bWasDBNO)
{
    static auto ToDeathCause = Anchor::Function<EDeathCause (*)(const FGameplayTagContainer&, bool)>(TEXT("AFortPlayerStateAthena::ToDeathCause"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:ToDeathCause"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Call()
        .Or()
        .Event(TEXT("ClientReportDBNO"))
        .CallSite()
        .Start()
        .Find("cmov* rsi, r*")
        .Find("call 0x*")
        .LastSince("xor edx, edx", "cmov* rsi, r*")
        .Call()
        .Get();

    return ToDeathCause(InTags, bWasDBNO);
}

int32 AFortPlayerStateAthena::ToDeathCircumstance(AFortPlayerPawnAthena* Pawn)
{
    static auto ToDeathCircumstance = Anchor::Function<int32 (*)(AFortPlayerPawnAthena*)>(TEXT("AFortPlayerStateAthena::ToDeathCircumstance"))
        .Event(TEXT("ClientReportDBNO"))
        .CallSite()
        .Start()
        .Find("call 0x*; mov rcx, r*; mov [r*], al; call 0x*; mov [r*], eax")
        .Find("call 0x*")
        .Target()
        .Get();

    return ToDeathCircumstance(Pawn);
}

FDeathInfo& AFortPlayerStateAthena::GetDeathInfo()
{
    static auto DeathInfo = GetClass()->FindPropertyByName(FName(TEXT("DeathInfo")));

    return *DeathInfo->ContainerPtrToValuePtr<FDeathInfo>(this);
}

void AFortPlayerStateAthena::InitializeDeathInfo(const FDeathInfo& NewDeathInfo)
{
    static auto DeathInfo = GetClass()->FindPropertyByName(FName(TEXT("DeathInfo")));

    static auto InitializeDeathInfo = Anchor::Optional<void (*)(AFortPlayerStateAthena*, const FDeathInfo&)>(TEXT("AFortPlayerStateAthena::InitializeDeathInfo"))
        .Event(TEXT("ClientReportDBNO"))
        .CallSite()
        .Find("lea rdx, [r*]; mov rcx, r*; call 0x*")
        .Call()
        .FieldHas("*+%x]*", DeathInfo)
        .Or()
        .Event(TEXT("ClientReportDBNO"))
        .CallSite()
        .Find("lea rdx, [r*]; mov rcx, r*; call 0x*")
        .Call()
        .FieldHas("add rcx, %x", DeathInfo)
        .Get();

    if (InitializeDeathInfo)
        return InitializeDeathInfo(this, NewDeathInfo);

    FDeathInfo::StaticStruct()->CopyScriptStruct(&GetDeathInfo(), &NewDeathInfo);

    GetDeathInfo().SetInitialized(true);

    ForceNetUpdate();
}

void AFortPlayerStateAthena::ClientReportKill(AFortPlayerStateAthena* Player)
{
    static auto ClientReportKill = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerStateAthena:ClientReportKill"));

    if (ClientReportKill->GetPropertiesSize() != sizeof(Player))
        return;

    struct
    {
        AFortPlayerStateAthena* Player;
    } Parms{ Player };

    CallRemoteFunction(ClientReportKill, &Parms, nullptr, nullptr);
}

void AFortPlayerStateAthena::ClientReportTeamKill(int32 TeamKills)
{
    static auto ClientReportTeamKill = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerStateAthena:ClientReportTeamKill"));

    if (!ClientReportTeamKill)
        return;

    struct
    {
        int32 TeamKills;
    } Parms{ TeamKills };

    CallRemoteFunction(ClientReportTeamKill, &Parms, nullptr, nullptr);
}

int32& AFortPlayerStateAthena::GetKillScore()
{
    static auto KillScore = GetClass()->FindPropertyByName(FName(TEXT("KillScore")));

    return *KillScore->ContainerPtrToValuePtr<int32>(this);
}

int32& AFortPlayerStateAthena::GetTeamKillScore()
{
    static auto TeamKillScore = GetClass()->FindPropertyByName(FName(TEXT("TeamKillScore")));

    return *TeamKillScore->ContainerPtrToValuePtr<int32>(this);
}

uint8 AFortPlayerStateAthena::GetSquadId() const
{
    static auto SquadId = GetClass()->FindPropertyByName(FName(TEXT("SquadId")));

    return *SquadId->ContainerPtrToValuePtr<uint8>(this);
}

uint8 AFortPlayerStateAthena::GetTeamIndex() const
{
    static auto TeamIndex = GetClass()->FindPropertyByName(FName(TEXT("TeamIndex")));

    return *TeamIndex->ContainerPtrToValuePtr<uint8>(this);
}

void AFortPlayerStateAthena::PlayerPawnRebooted()
{
    if (HasAuthority())
        GetRebootCounter()++;
}

int32& AFortPlayerStateAthena::GetRebootCounter()
{
    static auto RebootCounter = GetClass()->FindPropertyByName(FName(TEXT("RebootCounter")));

    return *RebootCounter->ContainerPtrToValuePtr<int32>(this);
}
