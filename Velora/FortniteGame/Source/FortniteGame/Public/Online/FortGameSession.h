#pragma once

#include "GameFramework/GameSession.h"

class UObject;

class AFortGameSession : public AGameSession
{
public:
    static void OpenMatchLevel(const UObject* WorldContextObject);

    void Restart();
};
