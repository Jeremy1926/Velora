#pragma once

#include "Components/Widget.h"
#include "Templates/SharedPointer.h"

struct FFortActivity;

class UFortAthenaMatchmakingWidgetLegacy : public UWidget
{
public:
    void HandleActivityQueryComplete(TSharedPtr<FFortActivity> Activity);
};
