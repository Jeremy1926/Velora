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
        auto StoreAndForceNetUpdate = std::format("push rbx; sub rsp, 0x20; cmp byte ptr [rcx+0x*], 0x3; mov rbx, rcx; jnz *; mov rax, [rcx]; mov [rcx+{:#x}], rdx; call [rax+0x*]", EditActor);

        return Anchor::Slot(TEXT("AFortWeap_EditingTool::SetEditActor"))
            .Virtual(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), [&](const Anchor::FCode& Entry) { return Entry.Is(Store) || Entry.Is(StoreAndForceNetUpdate); })
            .Get();
    }();

    Scanner::Virtual<void (*)(AFortWeap_EditingTool*, ABuildingSMActor*)>(this, SetEditActor)(this, InEditActor);
}
