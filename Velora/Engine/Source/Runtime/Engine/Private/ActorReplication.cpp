#include "pch.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

bool AActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    static auto IsNetRelevantFor = Anchor::VirtualFunction<bool (*)(const AActor*, const AActor*, const AActor*, const TGameValue<FVector>&)>(TEXT("AActor::IsNetRelevantFor"))
        .String(TEXT("Actor %s / %s has no root component in AActor::IsNetRelevantFor. (Make bAlwaysRelevant=true?)"))
        .Start()
        .Find("call [*+0x*]")
        .Get();

    return IsNetRelevantFor(this, RealViewer, ViewTarget, SrcLocation);
}

float AActor::GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    static auto GetNetPriority = Anchor::VirtualFunction<float (*)(AActor*, const TGameValue<FVector>&, const TGameValue<FVector>&, AActor*, AActor*, UActorChannel*, float, bool)>(TEXT("AActor::GetNetPriority"))
        .String(TEXT("Actor %s / %s has no root component in AActor::IsNetRelevantFor. (Make bAlwaysRelevant=true?)"))
        .Start()
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AActor::IsNetRelevantFor"))
        .SelfCallerBelow(TEXT("/Script/Engine.Default__Actor"))
        .Get();

    return GetNetPriority(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

bool AActor::GetNetDormancy(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    static auto GetNetDormancy = Anchor::VirtualFunction<bool (*)(AActor*, const TGameValue<FVector>&, const TGameValue<FVector>&, AActor*, AActor*, UActorChannel*, float, bool)>(TEXT("AActor::GetNetDormancy"))
        .String(TEXT("Actor %s / %s has no root component in AActor::IsNetRelevantFor. (Make bAlwaysRelevant=true?)"))
        .Start()
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AActor::IsNetRelevantFor"))
        .SelfCallerBelow(TEXT("/Script/Engine.Default__Actor"))
        .LogSlot(TEXT("AActor::GetNetPriority"))
        .SlotShift(TEXT("/Script/Engine.Default__Actor"), 2 * sizeof(void*))
        .SlotForwarded(TEXT("/Script/Engine.Default__Actor"))
        .Get();

    return GetNetDormancy(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

void AActor::CallPreReplication(UNetDriver* NetDriver)
{
    static auto CallPreReplication = Anchor::Function<void (*)(AActor*, UNetDriver*)>(TEXT("AActor::CallPreReplication"))
        .String(TEXT("Actor %s took more than 95%% of maximum desired recording time. Actor: %.3fms. Max: %.3fms."))
        .Is("lea *, [0x*]")
        .Last("xor r9d, r9d")
        .Last("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Actor %s took more than 95%% of maximum desired recording time. Actor: %.3fms. Max: %.3fms."))
        .Start()
        .Find("call 0x*; mov r8, [*]; xor r9d, r9d")
        .Target()
        .Or()
        .String(TEXT("Attempt to replicate function '%s' on Actor '%s' while it is in the middle of variable replication!"))
        .Find("inc [*]; mov rdx, rcx; mov rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    CallPreReplication(this, NetDriver);
}
