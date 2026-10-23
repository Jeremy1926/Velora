#pragma once

#include "UObject/Object.h"
#include "Delegates/Delegate.h"

class AActor;
class UNetDriver;
class UReplicationDriver;
class UWorld;
struct FURL;

DECLARE_DELEGATE_RetVal_ThreeParams(UReplicationDriver*, FCreateReplicationDriver, UNetDriver*, const FURL&, UWorld*);

class UReplicationDriver : public UObject
{
public:
    static UReplicationDriver* CreateReplicationDriver(UNetDriver* NetDriver, const FURL& URL, UWorld* World);

    static FCreateReplicationDriver& CreateReplicationDriverDelegate();

    void FlushNetDormancy(AActor* Actor, bool bWasDormInitial);

    int32 ServerReplicateActors(float DeltaSeconds);
};

class UReplicationConnectionDriver : public UObject
{
};

void BindCreateReplicationDriverDelegate();
