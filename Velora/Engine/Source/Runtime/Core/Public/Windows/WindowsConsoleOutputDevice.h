#pragma once

#include "Misc/OutputDevice.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

class FWindowsConsoleOutputDevice : public FOutputDevice
{
public:
    FWindowsConsoleOutputDevice();

    virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category, const double Time) override;

    virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category) override;

    void ExcludeCategory(const FName& InCategoryName);

private:
    HANDLE ConsoleHandle;
    TArray<FName> ExcludedCategories;
};
