#include "pch.h"
#include "Online/FortMcpProfile.h"
#include "Items/FortAccountItem.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectGlobals.h"
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
    static auto bInlinedGetItemInstancesForTemplate = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rdx+0x*]; mov rdx, r8; mov rax, [rcx]; call [rax+0x*]")
        .Get();

    if (!bInlinedGetItemInstancesForTemplate)
    {
        GetItemInstancesForTemplate(this, Template, OutInstances);
        return;
    }

    static auto ItemStore = Anchor::Offset(TEXT("UMcpProfile::ItemStore"))
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rdx+0x*]; mov rdx, r8; mov rax, [rcx]; call [rax+0x*]")
        .Disp();

    static auto FindItemsByTemplate = Anchor::VirtualFunction<const TArray<TSharedPtr<uint8>>* (*)(void*, const FString&)>(TEXT("UMcpProfile::ItemStore::FindItemsByTemplate"))
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [rdx+0x*]; mov rdx, r8; mov rax, [rcx]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    static auto ItemInstance = Anchor::Offset(TEXT("UMcpProfile::ItemStore item Instance"))
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("mov rax, [rbx]; mov rsi, [rax+0x*]; test rsi, rsi")
        .Find("mov rsi, [rax+0x*]")
        .Disp();

    static auto AccountItemClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAccountItem"));

    OutInstances.Empty();

    auto Items = FindItemsByTemplate(*(void**)((uint8*)this + ItemStore), Template);

    if (!Items)
        return;

    for (auto& Item : *Items)
    {
        auto Instance = *(UFortAccountItem**)(Item.Get() + ItemInstance);

        if (Instance && Instance->IsA(AccountItemClass) && Instance->GetItemDefinitionBP())
            OutInstances.Add(Instance);
    }
}
