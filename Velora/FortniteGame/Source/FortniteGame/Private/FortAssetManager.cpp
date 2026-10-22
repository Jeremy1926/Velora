#include "pch.h"
#include "FortAssetManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortAssetManager& UFortAssetManager::Get()
{
    static auto Get = Anchor::Function<UFortAssetManager& (*)()>(TEXT("UFortAssetManager::Get"))
        .Exec(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost"))
        .Call()
        .Or()
        .Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Target()
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
        .Exec(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Find("mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost"))
        .Call(2)
        .Or()
        .Native(TEXT("/Script/FortniteUI.FortInventoryContext:GetResourceItemDefinition"))
        .Find("call 0x*; mov rcx, rax; call 0x*")
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
        .Find("call 0x*; mov rcx, rax; call 0x*; lea rcx, [rax+0x*]")
        .Call(2)
        .Or()
        .String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!"))
        .Start()
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Call()
        .Find("jmp 0x*")
        .Target()
        .Or()
        .String(TEXT("InitializePlayerGameplayAbilities with invalid PlayerStateOrProxy!"))
        .Start()
        .Find("test al, al; jz 0x*; call 0x*; lea rcx, [rax+0x*]")
        .Find("call 0x*")
        .Target()
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
        .Has("mov *, [rcx+0x*]")
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortBannerUtilities:GetBannerColorCategoryName"))
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Call(2)
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortBannerUtilities:GetBannerColorCategoryName"))
        .Find("call 0x*; mov rcx, rax; add rsp, *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Has("mov *, [rcx+0x*]")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortBannerUtilities:GetBannerColorCategoryName"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*; mov rcx, rax; call 0x*")
        .Call(2)
        .Get();

    return GetGameDataCosmetics(this);
}

UObject* UFortAssetManager::GetAssetForPersistentName(const FString& PersistentName)
{
    static auto GetAssetForPersistentName = Anchor::Function<UObject* (*)(UFortAssetManager*, const FString&)>(TEXT("UFortAssetManager::GetAssetForPersistentName"))
        .Exec(TEXT("/Script/FortniteUI.FortUIBlueprintFunctionLibrary:GetAssetFromTemplateId"))
        .Find("call 0x*; lea rdx, [rsp+0x*]; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return GetAssetForPersistentName(this, PersistentName);
}
