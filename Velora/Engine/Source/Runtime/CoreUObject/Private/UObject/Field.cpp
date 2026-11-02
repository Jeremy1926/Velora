#include "pch.h"
#include "UObject/Field.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

const FField::FEncryptedLayout* FField::GetEncryptedLayout()
{
    if (!FUObjectArray::GetEncryptedLayout())
        return nullptr;

    static FEncryptedLayout Layout
    {
        Anchor::Cipher(TEXT("FField::Next"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
            .Find("test rax, rax; jz 0x*")
            .Find("jz 0x*")
            .Target()
            .Find("mov rax, [rcx+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("FField::ClassPrivate"))
            .String(TEXT("FWeakObjectProperty"))
            .Start()
            .Reference("call 0x*; mov r*, r*; mov r*, r*; call 0x*")
            .Find("call 0x*")
            .Target()
            .Find("mov r*, [rcx+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("FField::Owner"))
            .Exec(TEXT("/Script/GameplayAbilities.AbilitySystemBlueprintLibrary:EvaluateAttributeValueWithTagsAndBase"))
            .Find("call 0x*; test al, al; jz 0x*")
            .Call()
            .Find("call 0x*; test al, al; jnz 0x*")
            .Call()
            .Call()
            .Find("mov r*, [r*+0x*]; and r*, 0xfffffffffffffffe")
            .Get()
    };

    return &Layout;
}

const UProperty::FEncryptedLayout* UProperty::GetEncryptedLayout()
{
    if (!FUObjectArray::GetEncryptedLayout())
        return nullptr;

    static FEncryptedLayout Layout
    {
        Anchor::Cipher(TEXT("FProperty::ElementSize"))
            .Exec(TEXT("/Script/Engine.KismetArrayLibrary:Array_Add"))
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Find("call [rax+0x*]")
            .Find("mov e*, [r*+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("FProperty::Offset_Internal"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:IsValid"))
            .Find("test rax, rax; jz 0x*")
            .Find("jz 0x*")
            .Target()
            .Find("jnb 0x*")
            .Target()
            .Find("mov e*, [rcx+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("FBoolProperty::ByteOffset"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:SetBoolPropertyByName"))
            .Find("add [*+0x20], *")
            .Find("test rax, rax; jz 0x*")
            .Find("movzx e*, byte ptr [rax+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("FBoolProperty::ByteMask"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:SetBoolPropertyByName"))
            .Find("add [*+0x20], *")
            .Find("test rax, rax; jz 0x*")
            .Find("mov *l, [rax+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("FBoolProperty::FieldMask"))
            .Exec(TEXT("/Script/Engine.KismetSystemLibrary:SetBoolPropertyByName"))
            .Find("add [*+0x20], *")
            .Find("test rax, rax; jz 0x*")
            .Find("mov *l, [rax+0x*]")
            .Find("mov *l, [rax+0x*]")
            .Get(),
        Anchor::Cipher(TEXT("FArrayProperty::Inner"))
            .Exec(TEXT("/Script/Engine.KismetArrayLibrary:Array_Add"))
            .Find("add [*+0x20], *")
            .Find("call 0x*")
            .Target()
            .Find("mov rcx, [rdx+0x*]")
            .Get()
    };

    return &Layout;
}

bool UProperty::IsWeakObjectProperty() const
{
    static auto StaticClass = Anchor::Optional<FFieldClass* (*)()>(TEXT("FWeakObjectProperty::StaticClass"))
        .If(FField::GetEncryptedLayout() != nullptr)
        .String(TEXT("FWeakObjectProperty"))
        .Start()
        .Get();

    if (auto Layout = FField::GetEncryptedLayout())
        return StaticClass && Layout->ClassPrivate.Load<FFieldClass*>(this) == StaticClass();

    return FField::StaticClass() && TGameLayout<FName>::Load(*(const FName* const*)((const uint8*)this + sizeof(void*))) == FName(TEXT("WeakObjectProperty"));
}

bool UProperty::IsSoftObjectProperty() const
{
    static auto StaticClass = Anchor::Optional<FFieldClass* (*)()>(TEXT("FSoftObjectProperty::StaticClass"))
        .If(FField::GetEncryptedLayout() != nullptr)
        .String(TEXT("FSoftObjectProperty"))
        .Start()
        .Get();

    if (auto Layout = FField::GetEncryptedLayout())
        return StaticClass && Layout->ClassPrivate.Load<FFieldClass*>(this) == StaticClass();

    return FField::StaticClass() && TGameLayout<FName>::Load(*(const FName* const*)((const uint8*)this + sizeof(void*))) == FName(TEXT("SoftObjectProperty"));
}

FFieldClass* FField::StaticClass()
{
    static auto StaticClass = Anchor::Optional<FFieldClass* (*)()>(TEXT("FField::StaticClass"))
        .String(TEXT("FField"), "lea rdx, [0x*]; and qword ptr [rsp+0x20], 0x0; lea rcx, [0x*]; mov r9d, r8d; call 0x*")
        .Start()
        .Or()
        .If(FUObjectArray::GetEncryptedLayout() != nullptr)
        .String(TEXT("FField"), "lea *, [0x*]")
        .Smallest(TEXT("FField"))
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
        auto Layout = FField::GetEncryptedLayout();

        return Property && *(const UPTRINT*)(Layout ? (const uint8*)Property + Layout->Owner.Offset : (const uint8*)&Property->Owner) == ((UPTRINT)PrimaryAssetType | 1);
    }();

    return bTaggedPointer;
}
