#pragma once

#include "HAL/Platform.h"

enum ERemoveStreamingViews
{
    RemoveStreamingViews_Normal,
    RemoveStreamingViews_All
};

struct IStreamingManager
{
    virtual ~IStreamingManager()
    {
    }

    static struct FStreamingManagerCollection& Get();

    virtual void Tick(float DeltaTime, bool bProcessEverything = false) = 0;

    void RemoveStreamingViews(ERemoveStreamingViews RemovalType);
};

struct FStreamingManagerCollection : public IStreamingManager
{
};
