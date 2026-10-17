#pragma once

#include "HAL/Platform.h"

class FInBunch;
class UChannel;
class UNetConnection;

namespace EAcceptConnection
{
    enum Type
    {
        Reject,
        Accept,
        Ignore
    };
}

class FNetworkNotify
{
public:
    virtual EAcceptConnection::Type NotifyAcceptingConnection() = 0;

    virtual void NotifyAcceptedConnection(UNetConnection* Connection) = 0;

    virtual bool NotifyAcceptingChannel(UChannel* Channel) = 0;

    virtual void NotifyControlMessage(UNetConnection* Connection, uint8 MessageType, FInBunch& Bunch) = 0;
};
