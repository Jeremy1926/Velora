#include "pch.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

int32 FEnvQueryRequest::Execute(EEnvQueryRunMode::Type RunMode, const FQueryFinishedSignature& FinishDelegate)
{
    static auto Execute = Anchor::Function<int32 (*)(FEnvQueryRequest*, EEnvQueryRunMode::Type, const FQueryFinishedSignature*)>(TEXT("FEnvQueryRequest::Execute"))
        .Exec(TEXT("/Script/AIModule.EnvQueryManager:RunEQSQuery"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*; lea rcx, [*]; call 0x*")
        .Target()
        .Find("lea r8, [*]; mov edx, *; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return Execute(this, RunMode, &FinishDelegate);
}

UEnvQueryManager* UEnvQueryManager::GetCurrent(UObject* WorldContextObject)
{
    static auto GetCurrent = Anchor::Function<UEnvQueryManager* (*)(UObject*)>(TEXT("UEnvQueryManager::GetCurrent"))
        .Exec(TEXT("/Script/AIModule.EnvQueryManager:RunEQSQuery"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetCurrent(WorldContextObject);
}

UEnvQueryInstanceBlueprintWrapper* UEnvQueryManager::RunEQSQuery(UObject* WorldContextObject, UEnvQuery* QueryTemplate, UObject* Querier, TEnumAsByte<EEnvQueryRunMode::Type> RunMode, TSubclassOf<UEnvQueryInstanceBlueprintWrapper> WrapperClass)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/AIModule.EnvQueryManager:RunEQSQuery"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        if (!QueryTemplate || !Querier)
            return nullptr;

        auto EQSManager = GetCurrent(WorldContextObject);
        UEnvQueryInstanceBlueprintWrapper* QueryInstanceWrapper = nullptr;

        if (EQSManager)
        {
            static auto bAllowControllersAsEQSQuerier = ((UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.AISystem")))->FindPropertyByName(FName(TEXT("bAllowControllersAsEQSQuerier")));

            auto AISystem = StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.Default__AISystem"));
            bool bValidQuerier = true;

            if (AISystem && !*bAllowControllersAsEQSQuerier->ContainerPtrToValuePtr<bool>(AISystem))
            {
                if (auto Controller = Cast<AController>(Querier))
                {
                    if (Controller->GetPawn())
                        Querier = Controller->GetPawn();
                    else
                        bValidQuerier = false;
                }
            }

            if (bValidQuerier)
            {
                QueryInstanceWrapper = NewObject<UEnvQueryInstanceBlueprintWrapper>(EQSManager, *WrapperClass ? *WrapperClass : UEnvQueryInstanceBlueprintWrapper::StaticClass());

                if (QueryInstanceWrapper)
                {
                    FEnvQueryRequest QueryRequest(QueryTemplate, Querier);

                    QueryInstanceWrapper->RunQuery(RunMode, QueryRequest);
                }
            }
        }

        return QueryInstanceWrapper;
    }

    static auto RunEQSQuery = Anchor::Function<UEnvQueryInstanceBlueprintWrapper* (*)(UObject*, UEnvQuery*, UObject*, TEnumAsByte<EEnvQueryRunMode::Type>, TSubclassOf<UEnvQueryInstanceBlueprintWrapper>)>(TEXT("UEnvQueryManager::RunEQSQuery"))
        .Native(TEXT("/Script/AIModule.EnvQueryManager:RunEQSQuery"))
        .Get();

    return RunEQSQuery(WorldContextObject, QueryTemplate, Querier, RunMode, WrapperClass);
}
