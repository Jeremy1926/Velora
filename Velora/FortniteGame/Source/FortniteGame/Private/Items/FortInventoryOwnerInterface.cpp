#include "pch.h"
#include "Items/FortInventoryOwnerInterface.h"
#include "AI/FortAthenaAIBotController.h"
#include "Items/FortInventory.h"
#include "Player/FortPlayerController.h"
#include "UObject/Class.h"
#include "UObject/ScriptInterface.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* IFortInventoryOwnerInterface::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));

    return Class;
}

AFortInventory* IFortInventoryOwnerInterface::GetWorldInventory()
{
    static auto GetWorldInventory = Anchor::VirtualFunction<AFortInventory* (*)(IFortInventoryOwnerInterface*)>(TEXT("IFortInventoryOwnerInterface::GetWorldInventory"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerControllerAthena:GetOverflowCountIfItemWasAddedToInventory"))
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerControllerAthena:GetOverflowCountIfItemWasAddedToInventory"))
        .Find("sbb rcx, rcx; and rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call [rax+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerControllerAthena:GetOverflowCountIfItemWasAddedToInventory"))
        .Find("add r*, 0x*; mov rcx, r*; call 0x*")
        .Find("call 0x*")
        .Target()
        .Is("push *; sub rsp, 0x*; mov *, rdx; test rcx, rcx; jz *; mov rax, [rcx]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    static auto WorldInventory = AFortPlayerController::StaticClass()->FindPropertyByName(FName(TEXT("WorldInventory")));
    static auto FortInventoryInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryInterface"));

    auto Inventory = GetWorldInventory(this);

    if (!Inventory || !WorldInventory || WorldInventory->GetSize() != sizeof(FScriptInterface))
        return Inventory;

    auto DefaultInventory = AFortInventory::StaticClass()->GetDefaultObject(false);

    return (AFortInventory*)((uint8*)Inventory - ((uint8*)DefaultInventory->GetInterfaceAddress(FortInventoryInterface) - (uint8*)DefaultInventory));
}

bool IFortInventoryOwnerInterface::CreateInventory()
{
    DETOUR(IFortInventoryOwnerInterface, CreateInventory, Anchor::Hook()
        .At(DETOUR_ORIGINAL(IFortInventoryOwnerInterface, InitializeInventory))
        .Optional()
        .Find("mov rax, [*]; mov rcx, *; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("IFortInventoryOwnerInterface::CreateInventory"))
        .InInterfaceVTables(AFortAthenaAIBotController::StaticClass(), StaticClass(), "xor al, al; ret*"));

    static auto DefaultObject = AFortAthenaAIBotController::StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(StaticClass()) - (uint8*)DefaultObject;

    auto BotController = (AFortAthenaAIBotController*)((uint8*)this - PointerOffset);

    return BotController->CreateInventory();
}

void IFortInventoryOwnerInterface::InitializeInventory(const TArray<FItemAndCount>& Items)
{
    DETOUR(IFortInventoryOwnerInterface, InitializeInventory, Anchor::Hook()
        .String(TEXT("UFortAthenaAISpawnerDataComponent_InventoryBase::OnSpawned Failed to initialize Inventory on Pawn=%s Controller=%s (Can't create/initialize inventory on AI)"))
        .Start()
        .Find("mov rax, [rdi]; lea rdx, [rbp+0x*]; mov rcx, rdi; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Or()
        .FieldPattern("48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Optional()
        .Is("test rdx, rdx; jz *; mov [rcx+0x*], rdx; jmp *")
        .Find("jmp [rax+0x*]")
        .Or()
        .FieldPattern("48 83 EC 28 48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("IFortInventoryOwnerInterface::InitializeInventory"))
        .InInterfaceVTable(TEXT("/Script/FortniteGame.Default__FortAthenaAIBotController"), TEXT("/Script/FortniteGame.FortInventoryOwnerInterface")));

    Original(Items);

    static auto DefaultObject = AFortAthenaAIBotController::StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(StaticClass()) - (uint8*)DefaultObject;

    auto BotController = (AFortAthenaAIBotController*)((uint8*)this - PointerOffset);

    BotController->AddStartingItemsToInventory();
}
