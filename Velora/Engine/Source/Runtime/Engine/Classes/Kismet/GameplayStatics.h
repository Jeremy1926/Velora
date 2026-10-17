#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Containers/UnrealString.h"
#include "UObject/NameTypes.h"

class UGameplayStatics : public UBlueprintFunctionLibrary
{
public:
    static void OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute = true, FString Options = FString(TEXT("")));
};
