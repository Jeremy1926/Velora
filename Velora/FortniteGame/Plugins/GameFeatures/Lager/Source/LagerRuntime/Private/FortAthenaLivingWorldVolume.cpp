#include "pch.h"
#include "FortAthenaLivingWorldVolume.h"
#include "FortAthenaLivingWorldManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortAthenaLivingWorldVolume::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume"));

    return Class;
}

void AFortAthenaLivingWorldVolume::OnEnvQueryFinished(const TSharedPtr<FEnvQueryResult>& Result)
{
    DETOUR(AFortAthenaLivingWorldVolume, OnEnvQueryFinished, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnCurrentPlaylistLoaded"))
        .Find("lea rax, [0x*]; mov [rsp+0x*], r13")
        .Target()
        .Inline());

    Original(Result);

    if (GetPoints().Num() > 0 && GetWorld())
        UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider(this);
}

TArray<FVector>& AFortAthenaLivingWorldVolume::GetPoints()
{
    static auto Points = Anchor::Offset(TEXT("AFortAthenaLivingWorldVolume::Points"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldVolume:OnCurrentPlaylistLoaded"))
        .Find("lea rax, [0x*]; mov [rsp+0x*], r13")
        .Target()
        .Find("lea *, [rcx+0x*]")
        .Disp();

    return *(TArray<FVector>*)((uint8*)this + Points);
}
