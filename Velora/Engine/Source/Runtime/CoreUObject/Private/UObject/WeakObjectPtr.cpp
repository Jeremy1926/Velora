#include "pch.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

void FWeakObjectPtr::operator=(const UObject* Object)
{
    static auto Assign = Scanner::Function<void (*)(FWeakObjectPtr*, const UObject*)>(Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("add rcx, 0x*").Last("call 0x*").Target(TEXT("FWeakObjectPtr::operator=")));

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
