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

        for (auto Index = IsNetRelevantForSlot / (int32_t)sizeof(void*) - 1; Index > 0; Index--)
        {
            auto Function = Scanner::At(VTable[Index]);
            auto Self = std::format("[*+{:#x}]", Index * sizeof(void*));

            if (Function.Has("call " + Self) || Function.Has("jmp " + Self))
                return Scanner::Slot(TEXT("AActor::GetNetPriority"), Index * (int32_t)sizeof(void*));
        }

        return Scanner::Slot(TEXT("AActor::GetNetPriority"), 0);
    }();

    return GetNetPriority;
}

bool AActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    return Scanner::Virtual<bool (*)(const AActor*, const AActor*, const AActor*, const FVector&)>(this, IsNetRelevantForSlot)(this, RealViewer, ViewTarget, SrcLocation);
}

float AActor::GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    return Scanner::Virtual<float (*)(AActor*, const FVector&, const FVector&, AActor*, AActor*, UActorChannel*, float, bool)>(this, GetNetPrioritySlot())(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

bool AActor::GetNetDormancy(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    static auto GetNetDormancy = []
    {
        auto DefaultObject = StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__Actor"));
        auto GetNetDormancy = Scanner::IsSlot(GetNetPrioritySlot()) ? GetNetPrioritySlot() + 2 * (int32_t)sizeof(void*) : 0;

        if (Scanner::At(Scanner::Virtual<const void*>(DefaultObject, GetNetDormancy)).Has(std::format("jmp [*+{:#x}]", GetNetDormancy + sizeof(void*))))
            GetNetDormancy += (int32_t)sizeof(void*);

        return Scanner::Slot(TEXT("AActor::GetNetDormancy"), GetNetDormancy);
    }();

    return Scanner::Virtual<bool (*)(AActor*, const FVector&, const FVector&, AActor*, AActor*, UActorChannel*, float, bool)>(this, GetNetDormancy)(this, ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);
}

void AActor::CallPreReplication(UNetDriver* NetDriver)
{
    static auto CallPreReplication = Scanner::Function<void (*)(AActor*, UNetDriver*)>([]
    {
        auto TickDemoRecord = Scanner::String(TEXT("Actor %s took more than 95%% of maximum desired recording time. Actor: %.3fms. Max: %.3fms."));
        auto Call = TickDemoRecord.Last("xor r9d, r9d").Last("call 0x*");

        return (Call.Get() ? Call : TickDemoRecord.Start().Find("call 0x*; mov r8, [*]; xor r9d, r9d")).Target(TEXT("AActor::CallPreReplication"));
    }());

    CallPreReplication(this, NetDriver);
}
