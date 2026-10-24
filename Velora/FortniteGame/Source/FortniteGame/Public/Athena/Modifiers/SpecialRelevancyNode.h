#pragma once

#include "ReplicationGraph.h"
#include "Athena/Modifiers/FortAthenaMutator.h"
#include "Delegates/Delegate.h"
#include "Templates/SubclassOf.h"

class UClass;

class UFortReplicationGraphNode_SpecialRelevancy : public UReplicationGraphNode_GridSpatialization2D
{
public:
    static UClass* StaticClass();

    TMulticastDelegate<void, bool>& GetOnSpecialRelevancyEnabled();
};

class AFortAthenaMutator_SpecialRelevancy : public AFortAthenaMutator
{
public:
    static UClass* StaticClass();

    TSubclassOf<UFortReplicationGraphNode_SpecialRelevancy> GetRootGridNodeClass() const;
};
