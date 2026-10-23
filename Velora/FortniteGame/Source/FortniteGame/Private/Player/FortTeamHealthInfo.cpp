#include "pch.h"
#include "Player/FortTeamHealthInfo.h"
#include "UObject/UnrealType.h"

uint8 AFortTeamHealthInfo::GetTeamNum() const
{
    static auto TeamNum = GetClass()->FindPropertyByName(FName(TEXT("TeamNum")));

    return *TeamNum->ContainerPtrToValuePtr<uint8>(this);
}
