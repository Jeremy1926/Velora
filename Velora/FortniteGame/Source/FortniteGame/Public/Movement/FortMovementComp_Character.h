#pragma once

#include "GameFramework/CharacterMovementComponent.h"

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
    static UClass* StaticClass();

    float GetMaxSpeed() const;

    void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel);

    void ServerUpdateReceivedPackMovementModeExtension();
};
