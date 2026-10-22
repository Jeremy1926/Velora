#include "pch.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/TransformVectorized.h"

FTransform UKismetMathLibrary::ComposeTransforms(const FTransform& A, const FTransform& B)
{
    FTransform Result;

    FTransform::Multiply(&Result, &A, &B);

    return Result;
}
