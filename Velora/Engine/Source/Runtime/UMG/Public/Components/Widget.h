#pragma once

#include "UObject/Object.h"

class ULocalPlayer;

class UWidget : public UObject
{
public:
    ULocalPlayer* GetOwningLocalPlayer() const;
};
