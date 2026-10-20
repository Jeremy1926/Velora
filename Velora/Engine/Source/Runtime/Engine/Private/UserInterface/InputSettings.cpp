#include "pch.h"
#include "GameFramework/InputSettings.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

UInputSettings* UInputSettings::GetInputSettings()
{
    static auto InputSettings = (UInputSettings*)StaticFindObject(nullptr, nullptr, TEXT("/Script/Engine.Default__InputSettings"));

    return InputSettings;
}

TArray<FKey>& UInputSettings::GetConsoleKeys()
{
    static auto ConsoleKeys = GetClass()->FindPropertyByName(FName(TEXT("ConsoleKeys")));

    return *ConsoleKeys->ContainerPtrToValuePtr<TArray<FKey>>(this);
}
