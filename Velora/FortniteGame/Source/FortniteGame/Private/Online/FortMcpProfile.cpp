#include "pch.h"
#include "Online/FortMcpProfile.h"
#include "Items/FortAccountItem.h"
#include "Scanner.h"

void UFortMcpProfile::GetItemInstancesForTemplate(const FString& Template, TArray<UFortAccountItem*>& OutInstances)
{
    static auto GetItemInstancesForTemplateView = Anchor::Optional<void (*)(UFortMcpProfile*, const FStringView&, TArray<UFortAccountItem*>&)>(TEXT("UFortMcpProfile::GetItemInstancesForTemplate<UFortAccountItem>"))
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov [rbp-0x*], *; mov rsi, rax")
        .Find("lea r8, [rbp-0x*]; mov [rbp-0x*], edx; lea rdx, [rbp-0x*]; movaps xmm0, [rbp-0x*]; movdqa [rbp-0x*], xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("movaps xmm0, [rsp+0x*]; movdqa [rsp+0x*], xmm0; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (GetItemInstancesForTemplateView)
    {
        GetItemInstancesForTemplateView(this, FStringView(Template), OutInstances);
        return;
    }

    static auto GetItemInstancesForTemplate = Anchor::Function<void (*)(UFortMcpProfile*, const FString&, TArray<UFortAccountItem*>&)>(TEXT("UFortMcpProfile::GetItemInstancesForTemplate<UFortAccountItem>"))
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]")
        .Find("lea r8, [rbp-0x*]; mov rdx, rax; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    GetItemInstancesForTemplate(this, Template, OutInstances);
}
