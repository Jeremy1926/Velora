#include "pch.h"
#include "Weapons/FortWeapon.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortWeapon::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortWeapon"));

    return Class;
}

void AFortWeapon::PlayWeaponFireFX(bool bPersistentFire)
{
    static auto PlayWeaponFireFX = []
    {
        auto OnPlayWeaponFireFX = Scanner::Reference(Scanner::String(TEXT("OnPlayWeaponFireFX")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get();
        auto Function = Scanner::CallTo(OnPlayWeaponFireFX).Start().Get();
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortWeapon"));
        auto Index = 0;

        while (VTable[Index] != Function)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortWeapon*, bool))(VTable + PlayWeaponFireFX))(this, bPersistentFire);
}
