#include "pch.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/UObjectArray.h"
#include "UObject/Object.h"
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

    static auto MasterSerialNumber = Anchor::Data<volatile long*>(TEXT("FUObjectArray::MasterSerialNumber"))
        .Pattern("F0 0F C1 0D ? ? ? ? FF C1 89 4D ? 81 F9 E8 03 00 00")
        .Target()
        .Get();

    ObjectIndex = (int32)Object->GetUniqueID();

    auto ObjectItem = GUObjectArray.IndexToObject(ObjectIndex);
    auto SerialNumber = ObjectItem->SerialNumber;

    if (!SerialNumber && MasterSerialNumber)
    {
        SerialNumber = _InterlockedIncrement(MasterSerialNumber);

        if (auto ValueWas = _InterlockedCompareExchange((volatile long*)&ObjectItem->SerialNumber, SerialNumber, 0))
            SerialNumber = ValueWas;
    }

    ObjectSerialNumber = SerialNumber;
}

UObject* FWeakObjectPtr::Get() const
{
    return Internal_Get(false);
}

bool FWeakObjectPtr::IsValid() const
{
    return Internal_IsValid(false, false);
}
