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
        .Get();

    AddOutputDevice(this, OutputDevice);
}
