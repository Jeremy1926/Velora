#include "pch.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Scanner.h"

FUniqueNetIdRepl& FUniqueNetIdRepl::operator=(const FUniqueNetIdRepl& Other)
{
    static auto CopyAssign = Anchor::Function<FUniqueNetIdRepl& (*)(FUniqueNetIdRepl*, const FUniqueNetIdRepl&)>(TEXT("FUniqueNetIdRepl::operator=(const FUniqueNetIdRepl&)"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerState:GetUniqueID"))
        .Call(2)
        .Get();

    return CopyAssign(this, Other);
}
