#include "pch.h"
#include "Perception/AISense_Hearing.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UScriptStruct* FAINoiseEvent::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.AINoiseEvent"));

    return Struct;
}

FAINoiseEvent::FAINoiseEvent(AActor* InInstigator, const FVector& InNoiseLocation, float InLoudness, float InMaxRange, FName InTag)
{
    static auto AINoiseEvent = Anchor::Function<FAINoiseEvent* (*)(FAINoiseEvent*, AActor*, const TGameValue<FVector>&, float, float, FName)>(TEXT("FAINoiseEvent::FAINoiseEvent"))
        .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Find("movss [rsp+0x20], xmm*; call 0x*")
        .Call()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Find("movss [rsp+0x20], xmm*; lea rcx, [rbp-0x*]; cmp al, 0x1; jnz *; call 0x*")
        .Call()
        .Optional()
        .Get();

    if (AINoiseEvent)
    {
        AINoiseEvent(this, InInstigator, InNoiseLocation, InLoudness, InMaxRange, InTag);
        return;
    }

    static auto Compile = Anchor::Function<void (*)(FAINoiseEvent*)>(TEXT("FAINoiseEvent::Compile"))
        .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Find("mov dword ptr [*], 0x3f800000; mov [*], *; mov [*], *; mov [*], al; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();
    static auto NoiseLocation = StaticStruct()->FindPropertyByName(FName(TEXT("NoiseLocation")));
    static auto Loudness = StaticStruct()->FindPropertyByName(FName(TEXT("Loudness")));
    static auto MaxRange = StaticStruct()->FindPropertyByName(FName(TEXT("MaxRange")));
    static auto Instigator = StaticStruct()->FindPropertyByName(FName(TEXT("Instigator")));
    static auto Tag = StaticStruct()->FindPropertyByName(FName(TEXT("Tag")));

    memset(Data, 0, sizeof(Data));
    NoiseLocation->SetPropertyValue_InContainer<FVector>(this, InNoiseLocation);
    *Loudness->ContainerPtrToValuePtr<float>(this) = InLoudness;
    *MaxRange->ContainerPtrToValuePtr<float>(this) = InMaxRange;
    *Instigator->ContainerPtrToValuePtr<AActor*>(this) = InInstigator;
    Tag->SetPropertyValue_InContainer<FName>(this, InTag);

    Compile(this);
}
