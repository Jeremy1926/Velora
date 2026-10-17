#include "pch.h"
#include "Kismet/GameplayStatics.h"
#include "Templates/UnrealTemplate.h"
#include "Scanner.h"

void UGameplayStatics::OpenLevel(const UObject* WorldContextObject, FName LevelName, bool bAbsolute, FString Options)
{
    static auto OpenLevel = (void (*)(const UObject*, FName, bool, FString))(Scanner::Base() + Scanner::FindFunction(Scanner::FindReference(Scanner::FindString(TEXT("WARNING: The map '%s' does not exist."))))->BeginAddress);

    OpenLevel(WorldContextObject, LevelName, bAbsolute, MoveTemp(Options));
}
