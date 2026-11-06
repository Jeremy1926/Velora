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

        return Property && *(const UPTRINT*)&Property->Owner == ((UPTRINT)PrimaryAssetType | 1);
    }();

    return bTaggedPointer;
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
