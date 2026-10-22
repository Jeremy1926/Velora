#include "pch.h"
#include "UObject/PrimaryAssetId.h"
#include "Scanner.h"

FPrimaryAssetId FPrimaryAssetId::ParseTypeAndName(const FString& TypeAndName)
{
    static auto ParseTypeAndName = Anchor::Function<FPrimaryAssetId (*)(const FString&)>(TEXT("FPrimaryAssetId::ParseTypeAndName"))
        .Exec(TEXT("/Script/FortniteUI.FortUIBlueprintFunctionLibrary:GetAssetFromTemplateId"))
        .Find("call 0x*; lea rdx, [rsp+0x*]; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    return ParseTypeAndName(TypeAndName);
}
