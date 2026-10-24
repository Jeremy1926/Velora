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

void FDeathInfo::SetDeathLocation(const FVector& InDeathLocation)
{
    static auto DeathLocation = StaticStruct()->FindPropertyByName(FName(TEXT("DeathLocation")));

    if (DeathLocation)
        DeathLocation->SetPropertyValue_InContainer<FVector>(this, InDeathLocation);
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
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:ToDeathCause"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
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

TMulticastDelegate<void, int32, ABuildingContainer*>& AFortPlayerStateAthena::GetNotifySquadSpecificContainerSpawned()
{
    static auto NotifySquadSpecificContainerSpawned = Anchor::Data<TMulticastDelegate<void, int32, ABuildingContainer*>*>(TEXT("AFortPlayerStateAthena::NotifySquadSpecificContainerSpawned"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:OnSquadSpecificContainerSpawned"))
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifySquadSpecificContainerSpawned;
}

TMulticastDelegate<void, int32, ABuildingContainer*>& AFortPlayerStateAthena::GetNotifySquadSpecificContainerDestroyed()
{
    static auto NotifySquadSpecificContainerDestroyed = Anchor::Data<TMulticastDelegate<void, int32, ABuildingContainer*>*>(TEXT("AFortPlayerStateAthena::NotifySquadSpecificContainerDestroyed"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:OnSquadSpecificContainerDestroyed"))
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifySquadSpecificContainerDestroyed;
}

TMulticastDelegate<void, AFortPlayerStateAthena*, ABuildingGameplayActorSpawnChip*>& AFortPlayerStateAthena::GetNotifyRebootCardSpawned()
{
    static auto NotifyRebootCardSpawned = Anchor::Data<TMulticastDelegate<void, AFortPlayerStateAthena*, ABuildingGameplayActorSpawnChip*>*>(TEXT("AFortPlayerStateAthena::NotifyRebootCardSpawned"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSpawnChip"))
        .Find("mov rcx, [*+0x*]; test rcx, rcx; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyRebootCardSpawned;
}

TMulticastDelegate<void, AFortPlayerStateAthena*, ABuildingGameplayActorSpawnChip*>& AFortPlayerStateAthena::GetNotifyRebootCardDestroyed()
{
    static auto NotifyRebootCardDestroyed = Anchor::Data<TMulticastDelegate<void, AFortPlayerStateAthena*, ABuildingGameplayActorSpawnChip*>*>(TEXT("AFortPlayerStateAthena::NotifyRebootCardDestroyed"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .SlotShift(TEXT("/Script/Engine.Default__Actor"), 0x8)
        .Entry(TEXT("/Script/FortniteGame.Default__BuildingGameplayActorSpawnChip"))
        .Find("mov rdx, [*+0x*]; test rdx, rdx; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyRebootCardDestroyed;
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

int32 AFortPlayerStateAthena::GetPlace() const
{
    static auto Place = GetClass()->FindPropertyByName(FName(TEXT("Place")));

    return *Place->ContainerPtrToValuePtr<int32>(this);
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

bool AFortPlayerStateAthena::IsTeamOutOfGame() const
{
    static auto IsTeamOutOfGame = Anchor::Function<bool (*)(const AFortPlayerStateAthena*)>(TEXT("AFortPlayerStateAthena::IsTeamOutOfGame"))
        .Pattern("48 89 5C 24 08 48 89 6C 24 18 56 57 41 55 41 56 41 57 48 83 EC 20 48 8B F9 E8 ? ? ? ? 4C 8B F0 48 85 C0 0F 84 ? ? ? ? 48 8B 98 ? ? ? ? 48 85 DB 74 ? E8 ? ? ? ? 48 8B 4B 10 48 8B D0 E8 ? ? ? ? 84 C0 75 02 33 DB 48 85 DB 0F 84 ? ? ? ? 48 8D 8F ? ? ? ? 48 8B 01 FF 50 28 8A D0 48 8B CB E8 ? ? ? ? 48 8B 38 48 63 40 08 4C 8D 3C C7 49 3B FF 0F 84")
        .Or()
        .Pattern("48 89 5C 24 08 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 48 8B F9 E8 ? ? ? ? 48 8B E8 48 85 C0 0F 84 ? ? ? ? 48 8B 98 ? ? ? ? 48 85 DB 74 ? E8 ? ? ? ? 48 8B 4B 10 48 8B D0 E8 ? ? ? ? 84 C0 75 02 33 DB 48 85 DB 0F 84 ? ? ? ? 48 8D 8F ? ? ? ? 48 8B 01 FF 50 28 8A D0 48 8B CB E8 ? ? ? ? 48 8B 38 48 63 40 08 4C 8D 34 C7 49 3B FE")
        .Get();

    return IsTeamOutOfGame(this);
}
