#include "pch.h"
#include "FortAssetManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortAssetManager& UFortAssetManager::Get()
{
    static auto Get = Scanner::Function<UFortAssetManager& (*)()>([]
    {
        auto GetPersonalXpBoost = Scanner::Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost"));

        return (GetPersonalXpBoost.Get() ? GetPersonalXpBoost : Scanner::Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition")).Call()).Call().Get(TEXT("UFortAssetManager::Get"));
    }());

    return Get();
}

UFortGameData* UFortAssetManager::GetGameData()
{
    static auto GetGameData = Scanner::Function<UFortGameData* (*)(UFortAssetManager*)>([]
    {
        auto GetPersonalXpBoost = Scanner::Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost"));

        return (GetPersonalXpBoost.Get() ? GetPersonalXpBoost.Call(2) : Scanner::At(Scanner::Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition")).Call().Find("jmp 0x*").Target())).Get(TEXT("UFortAssetManager::GetGameData"));
    }());

    return GetGameData(this);
}

UGameDataBR* UFortAssetManager::GetGameDataBR()
{
    static auto GetGameDataBR = Scanner::Function<UGameDataBR* (*)(UFortAssetManager*)>(Scanner::At(Scanner::String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!")).Start().Find("call 0x*; mov rcx, rax; call 0x*").Call().Find("jmp 0x*").Target()).Get(TEXT("UFortAssetManager::GetGameDataBR")));

    return GetGameDataBR(this);
}

UGameDataCosmetics* UFortAssetManager::GetGameDataCosmetics()
{
    static auto GetGameDataCosmetics = Scanner::Function<UGameDataCosmetics* (*)(UFortAssetManager*)>(Scanner::At(Scanner::Native(TEXT("/Script/FortniteGame.FortBannerUtilities:GetBannerColorCategoryName")).Call().Find("jmp 0x*").Target()).Get(TEXT("UFortAssetManager::GetGameDataCosmetics")));

    return GetGameDataCosmetics(this);
}
