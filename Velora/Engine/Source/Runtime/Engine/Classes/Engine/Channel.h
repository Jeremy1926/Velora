#pragma once

#include "Engine/EngineBaseTypes.h"
#include "UObject/Object.h"
#include "UObject/CoreNet.h"
#include "UObject/NameTypes.h"

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

    int32 GetChIndex() const;

    FName GetChName() const;

    bool GetClosing() const;

    bool GetDormant() const;

    bool GetPendingDormancy() const;

    int32 GetNumOutRec() const;

    int32 IsNetReady(bool Saturate);

    FPacketIdRange SendBunch(FOutBunch* Bunch, bool Merge);

    int64 Close(EChannelCloseReason Reason);
};
