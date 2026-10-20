#include "pch.h"
#include "UObject/CoreNet.h"
#include "Templates/UnrealTemplate.h"
#include "Scanner.h"

bool UPackageMap::WriteObject(FArchive& Ar, UObject* InOuter, FNetworkGUID NetGUID, FString ObjName)
{
    static auto WriteObject = Anchor::VirtualFunction<bool (*)(UPackageMap*, FArchive&, UObject*, FNetworkGUID, FString)>(TEXT("UPackageMapClient::WriteObject"))
        .String(TEXT("WroteObject %s NetGUID <%s>"))
        .Start()
        .Log(TEXT("UPackageMapClient::WriteObject"))
        .SlotIn(this)
        .Get();

    return WriteObject(this, Ar, InOuter, NetGUID, MoveTemp(ObjName));
}
