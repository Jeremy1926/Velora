#include "pch.h"
#include "UpdateManager.h"
#include "Scanner.h"

FOnUpdateCheckComplete& UUpdateManager::OnUpdateCheckComplete()
{
    static auto UpdateCheckCompleteDelegates = Anchor::Offset(TEXT("UUpdateManager::UpdateCheckCompleteDelegates"))
        .String(TEXT("CheckComplete %s"))
        .Start()
        .Find("lea rax, [0x*]; mov rcx, rbx")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Find("mov rcx, [r*]; add rcx, 0x*")
        .Find("add rcx, 0x*")
        .Or()
        .String(TEXT("CheckComplete %s"))
        .Start()
        .Find("lea rax, [0x*]; mov rcx, rbx")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Find("add rcx, 0x*; mov rbx, [rsp+0x*]; add rsp, 0x*; pop rdi; jmp 0x*")
        .Or()
        .String(TEXT("CheckComplete %s"))
        .Start()
        .Find("lea rax, [0x*]; mov rcx, rbx")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Find("lea rbx, [rax+0x*]; mov byte ptr [rax+0x*], *")
        .Or()
        .String(TEXT("CheckComplete %s"))
        .Start()
        .Find("lea rax, [0x*]; mov rcx, rbx")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Find("lea rsi, [rax+0x*]; mov byte ptr [rax+0x*], 0x0")
        .Get();

    return *(FOnUpdateCheckComplete*)((uint8*)this + UpdateCheckCompleteDelegates);
}
