#pragma once

#include "GameFramework/Actor.h"

enum class EFortQuickBars : uint8
{
    Primary,
    Secondary,
    Max_None
};

class AFortQuickBars : public AActor
{
public:
    static UClass* StaticClass();

    void ActivateSlot(EFortQuickBars InQuickBar, int32 Slot, float ActivateDelay, bool bUpdatePreviousFocusedSlot);
};
