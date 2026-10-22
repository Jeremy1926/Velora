#include "pch.h"
#include "Athena/AI/AthenaAIServiceManager.h"
#include "Scanner.h"
#include "Detour.h"

UAthenaAIService* UAthenaAIServiceManager::CreateAIService(UClass* ServiceSubClass)
{
    DETOUR(UAthenaAIServiceManager, CreateAIService, Anchor::Hook()
        .String(TEXT("Creating AI Service of class %s"))
        .Optional()
        .Start()
        .Inline());

    if (!ServiceSubClass)
        return nullptr;

    return Original(ServiceSubClass);
}
