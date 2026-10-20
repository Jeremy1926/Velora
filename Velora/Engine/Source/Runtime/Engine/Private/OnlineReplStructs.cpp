#include "pch.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Scanner.h"

FUniqueNetIdRepl& FUniqueNetIdRepl::operator=(const FUniqueNetIdRepl& Other)
{
    static auto CopyAssign = Scanner::Function<FUniqueNetIdRepl& (*)(FUniqueNetIdRepl*, const FUniqueNetIdRepl&)>(Scanner::Exec(TEXT("/Script/FortniteGame.FortPlayerState:GetUniqueID")).Call(2).Get(TEXT("FUniqueNetIdRepl::operator=(const FUniqueNetIdRepl&)")));

    return CopyAssign(this, Other);
}
