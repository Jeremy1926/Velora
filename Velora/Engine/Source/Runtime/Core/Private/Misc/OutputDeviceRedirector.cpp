#include "pch.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Scanner.h"

void FOutputDeviceRedirector::AddOutputDevice(FOutputDevice* OutputDevice)
{
    static auto AddOutputDevice = Anchor::Function<void (*)(FOutputDeviceRedirector*, FOutputDevice*)>(TEXT("FOutputDeviceRedirector::AddOutputDevice"))
        .String(TEXT("NOCONSOLE"))
        .Call(3)
        .Or()
        .String(TEXT("%s/profilego/%s.%s"))
        .Start()
        .Find("mov rdx, [*]; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("48 85 D2 0F 84 ? ? ? ? 48 89 5C 24 08 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 20 45 33 FF 48 8D 99 ? ? ? ? 41 8A F7 48 8B FA 48 8B E9")
        .Get();

    AddOutputDevice(this, OutputDevice);
}
