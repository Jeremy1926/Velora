#include "pch.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Scanner.h"

void FOutputDeviceRedirector::AddOutputDevice(FOutputDevice* OutputDevice)
{
    static auto AddOutputDevice = Anchor::Function<void (*)(FOutputDeviceRedirector*, FOutputDevice*)>(TEXT("FOutputDeviceRedirector::AddOutputDevice"))
        .String(TEXT("NOCONSOLE"))
        .Call(3)
        .Get();

    AddOutputDevice(this, OutputDevice);
}
