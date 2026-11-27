#include "pch.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

bool FTimerManager::CanSetTimers()
{
    static auto bCanSetTimers = IDelegateInstance::GetLayout() >= 0 && Anchor::Exists()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Last("call 0x*")
        .Last("call 0x*")
        .IsTarget("mov rax, [rcx+0x*]; test rax, rax; jz 0x*; mov rax, [rax+0x*]; ret")
        .Or()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .FieldFind("mov *, [*+%x]; test *, *; jz 0x*; mov *, [*+0x*]; jmp 0x*", UWorld::StaticClass()->FindPropertyByName(FName(TEXT("OwningGameInstance"))))
        .Find("mov [rsp+0x20], *")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Last("call 0x*")
        .Last("call 0x*")
        .IsTarget("mov rax, [rcx+0x*]; test rax, rax; jz 0x*; mov rax, [rax+0x*]; ret")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .FieldFind("mov *, [*+%x]; test *, *; jz 0x*; mov *, [*+0x*]; jmp 0x*", UWorld::StaticClass()->FindPropertyByName(FName(TEXT("OwningGameInstance"))))
        .Find("mov [rsp+0x20], *")
        .Get();

    return bCanSetTimers;
}

void FTimerManager::InternalSetTimer(FTimerHandle& InOutHandle, FTimerUnifiedDelegate&& InDelegate, float InRate, bool InbLoop, float InFirstDelay)
{
    static auto InternalSetTimer = Anchor::Function<void (*)(FTimerManager*, FTimerHandle&, void*, float, bool, float)>(TEXT("FTimerManager::InternalSetTimer"))
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Target()
        .Get();
    static auto bTimerParameters = Anchor::Exists()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Is("mov [rsp+0x20], rax")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Is("mov [rsp+0x20], rax")
        .Get();
    static auto bVariantDelegate = Anchor::Exists()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; movzx eax, byte ptr [*]")
        .Or()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; *; movzx eax, byte ptr [*]")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; movzx eax, byte ptr [*]")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; *; movzx eax, byte ptr [*]")
        .Get();

    FTimerManagerTimerParameters TimerParameters;
    TimerParameters.bLoop = InbLoop;
    TimerParameters.FirstDelay = InFirstDelay;

    if (!bVariantDelegate)
    {
        if (bTimerParameters)
            ((void (*)(FTimerManager*, FTimerHandle&, FTimerUnifiedDelegate*, float, const FTimerManagerTimerParameters&))InternalSetTimer)(this, InOutHandle, &InDelegate, InRate, TimerParameters);
        else
            InternalSetTimer(this, InOutHandle, &InDelegate, InRate, InbLoop, InFirstDelay);

        return;
    }

    static auto TypeIndex = Anchor::Offset(TEXT("FTimerUnifiedDelegate::VariantDelegate TypeIndex"))
        .At((const void*)InternalSetTimer)
        .Find("movsd [*], xmm0; mov rdx, *")
        .Find("call 0x*")
        .Target()
        .Find("movzx eax, byte ptr [rdx+0x*]")
        .Or()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; movzx eax, byte ptr [rbp-0x*]; lea rcx, [rbp-0x*]")
        .Origin("lea rcx, [rbp-0x*]")
        .Find("movzx eax, byte ptr [rbp-0x*]")
        .Or()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; lea rcx, [rbp-0x*]; movzx eax, byte ptr [rbp-0x*]")
        .Origin("lea rcx, [rbp-0x*]")
        .Find("movzx eax, byte ptr [rbp-0x*]")
        .Or()
        .Native(TEXT("/Script/Engine.KismetSystemLibrary:K2_SetTimerDelegate"))
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; lea rdx, [0x*]; movzx eax, byte ptr [rbp-0x*]; lea rcx, [rbp-0x*]")
        .Origin("lea rcx, [rbp-0x*]")
        .Find("movzx eax, byte ptr [rbp-0x*]")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; movzx eax, byte ptr [rbp-0x*]; lea rcx, [rbp-0x*]")
        .Origin("lea rcx, [rbp-0x*]")
        .Find("movzx eax, byte ptr [rbp-0x*]")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; lea rcx, [rbp-0x*]; movzx eax, byte ptr [rbp-0x*]")
        .Origin("lea rcx, [rbp-0x*]")
        .Find("movzx eax, byte ptr [rbp-0x*]")
        .Or()
        .String(TEXT("SetTimer passed a bad function (%s) or object (%s)"))
        .Start()
        .Find("mov [rsp+0x20], *")
        .Find("call 0x*")
        .Is("call 0x*; lea rdx, [0x*]; movzx eax, byte ptr [rbp-0x*]; lea rcx, [rbp-0x*]")
        .Origin("lea rcx, [rbp-0x*]")
        .Find("movzx eax, byte ptr [rbp-0x*]")
        .Disp();

    if (TypeIndex < (int32)sizeof(FTimerDelegate) || TypeIndex >= (int32)sizeof(FTimerDelegateVariant))
        return;

    FTimerDelegateVariant VariantDelegate(InDelegate.FuncDelegate, TypeIndex);

    ((void (*)(FTimerManager*, FTimerHandle&, FTimerDelegateVariant*, float, const FTimerManagerTimerParameters&))InternalSetTimer)(this, InOutHandle, &VariantDelegate, InRate, TimerParameters);
}
