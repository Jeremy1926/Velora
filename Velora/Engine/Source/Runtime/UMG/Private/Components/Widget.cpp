#include "pch.h"
#include "Components/Widget.h"
#include "Scanner.h"

ULocalPlayer* UWidget::GetOwningLocalPlayer() const
{
    static auto GetOwningLocalPlayer = Anchor::VirtualFunction<ULocalPlayer* (*)(const UWidget*)>(TEXT("UWidget::GetOwningLocalPlayer"))
        .Exec(TEXT("/Script/UMG.Widget:GetOwningLocalPlayer"))
        .Find("call [rax+0x*]")
        .Get();

    return GetOwningLocalPlayer(this);
}
