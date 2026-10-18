#include "pch.h"
#include "AI/FortAIFunctionLibrary.h"
#include "Scanner.h"

void UFortAIFunctionLibrary::MakeNoiseEventAtLocation(AActor* NoiseMaker, float MaxRange, const FVector& NoiseLocation)
{
    static auto MakeNoiseEventAtLocation = (void (*)(AActor*, float, const FVector&))Scanner::Native(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation")).Get(TEXT("UFortAIFunctionLibrary::MakeNoiseEventAtLocation"));

    MakeNoiseEventAtLocation(NoiseMaker, MaxRange, NoiseLocation);
}
