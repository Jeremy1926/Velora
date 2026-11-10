#include "pch.h"
#include <ctime>
#include "Math/UnrealMathUtility.h"
#include "Misc/EngineVersion.h"
#include "Misc/OutputDeviceFile.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Windows/WindowsConsoleOutputDevice.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "Config.h"
#include "GUI.h"
#include "MinHook.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

static auto bGameLayoutDecided = []
{
    UE::LWC::IsEnabled();
    FName::IsOutlineNumber();

    return true;
}();

static auto bOutputDevicesRegistered = []
{
    TCHAR Filename[MAX_PATH];
    auto Length = GetModuleFileNameW(nullptr, Filename, MAX_PATH);

    while (Length && Filename[Length - 1] != TEXT('\\'))
        Length--;

    wcscpy_s(Filename + Length, MAX_PATH - Length, TEXT("Velora.log"));

    auto LogFile = new FOutputDeviceFile(Filename);
    LogFile->IncludeCategory(FName(TEXT("LogVelora")));
    GLog->AddOutputDevice(LogFile);

    if (Config::bConsole && !Config::bGUI)
    {
        auto LogConsole = new FWindowsConsoleOutputDevice();
        LogConsole->ExcludeCategory(FName(TEXT("LogVelora")));
        GLog->AddOutputDevice(LogConsole);
    }

    return true;
}();

FLogCategoryBase LogVelora(TEXT("LogVelora"), ELogVerbosity::Log, ELogVerbosity::All);

static auto bAnchorHostSet = []
{
    Anchor::SetHost({ .Log = &Scanner::Log, .FindVTable = &Scanner::FindVTable });

    return true;
}();

extern "C" IMAGE_DOS_HEADER __ImageBase;

static auto bStartupLogged = []
{
    auto LinkTime = (__time64_t)((IMAGE_NT_HEADERS*)((uint8_t*)&__ImageBase + __ImageBase.e_lfanew))->FileHeader.TimeDateStamp;
    tm Built;
    _localtime64_s(&Built, &LinkTime);

    auto& Version = FEngineVersion::Current();

    UE_LOG(LogVelora, Display, TEXT("Velora built %04d-%02d-%02d %02d:%02d:%02d, CL %u, UE %hu.%hu, base 0x%llX"), Built.tm_year + 1900, Built.tm_mon + 1, Built.tm_mday, Built.tm_hour, Built.tm_min, Built.tm_sec, Version.GetChangelist(), Version.GetMajor(), Version.GetMinor(), (uintptr_t)Anchor::Base());

    return true;
}();

void Main()
{
    if (auto Status = MH_EnableHook(MH_ALL_HOOKS); Status != MH_OK)
    {
        UE_LOG(LogVelora, Error, TEXT("Could not enable hooks (%hs)"), MH_StatusToString(Status));
    }

    if (Config::bGUI)
        FGUI::Run();
}

BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, LPVOID Reserved)
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        DetourInstall();
        std::thread(Main).detach();
    }

    return TRUE;
}
