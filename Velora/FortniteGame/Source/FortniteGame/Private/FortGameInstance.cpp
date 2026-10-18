#include "pch.h"
#include "FortGameInstance.h"
#include "Scanner.h"

TSharedPtr<IAnalyticsProvider>& UFortGameInstance::GetServerAnalyticsProvider()
{
    static auto ServerAnalyticsProvider = []
    {
        auto Assign = Scanner::String(TEXT("TeamChatroom")).Start().Find("mov dword ptr [rsp+0x20], 0x2").Find("call 0x*").Find("*");

        return Scanner::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"), Assign.Imm() + Assign.Disp());
    }();

    return *(TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider);
}
