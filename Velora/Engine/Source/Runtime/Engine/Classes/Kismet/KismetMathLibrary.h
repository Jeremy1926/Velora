#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

struct FTransform;

class UKismetMathLibrary : public UBlueprintFunctionLibrary
{
public:
    static FTransform ComposeTransforms(const FTransform& A, const FTransform& B);
};
