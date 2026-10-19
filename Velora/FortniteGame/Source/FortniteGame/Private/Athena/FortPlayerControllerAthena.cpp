#include "pch.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UAthenaPickaxeItemDefinition*& FFortAthenaLoadout::GetPickaxe()
{
    static auto Pickaxe = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaLoadout")))->FindPropertyByName(FName(TEXT("Pickaxe")));

    return *Pickaxe->ContainerPtrToValuePtr<UAthenaPickaxeItemDefinition*>(this);
}

UClass* AFortPlayerControllerAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerAthena"));

    return Class;
}

FFortAthenaLoadout* AFortPlayerControllerAthena::GetCustomizationLoadout()
{
    static auto CustomizationLoadout = GetClass()->FindPropertyByName(FName(TEXT("CustomizationLoadout")));

    return CustomizationLoadout ? CustomizationLoadout->ContainerPtrToValuePtr<FFortAthenaLoadout>(this) : nullptr;
}

void AFortPlayerControllerAthena::ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution)
{
    static auto ClientEquipItem = Scanner::Function<void (*)(AFortPlayerControllerAthena*, const FGuid&, bool)>(Scanner::Event(TEXT("ClientEquipItem")).Get(TEXT("AFortPlayerControllerAthena::ClientEquipItem")));

    ClientEquipItem(this, ItemGuid, bForceExecution);
}
