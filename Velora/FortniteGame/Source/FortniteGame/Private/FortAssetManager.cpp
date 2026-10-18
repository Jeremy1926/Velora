#include "pch.h"
#include "FortAssetManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UFortAssetManager& UFortAssetManager::Get()
{
    static auto Get = (UFortAssetManager& (*)())Scanner::Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost")).Call().Get();

    return Get();
}

UFortGameData* UFortAssetManager::GetGameData()
{
    static auto GetGameData = (UFortGameData* (*)(UFortAssetManager*))Scanner::Native(TEXT("/Script/FortniteGame.FortGameData:GetPersonalXpBoost")).Call(2).Get();

    return GetGameData(this);
}
