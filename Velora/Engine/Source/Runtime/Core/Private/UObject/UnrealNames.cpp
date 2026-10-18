#include "pch.h"
#include "UObject/NameTypes.h"
#include "Scanner.h"

FName::FName(const WIDECHAR* Name, EFindName FindType)
{
    static auto Constructor = (void (*)(FName*, const WIDECHAR*, EFindName))Scanner::String(TEXT("Illegal call to StaticFindObject() while collecting garbage!")).Start().Find("mov r8d, 0x1").Call().Get();

    Constructor(this, Name, FindType);
}

FString FName::ToString() const
{
    static auto ToString = (void (*)(const FName*, FString&))Scanner::String(TEXT("Failed to find function %s in %s")).Start().Call(3).Get();

    FString Out;

    ToString(this, Out);

    return Out;
}
