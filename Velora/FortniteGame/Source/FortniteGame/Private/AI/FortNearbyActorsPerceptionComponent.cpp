#include "pch.h"
#include "AI/FortNearbyActorsPerceptionComponent.h"
#include "FortGameModeZone.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortNearbyActorsPerceptionComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortNearbyActorsPerceptionComponent"));

    return Class;
}

void UFortNearbyActorsPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    DETOUR(UFortNearbyActorsPerceptionComponent, TickComponent, Anchor::Hook()
        .String(TEXT("%s: AIDirector or AffiliationManager are nullptr, are you on the dedicated server side? AIDirector: %s, AffiliationManager: %s"))
        .Start()
        .SlotIn(TEXT("/Script/FortniteGame.Default__FortNearbyActorsPerceptionComponent"))
        .Or()
        .If(::Anchor::Exists().String(TEXT("%s: AIDirector or AffiliationManager are nullptr, are you on the dedicated server side? AIDirector: %s, AffiliationManager: %s")).Get())
        .InChildVTablesCounted(StaticClass()));

    static auto CachedAIDirector = Anchor::Offset(TEXT("UFortNearbyActorsPerceptionComponent::CachedAIDirector"))
        .String(TEXT("%s: AIDirector or AffiliationManager are nullptr, are you on the dedicated server side? AIDirector: %s, AffiliationManager: %s"))
        .Start()
        .Find("test rax, rax; *; jnz 0x*; *; *; *; *; call 0x*")
        .Last("lea r*, [r*+0x*]")
        .Or()
        .String(TEXT("%s: AIDirector or AffiliationManager are nullptr, are you on the dedicated server side? AIDirector: %s, AffiliationManager: %s"))
        .Start()
        .Find("test rax, rax; jnz 0x*; *; *; *; *; call 0x*")
        .Last("lea r*, [r*+0x*]")
        .Disp();

    auto World = CachedAIDirector ? GetWorld() : nullptr;
    auto GameMode = World ? Cast<AFortGameModeZone>(World->GetAuthGameMode()) : nullptr;
    auto& AIDirector = *(FWeakObjectPtr*)((uint8*)this + CachedAIDirector);
    static auto GameModeAIDirector = AFortGameModeZone::StaticClass() ? AFortGameModeZone::StaticClass()->FindPropertyByName(FName(TEXT("AIDirector"))) : nullptr;

    if (GameMode && GameModeAIDirector && !AIDirector.Get())
        AIDirector = *GameModeAIDirector->ContainerPtrToValuePtr<UObject*>(GameMode);

    Original(DeltaTime, TickType, ThisTickFunction);
}
