#include "pch.h"
#include "FortGameInstance.h"
#include "Scanner.h"

TSharedPtr<IAnalyticsProvider>& UFortGameInstance::GetServerAnalyticsProvider()
{
    static auto ServerAnalyticsProvider = []
    {
        auto Assign = Scanner::Reference(Scanner::String("AnalyticsET").Start().Get(), "call 0x*").Find("*");

        return Scanner::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"), Assign.Imm() + Assign.Disp());
    }();

    return *(TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider);
}
