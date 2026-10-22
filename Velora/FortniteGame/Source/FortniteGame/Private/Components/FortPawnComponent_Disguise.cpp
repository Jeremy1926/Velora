#include "pch.h"
#include "Components/FortPawnComponent_Disguise.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortPawnComponent_Disguise::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPawnComponent_Disguise"));

    return Class;
}

void UFortPawnComponent_Disguise::RevealDisguise(const AActor* RevealInstigator)
{
    static auto RevealDisguise = Anchor::Function<void (*)(UFortPawnComponent_Disguise*, const AActor*)>(TEXT("UFortPawnComponent_Disguise::RevealDisguise"))
        .Native(TEXT("/Script/FortniteGame.FortPawnComponent_Disguise:RevealDisguise"))
        .Get();

    RevealDisguise(this, RevealInstigator);
}
