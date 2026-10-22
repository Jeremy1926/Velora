#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Containers/UnrealString.h"
#include "Internationalization/Text.h"

class UKismetTextLibrary : public UBlueprintFunctionLibrary
{
public:
    static FText Conv_StringToText(const FString& InString);
};
