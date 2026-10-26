#include "pch.h"
#include "Movement/FortMovementComp_Character.h"
#include "Abilities/FortMovementSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

float UFortMovementComp_Character::GetMaxSpeed() const
{
    DETOUR(UFortMovementComp_Character, GetMaxSpeed, Anchor::Hook()
        .If(!UFortMovementSet::StaticClass()->FindPropertyByName(FName(TEXT("FlySpeed"))))
        .Exec(TEXT("/Script/Engine.MovementComponent:GetMaxSpeed"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"))
        .Inline());

    if (GetMovementMode() == MOVE_Flying && GetCheatFlying())
        return GetMaxFlySpeed();

    return Original();
}
