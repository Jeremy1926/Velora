#include "pch.h"
#include "Movement/FortMovementComp_Character.h"
#include "Abilities/FortMovementSet.h"
#include "Pawns/FortPawn.h"
#include "HAL/IConsoleManager.h"
#include "Templates/SharedPointer.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

float UFortMovementComp_Character::GetMaxSpeed() const
{
    DETOUR(UFortMovementComp_Character, GetMaxSpeed, Anchor::Hook()
        .If(!UFortMovementSet::StaticClass()->FindPropertyByName(FName(TEXT("FlySpeed"))))
        .Exec(TEXT("/Script/Engine.MovementComponent:GetMaxSpeed"))
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"))
        .Inline());

    if (GetMovementMode() == MOVE_Flying && GetCheatFlying())
        return GetMaxFlySpeed();

    return Original();
}

void UFortMovementComp_Character::ServerMove_PerformMovement(const FCharacterNetworkMoveData& MoveData)
{
    DETOUR(UFortMovementComp_Character, ServerMove_PerformMovement, Anchor::Hook()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const ::Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x94]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("add rsp, 0x*; pop r15; pop r14; pop *; jmp 0x*")
        .Find("jmp 0x*")
        .Optional()
        .AtCall());

    ServerUpdateReceivedPackMovementModeExtension();

    Original(MoveData);
}

void UFortMovementComp_Character::ServerUpdateReceivedPackMovementModeExtension()
{
    static auto ReceivedPackRuntimeData = Anchor::Offset(TEXT("UFortMovementComp_Character::ReceivedPackMovementModeExtensionRuntimeData"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x94]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x94]")
        .Find("lea rcx, [rbx+0x*]")
        .Disp();

    static auto PushedRuntimeData = Anchor::Offset(TEXT("UFortMovementComp_Character::PushedMovementModeExtensionRuntimeData"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x94]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x94]")
        .Find("mov rax, [rbx+0x*]")
        .Disp();

    static auto NewlyPushedRuntimeData = Anchor::Offset(TEXT("UFortMovementComp_Character::NewlyPushedMovementModeExtensions"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x94]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x94]")
        .Find("lea r14, [rbx+0x*]")
        .Disp();

    static auto CreateRuntimeDataForLogic = Anchor::Optional<void* (*)(AFortPawn*, TSharedPtr<FFortMovementMode_BaseExtRuntimeData, ESPMode::ThreadSafe>*, const int32*, uint8)>(TEXT("AFortPawn::CreateRuntimeDataForLogic"))
        .Pattern("45 8B 00 48 8B DA 48 8D 54 24 ? 41 8A F9 E8")
        .Start()
        .Get();

    static auto EndMMEOnServerIfNoUpdateReceived = IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.CharacterMovement.EndMMEOnServerIfNoUpdateReceived"));
    static auto CharacterOwnerProperty = GetClass()->FindPropertyByName(FName(TEXT("CharacterOwner")));
    static int32 ForceManualEndSlot = 0;

    if (ReceivedPackRuntimeData <= 0 || PushedRuntimeData <= 0 || NewlyPushedRuntimeData <= 0 || !CreateRuntimeDataForLogic || !CharacterOwnerProperty)
        return;

    struct FNewlyPushedExtension
    {
        int32 LogicID;
        uint8 RuntimeID;
    };

    auto& NewlyPushed = *(TArray<FNewlyPushedExtension>*)((uint8*)this + NewlyPushedRuntimeData);
    auto& ReceivedPack = *(TArray<TSharedPtr<FFortMovementMode_BaseExtRuntimeData, ESPMode::ThreadSafe>>*)((uint8*)this + ReceivedPackRuntimeData);
    auto& Pushed = *(TArray<TSharedPtr<FFortMovementMode_BaseExtRuntimeData, ESPMode::ThreadSafe>>*)((uint8*)this + PushedRuntimeData);
    auto CharacterOwner = *CharacterOwnerProperty->ContainerPtrToValuePtr<AActor*>(this);

    if (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_Authority)
    {
        auto Pawn = Cast<AFortPawn>(CharacterOwner);

        for (auto& NewExtension : NewlyPushed)
        {
            TSharedPtr<FFortMovementMode_BaseExtRuntimeData, ESPMode::ThreadSafe> RuntimeData;

            if (Pawn)
                CreateRuntimeDataForLogic(Pawn, &RuntimeData, &NewExtension.LogicID, NewExtension.RuntimeID);

            if (RuntimeData.IsValid())
                Pushed.Add(RuntimeData);
        }

        for (auto& PushedData : Pushed)
        {
            if (!PushedData.IsValid())
                continue;

            for (int32 Slot = 0; !ForceManualEndSlot && Slot < 0x80; Slot += sizeof(void*))
            {
                if (Anchor::At((const uint8*)PushedData->VTable[Slot / sizeof(void*)]).Is("mov byte ptr [rcx+0xe], 0x1; ret"))
                    ForceManualEndSlot = Slot;
            }

            bool bUpdateReceived = false;

            for (auto& ReceivedData : ReceivedPack)
            {
                if (!ReceivedData.IsValid() || ReceivedData->LogicID != PushedData->LogicID || ReceivedData->RuntimeID != PushedData->RuntimeID)
                    continue;

                if (ForceManualEndSlot)
                    ((void (*)(FFortMovementMode_BaseExtRuntimeData*, const FFortMovementMode_BaseExtRuntimeData*))PushedData->VTable[(ForceManualEndSlot - 0x30) / sizeof(void*)])(PushedData.Get(), ReceivedData.Get());

                bUpdateReceived = true;
            }

            if (!bUpdateReceived && ForceManualEndSlot && EndMMEOnServerIfNoUpdateReceived && EndMMEOnServerIfNoUpdateReceived->GetInt() != 0 && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
                ((void (*)(FFortMovementMode_BaseExtRuntimeData*))PushedData->VTable[ForceManualEndSlot / sizeof(void*)])(PushedData.Get());
        }
    }

    NewlyPushed.Reset();
    ReceivedPack.Reset();
}
