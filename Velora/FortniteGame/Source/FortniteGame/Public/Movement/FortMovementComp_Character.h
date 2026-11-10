#pragma once

#include "GameFramework/CharacterMovementComponent.h"

struct FCharacterNetworkMoveData;

struct FFortMovementMode_BaseExtRuntimeData
{
    void** VTable;
    int32 LogicID;
    uint8 RuntimeID;
    bool bForceManualActivation;
    bool bForceManualEnd;
};

class UFortMovementComp_Character : public UCharacterMovementComponent
{
public:
    float GetMaxSpeed() const;

    void ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData);

    void ServerUpdateReceivedPackMovementModeExtension();
};
