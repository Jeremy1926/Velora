#include "pch.h"
#include "Player/FortPlayerControllerZone.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static int32_t OnPawnRevivedSlot()
{
    static auto OnPawnRevived = []
    {
        auto ClientOnPawnRevived = Scanner::Event(TEXT("ClientOnPawnRevived")).Get(TEXT("AFortPlayerControllerZone::ClientOnPawnRevived"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"));

        return Scanner::Slot(TEXT("AFortPlayerControllerZone::OnPawnRevived"), VTable, [&](const Scanner::FCode& Entry)
        {
            auto Call = Entry.Find("call 0x*");

            for (int32 Index = 2; Call.Get() && Call.Target() != ClientOnPawnRevived; Index++)
                Call = Entry.Find("call 0x*", Index);

            return Call.Get() != nullptr;
        });
    }();

    return OnPawnRevived;
}

UClass* AFortPlayerControllerZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerZone"));

    return Class;
}

void AFortPlayerControllerZone::ServerAcknowledgePossession_Implementation(APawn* P)
{
    Super::ServerAcknowledgePossession_Implementation(P);
}

void AFortPlayerControllerZone::OnPawnRevived(AController* EventInstigator)
{
    Scanner::Virtual<void (*)(AFortPlayerControllerZone*, AController*)>(this, OnPawnRevivedSlot())(this, EventInstigator);
}

void AFortPlayerControllerZone::ApplyDeathDurabilityPenalty(float DurabilityPenalty)
{
    Scanner::Virtual<void (*)(AFortPlayerControllerZone*, float)>(this, OnPawnRevivedSlot(), sizeof(void*))(this, DurabilityPenalty);
}
