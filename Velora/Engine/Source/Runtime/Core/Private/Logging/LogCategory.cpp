#include "pch.h"
#include "Logging/LogCategory.h"
#include "Scanner.h"

FLogCategoryBase::FLogCategoryBase(const TCHAR* CategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity)
    : CategoryFName(NAME_None)
{
    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 25)
    {
        static auto Constructor = Scanner::Function<void (*)(FLogCategoryBase*, const FName&, ELogVerbosity::Type, ELogVerbosity::Type)>(Scanner::String(TEXT("LogFortBuilding")).Call(2).Get(TEXT("FLogCategoryBase::FLogCategoryBase")));

        Constructor(this, FName(CategoryName), InDefaultVerbosity, InCompileTimeVerbosity);
        return;
    }

    static auto Constructor = Scanner::Function<void (*)(FLogCategoryBase*, const TCHAR*, ELogVerbosity::Type, ELogVerbosity::Type)>(Scanner::String(TEXT("LogFortBuilding")).Call().Get(TEXT("FLogCategoryBase::FLogCategoryBase")));

    Constructor(this, CategoryName, InDefaultVerbosity, InCompileTimeVerbosity);
}
