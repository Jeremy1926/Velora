#pragma once

#include "UObject/Object.h"
#include "UObject/CoreNet.h"

class FOutBunch;
class UNetConnection;

enum EChannelType
{
    CHTYPE_None = 0,
    CHTYPE_Control = 1,
    CHTYPE_Actor = 2,
    CHTYPE_File = 3,
    CHTYPE_Voice = 4,
    CHTYPE_MAX = 8,
};

class UChannel : public UObject
{
public:
    UNetConnection* GetConnection() const;

    bool GetClosing() const;

    bool GetDormant() const;

    bool GetPendingDormancy() const;

    int32 GetNumOutRec() const;

    int32 IsNetReady(bool Saturate);

    FPacketIdRange SendBunch(FOutBunch* Bunch, bool Merge);
};
