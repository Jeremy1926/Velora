#include "pch.h"
#include "Math/Rotator.h"
#include "Math/TransformVectorized.h"
#include "Scanner.h"

FVector FRotator::Vector() const
{
    static auto Vector = (FVector* (*)(const FRotator*, FVector*))Scanner::Native(TEXT("/Script/Engine.KismetMathLibrary:Conv_RotatorToVector")).Get();

    FVector Result;

    return *Vector(this, &Result);
}

const FTransform FTransform::Identity;

FVector FMath::VRand()
{
    static auto VRand = (FVector* (*)(FVector*))Scanner::Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVector")).Call().Get();

    FVector Result;

    return *VRand(&Result);
}

FVector FMath::VRandCone(FVector const& Dir, float ConeHalfAngleRad)
{
    static auto VRandCone = (FVector* (*)(FVector*, FVector const&, float))Scanner::Native(TEXT("/Script/Engine.KismetMathLibrary:RandomUnitVectorInCone")).Get();

    FVector Result;

    return *VRandCone(&Result, Dir, ConeHalfAngleRad);
}
