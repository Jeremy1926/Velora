#include "pch.h"
#include "Misc/EngineVersion.h"
#include "Misc/OutputDeviceFile.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Windows/WindowsConsoleOutputDevice.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "MinHook.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

static auto bOutputDevicesRegistered = []
{
    TCHAR Filename[MAX_PATH];
    auto Length = GetModuleFileNameW(nullptr, Filename, MAX_PATH);

    while (Length && Filename[Length - 1] != TEXT('\\'))
        Length--;

    wcscpy_s(Filename + Length, MAX_PATH - Length, TEXT("Velora.log"));

    auto LogFile = new FOutputDeviceFile(Filename);
    auto LogConsole = new FWindowsConsoleOutputDevice();

    LogFile->IncludeCategory(FName(TEXT("LogVelora")));
    LogConsole->IncludeCategory(FName(TEXT("LogVelora")));

    GLog->AddOutputDevice(LogFile);
    GLog->AddOutputDevice(LogConsole);

    return true;
}();

FLogCategoryBase LogVelora(TEXT("LogVelora"), ELogVerbosity::Log, ELogVerbosity::All);

static auto bStartupLogged = []
{
    UE_LOG(LogVelora, Display, TEXT("Velora built %hs %hs"), __DATE__, __TIME__);

    auto& Version = FEngineVersion::Current();

    UE_LOG(LogVelora, Display, TEXT("Unreal Engine %hu.%hu.%hu-%u+%s"), Version.GetMajor(), Version.GetMinor(), Version.GetPatch(), Version.GetChangelist(), *Version.GetBranch());
    UE_LOG(LogVelora, Display, TEXT("Image base 0x%p"), Scanner::Base());

    return true;
}();

void Main()
{
    if (auto Status = MH_EnableHook(MH_ALL_HOOKS); Status == MH_OK)
    {
        UE_LOG(LogVelora, Display, TEXT("Hooks enabled"));
    }
    else
    {
        UE_LOG(LogVelora, Error, TEXT("Could not enable hooks (%hs)"), MH_StatusToString(Status));
    }
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
        std::thread(Main).detach();

    return TRUE;
}
