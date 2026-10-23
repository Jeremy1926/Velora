#include "pch.h"
#include "FortAthenaPointAroundPlayerProvider.h"
#include "FortAthenaLivingWorldManager.h"
#include "FortAthenaLivingWorldPointProviderInterface.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortAthenaPointAroundPlayerProvider::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/LagerRuntime.FortAthenaPointAroundPlayerProvider"));

    return Class;
}

void AFortAthenaPointAroundPlayerProvider::EnablePointProvider()
{
    DETOUR(AFortAthenaPointAroundPlayerProvider, EnablePointProvider, Anchor::Hook()
        .If(UFortAthenaLivingWorldManager::GetEventInstancesOffset() > 0)
        .Exec(TEXT("/Script/LagerRuntime.FortAthenaPointAroundPlayerProvider:EnablePointProvider"))
        .Is("xor eax, eax; cmp [rdx+0x20], rax; setnz al; add rcx, 0x*; add [rdx+0x20], rax; mov rax, [rcx]; jmp [rax+0x*]")
        .Find("jmp [rax+0x*]")
        .Entry(TEXT("/Script/LagerRuntime.Default__FortAthenaPointAroundPlayerProvider"), TEXT("/Script/LagerRuntime.FortAthenaLivingWorldPointProviderInterface"))
        .Optional()
        .Inline());

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(IFortAthenaLivingWorldPointProviderInterface::StaticClass()) - (uint8*)DefaultObject;

    Original();

    UFortAthenaLivingWorldManager::LivingWorldManagerRegisterPointProvider((AActor*)((uint8*)this - PointerOffset));
}
