#include "pch.h"
#include "UObject/UObjectArray.h"
#include "Scanner.h"

FUObjectArray& GUObjectArray = *(FUObjectArray*)(Anchor::Data<uint8_t*>(TEXT("GUObjectArray.ObjObjects"))
    .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
    .Last("cmp *, [0x*]")
    .Find("mov *, [0x*]")
    .Target()
    .Or()
    .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
    .Start()
    .Find("cmp *, [0x*]; jnl *")
    .Find("mov *, [0x*]")
    .Target()
    .Or()
    .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
    .Start()
    .Find("cmp e*, e*; jnl 0x*")
    .Find("mov r*, [0x*]")
    .Decoded()
    .Target()
    .Get() - offsetof(FUObjectArray, ObjObjects));

int32 FUObjectArray::GetObjectArrayNum() const
{
    static auto NumElements = Anchor::Field<int32>(TEXT("FChunkedFixedUObjectArray::NumElements"))
        .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
        .Start()
        .Find("cmp e*, e*; jnl 0x*")
        .Last("mov *, [0x*]")
        .Decoded()
        .Get(&ObjObjects);

    if (NumElements)
        return NumElements(&ObjObjects);

    auto& Version = FEngineVersion::Current();

    return Version.GetMajor() > 4 || Version.GetMinor() >= 21 ? ((const FChunkedFixedUObjectArray&)ObjObjects).Num() : ObjObjects.Num();
}

FUObjectItem* FUObjectArray::IndexToObject(int32 Index)
{
    static auto Objects = Anchor::Field<FUObjectItem**>(TEXT("FChunkedFixedUObjectArray::Objects"))
        .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
        .Start()
        .Find("cmp e*, e*; jnl 0x*")
        .Find("mov r*, [0x*]")
        .Decoded()
        .Get(&ObjObjects);

    static auto ChunkShift = Anchor::Offset(TEXT("FChunkedFixedUObjectArray::NumElementsPerChunk shift"))
        .If(!!Objects)
        .String(TEXT("%s failed to route PostInitializeComponents.  Please call Super::PostInitializeComponents() in your <className>::PostInitializeComponents() function. "))
        .Start()
        .Find("cmp e*, e*; jnl 0x*")
        .Find("shr r*, 0x*")
        .Optional()
        .Imm();

    if (Index >= GetObjectArrayNum())
        return nullptr;

    if (Objects && ChunkShift)
        return Objects(&ObjObjects)[Index >> ChunkShift] + (Index & ((1 << ChunkShift) - 1));

    auto& Version = FEngineVersion::Current();

    return const_cast<FUObjectItem*>(Version.GetMajor() > 4 || Version.GetMinor() >= 21 ? &((const FChunkedFixedUObjectArray&)ObjObjects)[Index] : &ObjObjects[Index]);
}

int32 FUObjectArray::AllocateSerialNumber(int32 Index)
{
    static auto MasterSerialNumber = Anchor::Data<volatile long*>(TEXT("FUObjectArray::MasterSerialNumber"))
        .Pattern("F0 0F C1 0D ? ? ? ? FF C1 89 4D ? 81 F9 E8 03 00 00")
        .Target()
        .Or()
        .Pattern("F0 0F C1 1D ? ? ? ? FF C3 81 FB E8 03 00 00")
        .Target()
        .Get();

    static auto SerialNumberField = Anchor::Field<int32>(TEXT("FUObjectItem::SerialNumber"))
        .String(TEXT("UObject serial numbers overflowed (trying to allocate serial number %d)."))
        .Start()
        .Find("cmp e*, e*; jnl 0x*")
        .Find("mov r*, [0x*]")
        .Decoded()
        .Find("mov e*, [r*+0x*]; test e*, e*")
        .Or()
        .String(TEXT("UObject serial numbers overflowed (trying to allocate serial number %d)."))
        .Last("mov r*, [0x*]")
        .Decoded()
        .Find("mov e*, [r*+0x*]; test e*, e*")
        .Get();

    auto SerialNumberPtr = (volatile long*)((uint8*)IndexToObject(Index) + (SerialNumberField ? SerialNumberField.Offset : (int32)offsetof(FUObjectItem, SerialNumber)));
    auto SerialNumber = (int32)*SerialNumberPtr;

    if (!SerialNumber && MasterSerialNumber)
    {
        SerialNumber = _InterlockedIncrement(MasterSerialNumber);

        if (auto ValueWas = _InterlockedCompareExchange(SerialNumberPtr, SerialNumber, 0))
            SerialNumber = ValueWas;
    }

    return SerialNumber;
}

int32 FUObjectItem::GetSerialNumber() const
{
    static auto SerialNumberField = Anchor::Field<int32>(TEXT("FUObjectItem::SerialNumber"))
        .String(TEXT("UObject serial numbers overflowed (trying to allocate serial number %d)."))
        .Start()
        .Find("cmp e*, e*; jnl 0x*")
        .Find("mov r*, [0x*]")
        .Decoded()
        .Find("mov e*, [r*+0x*]; test e*, e*")
        .Or()
        .String(TEXT("UObject serial numbers overflowed (trying to allocate serial number %d)."))
        .Last("mov r*, [0x*]")
        .Decoded()
        .Find("mov e*, [r*+0x*]; test e*, e*")
        .Get();

    return SerialNumberField ? SerialNumberField(this) : SerialNumber;
}

UObjectBase* FUObjectItem::GetObject() const
{
    static auto ObjectField = Anchor::Field<UObjectBase*>(TEXT("FUObjectItem::Object"))
        .String(TEXT("'{Message}': Bad or missing property '{PropertyName}' when trying to call {FunctionName}"))
        .Start()
        .Find("test e*, e*; js 0x*; mov *, [r*")
        .Find("mov *, [r*")
        .Decoded()
        .Get();

    return ObjectField ? ObjectField(this) : Object;
}

int32 FUObjectItem::GetFlags() const
{
    static auto FlagsField = Anchor::Field<int32>(TEXT("FUObjectItem::Flags"))
        .String(TEXT("STAT_AllocateObject"))
        .Start()
        .Find("cmp *, e*; jnl 0x*")
        .Find("mov r*, [0x*]")
        .Decoded()
        .Find("mov e*, [r*+0x*]; mov [rsp+0x*], e*; mov e*, [r*+0x*]")
        .Get();

    return FlagsField ? FlagsField(this) : Flags;
}

void FUObjectItem::SetRootSet()
{
    static auto FlagsField = Anchor::Field<int32>(TEXT("FUObjectItem::Flags"))
        .String(TEXT("STAT_AllocateObject"))
        .Start()
        .Find("cmp *, e*; jnl 0x*")
        .Find("mov r*, [0x*]")
        .Decoded()
        .Find("mov e*, [r*+0x*]; mov [rsp+0x*], e*; mov e*, [r*+0x*]")
        .Get();

    *(int32*)((uint8*)this + (FlagsField ? FlagsField.Offset : (int32)offsetof(FUObjectItem, Flags))) |= int32(EInternalObjectFlags::RootSet);
}

bool FUObjectItem::IsPendingKill() const
{
    static auto RefCountedBit = Anchor::Offset(TEXT("EInternalObjectFlags::RefCounted bit"))
        .Pattern("F0 FF 41 14 8B 41 08 0F BA E0 ?")
        .Find("bt eax, 0x*")
        .Optional()
        .Imm();

    return !!(GetFlags() & int32(EInternalObjectFlags::PendingKill) & ~(RefCountedBit ? 1 << RefCountedBit : 0));
}
