#include "pch.h"
#include "FortAssetManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortAssetManager& UFortAssetManager::Get()
{
    static auto Get = Scanner::Function<UFortAssetManager& (*)()>(Scanner::Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost")).Call().Get(TEXT("UFortAssetManager::Get")));

    return Get();
}

UFortGameData* UFortAssetManager::GetGameData()
{
    static auto GetGameData = Scanner::Function<UFortGameData* (*)(UFortAssetManager*)>(Scanner::Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost")).Call(2).Get(TEXT("UFortAssetManager::GetGameData")));

    return GetGameData(this);
}
