#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"
#include "Misc/Guid.h"

class UConversationNode : public UObject
{
public:
    static UClass* StaticClass();

    UObject*& GetEvalWorldContextObj() const;
};

class UConversationNodeWithLinks : public UConversationNode
{
public:
    static UClass* StaticClass();

    const TArray<FGuid>& GetOutputConnections() const;

    FGuid GetNodeGuid() const;
};
