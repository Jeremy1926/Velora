#pragma once

#include "Athena/Modifiers/FortAthenaMutator.h"

class AFortAthenaMutator_GameModeBase : public AFortAthenaMutator
{
public:
    static UClass* StaticClass();

    bool GetAutomaticallyFinishInitialization() const;

    void FinishMutatorInitializationAndUnpauseWarmup();
};
