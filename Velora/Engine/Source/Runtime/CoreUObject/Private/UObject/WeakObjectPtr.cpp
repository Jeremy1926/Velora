#include "pch.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/UObjectArray.h"
#include "UObject/Object.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

void FWeakObjectPtr::operator=(const UObject* Object)
{
    static auto Assign = Anchor::Optional<void (*)(FWeakObjectPtr*, const UObject*)>(TEXT("FWeakObjectPtr::operator="))
        .String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s*"))
        .Start()
        .Find("add rcx, 0x*")
        .Last("call 0x*")
        .Target()
        .Or()
        .Pattern("40 53 48 83 EC 20 48 8B D9 48 85 D2 74 ? 8B 52 0C 89 11 48 8D 0D ? ? ? ? E8 ? ? ? ? 89 43 04")
        .Or()
        .If(FEngineVersion::Current().GetMajor() > 4)
        .Pattern("48 89 5C 24 08 57 48 83 EC 20 48 8B D9 48 85 D2 74 ? 8B 42 0C 89 01 3B 05")
        .Or()
        .Pattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 8B F9 48 85 D2 74 ? 8B 42 0C 89 01 3B 05")
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
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortMissionLibrary:SpawnMissionItemPickup"))
        .Find("mov [rsp+0x20], rcx; lea rcx, [*]; call 0x*; lea rcx, [*]; call 0x*; lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("and dword ptr [rcx+0x4], 0x0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Has("mov [rcx], edx")
        .Get();

    if (Assign)
    {
        Assign(this, Object);

        return;
    }

    if (!Object)
    {
        ObjectIndex = -1;
        ObjectSerialNumber = 0;

        return;
    }

    ObjectIndex = (int32)Object->GetUniqueID();
    ObjectSerialNumber = GUObjectArray.AllocateSerialNumber(ObjectIndex);
}

UObject* FWeakObjectPtr::Get() const
{
    return Internal_Get(false);
}

UObject* FWeakObjectPtr::Get(bool bEvenIfPendingKill) const
{
    return Internal_Get(bEvenIfPendingKill);
}

bool FWeakObjectPtr::IsValid() const
{
    return Internal_IsValid(false, false);
}
