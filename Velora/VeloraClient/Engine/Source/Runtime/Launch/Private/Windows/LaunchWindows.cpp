#include "pch.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Windows/WindowsConsoleOutputDevice.h"
#include "Logging/LogCategory.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "MinHook.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

static auto bOutputDevicesRegistered = []
{
    auto LogConsole = new FWindowsConsoleOutputDevice();
    LogConsole->ExcludeCategory(FName(TEXT("LogVelora")));
    GLog->AddOutputDevice(LogConsole);

    return true;
}();

FLogCategoryBase LogVelora(TEXT("LogVelora"), ELogVerbosity::Log, ELogVerbosity::All);

static auto bAnchorHostSet = []
{
    Anchor::SetHost({ .Log = &Scanner::Log, .FindVTable = &Scanner::FindVTable });

    return true;
}();

void Main()
{
    MH_EnableHook(MH_ALL_HOOKS);
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
        std::thread(Main).detach();

    return TRUE;
}
