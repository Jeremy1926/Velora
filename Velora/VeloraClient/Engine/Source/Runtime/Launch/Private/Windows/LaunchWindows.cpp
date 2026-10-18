#include "pch.h"
#include "Misc/EngineVersion.h"
#include "Logging/LogCategory.h"
#include "MinHook.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

FLogCategoryBase LogVelora(TEXT("LogVelora"), ELogVerbosity::Log, ELogVerbosity::All);

void Main()
{
    AllocConsole();

    FILE* Stream = nullptr;
    freopen_s(&Stream, "CONOUT$", "w", stdout);

    auto& Version = FEngineVersion::Current();

    printf("Velora Client | Unreal Engine %hu.%hu.%hu\n", Version.GetMajor(), Version.GetMinor(), Version.GetPatch());

    MH_EnableHook(MH_ALL_HOOKS);
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
        std::thread(Main).detach();

    return TRUE;
}
