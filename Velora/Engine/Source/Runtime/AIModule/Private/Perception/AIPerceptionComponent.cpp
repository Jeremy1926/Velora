#include "pch.h"
#include "Perception/AIPerceptionComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UAIPerceptionComponent::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.AIPerceptionComponent"));

    return Class;
}

void UAIPerceptionComponent::GetPerceivedHostileActors(TArray<AActor*>& OutActors) const
{
    static auto GetPerceivedHostileActors = Anchor::VirtualFunction<void (*)(const UAIPerceptionComponent*, TArray<AActor*>&)>(TEXT("UAIPerceptionComponent::GetPerceivedHostileActors"))
        .Exec(TEXT("/Script/AIModule.AIPerceptionComponent:GetPerceivedHostileActors"))
        .Find("add [*+0x20], *")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/AIModule.AIPerceptionComponent:GetPerceivedHostileActors"))
        .Has("bt rax, 0x8")
        .Find("add r*, rax; mov [r*+0x20], r*")
        .Find("call [*+0x*]")
        .Or()
        .Exec(TEXT("/Script/AIModule.AIPerceptionComponent:GetPerceivedHostileActors"))
        .Find("call [rax+0x*]")
        .Get();

    GetPerceivedHostileActors(this, OutActors);
}
