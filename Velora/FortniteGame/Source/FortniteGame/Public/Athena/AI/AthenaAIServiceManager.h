#pragma once

#include "AISubsystem.h"

class UAthenaAIService;

class UAthenaAIServiceManager : public UAISubsystem
{
public:
    UAthenaAIService* CreateAIService(UClass* ServiceSubClass);
};
