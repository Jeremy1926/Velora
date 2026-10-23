#pragma once

#include "GameFramework/Info.h"

class AFortTeamHealthInfo : public AInfo
{
public:
    uint8 GetTeamNum() const;
};
