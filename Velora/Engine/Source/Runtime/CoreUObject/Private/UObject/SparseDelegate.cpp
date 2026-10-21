#include "pch.h"
#include "UObject/SparseDelegate.h"
#include "Scanner.h"

bool FSparseDelegateStorage::Add(const UObject* DelegateOwner, const FName DelegateName, FScriptDelegate Delegate)
{
    static auto Add = Anchor::Function<bool (*)(const UObject*, FName, FScriptDelegate*)>(TEXT("FSparseDelegateStorage::Add"))
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface:OnSpawnedActor"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPatrolPathPointProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Find("call 0x*; or [*+0x*], al")
        .Target()
        .Get();

    return Add(DelegateOwner, DelegateName, &Delegate);
}
