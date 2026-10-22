#pragma once

#include "GameFramework/Info.h"
#include "Containers/Array.h"
#include "Math/Vector.h"

struct FObjectKey;
class APlayerController;

class AFortSafeZoneIndicatorFuture : public AInfo
{
public:
    static UClass* StaticClass();

    static void SetWhitelistedPlayer(const APlayerController* PC, bool bWhitelist);

    static TArray<FObjectKey>* GetWhitelistedPlayers();

    FVector& GetNextNextCenter();

    float& GetNextNextRadius();
};
