#include "pch.h"
#include "Kismet/KismetTextLibrary.h"
#include "Scanner.h"

FText UKismetTextLibrary::Conv_StringToText(const FString& InString)
{
    static auto Conv_StringToText = Anchor::Function<FText (*)(const FString&)>(TEXT("UKismetTextLibrary::Conv_StringToText"))
        .Native(TEXT("/Script/Engine.KismetTextLibrary:Conv_StringToText"))
        .Get();

    return Conv_StringToText(InString);
}
