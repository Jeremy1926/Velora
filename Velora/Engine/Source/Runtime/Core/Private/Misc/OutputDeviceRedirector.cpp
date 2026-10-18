#include "pch.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Scanner.h"

void FOutputDeviceRedirector::AddOutputDevice(FOutputDevice* OutputDevice)
{
    static auto AddOutputDevice = Scanner::Function<void (*)(FOutputDeviceRedirector*, FOutputDevice*)>(Scanner::String(TEXT("NOCONSOLE")).Call(3).Get(TEXT("FOutputDeviceRedirector::AddOutputDevice")));

    AddOutputDevice(this, OutputDevice);
}
