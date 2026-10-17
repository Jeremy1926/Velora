#include "pch.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

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

FFortAthenaLoadout& AFortPlayerControllerAthena::GetCustomizationLoadout()
{
    static auto CustomizationLoadout = GetClass()->FindPropertyByName(FName(TEXT("CustomizationLoadout")));

    return *CustomizationLoadout->ContainerPtrToValuePtr<FFortAthenaLoadout>(this);
}
