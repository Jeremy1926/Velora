#pragma once

#include "UObject/Object.h"
#include "Misc/OutputDevice.h"

class UConsole : public UObject, public FOutputDevice
{
};
