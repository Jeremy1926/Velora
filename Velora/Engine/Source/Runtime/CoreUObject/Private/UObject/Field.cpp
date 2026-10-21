#include "pch.h"
#include "UObject/Field.h"
#include "Scanner.h"

FFieldClass* FField::StaticClass()
{
    static auto StaticClass = Anchor::Optional<FFieldClass* (*)()>(TEXT("FField::StaticClass"))
        .String(TEXT("FField"))
        .Start()
        .Get();

    return StaticClass ? StaticClass() : nullptr;
}
