#include "pch.h"
#include "Logging/LogCategory.h"
#include "Scanner.h"

FLogCategoryBase::FLogCategoryBase(const TCHAR* CategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity)
    : CategoryFName(NAME_None)
{
    static auto Constructor = Scanner::Function<void (*)(FLogCategoryBase*, const TCHAR*, ELogVerbosity::Type, ELogVerbosity::Type)>(Scanner::String(TEXT("LogFortBuilding")).Call().Get(TEXT("FLogCategoryBase::FLogCategoryBase")));

    Constructor(this, CategoryName, InDefaultVerbosity, InCompileTimeVerbosity);
}
