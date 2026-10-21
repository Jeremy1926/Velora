#include "pch.h"
#include "Items/FortInventoryOwnerInterface.h"
#include "AI/FortAthenaAIBotController.h"
#include "Items/FortInventory.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* IFortInventoryOwnerInterface::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortInventoryOwnerInterface"));

    return Class;
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
        .FieldPattern("48 85 D2 74 ? 48 89 91 %o", AFortAthenaAIBotController::StaticClass() ? AFortAthenaAIBotController::StaticClass()->FindPropertyByName(FName(TEXT("StartupInventory"))) : nullptr)
        .Optional()
        .Is("test rdx, rdx; jz *; mov [rcx+0x*], rdx; jmp *")
        .Find("jmp [rax+0x*]")
        .LogSlot(TEXT("IFortInventoryOwnerInterface::InitializeInventory"))
        .InInterfaceVTable(TEXT("/Script/FortniteGame.Default__FortAthenaAIBotController"), TEXT("/Script/FortniteGame.FortInventoryOwnerInterface")));

    Original(Items);

    static auto DefaultObject = AFortAthenaAIBotController::StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress(StaticClass()) - (uint8*)DefaultObject;

    auto BotController = (AFortAthenaAIBotController*)((uint8*)this - PointerOffset);

    BotController->AddStartingItemsToInventory();
}
