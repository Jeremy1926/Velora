#include "pch.h"
#include "Movement/FortMovementComp_Character.h"
#include "Abilities/FortMovementSet.h"
#include "Pawns/FortPawn.h"
#include "HAL/IConsoleManager.h"
#include "Templates/SharedPointer.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortMovementComp_Character::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMovementComp_Character"));

    return Class;
}

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

void UFortMovementComp_Character::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel)
{
    DETOUR(UFortMovementComp_Character, MoveAutonomous, Anchor::Hook()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const ::Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("add rsp, 0x*; pop r15; pop r14; pop *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("mov [rsp+0x20], rcx; movaps xmm1, xmm*; mov rcx, r*; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/Engine.Default__CharacterMovementComponent"))
        .Or()
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const ::Anchor::FCode& Entry) { return Entry.Has("lea r14, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]; mov rdx, r14; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; cmp byte ptr [0x*], 0x1"); })
        .Find("add rsp, 0x*; pop r15; pop r14; pop *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("mov [rsp+0x20], rcx; movaps xmm1, xmm*; mov rcx, r*; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/Engine.Default__CharacterMovementComponent"))
        .Optional()
        .Inline());

    if (IsA(UFortMovementComp_Character::StaticClass()))
        ServerUpdateReceivedPackMovementModeExtension();

    Original(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}

void UFortMovementComp_Character::ServerUpdateReceivedPackMovementModeExtension()
{
    static auto RuntimeDataClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortMovementMode_BaseExtRuntimeData"));

    if (RuntimeDataClass && RuntimeDataClass->IsA(UClass::StaticClass()))
    {
        static auto PushedProperty = GetClass()->FindPropertyByName(FName(TEXT("PushedMovementModeExtensionRuntimeData")));
        static auto ReceivedPackProperty = GetClass()->FindPropertyByName(FName(TEXT("ReceivedPackMovementModeExtensionRuntimeData")));
        static auto OwnerProperty = GetClass()->FindPropertyByName(FName(TEXT("CharacterOwner")));
        static auto LogicID = RuntimeDataClass->FindPropertyByName(FName(TEXT("LogicID")));
        static auto RuntimeID = RuntimeDataClass->FindPropertyByName(FName(TEXT("RuntimeID")));
        static auto bForceManualEnd = RuntimeDataClass->FindPropertyByName(FName(TEXT("bForceManualEnd")));
        static auto bForceManualActivationNetworked = RuntimeDataClass->FindPropertyByName(FName(TEXT("bForceManualActivationNetworked")));
        static auto bForceManualEndNetworked = RuntimeDataClass->FindPropertyByName(FName(TEXT("bForceManualEndNetworked")));
        static auto EndOnServerIfNoUpdateReceived = IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.CharacterMovement.EndMMEOnServerIfNoUpdateReceived"));

        static auto NewlyPushedExtensions = Anchor::Offset(TEXT("UFortMovementComp_Character::NewlyPushedMovementModeExtensions"))
            .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const Anchor::FCode& Entry) { return Entry.Has("lea r14, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]; mov rdx, r14; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; cmp byte ptr [0x*], 0x1"); })
            .Find("cmp byte ptr [0x*], 0x1")
            .Find("lea r14, [rbx+0x*]")
            .Disp();

        static auto CreateRuntimeData = Anchor::Optional<UObject* (*)(UFortMovementComp_Character*, const int32*, uint8)>(TEXT("UFortMovementComp_Character::CreateRuntimeDataForLogic"))
            .Pattern("33 DB 41 8A F0 4C 8B F2 48 8B F9 48 39 99 ? ? 00 00 74 ? E8")
            .Start()
            .Get();

        static int32 ObjectForceManualEndSlot = 0;
        static int32 ObjectCopyFromMoveDataSlot = 0;

        if (!PushedProperty || !ReceivedPackProperty || !OwnerProperty || !LogicID || !RuntimeID || !bForceManualEnd || !bForceManualActivationNetworked || !bForceManualEndNetworked || NewlyPushedExtensions <= 0 || !CreateRuntimeData)
            return;

        if (!ObjectForceManualEndSlot)
        {
            auto Image = Anchor::Base();
            auto Headers = (IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew);
            auto VTable = *(const uint8***)RuntimeDataClass->GetDefaultObject(false);
            char ForceManualEnd[64];
            char CopyFromMoveData[192];

            snprintf(ForceManualEnd, sizeof(ForceManualEnd), "mov byte ptr [rcx+0x%x], 0x1; ret", bForceManualEnd->GetOffset_ForInternal());
            snprintf(CopyFromMoveData, sizeof(CopyFromMoveData), "movzx eax, byte ptr [rdx+0x%x]; mov [rcx+0x%x], al; movzx eax, byte ptr [rdx+0x%x]; mov [rcx+0x%x], al; ret", bForceManualActivationNetworked->GetOffset_ForInternal(), bForceManualActivationNetworked->GetOffset_ForInternal(), bForceManualEndNetworked->GetOffset_ForInternal(), bForceManualEndNetworked->GetOffset_ForInternal());

            for (int32 Slot = 0; !ObjectForceManualEndSlot && Slot < 0x600 && VTable[Slot / sizeof(void*)] > Image && VTable[Slot / sizeof(void*)] < Image + Headers->OptionalHeader.SizeOfImage; Slot += sizeof(void*))
            {
                if (Anchor::At(VTable[Slot / sizeof(void*)]).Is(CopyFromMoveData))
                    ObjectCopyFromMoveDataSlot = Slot;
                else if (Anchor::At(VTable[Slot / sizeof(void*)]).Is(ForceManualEnd))
                    ObjectForceManualEndSlot = Slot;
            }

            UE_LOG(LogVelora, Log, TEXT("UFortMovementMode_BaseExtRuntimeData ForceManualEnd slot 0x%X, CopyFromMoveData slot 0x%X"), ObjectForceManualEndSlot, ObjectCopyFromMoveDataSlot);
        }

        struct FNewlyPushedObjectExtension
        {
            int32 LogicID;
            uint8 RuntimeID;
        };

        auto& NewlyPushedObjects = *(TArray<FNewlyPushedObjectExtension>*)((uint8*)this + NewlyPushedExtensions);
        auto& ReceivedObjects = *ReceivedPackProperty->ContainerPtrToValuePtr<TArray<UObject*>>(this);
        auto& PushedObjects = *PushedProperty->ContainerPtrToValuePtr<TArray<UObject*>>(this);
        auto Owner = *OwnerProperty->ContainerPtrToValuePtr<AActor*>(this);

        if (Owner && Owner->GetLocalRole() == ROLE_Authority)
        {
            for (auto& NewExtension : NewlyPushedObjects)
            {
                if (auto RuntimeData = CreateRuntimeData(this, &NewExtension.LogicID, NewExtension.RuntimeID))
                    PushedObjects.Add(RuntimeData);
            }

            for (auto PushedData : PushedObjects)
            {
                if (!PushedData)
                    continue;

                bool bUpdateReceived = false;

                for (auto ReceivedData : ReceivedObjects)
                {
                    if (!ReceivedData || *LogicID->ContainerPtrToValuePtr<int32>(ReceivedData) != *LogicID->ContainerPtrToValuePtr<int32>(PushedData) || *RuntimeID->ContainerPtrToValuePtr<uint8>(ReceivedData) != *RuntimeID->ContainerPtrToValuePtr<uint8>(PushedData))
                        continue;

                    if (ObjectCopyFromMoveDataSlot)
                        Scanner::Virtual<void (*)(UObject*, const UObject*)>(PushedData, ObjectCopyFromMoveDataSlot)(PushedData, ReceivedData);

                    bUpdateReceived = true;
                }

                if (!bUpdateReceived && ObjectForceManualEndSlot && EndOnServerIfNoUpdateReceived && EndOnServerIfNoUpdateReceived->GetInt() != 0 && Owner->GetRemoteRole() == ROLE_AutonomousProxy)
                    Scanner::Virtual<void (*)(UObject*)>(PushedData, ObjectForceManualEndSlot)(PushedData);
            }
        }

        NewlyPushedObjects.Reset();
        ReceivedObjects.Reset();

        return;
    }

    static auto ReceivedPackRuntimeData = Anchor::Offset(TEXT("UFortMovementComp_Character::ReceivedPackMovementModeExtensionRuntimeData"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]")
        .Find("lea rcx, [rbx+0x*]")
        .Disp();

    static auto PushedRuntimeData = Anchor::Offset(TEXT("UFortMovementComp_Character::PushedMovementModeExtensionRuntimeData"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]")
        .Find("mov rax, [rbx+0x*]")
        .Disp();

    static auto NewlyPushedRuntimeData = Anchor::Offset(TEXT("UFortMovementComp_Character::NewlyPushedMovementModeExtensions"))
        .Virtual(TEXT("/Script/FortniteGame.Default__FortMovementComp_Character"), [](const Anchor::FCode& Entry) { return Entry.Has("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]; mov [rbx+0x*], eax; mov al, [rdi+0x*]; mov [rbx+0x*], al; call 0x*; movsxd rax, dword ptr [rdi+0x*]; mov rsi, [rdi+0x*]; lea r15, [rsi+rax*8]"); })
        .Find("lea rdx, [rdi+0x*]; mov [rbx+0x*], eax; lea rcx, [rbx+0x*]; mov eax, [rdi+0x*]")
        .Find("lea r14, [rbx+0x*]")
        .Disp();

    static auto CreateRuntimeDataForLogic = Anchor::Optional<void* (*)(AFortPawn*, TSharedPtr<FFortMovementMode_BaseExtRuntimeData, ESPMode::ThreadSafe>*, const int32*, uint8)>(TEXT("AFortPawn::CreateRuntimeDataForLogic"))
        .Pattern("45 8B 00 48 8B DA 48 8D 54 24 ? 41 8A F9 E8")
        .Start()
        .Get();

    static auto EndMMEOnServerIfNoUpdateReceived = IConsoleManager::Get().FindConsoleVariable(TEXT("Fort.CharacterMovement.EndMMEOnServerIfNoUpdateReceived"));
    static auto CharacterOwnerProperty = GetClass()->FindPropertyByName(FName(TEXT("CharacterOwner")));
    static auto BaseCopyFromMoveData = Anchor::Data<const uint8*>(TEXT("FFortMovementMode_BaseExtRuntimeData::CopyFromMoveData"))
        .Pattern("0F B6 42 0D 88 41 0D 0F B6 42 0E 88 41 0E C3")
        .Optional()
        .Get();

    static int32 ForceManualEndSlot = 0;
    static int32 CopyFromMoveDataSlot = -1;

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

            if (ForceManualEndSlot && CopyFromMoveDataSlot < 0)
            {
                CopyFromMoveDataSlot = 0;

                auto Image = Anchor::Base();
                auto Headers = (IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew);
                auto Sections = IMAGE_FIRST_SECTION(Headers);

                for (int32 Index = 0; BaseCopyFromMoveData && !CopyFromMoveDataSlot && Index < Headers->FileHeader.NumberOfSections; Index++)
                {
                    if ((Sections[Index].Characteristics & IMAGE_SCN_MEM_EXECUTE) || !(Sections[Index].Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA))
                        continue;

                    for (auto Entry = (const uint8**)(Image + Sections[Index].VirtualAddress); !CopyFromMoveDataSlot && (const uint8*)(Entry + 0x10) < Image + Sections[Index].VirtualAddress + Sections[Index].Misc.VirtualSize; Entry++)
                    {
                        if (Entry[0] != BaseCopyFromMoveData || Entry[1] == BaseCopyFromMoveData)
                            continue;

                        for (int32 Slot = 1; !CopyFromMoveDataSlot && Slot < 0x10; Slot++)
                        {
                            if (Entry[Slot] > Image && Entry[Slot] < Image + Headers->OptionalHeader.SizeOfImage && Anchor::At(Entry[Slot]).Is("mov byte ptr [rcx+0xe], 0x1; ret"))
                                CopyFromMoveDataSlot = ForceManualEndSlot - (Slot + 1) * (int32)sizeof(void*);
                        }
                    }
                }

                UE_LOG(LogVelora, Log, TEXT("FFortMovementMode_BaseExtRuntimeData ForceManualEnd slot 0x%X, CopyFromMoveData slot 0x%X"), ForceManualEndSlot, CopyFromMoveDataSlot);
            }

            bool bUpdateReceived = false;

            for (auto& ReceivedData : ReceivedPack)
            {
                if (!ReceivedData.IsValid() || ReceivedData->LogicID != PushedData->LogicID || ReceivedData->RuntimeID != PushedData->RuntimeID)
                    continue;

                if (CopyFromMoveDataSlot > 0)
                    ((void (*)(FFortMovementMode_BaseExtRuntimeData*, const FFortMovementMode_BaseExtRuntimeData*))PushedData->VTable[CopyFromMoveDataSlot / sizeof(void*)])(PushedData.Get(), ReceivedData.Get());

                bUpdateReceived = true;
            }

            if (!bUpdateReceived && ForceManualEndSlot && EndMMEOnServerIfNoUpdateReceived && EndMMEOnServerIfNoUpdateReceived->GetInt() != 0 && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy)
                ((void (*)(FFortMovementMode_BaseExtRuntimeData*))PushedData->VTable[ForceManualEndSlot / sizeof(void*)])(PushedData.Get());
        }
    }

    NewlyPushed.Reset();
    ReceivedPack.Reset();
}
