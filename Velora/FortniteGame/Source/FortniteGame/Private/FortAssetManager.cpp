#include "pch.h"
#include "FortAssetManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortAssetManager& UFortAssetManager::Get()
{
    static auto Get = Anchor::Function<UFortAssetManager& (*)()>(TEXT("UFortAssetManager::Get"))
        .Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost"))
        .Call()
        .Or()
        .Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Call()
        .Call()
        .Get();

    return Get();
}

UFortGameData* UFortAssetManager::GetGameData()
{
    static auto GetGameData = Anchor::Function<UFortGameData* (*)(UFortAssetManager*)>(TEXT("UFortAssetManager::GetGameData"))
        .Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost"))
        .Call(2)
        .Or()
        .Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Call()
        .Find("jmp 0x*")
        .Target()
        .Get();

    return GetGameData(this);
}

UGameDataBR* UFortAssetManager::GetGameDataBR()
{
    static auto GetGameDataBR = Anchor::Function<UGameDataBR* (*)(UFortAssetManager*)>(TEXT("UFortAssetManager::GetGameDataBR"))
        .String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!"))
        .Start()
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Call()
        .Find("jmp 0x*")
        .Target()
        .Get();

    return GetGameDataBR(this);
}

UGameDataCosmetics* UFortAssetManager::GetGameDataCosmetics()
{
    static auto GetGameDataCosmetics = Anchor::Function<UGameDataCosmetics* (*)(UFortAssetManager*)>(TEXT("UFortAssetManager::GetGameDataCosmetics"))
        .Native(TEXT("/Script/FortniteGame.FortBannerUtilities:GetBannerColorCategoryName"))
        .Call()
        .Find("jmp 0x*")
        .Target()
        .Get();

    return GetGameDataCosmetics(this);
}
