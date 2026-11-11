#include "pch.h"
#include "Athena/FortPoiManager.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortPoiManager::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPoiManager"));

    return Class;
}

void AFortPoiManager::CheckPlayerControllerWithPoiGrid(AFortPlayerControllerZone* PlayerController)
{
    static auto CheckPlayerControllerWithPoiGrid = Anchor::Optional<void (*)(AFortPoiManager*, AFortPlayerControllerZone*)>(TEXT("AFortPoiManager::CheckPlayerControllerWithPoiGrid"))
        .String(TEXT("STAT_PoiManager_Tick"))
        .FieldFind("inc dword ptr [rbx+%x]", StaticClass()->FindPropertyByName(FName(TEXT("TestPlayerIndex"))))
        .Last("call 0x*")
        .Target()
        .Is("test rdx, rdx; jz 0x*")
        .Get();

    if (CheckPlayerControllerWithPoiGrid)
        CheckPlayerControllerWithPoiGrid(this, PlayerController);
}
