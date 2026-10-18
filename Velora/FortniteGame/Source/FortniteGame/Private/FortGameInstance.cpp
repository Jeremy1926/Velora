#include "pch.h"
#include "FortGameInstance.h"
#include "Scanner.h"

TSharedPtr<IAnalyticsProvider>& UFortGameInstance::GetServerAnalyticsProvider()
{
    static auto ServerAnalyticsProvider = Scanner::String(TEXT("TeamChatroom")).Start().Find("call 0x*; add r*, 0x???*").Find("add r*, 0x???*").Imm();

    return *(TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider);
}
