#include "pch.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "Templates/Casts.h"
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
    static auto Property = GetClass()->FindPropertyByName(FName(TEXT("CustomizationLoadout")));
    static auto CustomizationLoadout = Property ? Property : GetClass()->FindPropertyByName(FName(TEXT("CosmeticLoadoutPC")));

    return CustomizationLoadout ? CustomizationLoadout->ContainerPtrToValuePtr<FFortAthenaLoadout>(this) : nullptr;
}

void AFortPlayerControllerAthena::ClientEquipItem(const FGuid& ItemGuid, bool bForceExecution)
{
    static auto ClientEquipItem = Scanner::Function<void (*)(AFortPlayerControllerAthena*, const FGuid&, bool)>(Scanner::Event(TEXT("ClientEquipItem")).Get(TEXT("AFortPlayerControllerAthena::ClientEquipItem")));

    ClientEquipItem(this, ItemGuid, bForceExecution);
}

UAthenaMarkerComponent* AFortPlayerControllerAthena::GetMarkerComponent() const
{
    static auto MarkerComponent = GetClass()->FindPropertyByName(FName(TEXT("MarkerComponent")));

    return *MarkerComponent->ContainerPtrToValuePtr<UAthenaMarkerComponent*>(this);
}

uint8 AFortPlayerControllerAthena::GetSquadId() const
{
    auto FortPlayerState = Cast<AFortPlayerStateAthena>(GetPlayerState());

    return FortPlayerState ? FortPlayerState->GetSquadId() : 0;
}
