#pragma once

#include "AI/NavigationSystemBase.h"
#include "Containers/Array.h"

class ANavigationData;

class UNavigationSystemV1 : public UNavigationSystemBase
{
public:
    void ProcessRegistrationCandidates();

    void SpawnMissingNavigationData();

    TArray<ANavigationData*>& GetNavDataSet();

    TArray<ANavigationData*>& GetNavDataRegistrationQueue();
};
