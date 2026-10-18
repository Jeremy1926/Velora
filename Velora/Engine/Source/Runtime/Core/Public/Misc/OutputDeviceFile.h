#pragma once

#include "Misc/OutputDevice.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

class FOutputDeviceFile : public FOutputDevice
{
public:
    FOutputDeviceFile(const TCHAR* InFilename);

    virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category, const double Time) override;

    virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category) override;

    virtual void Flush() override;

    virtual void TearDown() override;

    void IncludeCategory(const FName& InCategoryName);

    static void CreateBackupCopy(const TCHAR* Filename);

private:
    FILE* LogFile;
    TCHAR Filename[1024];
    TArray<FName> AllowedCategories;
};
