#include "pch.h"
#include "GameFramework/OnlineReplStructs.h"
#include "HAL/UnrealMemory.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

FUniqueNetIdRepl::FUniqueNetIdRepl()
{
    FMemory::Memzero(this, sizeof(FUniqueNetIdRepl));

    StaticStruct()->InitializeStruct(this);
}

FUniqueNetIdRepl::~FUniqueNetIdRepl()
{
    StaticStruct()->DestroyStruct(this);
}

FUniqueNetIdRepl& FUniqueNetIdRepl::operator=(const FUniqueNetIdRepl& Other)
{
    static auto CopyAssign = Anchor::Function<FUniqueNetIdRepl& (*)(FUniqueNetIdRepl*, const FUniqueNetIdRepl&)>(TEXT("FUniqueNetIdRepl::operator=(const FUniqueNetIdRepl&)"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerState:GetUniqueID"))
        .Call(2)
        .Get();

    return CopyAssign(this, Other);
}

UScriptStruct* FUniqueNetIdRepl::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.UniqueNetIdRepl"));

    return Struct;
}
