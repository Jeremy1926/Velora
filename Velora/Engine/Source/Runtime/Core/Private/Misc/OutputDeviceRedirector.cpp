#include "pch.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Scanner.h"

void FOutputDeviceRedirector::AddOutputDevice(FOutputDevice* OutputDevice)
{
    static auto AddOutputDevice = Anchor::Optional<void (*)(FOutputDeviceRedirector*, FOutputDevice*)>(TEXT("FOutputDeviceRedirector::AddOutputDevice"))
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

    if (AddOutputDevice)
    {
        AddOutputDevice(this, OutputDevice);
        return;
    }

    static auto State = Anchor::Offset(TEXT("FOutputDeviceRedirector::State"))
        .String(TEXT("%s/profilego/%s.%s"))
        .Start()
        .Find("mov rdx, [*]; test rdx, rdx; jz *; mov rcx, [rax+0x*]; call 0x*")
        .Find("mov rcx, [rax+0x*]")
        .Get();

    static auto AddOutputDeviceToState = Anchor::Function<void (*)(void*, FOutputDevice*)>(TEXT("FOutputDeviceRedirectorState::AddOutputDevice"))
        .String(TEXT("%s/profilego/%s.%s"))
        .Start()
        .Find("mov rdx, [*]; test rdx, rdx; jz *; mov rcx, [rax+0x*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (OutputDevice)
        AddOutputDeviceToState(*(void**)((uint8*)this + State), OutputDevice);
}
