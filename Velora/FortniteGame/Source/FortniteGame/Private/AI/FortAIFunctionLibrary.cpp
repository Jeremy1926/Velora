#include "pch.h"
#include "AI/FortAIFunctionLibrary.h"
#include "Perception/AISense_Hearing.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "Player/FortPlayerController.h"
#include "GameFramework/Actor.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

void UFortAIFunctionLibrary::MakeNoiseEventAtLocation(AActor* NoiseMaker, float MaxRange, const FVector& NoiseLocation)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Get();

    if (bInlined)
    {
        if (!NoiseMaker)
            return;

        static auto GetSubGame = Anchor::Function<ESubGame (*)(const UObject*)>(TEXT("UFortGlobals::GetSubGame"))
            .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
            .Find("mov rcx, r*; call 0x*; mov r*d, 0x1; cmp al, r*b")
            .Call()
            .Get();

        static auto OnAthenaNoiseEvent = Anchor::Function<void (*)(UWorld*, const FAINoiseEvent&)>(TEXT("UAIPerceptionSystem::OnEvent<FAINoiseEvent> (Athena.AI.NoiseMaker)"))
            .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
            .Find("mov rcx, rax; lea rdx, [rbp-0x*]; call 0x*")
            .Call()
            .Get();

        static auto OnNoiseEvent = Anchor::Function<void (*)(UWorld*, const FAINoiseEvent&)>(TEXT("UAIPerceptionSystem::OnEvent<FAINoiseEvent>"))
            .Exec(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
            .Find("mov rcx, rax; lea rdx, [rbp-0x*]; call 0x*", 2)
            .Call()
            .Get();

        if (GetSubGame(NoiseMaker) == ESubGame::Athena)
        {
            static auto GameplayTagAssetInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayTags.GameplayTagAssetInterface"));
            static auto NoiseMakerTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Athena.AI.NoiseMaker")), false);

            FGameplayTagContainer OwnedTags;

            if (auto AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NoiseMaker))
                ((IGameplayTagAssetInterface*)AbilitySystemComponent->GetInterfaceAddress(GameplayTagAssetInterface))->GetOwnedGameplayTags(OwnedTags);

            if (NoiseMakerTag.IsValid() && OwnedTags.GetGameplayTagArray().Contains(NoiseMakerTag))
                OnAthenaNoiseEvent(NoiseMaker->GetWorld(), FAINoiseEvent(NoiseMaker, NoiseLocation, 1.0f, MaxRange, FName(NAME_None)));

            return;
        }

        OnNoiseEvent(NoiseMaker->GetWorld(), FAINoiseEvent(NoiseMaker, NoiseLocation, 1.0f, MaxRange, FName(NAME_None)));
        return;
    }

    static auto MakeNoiseEventAtLocation = Anchor::Function<void (*)(AActor*, float, const FVector&, FName)>(TEXT("UFortAIFunctionLibrary::MakeNoiseEventAtLocation"))
        .Native(TEXT("/Script/FortniteGame.FortAIFunctionLibrary:MakeNoiseEventAtLocation"))
        .Get();

    MakeNoiseEventAtLocation(NoiseMaker, MaxRange, NoiseLocation, FName(NAME_None));
}
