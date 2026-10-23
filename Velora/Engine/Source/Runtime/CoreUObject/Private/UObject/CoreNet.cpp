#include "pch.h"
#include "UObject/CoreNet.h"
#include "HAL/UnrealMemory.h"
#include "Templates/UnrealTemplate.h"
#include "Scanner.h"

bool UPackageMap::WriteObject(FArchive& Ar, UObject* InOuter, FNetworkGUID NetGUID, FString ObjName)
{
    static auto WriteObject = Anchor::VirtualFunction<bool (*)(UPackageMap*, FArchive&, UObject*, FNetworkGUID, FString)>(TEXT("UPackageMapClient::WriteObject"))
        .String(TEXT("WroteObject %s NetGUID <%s>"))
        .Start()
        .Log(TEXT("UPackageMapClient::WriteObject"))
        .SlotIn(this)
        .Get();

    return WriteObject(this, Ar, InOuter, NetGUID, MoveTemp(ObjName));
}

const FFieldNetCache* FClassNetCache::GetFromField(FFieldVariant Field) const
{
    static auto GetFromField = Anchor::Function<const FFieldNetCache* (*)(const FClassNetCache*, FFieldVariant)>(TEXT("FClassNetCache::GetFromField"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; cmovz *")
        .Find("call 0x*", 2)
        .Target()
        .Get();

    return GetFromField(this, Field);
}

const FClassNetCache* FClassNetCacheMgr::GetClassNetCache(UClass* Class)
{
    static auto GetClassNetCache = Anchor::Function<const FClassNetCache* (*)(FClassNetCacheMgr*, UClass*)>(TEXT("FClassNetCacheMgr::GetClassNetCache"))
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; cmovz *")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetClassNetCache(this, Class);
}

int32 FNetBitWriter::GetTypeSize()
{
    static auto PackageMap = Anchor::Offset(TEXT("FNetBitWriter::PackageMap"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Call()
        .Find("mov [rdi+0x*], rbx")
        .Disp();

    return PackageMap + (int32)sizeof(UPackageMap*);
}

void* FNetBitWriter::operator new(size_t Size)
{
    return FMemory::Realloc(nullptr, GetTypeSize());
}

void FNetBitWriter::operator delete(void* Ptr)
{
    FMemory::Free(Ptr);
}

FNetBitWriter::FNetBitWriter(UPackageMap* InPackageMap, int64 InMaxBits)
{
    static auto Constructor = Anchor::Function<FNetBitWriter* (*)(FNetBitWriter*, UPackageMap*, int64)>(TEXT("FNetBitWriter::FNetBitWriter"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Call()
        .Get();

    Constructor(this, InPackageMap, InMaxBits);
}

FNetBitWriter::~FNetBitWriter()
{
    (**(void (***)(FNetBitWriter*, uint32))this)(this, 0);
}
