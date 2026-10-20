#pragma once

#include "GameFramework/Info.h"
#include "GameFramework/OnlineReplStructs.h"

class APlayerState : public AInfo
{
public:
    const FUniqueNetIdRepl& GetUniqueId() const;
};
