#include "pch.h"
#include "Weapons/FortWeap_EditingTool.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortWeap_EditingTool::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeap_EditingTool"));

    return Class;
}

void AFortWeap_EditingTool::SetEditActor(ABuildingSMActor* InEditActor)
{
    static auto SetEditActor = []
    {
        auto EditActor = StaticClass()->FindPropertyByName(FName(TEXT("EditActor")))->GetOffset_ForInternal();
        auto Store = std::format("cmp byte ptr [rcx+0x*], 0x3; jnz *; mov [rcx+{:#x}], rdx; jmp 0x*", EditActor);
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"));
        auto Index = 0;

        while (!Scanner::At(VTable[Index]).Is(Store))
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortWeap_EditingTool*, ABuildingSMActor*))(VTable + SetEditActor))(this, InEditActor);
}
