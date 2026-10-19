#include "pch.h"
#include "UObject/CoreNet.h"
#include "Templates/UnrealTemplate.h"
#include "Scanner.h"

bool UPackageMap::WriteObject(FArchive& Ar, UObject* InOuter, FNetworkGUID NetGUID, FString ObjName)
{
    static auto WriteObject = [this]
    {
        auto Function = Scanner::String(TEXT("WroteObject %s NetGUID <%s>")).Start().Get(TEXT("UPackageMapClient::WriteObject"));

        return Scanner::Slot(TEXT("UPackageMapClient::WriteObject"), *(const void***)this, [&](const Scanner::FCode& Entry) { return Entry.Get() == Function; });
    }();

    return Scanner::Virtual<bool (*)(UPackageMap*, FArchive&, UObject*, FNetworkGUID, FString)>(this, WriteObject)(this, Ar, InOuter, NetGUID, MoveTemp(ObjName));
}
