#include "pch.h"
#include "Logging/LogCategory.h"
#include "Scanner.h"

FLogCategoryBase::FLogCategoryBase(const TCHAR* CategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity)
    : CategoryFName(NAME_None)
{
    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 25)
    {
        static auto Constructor = Anchor::Function<void (*)(FLogCategoryBase*, const FName&, ELogVerbosity::Type, ELogVerbosity::Type)>(TEXT("FLogCategoryBase::FLogCategoryBase"))
            .String(TEXT("LogFortBuilding"), "lea rdx, [0x*]; mov r8b, *; mov r9b, *; call 0x*")
            .Find("call 0x*")
            .Target()
            .Call(2)
            .Or()
            .String(TEXT("LogFortBuilding"))
            .Call(2)
            .Get();

        Constructor(this, FName(CategoryName), InDefaultVerbosity, InCompileTimeVerbosity);
        return;
    }

    static auto Constructor = Anchor::Function<void (*)(FLogCategoryBase*, const TCHAR*, ELogVerbosity::Type, ELogVerbosity::Type)>(TEXT("FLogCategoryBase::FLogCategoryBase"))
        .String(TEXT("LogFortBuilding"))
        .Call()
        .Get();

    Constructor(this, CategoryName, InDefaultVerbosity, InCompileTimeVerbosity);
}
