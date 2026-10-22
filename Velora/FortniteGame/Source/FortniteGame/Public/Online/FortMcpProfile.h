#pragma once

#include "McpProfile.h"
#include "Containers/Array.h"
#include "Containers/UnrealString.h"

class UFortAccountItem;

class UFortMcpProfile : public UMcpProfile
{
public:
    void GetItemInstancesForTemplate(const FString& Template, TArray<UFortAccountItem*>& OutInstances);
};
