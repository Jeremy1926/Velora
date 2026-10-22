#include "pch.h"
#include "Online/FortMcpProfile.h"
#include "Items/FortAccountItem.h"
#include "Scanner.h"

void UFortMcpProfile::GetItemInstancesForTemplate(const FString& Template, TArray<UFortAccountItem*>& OutInstances)
{
    static auto GetItemInstancesForTemplate = Anchor::Function<void (*)(UFortMcpProfile*, const FString&, TArray<UFortAccountItem*>&)>(TEXT("UFortMcpProfile::GetItemInstancesForTemplate<UFortAccountItem>"))
        .Exec(TEXT("/Script/FortniteGame.FortMcpContext:GetAccountItemWithDefinition"))
        .Find("call [rax+0x*]; mov r8, rax; mov rdx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    GetItemInstancesForTemplate(this, Template, OutInstances);
}
