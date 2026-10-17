#pragma once

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
