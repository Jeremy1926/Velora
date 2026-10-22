#include "pch.h"
#include "AI/FortAIFunctionLibrary.h"
#include "Scanner.h"

void UFortAIFunctionLibrary::MakeNoiseEventAtLocation(AActor* NoiseMaker, float MaxRange, const FVector& NoiseLocation)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
        return;

    static auto MakeNoiseEventAtLocation = Anchor::Function<void (*)(AActor*, float, const FVector&, FName)>(TEXT("UFortAIFunctionLibrary::MakeNoiseEventAtLocation"))
        .Native(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Get();

    MakeNoiseEventAtLocation(NoiseMaker, MaxRange, NoiseLocation, FName(NAME_None));
}
