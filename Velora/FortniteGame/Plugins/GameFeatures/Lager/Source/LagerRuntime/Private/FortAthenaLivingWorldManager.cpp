#include "pch.h"
#include "FortAthenaLivingWorldManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* UFortAthenaLivingWorldManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager"));

    return Class;
}

int32 UFortAthenaLivingWorldManager::GetEventInstancesOffset()
{
    static auto EventInstances = Anchor::Offset(TEXT("UFortAthenaLivingWorldManager::EventInstances"))
        .If(StaticClass())
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Optional()
        .Find("movzx eax, byte ptr [rax+0x*]; xor r14d, r14d; mov [rsp+0x*], eax; cmp [r15+0x*], r14d")
        .Find("lea r12, [r15+0x*]")
        .Disp();

    return EventInstances;
}
