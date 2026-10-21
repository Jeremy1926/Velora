#include "pch.h"
#include "Athena/FortPlayerStateAthena.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

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
        .Get();

    return ToDeathCause(InTags, bWasDBNO);
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
