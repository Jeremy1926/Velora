#include "pch.h"
#include "Analytics/FortAnalytics.h"
#include "Scanner.h"
#include "hde/hde64.h"

TSharedPtr<IAnalyticsProvider> UFortAnalytics::CreateAnalyticsProvider(const FString& UserID, const FString& GameSessionID, const FString& Platform, EAnalyticsProviderType ProviderType, const FString& GameState)
{
    static auto CreateAnalyticsProvider = []
    {
        auto Function = Scanner::FindFunction(Scanner::FindStringReference(TEXT("TeamChatroom")));
        auto End = Scanner::FindFunctionEnd(Function);

        for (auto Address = (const uint8_t*)Scanner::Base() + Function->BeginAddress; Address < End;)
        {
            hde64s Instruction;
            auto Length = hde64_disasm(Address, &Instruction);

            if (Instruction.opcode == 0xE8)
            {
                hde64s Assignment;
                hde64_disasm(Address + Length, &Assignment);

                if (Assignment.opcode == 0x81 && Assignment.rex_w && Assignment.modrm_mod == 3 && Assignment.modrm_reg == 0)
                    return (TSharedPtr<IAnalyticsProvider> (*)(const FString&, const FString&, const FString&, EAnalyticsProviderType, const FString&))(Address + Length + (int32_t)Instruction.imm.imm32);
            }

            Address += Length;
        }

        return (TSharedPtr<IAnalyticsProvider> (*)(const FString&, const FString&, const FString&, EAnalyticsProviderType, const FString&))nullptr;
    }();

    return CreateAnalyticsProvider(UserID, GameSessionID, Platform, ProviderType, GameState);
}
