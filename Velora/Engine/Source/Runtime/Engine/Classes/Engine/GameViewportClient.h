#pragma once

#include "Engine/ScriptViewportClient.h"

class UConsole;

class UGameViewportClient : public UScriptViewportClient
{
public:
    UConsole* GetViewportConsole() const;

    void SetViewportConsole(UConsole* InViewportConsole);
};
