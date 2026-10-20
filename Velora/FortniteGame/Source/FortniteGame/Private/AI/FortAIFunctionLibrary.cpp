#include "pch.h"
#include "AI/FortAIFunctionLibrary.h"
#include "Scanner.h"

void UFortAIFunctionLibrary::MakeNoiseEventAtLocation(AActor* NoiseMaker, float MaxRange, const FVector& NoiseLocation)
{
    static auto MakeNoiseEventAtLocation = Anchor::Function<void (*)(AActor*, float, const FVector&, FName)>(TEXT("UFortAIFunctionLibrary::MakeNoiseEventAtLocation"))
        .Native(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Get();

    MakeNoiseEventAtLocation(NoiseMaker, MaxRange, NoiseLocation, FName(NAME_None));
}
