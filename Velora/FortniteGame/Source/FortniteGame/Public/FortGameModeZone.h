#pragma once

#include "FortGameMode.h"
#include "Containers/Array.h"

class AFortAIDirector;
class AFortWeapon;
struct FItemAndCount;
enum class EFortWeaponAbilityRemovalReason : uint8;

class AFortGameModeZone : public AFortGameMode
{
public:
    typedef AFortGameMode Super;

    static UClass* StaticClass();

    void FinishWorldInitialization();

    void CreateAIDirector();

    AFortAIDirector* GetAIDirector();

    int32 GetOverrideAIDirectorIndex() const;

    bool ShouldWeaponRemoveGrantedAbilities(AFortWeapon* Weapon, EFortWeaponAbilityRemovalReason Reason) const;

    TArray<FItemAndCount>& GetStartingItems();
};
