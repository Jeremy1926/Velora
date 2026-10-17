#pragma once

#include "McpItemDefinitionBase.h"

class UAbilitySystemComponent;
struct FGameplayTagContainer;

enum class EFortItemType : uint8
{
    WorldItem,
    Ammo,
    Badge,
    BackpackPickup,
    BuildingPiece,
    CharacterPart,
    Deco,
    EditTool,
    Ingredient,
    Food,
    Gadget,
    HomebaseGadget,
    HeroAbility,
    MissionItem,
    Trap,
    Weapon,
    WeaponMelee,
    WeaponRanged,
    WeaponHarvest,
    WorldResource,
    AccountItem,
    AccountResource,
    CollectedResource,
    Alteration,
    CardPack,
    CharacterCosmetic,
    Currency,
    Hero,
    Schematic,
    Worker,
    Token,
    DailyRewardScheduleToken,
    CodeToken,
    Buff,
    BuffCredit,
    Quest,
    Compendium,
    CompendiumBundle,
    GameplayModifier,
    Outpost,
    HomebaseNode,
    Defender,
    ConversionControl,
    DeployableBaseCloudSave,
    ConsumableAccountItem,
    Quota,
    Expedition,
    HomebaseBannerIcon,
    HomebaseBannerColor,
    AthenaGlider,
    AthenaPickaxe,
    AthenaHat,
    AthenaBackpack,
    AthenaCharacter,
    AthenaDance,
    AthenaConsumableEmote,
    AthenaLoadingScreen,
    AthenaBattleBus,
    AthenaVictoryPose,
    AthenaMedal,
    SpecialItem,
    Emote,
    Stack,
    CollectionBookPage,
    Profile,
    Max_None,
};

enum class EFortRarity : uint8
{
    Handmade,
    Ordinary,
    Sturdy,
    Quality,
    Fine,
    Elegant,
    Masterwork,
    Epic,
    Badass,
    Legendary,
    NumRarityValues,
};

class UFortItemDefinition : public UMcpItemDefinitionBase
{
public:
    int32 GetMinLevel() const;

    void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const;

    EFortItemType GetItemType() const;

    EFortRarity GetRarity() const;

    int32 ClampItemLevel(int32 ItemLevel) const;

    bool DoesAllowMultipleStacks() const;

    int32 GetMaxStackSize(const UAbilitySystemComponent* AbilitySystemComponent = nullptr) const;
};
