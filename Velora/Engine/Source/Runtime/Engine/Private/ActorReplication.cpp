#include "pch.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static auto IsNetRelevantForSlot = Scanner::String(TEXT("Actor %s / %s has no root component in AActor::IsNetRelevantFor. (Make bAlwaysRelevant=true?)")).Start().Find("call [*+0x*]").Slot(TEXT("AActor::IsNetRelevantFor"));

static int32_t GetNetPrioritySlot()
{
    static auto GetNetPriority = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));

        for (auto Index = 0; Index * (int32_t)sizeof(void*) < IsNetRelevantForSlot; Index++)
        {
            auto Function = Scanner::At(VTable[Index]);

            if (Function.Is("movss xmm0, dword ptr [0x*]; ret") && *(float*)Function.Target() == -17.0f)
                return Scanner::Slot(TEXT("AActor::GetNetPriority"), (Index + 1) * (int32_t)sizeof(void*));
        }

        return Scanner::Slot(TEXT("AActor::GetNetPriority"), 0);
    }();

    return GetNetPriority;
}

bool AActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(const AActor*, const AActor*, const AActor*, const FVector&))(VTable + IsNetRelevantForSlot))(this, RealViewer, ViewTarget, SrcLocation);
}

float AActor::GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    auto VTable = *(uint8_t**)this;

    return (*(float (**)(AActor*, const FVector&, const FVector&, AActor*, AActor*, UActorChannel*, float, bool))(VTable + GetNetPrioritySlot()))(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

bool AActor::GetNetDormancy(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    static auto GetNetDormancy = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto Index = GetNetPrioritySlot() / (int32_t)sizeof(void*);

        while (!Scanner::At(VTable[Index]).Start().Has(std::format("jmp [*+{:#x}]", (Index + 1) * sizeof(void*))))
            Index++;

        return Scanner::Slot(TEXT("AActor::GetNetDormancy"), (Index + 1) * (int32_t)sizeof(void*));
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(AActor*, const FVector&, const FVector&, AActor*, AActor*, UActorChannel*, float, bool))(VTable + GetNetDormancy))(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

void AActor::CallPreReplication(UNetDriver* NetDriver)
{
    static auto CallPreReplication = (void (*)(AActor*, UNetDriver*))Scanner::String(TEXT("Actor %s took more than 95%% of maximum desired recording time. Actor: %.3fms. Max: %.3fms.")).Last("xor r9d, r9d").Last("call 0x*").Target(TEXT("AActor::CallPreReplication"));

    CallPreReplication(this, NetDriver);
}
