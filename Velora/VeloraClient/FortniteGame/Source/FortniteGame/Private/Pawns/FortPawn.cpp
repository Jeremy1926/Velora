#include "pch.h"
#include "Pawns/FortPawn.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UFortMovementSet* AFortPawn::GetMovementSet() const
{
    static auto MovementSet = GetClass()->FindPropertyByName(FName(TEXT("MovementSet")));

    return *MovementSet->ContainerPtrToValuePtr<UFortMovementSet*>(this);
}

void AFortPawn::SetCustomTimeDilation(float InCustomTimeDilation)
{
    static auto SetCustomTimeDilation = Anchor::Function<void (*)(AFortPawn*, float)>(TEXT("AFortPawn::SetCustomTimeDilation"))
        .Body(TEXT("/Script/FortniteGame.FortMovementSet:OnRep_SpeedMultiplier"))
        .Find("movaps xmm1, xmm0; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    SetCustomTimeDilation(this, InCustomTimeDilation);
}
