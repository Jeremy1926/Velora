#include "pch.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/TransformVectorized.h"
#include "Scanner.h"

FTransform UKismetMathLibrary::ComposeTransforms(const FTransform& A, const FTransform& B)
{
    static auto ComposeTransforms = Scanner::Function<FTransform* (*)(FTransform*, const FTransform&, const FTransform&)>(Scanner::Native(TEXT("/Script/Engine.KismetMathLibrary:ComposeTransforms")).Get(TEXT("UKismetMathLibrary::ComposeTransforms")));

    FTransform Result;

    return *ComposeTransforms(&Result, A, B);
}
