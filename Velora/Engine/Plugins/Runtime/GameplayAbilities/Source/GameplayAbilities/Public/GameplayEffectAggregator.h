#pragma once

#include "Templates/SharedPointer.h"

struct FAggregator;

struct FAggregatorRef
{
    FAggregator* Get() const
    {
        return Data.Get();
    }

private:
    TSharedPtr<FAggregator> Data;
};
