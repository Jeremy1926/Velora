#include "pch.h"
#include "Misc/OutputDeviceFile.h"
#include <share.h>

FOutputDeviceFile::FOutputDeviceFile(const TCHAR* InFilename)
    : LogFile(nullptr)
{
    wcscpy_s(Filename, InFilename);
}

void FOutputDeviceFile::Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category, const double Time)
{
    if (AllowedCategories.Num() && !AllowedCategories.Contains(Category))
        return;

    if (!LogFile)
    {
        CreateBackupCopy(Filename);

        LogFile = _wfsopen(Filename, TEXT("w, ccs=UTF-8"), _SH_DENYWR);

        if (!LogFile)
            return;
    }

    SYSTEMTIME LocalTime;
    GetLocalTime(&LocalTime);

    const auto LogVerbosity = Verbosity & ELogVerbosity::VerbosityMask;
    const TCHAR* Prefix = TEXT("");

    if (LogVerbosity == ELogVerbosity::Fatal || LogVerbosity == ELogVerbosity::Error)
        Prefix = TEXT("Error: ");
    else if (LogVerbosity == ELogVerbosity::Warning)
        Prefix = TEXT("Warning: ");

    fwprintf(LogFile, TEXT("[%04hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu] %s%s\n"), LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, LocalTime.wMilliseconds, Prefix, Data);
    fflush(LogFile);
}

void FOutputDeviceFile::Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category)
{
    Serialize(Data, Verbosity, Category, -1.0);
}

void FOutputDeviceFile::Flush()
{
    if (LogFile)
        fflush(LogFile);
}

void FOutputDeviceFile::TearDown()
{
    if (LogFile)
    {
        fclose(LogFile);
        LogFile = nullptr;
    }
}

void FOutputDeviceFile::IncludeCategory(const FName& InCategoryName)
{
    AllowedCategories.Add(InCategoryName);
}

void FOutputDeviceFile::CreateBackupCopy(const TCHAR* Filename)
{
    WIN32_FILE_ATTRIBUTE_DATA Attributes;

    if (!GetFileAttributesExW(Filename, GetFileExInfoStandard, &Attributes))
        return;

    FILETIME LocalFileTime;
    SYSTEMTIME LocalTime;
    FileTimeToLocalFileTime(&Attributes.ftLastWriteTime, &LocalFileTime);
    FileTimeToSystemTime(&LocalFileTime, &LocalTime);

    TCHAR BackupFilename[1024];
    swprintf_s(BackupFilename, TEXT("%.*s-backup-%04hu.%02hu.%02hu-%02hu.%02hu.%02hu.log"), (int32)(wcsrchr(Filename, TEXT('.')) - Filename), Filename, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond);

    MoveFileExW(Filename, BackupFilename, MOVEFILE_REPLACE_EXISTING);
}
