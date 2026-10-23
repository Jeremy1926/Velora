#pragma once

#include "HAL/Platform.h"

class FNetBitWriter;
class UActorChannel;
class UFunction;

class FRepLayout
{
public:
    void SendPropertiesForRPC(UFunction* Function, UActorChannel* Channel, FNetBitWriter& Writer, void* Data) const;

    void BuildSharedSerializationForRPC(void* Data);

    void ClearSharedSerializationForRPC();
};
