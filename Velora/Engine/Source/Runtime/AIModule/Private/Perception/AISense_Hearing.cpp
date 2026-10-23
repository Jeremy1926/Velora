#include "pch.h"
#include "Perception/AISense_Hearing.h"
#include "Scanner.h"

FAINoiseEvent::FAINoiseEvent(AActor* InInstigator, const FVector& InNoiseLocation, float InLoudness, float InMaxRange, FName InTag)
{
    static auto AINoiseEvent = Anchor::Function<FAINoiseEvent* (*)(FAINoiseEvent*, AActor*, const TGameValue<FVector>&, float, float, FName)>(TEXT("FAINoiseEvent::FAINoiseEvent"))
        .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Find("movss [rsp+0x20], xmm*; call 0x*")
        .Call()
        .Get();

    AINoiseEvent(this, InInstigator, InNoiseLocation, InLoudness, InMaxRange, InTag);
}
