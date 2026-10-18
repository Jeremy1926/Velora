#include "pch.h"
#include "UObject/CoreNet.h"
#include "Templates/UnrealTemplate.h"
#include "Scanner.h"

bool UPackageMap::WriteObject(FArchive& Ar, UObject* InOuter, FNetworkGUID NetGUID, FString ObjName)
{
    static auto WriteObject = [this]
    {
        auto Function = Scanner::String(TEXT("WroteObject %s NetGUID <%s>")).Start().Get(TEXT("UPackageMapClient::WriteObject"));
        auto VTable = *(uint8_t***)this;
        auto Index = 0;

        while (VTable[Index] != Function)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    return (*(bool (**)(UPackageMap*, FArchive&, UObject*, FNetworkGUID, FString))(VTable + WriteObject))(this, Ar, InOuter, NetGUID, MoveTemp(ObjName));
}
