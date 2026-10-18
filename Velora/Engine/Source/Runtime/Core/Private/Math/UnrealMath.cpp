#include "pch.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "Math/TransformVectorized.h"
#include "Scanner.h"

FVector FRotator::Vector() const
{
    static auto Vector = (FVector* (*)(const FRotator*, FVector*))Scanner::Native(TEXT("/Script/Engine.KismetMathLibrary:Conv_RotatorToVector")).Get(TEXT("FRotator::Vector"));

    FVector Result;

    return *Vector(this, &Result);
}

FQuat FRotator::Quaternion() const
{
    float SP = sinf(Pitch * 0.008726646f);
    float CP = cosf(Pitch * 0.008726646f);
    float SY = sinf(Yaw * 0.008726646f);
    float CY = cosf(Yaw * 0.008726646f);
    float SR = sinf(Roll * 0.008726646f);
    float CR = cosf(Roll * 0.008726646f);

    FQuat RotationQuat;
    RotationQuat.X = CR * SP * SY - SR * CP * CY;
    RotationQuat.Y = -CR * SP * CY - SR * CP * SY;
    RotationQuat.Z = CR * CP * SY - SR * SP * CY;
    RotationQuat.W = CR * CP * CY + SR * SP * SY;

    return RotationQuat;
}

FRotator FQuat::Rotator() const
{
    static auto Rotator = (FRotator* (*)(const FQuat*, FRotator*))Scanner::Native(TEXT("/Script/Engine.SceneComponent:K2_GetComponentRotation")).Call().Get(TEXT("FQuat::Rotator"));

    FRotator Result;

    return *Rotator(this, &Result);
}

const FTransform FTransform::Identity;

FVector FMath::VRand()
{
    static auto VRand = (FVector* (*)(FVector*))Scanner::Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVector")).Call().Get(TEXT("FMath::VRand"));

    FVector Result;

    return *VRand(&Result);
}

FVector FMath::VRandCone(FVector const& Dir, float ConeHalfAngleRad)
{
    static auto VRandCone = (FVector* (*)(FVector*, FVector const&, float))Scanner::Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVectorInCone")).Get(TEXT("FMath::VRandCone"));

    FVector Result;

    return *VRandCone(&Result, Dir, ConeHalfAngleRad);
}
