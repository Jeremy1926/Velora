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

    static auto bClassByAddress = Anchor::Exists()
        .String(TEXT("Creating AI Service of class %s"))
        .Start()
        .Find("mov *, [rdx]; *; mov *, rdx")
        .Or()
        .String(TEXT("Creating AI Service of class %s"))
        .Start()
        .Find("mov *, [rdx]; *; *; mov *, rdx")
        .Get();

    if (!ServiceSubClass || (bClassByAddress && !*(UClass**)ServiceSubClass))
        return nullptr;

    return Original(ServiceSubClass);
}
