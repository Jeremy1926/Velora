#pragma once

#include "Containers/UnrealString.h"
#include "Containers/Array.h"

enum ENetMode
{
    NM_Standalone,
    NM_DedicatedServer,
    NM_ListenServer,
    NM_Client,
    NM_MAX,
};

namespace ENetworkFailure
{
    enum Type
    {
        NetDriverAlreadyExists,
        NetDriverCreateFailure,
        NetDriverListenFailure,
        ConnectionLost,
        ConnectionTimeout,
        FailureReceived,
        OutdatedClient,
        OutdatedServer,
        PendingConnectionFailure,
        NetGuidMismatch,
        NetChecksumMismatch
    };
}

struct FURL
{
    FString Protocol;
    FString Host;
    int32 Port;
    FString Map;
    FString RedirectURL;
    TArray<FString> Op;
    FString Portal;
    int32 Valid;
};
