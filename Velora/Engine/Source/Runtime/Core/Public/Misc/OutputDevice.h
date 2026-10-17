#pragma once

class FOutputDevice
{
public:
    virtual ~FOutputDevice()
    {
    }

protected:
    bool bSuppressEventTag;
    bool bAutoEmitLineTerminator;
};
