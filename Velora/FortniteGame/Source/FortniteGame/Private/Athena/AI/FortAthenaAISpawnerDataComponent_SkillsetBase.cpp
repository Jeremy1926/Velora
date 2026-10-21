#include "pch.h"
#include "Athena/AI/FortAthenaAISpawnerDataComponent_SkillsetBase.h"
#include "Scanner.h"

float UFortAthenaAISpawnerDataComponent_SkillsetBase::GetSkill()
{
    static auto GetSkill = Anchor::VirtualFunction<float (*)(UFortAthenaAISpawnerDataComponent_SkillsetBase*)>(TEXT("UFortAthenaAISpawnerDataComponent_SkillsetBase::GetSkill_Implementation"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAISpawnerDataComponent_SkillsetBase:GetSkill"))
        .Find("call [rax+0x*]")
        .Get();

    return GetSkill(this);
}

TArray<UClass*> UFortAthenaAISpawnerDataComponent_SkillsetBase::GetSkillSets()
{
    static auto GetSkillSets = Anchor::VirtualFunction<TArray<UClass*>* (*)(UFortAthenaAISpawnerDataComponent_SkillsetBase*, TArray<UClass*>*)>(TEXT("UFortAthenaAISpawnerDataComponent_SkillsetBase::GetSkillSets_Implementation"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAISpawnerDataComponent_SkillsetBase:GetSkillSets"))
        .Find("call [rax+0x*]")
        .Get();

    TArray<UClass*> SkillSets;
    GetSkillSets(this, &SkillSets);

    return SkillSets;
}

bool UFortAthenaAISpawnerDataComponent_SkillsetBase::ShouldUseMatchMMRToOverrideSkillLevel()
{
    static auto ShouldUseMatchMMRToOverrideSkillLevel = Anchor::VirtualFunction<bool (*)(UFortAthenaAISpawnerDataComponent_SkillsetBase*)>(TEXT("UFortAthenaAISpawnerDataComponent_SkillsetBase::ShouldUseMatchMMRToOverrideSkillLevel_Implementation"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaAISpawnerDataComponent_SkillsetBase:ShouldUseMatchMMRToOverrideSkillLevel"))
        .Find("call [rax+0x*]")
        .Get();

    return ShouldUseMatchMMRToOverrideSkillLevel(this);
}
