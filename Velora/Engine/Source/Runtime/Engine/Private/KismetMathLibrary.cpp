#include "pch.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/TransformVectorized.h"
#include "Scanner.h"

FTransform UKismetMathLibrary::ComposeTransforms(const FTransform& A, const FTransform& B)
{
    static auto ComposeTransforms = Anchor::Function<FTransform* (*)(FTransform*, const FTransform&, const FTransform&)>(TEXT("UKismetMathLibrary::ComposeTransforms"))
        .Native(TEXT("/Script/Engine.KismetMathLibrary:ComposeTransforms"))
        .Get();

    FTransform Result;

    return *ComposeTransforms(&Result, A, B);
}
