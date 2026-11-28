#include "pch.h"
#include "UObject/Field.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

FFieldClass* FField::StaticClass()
{
    static auto StaticClass = Anchor::Optional<FFieldClass* (*)()>(TEXT("FField::StaticClass"))
        .String(TEXT("FField"), "lea rdx, [0x*]; and qword ptr [rsp+0x20], 0x0; lea rcx, [0x*]; mov r9d, r8d; call 0x*")
        .Start()
        .Or()
        .String(TEXT("FField"), "lea rdx, [0x*]; and qword ptr [rsp+0x20], 0x0; lea rcx, [0x*]; call 0x*")
        .Find("lea rcx, [0x*]")
        .Target()
        .Reference("lea rax, [0x*]; ret")
        .Start()
        .Or()
        .String(TEXT("FField"))
        .Start()
        .Or()
        .String(TEXT("FProperty"))
        .Last("call 0x*")
        .Target()
        .Get();

    return StaticClass ? StaticClass() : nullptr;
}

bool FField::IsClassEncoded()
{
    static auto bEncoded = Anchor::Exists()
        .String(TEXT("Attempted to access index {0} from array {1} of length {2}!"))
        .Start()
        .Find("call 0x*; cmp r*, rax")
        .Last("mov r*, [r*+0x*]")
        .Decoded()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetArrayLibrary:Array_Length"))
        .Find("mov eax, [rax+0x*]; bt rax, 0x*")
        .Last("mov r*, [r*+0x*]")
        .Decoded()
        .Get();

    return bEncoded;
}

FFieldClass* FField::GetClass() const
{
    static auto ClassField = Anchor::Field<FFieldClass*>(TEXT("FField::ClassPrivate"))
        .String(TEXT("Attempted to access index {0} from array {1} of length {2}!"))
        .Start()
        .Find("call 0x*; cmp r*, rax")
        .Last("mov r*, [r*+0x*]")
        .Decoded()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetArrayLibrary:Array_Length"))
        .Find("mov eax, [rax+0x*]; bt rax, 0x*")
        .Last("mov r*, [r*+0x*]")
        .Decoded()
        .Get();

    return ClassField ? ClassField(this) : ClassPrivate;
}

FField* FField::GetNext() const
{
    static auto NextField = Anchor::Field<FField*>(TEXT("FField::Next"))
        .String(TEXT("Divide by zero: Divide_DoubleDouble"))
        .Start()
        .Find("mov *, [r*]; mov *, [r*+0x*]")
        .Find("mov *, [r*+0x*]")
        .Decoded()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetSystemLibrary:PrintString"))
        .Find("mov rcx, [r*+0x88]; mov r*, [rcx+0x*]; test r*, r*")
        .Find("mov r*, [rcx+0x*]")
        .Decoded()
        .Get();

    if (NextField)
        return NextField(this);

    return FFieldVariant::IsTaggedPointer() ? ((const FTaggedOwnerField*)this)->Next : Next;
}

int32 FField::GetFieldSize()
{
    struct FOutlineNumberField
    {
        void* VTable;
        FFieldClass* ClassPrivate;
        FFieldVariant Owner;
        FField* Next;
        NAME_INDEX NamePrivate;
        EObjectFlags FlagsPrivate;
    };

    if (FFieldVariant::IsTaggedPointer())
        return (int32)sizeof(FTaggedOwnerField);

    return FName::IsOutlineNumber() ? (int32)sizeof(FOutlineNumberField) : (int32)sizeof(FField);
}

bool FFieldVariant::IsTaggedPointer()
{
    static auto bTaggedPointer = []
    {
        auto PrimaryAssetType = FField::StaticClass() ? (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CoreUObject.PrimaryAssetType")) : nullptr;
        auto Property = PrimaryAssetType ? (const FField*)PrimaryAssetType->GetPropertyLink() : nullptr;

        return Property && *(const UPTRINT*)&Property->Owner == ((UPTRINT)PrimaryAssetType | 1);
    }();

    return bTaggedPointer;
}

bool UProperty::IsWeakObjectProperty() const
{
    static auto WeakObjectPropertyClass = Anchor::Optional<FFieldClass*>(TEXT("FWeakObjectProperty::StaticClass"))
        .If(FField::IsClassEncoded())
        .String(TEXT("FWeakObjectProperty"))
        .Find("call 0x*")
        .Last("lea rcx, [0x*]")
        .Target()
        .Get();

    if (WeakObjectPropertyClass)
        return ((const FField*)this)->GetClass() == WeakObjectPropertyClass;

    return FField::StaticClass() && TGameLayout<FName>::Load(((const FField*)this)->GetClass()) == FName(TEXT("WeakObjectProperty"));
}

bool UProperty::IsSoftObjectProperty() const
{
    static auto SoftObjectPropertyClass = Anchor::Optional<FFieldClass*>(TEXT("FSoftObjectProperty::StaticClass"))
        .If(FField::IsClassEncoded())
        .String(TEXT("FSoftObjectProperty"))
        .Find("call 0x*")
        .Last("lea rcx, [0x*]")
        .Target()
        .Get();

    if (SoftObjectPropertyClass)
        return ((const FField*)this)->GetClass() == SoftObjectPropertyClass;

    return FField::StaticClass() && TGameLayout<FName>::Load(((const FField*)this)->GetClass()) == FName(TEXT("SoftObjectProperty"));
}

int32 UProperty::GetOffset_ForInternal() const
{
    static auto Offset_Internal = Anchor::Field<int32>(TEXT("FProperty::Offset_Internal"))
        .String(TEXT("'{Message}': Bad or missing property '{PropertyName}' when trying to call {FunctionName}"))
        .Start()
        .Find("movsxd r*, e*; add r*, r*; call [r*+0x*]")
        .Last("mov *, [r*+0x*]")
        .Decoded()
        .Or()
        .String(TEXT("'{Message}': Bad or missing property '{PropertyName}' when trying to call {FunctionName}"))
        .Start()
        .Find("movsxd r*, dword ptr [r*+0x*]; mov rax, [r*]; xor r*, 0x*; add r*, r*; call [r*+0x*]")
        .Decoded()
        .Get();

    return Offset_Internal ? Offset_Internal(this) : GetProperty()->Offset_Internal;
}

int32 UProperty::GetElementSize() const
{
    static auto ElementSizeField = Anchor::Field<int32>(TEXT("FProperty::ElementSize"))
        .String(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"))
        .Start()
        .Find("movsxd r*, dword ptr [r*+0x*]; imul r*, r*")
        .Last("mov e*, [r*+0x*]")
        .Decoded()
        .Or()
        .String(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"))
        .Start()
        .Find("movsxd r*, dword ptr [r*+0x*]; movsxd r*, dword ptr [r*+0x*]; xor r*, 0x*")
        .Decoded()
        .Get();

    return ElementSizeField ? ElementSizeField(this) : GetProperty()->ElementSize;
}

int32 UProperty::GetArrayDim() const
{
    static auto ArrayDimField = Anchor::Field<int32>(TEXT("FProperty::ArrayDim"))
        .String(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"))
        .Start()
        .Find("movsxd r*, dword ptr [r*+0x*]; imul r*, r*")
        .Last("mov e*, [r*+0x*]")
        .Decoded()
        .Find("movsxd r*, dword ptr [r*+0x*]")
        .Or()
        .String(TEXT("Attempted to access index %d from array '%s' of length %d in '%s'!"))
        .Start()
        .Find("movsxd r*, dword ptr [r*+0x*]; movsxd r*, dword ptr [r*+0x*]; xor r*, 0x*")
        .Decoded()
        .Find("movsxd r*, dword ptr [r*+0x*]")
        .Get();

    return ArrayDimField ? ArrayDimField(this) : GetProperty()->ArrayDim;
}

uint8 UBoolProperty::GetByteOffset() const
{
    static auto ByteOffsetField = Anchor::Field<uint8>(TEXT("FBoolProperty::ByteOffset"))
        .Pattern("0F B6 ? ? 8A 0C ? 84 ? ? 0F 95 44 24 20")
        .Last("mov e*, [r*+0x*]")
        .Decoded()
        .Find("movzx eax, byte ptr [r*+0x*]")
        .Or()
        .Pattern("0F B6 ? ? 8A 0C ? 84 ? ? 0F 95 44 24 20")
        .Last("movsxd r*, dword ptr [r*+0x*]")
        .Decoded()
        .Find("movzx eax, byte ptr [r*+0x*]")
        .Get();

    return ByteOffsetField ? ByteOffsetField(this) : GetProperty()->ByteOffset;
}

uint8 UBoolProperty::GetFieldMask() const
{
    static auto FieldMaskField = Anchor::Field<uint8>(TEXT("FBoolProperty::FieldMask"))
        .Pattern("0F B6 ? ? 8A 0C ? 84 ? ? 0F 95 44 24 20")
        .Last("mov e*, [r*+0x*]")
        .Decoded()
        .Find("test [r*+0x*], cl")
        .Or()
        .Pattern("0F B6 ? ? 8A 0C ? 84 ? ? 0F 95 44 24 20")
        .Last("movsxd r*, dword ptr [r*+0x*]")
        .Decoded()
        .Find("test [r*+0x*], cl")
        .Get();

    return FieldMaskField ? FieldMaskField(this) : GetProperty()->FieldMask;
}

uint8 UBoolProperty::GetByteMask() const
{
    static auto FieldMaskField = Anchor::Field<uint8>(TEXT("FBoolProperty::FieldMask"))
        .Pattern("0F B6 ? ? 8A 0C ? 84 ? ? 0F 95 44 24 20")
        .Last("mov e*, [r*+0x*]")
        .Decoded()
        .Find("test [r*+0x*], cl")
        .Or()
        .Pattern("0F B6 ? ? 8A 0C ? 84 ? ? 0F 95 44 24 20")
        .Last("movsxd r*, dword ptr [r*+0x*]")
        .Decoded()
        .Find("test [r*+0x*], cl")
        .Get();

    if (FieldMaskField)
        return FieldMaskField(this) == 0xFF ? 1 : FieldMaskField(this);

    return GetProperty()->ByteMask;
}

UProperty* UArrayProperty::GetInner() const
{
    static auto Inner = Anchor::Offset(TEXT("FArrayProperty::Inner"))
        .String(TEXT("TARRAY"))
        .Start()
        .Find("mov rcx, [rcx+0x*]")
        .Optional()
        .Disp();

    return Inner > 0 ? *(UProperty* const*)((const uint8*)this + Inner) : GetProperty()->Inner;
}

UProperty* UMapProperty::GetKeyProp() const
{
    static auto KeyProp = Anchor::Field<UProperty*>(TEXT("FMapProperty::KeyProp"))
        .Exec(TEXT("/Script/Engine.BlueprintMapLibrary:Map_Find"))
        .Find("test [r*], r*")
        .Last("mov r*, [r*+0x*]")
        .Decoded()
        .Find("mov r*, [r*+0x*]")
        .Get();

    return KeyProp ? KeyProp(this) : GetProperty()->KeyProp;
}
