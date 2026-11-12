#include "pch.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/AssetPtr.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "Templates/TypeCompatibleBytes.h"
#include "Scanner.h"

FString FSoftObjectPath::ToString() const
{
    if (!IsValid())
        return FString();

    FString FullPathString = AssetPathName.ToString();

    if (!SubPathString.IsEmpty())
    {
        FullPathString += TEXT(":");
        FullPathString += SubPathString;
    }

    return FullPathString;
}

FString FSoftObjectPath::GetLongPackageName() const
{
    FString PathName = AssetPathName.ToString();
    FString PackageName;

    if (auto Delimiter = wcschr(*PathName, TEXT('.')))
        PackageName.AppendChars(*PathName, (int32)(Delimiter - *PathName));

    return PackageName;
}

UObject* FSoftObjectPath::TryLoad() const
{
    UObject* LoadedObject = nullptr;

    if (IsValid())
        LoadedObject = LoadObject<UObject>(nullptr, *ToString());

    return LoadedObject;
}

FSoftObjectPath FSoftObjectPath::GetOrCreateIDForObject(const UObject* Object)
{
    static auto Construct = Anchor::Function<FSoftObjectPath* (*)(FSoftObjectPath*, const UObject*)>(TEXT("FSoftObjectPath::FSoftObjectPath"))
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:GetSoftObjectPath"))
        .Find("add [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("mov rdx, *; lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("test rbx, rbx; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("mov rdx, *; lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("add [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rdx, *; lea rcx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Call()
        .Get();

    TAlignedBytes<sizeof(FSoftObjectPath), alignof(FSoftObjectPath)> ResultBytes;
    auto NativeResult = Construct((FSoftObjectPath*)&ResultBytes, Object);

    FSoftObjectPath Result = *NativeResult;
    NativeResult->~FSoftObjectPath();

    return Result;
}

int32 FSoftObjectPath::GetCurrentTag()
{
    static auto CurrentTag = Anchor::Data<volatile long*>(TEXT("FSoftObjectPath::CurrentTag"))
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("lock cmpxchg [0x*], *")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("test rbx, rbx; jnz 0x*")
        .Find("jnz 0x*")
        .Target()
        .Find("lock cmpxchg [0x*], *")
        .Target()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("add [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lock cmpxchg [0x*], *")
        .Target()
        .Get();

    return _InterlockedCompareExchange(CurrentTag, 0, 0);
}

template<>
bool TPersistentObjectPtr<FSoftObjectPath>::HasTagAtLastTest()
{
    static auto bTagAtLastTest = !Anchor::Exists()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("add [*+0x20], *; call 0x*; mov rcx, [rax]; lea rdx, [rax+0x8]")
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("lea rcx, [rsi+0x10]; *; *; mov [rsi+0x8], r*; call 0x*")
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:Conv_ObjectToSoftObjectReference"))
        .Find("lea rcx, [rsi+0x10]")
        .Find("mov [rsi+0x8], r*")
        .Get();

    return bTagAtLastTest;
}
