#include "pch.h"
#include "Engine/GameViewportClient.h"
#include "UObject/UnrealType.h"

UConsole* UGameViewportClient::GetViewportConsole() const
{
    static auto ViewportConsole = GetClass()->FindPropertyByName(FName(TEXT("ViewportConsole")));

    return *ViewportConsole->ContainerPtrToValuePtr<UConsole*>(this);
}

void UGameViewportClient::SetViewportConsole(UConsole* InViewportConsole)
{
    static auto ViewportConsole = GetClass()->FindPropertyByName(FName(TEXT("ViewportConsole")));

    *ViewportConsole->ContainerPtrToValuePtr<UConsole*>(this) = InViewportConsole;
}
