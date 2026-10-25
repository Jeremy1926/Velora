#include "pch.h"
#include <ctime>
#include "Misc/EngineVersion.h"
#include "Logging/LogCategory.h"
#include "CoreGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "MinHook.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

static auto bGameLayoutDecided = []
{
    FName::IsOutlineNumber();

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

    UE_LOG(LogVelora, Display, TEXT("VeloraClient built %04d-%02d-%02d %02d:%02d:%02d, CL %u, UE %hu.%hu, base 0x%llX"), Built.tm_year + 1900, Built.tm_mon + 1, Built.tm_mday, Built.tm_hour, Built.tm_min, Built.tm_sec, Version.GetChangelist(), Version.GetMajor(), Version.GetMinor(), (uintptr_t)Anchor::Base());

    return true;
}();

void Main()
{
    MH_EnableHook(MH_ALL_HOOKS);
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
