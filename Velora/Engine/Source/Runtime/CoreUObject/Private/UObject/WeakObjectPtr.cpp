#include "pch.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

void FWeakObjectPtr::operator=(const UObject* Object)
{
    static auto Assign = Scanner::Function<void (*)(FWeakObjectPtr*, const UObject*)>([]
    {
        auto Close = Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start();
        auto ActorChannels = Close.Find("add rcx, 0x*");

        return ActorChannels.Get() ? ActorChannels.Last("call 0x*").Target(TEXT("FWeakObjectPtr::operator=")) : Close.Find("mov qword ptr [*], 0x0").Last("call 0x*").Call().Call().Get(TEXT("FWeakObjectPtr::operator="));
    }());

    Assign(this, Object);
}

UObject* FWeakObjectPtr::Get() const
{
    return Internal_Get(false);
}

bool FWeakObjectPtr::IsValid() const
{
    return Internal_IsValid(false, false);
}
