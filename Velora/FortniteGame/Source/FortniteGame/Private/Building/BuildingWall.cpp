#include "pch.h"
#include "Building/BuildingWall.h"
#include "Pawns/FortPawn.h"
#include "GameFramework/Controller.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* ABuildingWall::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.BuildingWall"));

    return Class;
}

void ABuildingWall::Open_Implementation(AController* ControllerInstigator)
{
    DETOUR(ABuildingWall, Open_Implementation, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:OpenActor")))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:OpenActor"))
        .Call(3)
        .Find("call [r8+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:OpenActor"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Call(3)
        .Find("call [r8+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:OpenActor"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Call(3)
        .Find("mov rcx, rax; call [r9+0x*]")
        .Find("call [r9+0x*]")
        .LogSlot(TEXT("IOpenableInterface::Open_Implementation"))
        .InInterfaceVTable(TEXT("/Script/FortniteGame.Default__BuildingWall"), TEXT("/Script/FortniteGame.OpenableInterface")));

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress((UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.OpenableInterface"))) - (uint8*)DefaultObject;

    auto BuildingWall = (ABuildingWall*)((uint8*)this - PointerOffset);

    BuildingWall->SetIsDoorOpen(EDoorOpenStyle::Open, ControllerInstigator ? Cast<AFortPawn>(ControllerInstigator->GetPawn()) : nullptr);
}

void ABuildingWall::Close_Implementation(AController* ControllerInstigator)
{
    DETOUR(ABuildingWall, Close_Implementation, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortKismetLibrary:CloseActor")))
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:CloseActor"))
        .Call(3)
        .Find("call [r8+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:CloseActor"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Call(3)
        .Find("call [r8+0x*]")
        .LogSlot(TEXT("IOpenableInterface::Close_Implementation"))
        .InInterfaceVTable(TEXT("/Script/FortniteGame.Default__BuildingWall"), TEXT("/Script/FortniteGame.OpenableInterface")));

    static auto DefaultObject = StaticClass()->GetDefaultObject(false);
    static auto PointerOffset = (uint8*)DefaultObject->GetInterfaceAddress((UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.OpenableInterface"))) - (uint8*)DefaultObject;

    auto BuildingWall = (ABuildingWall*)((uint8*)this - PointerOffset);

    BuildingWall->SetIsDoorOpen(EDoorOpenStyle::Close, ControllerInstigator ? Cast<AFortPawn>(ControllerInstigator->GetPawn()) : nullptr);
}

void ABuildingWall::SetIsDoorOpen(EDoorOpenStyle InDoorOpenStyle, AFortPawn* InteractingPawn)
{
    static auto SetIsDoorOpen = Anchor::Function<void (*)(ABuildingWall*, EDoorOpenStyle, AFortPawn*)>(TEXT("ABuildingWall::SetIsDoorOpen"))
        .Native(TEXT("/Script/FortniteGame.BuildingWall:OnNearDoorTriggerTouched"))
        .Find("mov dl, 0x1; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.BuildingWall:OnNearDoorTriggerTouched"))
        .Find("jz 0x*")
        .Target()
        .Find("mov dl, 0x1; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("41 B6 03 4D 8B F8 40 8A FA 48 8B F1 44 38 B1 ? ? ? ? 0F 85")
        .Start()
        .Get();

    SetIsDoorOpen(this, InDoorOpenStyle, InteractingPawn);
}
