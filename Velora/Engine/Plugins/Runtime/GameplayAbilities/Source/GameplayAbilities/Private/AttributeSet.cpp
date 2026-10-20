#include "pch.h"
#include "AttributeSet.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Misc/EngineVersion.h"
#include "Scanner.h"

FGameplayAttribute::FGameplayAttribute(UProperty* NewProperty)
{
    auto& Version = FEngineVersion::Current();

    if (Version.GetMajor() > 4 || Version.GetMinor() >= 25)
    {
        static auto GameplayAttribute = Anchor::Function<FGameplayAttribute* (*)(FGameplayAttribute*, UProperty*)>(TEXT("FGameplayAttribute::FGameplayAttribute"))
            .String(TEXT("IncomingDuration"))
            .Find("mov rdx, [0x*]; lea rcx, *; call 0x*")
            .Call()
            .Get();

        GameplayAttribute(this, NewProperty);
        return;
    }

    AttributeName = NewProperty ? NewProperty->GetName() : FString();
    *(UProperty**)&Attribute = NewProperty;
    *(UStruct**)((uint8*)&Attribute + sizeof(UProperty*)) = NewProperty ? (UStruct*)NewProperty->GetOuter() : nullptr;
}

float FGameplayAttributeData::GetCurrentValue() const
{
    return CurrentValue;
}
