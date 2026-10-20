#pragma once

#include "GameFramework/Info.h"

struct FReservedRandomValues;

class AFortWorldManager : public AInfo
{
public:
    FReservedRandomValues& GetFoundationBuildingActorsRRV();
};
