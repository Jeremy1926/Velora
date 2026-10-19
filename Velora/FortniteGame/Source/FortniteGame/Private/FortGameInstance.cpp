#include "pch.h"
#include "FortGameInstance.h"
#include "Scanner.h"

TSharedPtr<IAnalyticsProvider>& UFortGameInstance::GetServerAnalyticsProvider()
{
    static auto ServerAnalyticsProvider = []
    {
        auto GetServerAnalyticsProvider = Scanner::String(TEXT("Changing GameSessionId from '%s' to '%s'")).Find("cmp *, 0x1; jnz *").Call();

        if (GetServerAnalyticsProvider.Is("lea rax, [rcx+0x*]; ret"))
            return Scanner::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"), (int64)GetServerAnalyticsProvider.Disp());

        auto Assign = Scanner::Reference(Scanner::String("AnalyticsET").Start().Get(), "call 0x*").Find("*");

        return Scanner::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"), Assign.Imm() + Assign.Disp());
    }();

    return *(TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider);
}
