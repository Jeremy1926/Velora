#include "pch.h"
#include "Logging/LogCategory.h"
#include "Scanner.h"

FLogCategoryBase::FLogCategoryBase(const TCHAR* CategoryName, ELogVerbosity::Type InDefaultVerbosity, ELogVerbosity::Type InCompileTimeVerbosity)
    : CategoryFName(NAME_None)
{
    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 25)
    {
        static auto SetVerbosity = Anchor::Function<void (*)(FLogCategoryBase*, ELogVerbosity::Type)>(TEXT("FLogCategoryBase::SetVerbosity"))
            .String(TEXT("LogFortBuilding"))
            .Find("mov dl, 0x*; mov word ptr [0x*], 0x*; lea rcx, [0x*]; mov [0x*], eax; call 0x*")
            .Find("call 0x*")
            .Target()
            .Optional()
            .Get();

        if (SetVerbosity)
        {
            static auto GetSuppressionOwner = Anchor::Function<uint8* (*)()>(TEXT("FLogSuppressionInterface::Get"))
                .String(TEXT("LogFortBuilding"))
                .Find("call 0x*; lea rdx, [0x*]; mov rcx, [rax+0x*]; mov rax, [rcx]; call [rax]")
                .Target()
                .Get();
            static auto SuppressionInterface = Anchor::Offset(TEXT("FLogSuppressionInterface::Get interface"))
                .String(TEXT("LogFortBuilding"))
                .Find("call 0x*; lea rdx, [0x*]; mov rcx, [rax+0x*]; mov rax, [rcx]; call [rax]")
                .Find("mov rcx, [rax+0x*]")
                .Disp();

            FName Name(CategoryName);

            DefaultVerbosity = InDefaultVerbosity;
            CompileTimeVerbosity = InCompileTimeVerbosity;
            memcpy(&CompileTimeVerbosity + 1, &Name, sizeof(uint32));
            SetVerbosity(this, InDefaultVerbosity);

            if (CompileTimeVerbosity > ELogVerbosity::NoLogging)
            {
                auto Interface = *(void**)(GetSuppressionOwner() + SuppressionInterface);
                auto AssociateSuppress = (void (*)(void*, FLogCategoryBase*))(*(void***)Interface)[0];

                AssociateSuppress(Interface, this);
            }

            return;
        }

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
