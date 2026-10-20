#include "pch.h"
#include "FortGameModeZone.h"
#include "FortGameData.h"
#include "AI/FortAIDirector.h"
#include "Athena/AI/AthenaAIDirector.h"
#include "Items/FortInventory.h"
#include "Engine/World.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

static auto FinishWorldInitialization = []
{
    auto AthenaFinishWorldInitialization = Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start().Get();
    auto AthenaVTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
    auto Slot = Scanner::Slot(TEXT("AFortGameMode::FinishWorldInitialization"), AthenaVTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == AthenaFinishWorldInitialization; });
    auto PvPBaseFinishWorldInitialization = Scanner::Virtual<const void*>(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGamePvPBase")), Slot);
    auto Call = Scanner::At(PvPBaseFinishWorldInitialization).Find("call 0x*");

    if (!Call.Get())
        Call = Scanner::At(AthenaFinishWorldInitialization).Find(std::format("call 0x{:x}", (uintptr_t)PvPBaseFinishWorldInitialization));

    return DetourCall(TEXT("AFortGameModeZone::FinishWorldInitialization"), Call.Get(), &AFortGameModeZone::FinishWorldInitialization);
}();

static auto CreateAIDirector = []
{
    auto AthenaCreateAIDirector = Scanner::String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i"));

    return AthenaCreateAIDirector.Get() ? DetourCall(TEXT("AFortGameModeZone::CreateAIDirector"), AthenaCreateAIDirector.Start().Find("call 0x*").Get(), &AFortGameModeZone::CreateAIDirector) : nullptr;
}();

void AFortGameModeZone::FinishWorldInitialization()
{
    Super::FinishWorldInitialization();
}

void AFortGameModeZone::CreateAIDirector()
{
    static auto CanCreateInCurrentContext = DetourCall(TEXT("UObject::CanCreateInCurrentContext"), Scanner::String(TEXT("Unable to spawn class '%s' due to client/server context.")).Last("call 0x*; test al, al").Get(), &UObject::CanCreateInCurrentContext);
    static auto AIDirector = GetClass()->FindPropertyByName(FName(TEXT("AIDirector")));

    auto& AIDirectors = UFortGameData::Get()->GetAIDirectors();
    auto AIDirectorIndex = GetOverrideAIDirectorIndex();

    if (AIDirectorIndex < 0 || AIDirectorIndex >= AIDirectors.Num())
        return;

    auto AIDirectorClass = AIDirectors[AIDirectorIndex].LoadSynchronous();

    if (!AIDirectorClass)
        AIDirectorClass = AAthenaAIDirector::StaticClass();

    *AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this) = (AFortAIDirector*)GetWorld()->SpawnActor(AIDirectorClass);
}

AFortAIDirector* AFortGameModeZone::GetAIDirector()
{
    static auto AIDirector = GetClass()->FindPropertyByName(FName(TEXT("AIDirector")));
    static auto CreateAIDirector = []
    {
        auto AthenaCreateAIDirector = Scanner::String(TEXT("AFortGameModeAthena::CreateAIDirector setting initial MaxActiveAlive override to %i")).Start().Get();
        auto VTable = *(const void***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));

        return Scanner::Slot(TEXT("AFortGameModeZone::CreateAIDirector"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Get() == AthenaCreateAIDirector; });
    }();

    if (!*AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this))
        Scanner::Virtual<void (*)(AFortGameModeZone*)>(this, CreateAIDirector)(this);

    return *AIDirector->ContainerPtrToValuePtr<AFortAIDirector*>(this);
}

int32 AFortGameModeZone::GetOverrideAIDirectorIndex() const
{
    static auto OverrideAIDirectorIndex = GetClass()->FindPropertyByName(FName(TEXT("OverrideAIDirectorIndex")));

    return *OverrideAIDirectorIndex->ContainerPtrToValuePtr<int32>(this);
}

TArray<FItemAndCount>& AFortGameModeZone::GetStartingItems()
{
    static auto StartingItems = GetClass()->FindPropertyByName(FName(TEXT("StartingItems")));

    return *StartingItems->ContainerPtrToValuePtr<TArray<FItemAndCount>>(this);
}
