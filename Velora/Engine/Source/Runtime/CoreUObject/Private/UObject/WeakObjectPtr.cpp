#include "pch.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"

void FWeakObjectPtr::operator=(const UObject* Object)
{
    static auto Assign = Anchor::Function<void (*)(FWeakObjectPtr*, const UObject*)>(TEXT("FWeakObjectPtr::operator="))
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s*"))
        .Start()
        .Find("add rcx, 0x*")
        .Last("call 0x*")
        .Target()
        .Or()
        .Pattern("40 53 48 83 EC 20 48 8B D9 48 85 D2 74 ? 8B 52 0C 89 11 48 8D 0D ? ? ? ? E8 ? ? ? ? 89 43 04")
        .Or()
        .Pattern("48 85 D2 0F 84 ? ? ? ? 8B 42 0C 89 01 3B 05")
        .Start()
        .Or()
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s*"))
        .Start()
        .Find("mov *[*], *; mov rcx, *; call 0x*")
        .Last("call 0x*")
        .Call()
        .Call()
        .Get();

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
