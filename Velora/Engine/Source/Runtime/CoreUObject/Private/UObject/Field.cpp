#include "pch.h"
#include "UObject/Field.h"
#include "Scanner.h"

FFieldClass* FField::StaticClass()
{
    static auto StaticClass = Anchor::Optional<FFieldClass* (*)()>(TEXT("FField::StaticClass"))
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

    return FName::IsOutlineNumber() ? (int32)sizeof(FOutlineNumberField) : (int32)sizeof(FField);
}
