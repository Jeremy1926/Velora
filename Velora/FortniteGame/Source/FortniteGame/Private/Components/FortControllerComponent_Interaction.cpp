#include "pch.h"
#include "Components/FortControllerComponent_Interaction.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortControllerComponent_Interaction::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortControllerComponent_Interaction"));

    return Class;
}

float UFortControllerComponent_Interaction::GetInteractDistance() const
{
    static auto GetInteractDistance = Anchor::Function<float (*)(const UFortControllerComponent_Interaction*)>(TEXT("UFortControllerComponent_Interaction::GetInteractDistance"))
        .Exec(TEXT("/Script/FortniteGame.FortControllerComponent_Interaction:ServerAttemptInteract"))
        .Find("call r10; test al, al")
        .Find("call [r10+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortControllerComponent_Interaction"))
        .Find("call 0x*")
        .Target()
        .Has("movss xmm6, dword ptr [rbx+0x*]; cmp byte ptr [rbx+0x*], 0x0")
        .Get();

    return GetInteractDistance(this);
}
