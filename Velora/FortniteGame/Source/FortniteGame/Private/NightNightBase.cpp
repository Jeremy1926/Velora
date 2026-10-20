#include "pch.h"
#include "NightNightBase.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

static auto execMakePreparationsForNightNight = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NightNightBase:MakePreparationsForNightNight"));

    if (!Function || !Scanner::At(Scanner::Exec(TEXT("/Script/FortniteGame.NightNightBase:MakePreparationsForNightNight")).Find("jmp 0x*").Target()).Is("ret*"))
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked ANightNightBase::execMakePreparationsForNightNight, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&ANightNightBase::execMakePreparationsForNightNight);

    return true;
}();

static auto execReturnToNight = []
{
    auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.NightNightBase:ReturnToNight"));

    if (!Function || !Scanner::At(Scanner::Exec(TEXT("/Script/FortniteGame.NightNightBase:ReturnToNight")).Find("jmp 0x*").Target()).Is("ret*"))
        return false;

    UE_LOG(LogVelora, Log, TEXT("Hooked ANightNightBase::execReturnToNight, was 0x%llX"), std::bit_cast<const uint8_t*>(Function->GetNativeFunc()) - Scanner::Base());

    Function->SetNativeFunc((Native)&ANightNightBase::execReturnToNight);

    return true;
}();

void ANightNightBase::MakePreparationsForNightNight()
{
    static auto bPrepareClient = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bPrepareClient")));

    bPrepareClient->SetPropertyValue_InContainer(this, true);
}

void ANightNightBase::ReturnToNight()
{
    TArray<AActor*> PlayerControllers;
    UGameplayStatics::GetAllActorsOfClass(this, AFortPlayerControllerAthena::StaticClass(), PlayerControllers);

    for (auto PlayerController : PlayerControllers)
        ((AFortPlayerControllerAthena*)PlayerController)->ClientReportPhaseFound(TEXT("NoBernieNo"));
}

void ANightNightBase::execMakePreparationsForNightNight(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    MakePreparationsForNightNight();
}

void ANightNightBase::execReturnToNight(FFrame& Stack, void* const Z_Param__Result)
{
    Stack.Code += !!Stack.Code;

    ReturnToNight();
}
