#pragma once

#include "UObject/Object.h"

class AActor;
struct FMcpQueryResult;

class UFortRegisteredPlayerInfo : public UObject
{
public:
    void AthenaProfileQuery_Completed(const FMcpQueryResult& Result, int32 RequestId);

    AActor* UpdateAbilitySystemActor(bool bForceUpdate);
};
