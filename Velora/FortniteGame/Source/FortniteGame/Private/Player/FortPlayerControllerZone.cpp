#include "pch.h"
#include "Player/FortPlayerControllerZone.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static int32_t OnPawnRevivedSlot()
{
    static auto OnPawnRevived = []
    {
        auto ZoneOnPawnRevived = Scanner::CallTo(Scanner::Event(TEXT("ClientOnPawnRevived")).Get(TEXT("AFortPlayerControllerZone::ClientOnPawnRevived"))).Start().Get(TEXT("AFortPlayerControllerZone::OnPawnRevived"));
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortPlayerControllerZone"));

        return Scanner::Slot(TEXT("AFortPlayerControllerZone::OnPawnRevived"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == ZoneOnPawnRevived; });
    }();

    return OnPawnRevived;
}

UClass* AFortPlayerControllerZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerZone"));

    return Class;
}

void AFortPlayerControllerZone::OnPawnRevived(AController* EventInstigator)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortPlayerControllerZone*, AController*))(VTable + OnPawnRevivedSlot()))(this, EventInstigator);
}

void AFortPlayerControllerZone::ApplyDeathDurabilityPenalty(float DurabilityPenalty)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortPlayerControllerZone*, float))(VTable + OnPawnRevivedSlot() + sizeof(void*)))(this, DurabilityPenalty);
}
