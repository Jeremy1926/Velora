#include "pch.h"
#include "AI/FortAIFunctionLibrary.h"
#include "Scanner.h"

void UFortAIFunctionLibrary::MakeNoiseEventAtLocation(AActor* NoiseMaker, float MaxRange, const FVector& NoiseLocation)
{
    static auto MakeNoiseEventAtLocation = Scanner::Function<void (*)(AActor*, float, const FVector&, FName)>(Scanner::Native(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation")).Get(TEXT("UFortAIFunctionLibrary::MakeNoiseEventAtLocation")));

    MakeNoiseEventAtLocation(NoiseMaker, MaxRange, NoiseLocation, FName(NAME_None));
}
