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
        .Find("lea rdx, [*]; lea rcx, [*]; call 0x*; mov rdx, rax; mov rcx, r*; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("test *, *; mov rcx, [*+0x*]; cmovz *")
        .Find("lea rcx, [*]; lea rdx, [*]; call 0x*; mov rcx, r*; mov rdx, rax; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; cmovz *")
        .Find("lea rcx, [*]; lea rdx, [*]; call 0x*; mov rcx, r*; mov rdx, rax; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; cmovz *")
        .Find("call 0x*", 2)
        .Target()
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; mov [rsp+0x*], *; mov [rsp+0x*], *; mov *, *; cmovnz *")
        .Find("call 0x*", 2)
        .Target()
        .Get();

    static auto bVariantByReference = Anchor::Exists()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; cmovz *")
        .Find("lea rdx, [*]; lea rcx, [*]; call 0x*; mov rdx, rax; mov rcx, r*; call 0x*")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("test *, *; mov rcx, [*+0x*]; cmovz *")
        .Find("lea rcx, [*]; lea rdx, [*]; call 0x*; mov rcx, r*; mov rdx, rax; call 0x*")
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; cmovz *")
        .Find("lea rcx, [*]; lea rdx, [*]; call 0x*; mov rcx, r*; mov rdx, rax; call 0x*")
        .Get();

    if (bVariantByReference)
    {
        auto TaggedField = Field.ToTaggedPointer();

        return ((const FFieldNetCache* (*)(const FClassNetCache*, const UPTRINT*))GetFromField)(this, &TaggedField);
    }

    if (FFieldVariant::IsTaggedPointer())
        return ((const FFieldNetCache* (*)(const FClassNetCache*, UPTRINT))GetFromField)(this, Field.ToTaggedPointer());

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
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("test *, *; mov rcx, [*+0x*]; cmovz *")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Can't send function '%s' on actor '%s' because client hasn't loaded the level '%s' containing it"))
        .Start()
        .Find("mov rcx, [*+0x*]; test *, *; mov [rsp+0x*], *; mov [rsp+0x*], *; mov *, *; cmovnz *")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetClassNetCache(this, Class);
}

int32 FNetBitWriter::GetTypeSize()
{
    static auto InlinedPackageMap = Anchor::Offset(TEXT("FNetBitWriter::PackageMap"))
        .String(TEXT("UChannel::Close: Sending CloseBunch. ChIndex == 0. Name: %s"))
        .Start()
        .Find("mov r8b, 0x1")
        .Call()
        .Find("call 0x*; xor ecx, ecx; mov [r14+0x*], rbx")
        .Find("mov [r14+0x*], rbx")
        .Optional()
        .Disp();

    if (InlinedPackageMap > 0)
        return InlinedPackageMap + (int32)sizeof(UPackageMap*);

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
    static auto VTable = Anchor::Data<void*>(TEXT("FNetBitWriter::vftable"))
        .Pattern("B9 ? ? ? ? E8 ? ? ? ? 48 8B D8 48 85 C0 74 ? 41 B0 01 33 D2 48 8B C8 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 C7 83 ? ? ? ? 00 00 00 00 48 89 03")
        .Find("lea rax, [0x*]")
        .Target()
        .Optional()
        .Get();

    if (VTable)
    {
        static auto BitWriterConstructor = Anchor::Function<void* (*)(void*, int64, bool)>(TEXT("FBitWriter::FBitWriter"))
            .Pattern("B9 ? ? ? ? E8 ? ? ? ? 48 8B D8 48 85 C0 74 ? 41 B0 01 33 D2 48 8B C8 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 C7 83 ? ? ? ? 00 00 00 00 48 89 03")
            .Find("mov rcx, rax; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get();

        BitWriterConstructor(this, InMaxBits, true);
        *(UPackageMap**)((uint8*)this + GetTypeSize() - sizeof(UPackageMap*)) = InPackageMap;
        *(void**)this = VTable;

        return;
    }

    static auto Constructor =Anchor::Function<FNetBitWriter* (*)(FNetBitWriter*, UPackageMap*, int64)>(TEXT("FNetBitWriter::FNetBitWriter"))
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
