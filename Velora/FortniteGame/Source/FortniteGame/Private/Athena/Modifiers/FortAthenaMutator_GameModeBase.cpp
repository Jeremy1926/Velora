#include "pch.h"
#include "Athena/Modifiers/FortAthenaMutator_GameModeBase.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortAthenaMutator_GameModeBase::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaMutator_GameModeBase"));

    return Class;
}

bool AFortAthenaMutator_GameModeBase::GetAutomaticallyFinishInitialization() const
{
    static auto bAutomaticallyFinishInitialization = (UBoolProperty*)StaticClass()->FindPropertyByName(FName(TEXT("bAutomaticallyFinishInitialization")));

    return bAutomaticallyFinishInitialization && bAutomaticallyFinishInitialization->GetPropertyValue_InContainer(this);
}

void AFortAthenaMutator_GameModeBase::FinishMutatorInitializationAndUnpauseWarmup()
{
    static auto bMutatorInitialized = Anchor::Offset(TEXT("AFortAthenaMutator_GameModeBase::bMutatorInitialized"))
        .String(TEXT("AFortAthenaMutator_GameModeBase::FortGameModeAthena_ShouldPauseWarmupTime Pausing warmup*"))
        .Start()
        .Find("cmp byte ptr [rcx+0x*], *")
        .Disp();

    if (bMutatorInitialized > 0)
        *((uint8*)this + bMutatorInitialized) = true;
}
