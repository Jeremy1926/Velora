#pragma once

#include "Engine/ScriptViewportClient.h"

class UConsole;
class UWorld;

class UGameViewportClient : public UScriptViewportClient
{
public:
    UConsole* GetViewportConsole() const;

    void SetViewportConsole(UConsole* InViewportConsole);

    UWorld* GetWorld() const;
};
