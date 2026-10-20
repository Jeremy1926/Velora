#include "pch.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"
#include "Math/TransformVectorized.h"
#include "Scanner.h"

FVector FRotator::Vector() const
{
    static auto Vector = Anchor::Function<FVector* (*)(const FRotator*, FVector*)>(TEXT("FRotator::Vector"))
        .Native(TEXT("/Script/Engine.KismetMathLibrary:Conv_RotatorToVector"))
        .Get();

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
    static auto Rotator = Anchor::Function<FRotator* (*)(const FQuat*, FRotator*)>(TEXT("FQuat::Rotator"))
        .Native(TEXT("/Script/Engine.SceneComponent:K2_GetComponentRotation"))
        .Call()
        .Get();

    FRotator Result;

    return *Rotator(this, &Result);
}

const FTransform FTransform::Identity;

const FQuat FQuat::Identity(0, 0, 0, 1);

FVector FMath::VRand()
{
    static auto VRand = Anchor::Function<FVector* (*)(FVector*)>(TEXT("UKismetMathLibrary::RandomUnitVector"))
        .Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVector"))
        .Get();

    FVector Result;

    return *VRand(&Result);
}

FVector FMath::VRandCone(FVector const& Dir, float ConeHalfAngleRad)
{
    static auto VRandCone = Anchor::Function<FVector* (*)(FVector*, FVector const&, float)>(TEXT("FMath::VRandCone"))
        .Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVectorInCone"))
        .Or()
        .Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVectorInConeInRadians"))
        .Get();

    FVector Result;

    return *VRandCone(&Result, Dir, ConeHalfAngleRad);
}
