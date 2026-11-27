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
    static auto SetEditActor = Anchor::VirtualFunction<void (*)(AFortWeap_EditingTool*, ABuildingSMActor*)>(TEXT("AFortWeap_EditingTool::SetEditActor"))
        .FieldVirtual(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), "cmp byte ptr [rcx+0x*], 0x3; jnz *; mov [rcx+%x], rdx; jmp 0x*", StaticClass()->FindPropertyByName(FName(TEXT("EditActor"))))
        .Or()
        .FieldVirtual(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), "push rbx; sub rsp, 0x20; cmp byte ptr [rcx+0x*], 0x3; mov rbx, rcx; jnz *; mov rax, [rcx]; mov [rcx+%x], rdx; call [rax+0x*]", StaticClass()->FindPropertyByName(FName(TEXT("EditActor"))))
        .Or()
        .FieldVirtual(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), "push rbx; sub rsp, 0x20; cmp byte ptr [rcx+0x*], 0x3; mov rbx, rcx; jnz *; add rcx, %x; call 0x*", StaticClass()->FindPropertyByName(FName(TEXT("EditActor"))))
        .Or()
        .FieldVirtual(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), "push rbx; add rsp, 0x*; cmp byte ptr [rcx+0x*], 0x3; mov rbx, rcx; jnz *; add rcx, %x; call 0x*", StaticClass()->FindPropertyByName(FName(TEXT("EditActor"))))
        .Or()
        .FieldVirtual(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), "push rbx; *; mov rbx, rcx; cmp byte ptr [rcx+0x*], 0x3; jnz *; add rcx, %x; call 0x*", StaticClass()->FindPropertyByName(FName(TEXT("EditActor"))))
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), "push rbx; *; mov rbx, rcx; cmp byte ptr [rcx+0x*], 0x3; jnz *; sub rcx, *; call 0x*; mov rcx, rbx")
        .Or()
        .VirtualMatch(TEXT("/Script/FortniteGame.Default__FortWeap_EditingTool"), "push rbx; *; cmp byte ptr [rcx+0x*], 0x3; mov rbx, rcx; jnz *; sub rcx, *; call 0x*; mov rcx, rbx")
        .Get();

    SetEditActor(this, InEditActor);
}
