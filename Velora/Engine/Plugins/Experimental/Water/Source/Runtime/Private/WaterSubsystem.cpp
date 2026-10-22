#include "pch.h"
#include "WaterSubsystem.h"
#include "Scanner.h"

UWaterSubsystem* UWaterSubsystem::GetWaterSubsystem(const UWorld* InWorld)
{
    static auto GetWaterSubsystem = Anchor::Function<UWaterSubsystem* (*)(const UWorld*)>(TEXT("UWaterSubsystem::GetWaterSubsystem"))
        .Exec(TEXT("/Script/Water.BuoyancyManager:GetBuoyancyComponentManager"))
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    return InWorld ? GetWaterSubsystem(InWorld) : nullptr;
}

float UWaterSubsystem::GetOceanFloodHeight() const
{
    static auto OceanFloodHeight = Anchor::Offset(TEXT("UWaterSubsystem::OceanFloodHeight"))
        .Exec(TEXT("/Script/Water.WaterSubsystem:GetOceanFloodHeight"))
        .Find("mov eax, [rcx+0x*]")
        .Disp();

    return *(const float*)((const uint8*)this + OceanFloodHeight);
}
