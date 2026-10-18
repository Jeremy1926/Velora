#include "pch.h"
#include "ContentStreaming.h"
#include "Scanner.h"

FStreamingManagerCollection& IStreamingManager::Get()
{
    static auto Get = (FStreamingManagerCollection& (*)())Scanner::String(TEXT("DemoRec")).Last("mov edx, 0x1; call 0x*").Last("call 0x*").Target(TEXT("IStreamingManager::Get"));

    return Get();
}

void IStreamingManager::RemoveStreamingViews(ERemoveStreamingViews RemovalType)
{
    static auto RemoveStreamingViews = (void (*)(IStreamingManager*, ERemoveStreamingViews))Scanner::String(TEXT("DemoRec")).Last("mov edx, 0x1; call 0x*").Call().Get(TEXT("FStreamingManagerCollection::RemoveStreamingViews"));

    RemoveStreamingViews(this, RemovalType);
}
