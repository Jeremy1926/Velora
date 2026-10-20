#include "pch.h"
#include "Windows/WindowsConsoleOutputDevice.h"

FWindowsConsoleOutputDevice::FWindowsConsoleOutputDevice()
{
    AllocConsole();

    ConsoleHandle = CreateFileW(TEXT("CONOUT$"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
}

void FWindowsConsoleOutputDevice::Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category, const double Time)
{
    if (!ConsoleHandle || ConsoleHandle == INVALID_HANDLE_VALUE)
        return;

    if (ExcludedCategories.Contains(Category))
        return;

    SYSTEMTIME LocalTime;
    GetLocalTime(&LocalTime);

    const auto LogVerbosity = Verbosity & ELogVerbosity::VerbosityMask;
    const TCHAR* Prefix = TEXT("");
    WORD Color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

    if (LogVerbosity == ELogVerbosity::Fatal || LogVerbosity == ELogVerbosity::Error)
    {
        Prefix = TEXT("Error: ");
        Color = FOREGROUND_RED | FOREGROUND_INTENSITY;
    }
    else if (LogVerbosity == ELogVerbosity::Warning)
    {
        Prefix = TEXT("Warning: ");
        Color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    }

    static thread_local bool bResolvingCategory = false;
    FString CategoryName;

    if (Category != NAME_None && !bResolvingCategory)
    {
        bResolvingCategory = true;
        CategoryName = Category.ToString();
        bResolvingCategory = false;
    }

    TCHAR Line[2048];
    auto Length = _snwprintf_s(Line, _TRUNCATE, TEXT("[%02hu:%02hu:%02hu] %s%s%s%s\r\n"), LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, *CategoryName, CategoryName.Len() ? TEXT(": ") : TEXT(""), Prefix, Data);

    if (Length < 0)
        Length = (int32)wcslen(Line);

    DWORD Written;
    SetConsoleTextAttribute(ConsoleHandle, Color);
    WriteConsoleW(ConsoleHandle, Line, Length, &Written, nullptr);
    SetConsoleTextAttribute(ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void FWindowsConsoleOutputDevice::Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category)
{
    Serialize(Data, Verbosity, Category, -1.0);
}

void FWindowsConsoleOutputDevice::ExcludeCategory(const FName& InCategoryName)
{
    ExcludedCategories.Add(InCategoryName);
}
