#include "pch.h"
#include "UObject/PrimaryAssetId.h"
#include "Scanner.h"

FPrimaryAssetId FPrimaryAssetId::ParseTypeAndName(const FString& TypeAndName)
{
    static auto ParseTypeAndName = Anchor::Optional<TGameValue<FPrimaryAssetId>* (*)(TGameValue<FPrimaryAssetId>*, const FString&)>(TEXT("FPrimaryAssetId::ParseTypeAndName"))
        .Exec(TEXT("/Script/FortniteUI.FortUIBlueprintFunctionLibrary:GetAssetFromTemplateId"))
        .Find("call 0x*; lea rdx, [rsp+0x*]; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    TGameValue<FPrimaryAssetId> Result;

    if (ParseTypeAndName)
    {
        ParseTypeAndName(&Result, TypeAndName);

        return Result.Get();
    }

    static auto ParseTypeAndNameView = Anchor::Function<TGameValue<FPrimaryAssetId>* (*)(TGameValue<FPrimaryAssetId>*, const TCHAR*, int32)>(TEXT("FPrimaryAssetId::ParseTypeAndName(FStringView)"))
        .Exec(TEXT("/Script/FortniteUI.FortUIBlueprintFunctionLibrary:GetAssetFromTemplateId"))
        .Find("lea rdx, [rsp+0x*]; mov rcx, r8; *; *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    ParseTypeAndNameView(&Result, *TypeAndName, TypeAndName.Len());

    return Result.Get();
}
