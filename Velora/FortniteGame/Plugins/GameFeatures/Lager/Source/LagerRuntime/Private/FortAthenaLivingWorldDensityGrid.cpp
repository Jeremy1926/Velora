#include "pch.h"
#include "FortAthenaLivingWorldDensityGrid.h"
#include "GameFramework/Actor.h"
#include "Scanner.h"

int32 FFortAthenaLivingWorldDensityGrid::NextActorUpdateIndex = 0;

bool FFortAthenaLivingWorldDensityGrid::IsInitialized() const
{
    static auto Flags = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::bInitialized"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [rcx+0x*], 0x*")
        .Disp();

    static auto Mask = (uint8)Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::bInitialized"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("test byte ptr [rcx+0x*], 0x*")
        .Imm();

    return Flags > 0 && (*((const uint8*)this + Flags) & Mask);
}

FIntVector FFortAthenaLivingWorldDensityGrid::GetCellCoordinates(const FVector& Location) const
{
    static auto GetCellCoordinates = Anchor::Function<FIntVector* (*)(const FFortAthenaLivingWorldDensityGrid*, FIntVector*, const FVector*)>(TEXT("FFortSpatialGrid::GetCellCoordinates"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:QueryEventBudget"))
        .Find("movaps xmm2, xmm0; lea rdx, [rsp+0x*]; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*", 2)
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Get();

    FIntVector Cell{ 0, 0, 0 };

    return *GetCellCoordinates(this, &Cell, &Location);
}

bool FFortAthenaLivingWorldDensityGrid::AddActor(AActor* InActor)
{
    if (!InActor || !IsInitialized())
        return false;

    auto& ActorToDataIndexMap = GetActorToDataIndexMap();
    const FObjectKey ActorKey(InActor);

    if (ActorToDataIndexMap.Find(ActorKey))
        return false;

    const FIntVector Cell = GetCellCoordinates(InActor->GetActorLocation());
    auto ActorDataAllocation = GetActorDataArray().AddUninitialized();
    auto ActorData = new (ActorDataAllocation) FFortAthenaLivingWorldDensityData();

    ActorData->Actor = InActor;
    ActorData->OccupiedCells.Add(Cell);

    ActorToDataIndexMap.Add(ActorKey, ActorDataAllocation.Index);
    GetCellToActorDataIndices().FindOrAdd(Cell).Add(ActorDataAllocation.Index);

    return true;
}

void FFortAthenaLivingWorldDensityGrid::UpdateActors(int32 MaxActorUpdates)
{
    auto& ActorDataArray = GetActorDataArray();
    const int32 MaxIndex = ActorDataArray.GetMaxIndex();

    if (!IsInitialized() || !MaxIndex)
        return;

    for (int32 Count = 0; Count < MaxIndex && MaxActorUpdates > 0; Count++)
    {
        const int32 ActorDataIndex = NextActorUpdateIndex++ % MaxIndex;

        if (!ActorDataArray.IsAllocated(ActorDataIndex))
            continue;

        auto Actor = (AActor*)ActorDataArray[ActorDataIndex].Actor.Get();

        if (!Actor)
            continue;

        SetActorDataCell(ActorDataIndex, GetCellCoordinates(Actor->GetActorLocation()));
        MaxActorUpdates--;
    }
}

void FFortAthenaLivingWorldDensityGrid::SetActorDataCell(int32 ActorDataIndex, const FIntVector& Cell)
{
    auto& OccupiedCells = GetActorDataArray()[ActorDataIndex].OccupiedCells;

    if (OccupiedCells.Num() == 1 && OccupiedCells[0] == Cell)
        return;

    auto& CellToActorDataIndices = GetCellToActorDataIndices();

    for (auto& OccupiedCell : OccupiedCells)
    {
        auto ActorDataIndices = CellToActorDataIndices.Find(OccupiedCell);

        if (!ActorDataIndices)
            continue;

        ActorDataIndices->RemoveSingleSwap(ActorDataIndex);

        if (!ActorDataIndices->Num())
            CellToActorDataIndices.Remove(OccupiedCell);
    }

    OccupiedCells.Empty();
    OccupiedCells.Add(Cell);
    CellToActorDataIndices.FindOrAdd(Cell).Add(ActorDataIndex);
}

TMap<FIntVector, TArray<int32>>& FFortAthenaLivingWorldDensityGrid::GetCellToActorDataIndices()
{
    static auto CellToActorDataIndices = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::CellToActorDataIndices"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov r8, rdi; lea rdx, [rsp+0x*]")
        .Disp();

    return *(TMap<FIntVector, TArray<int32>>*)((uint8*)this + CellToActorDataIndices);
}

TMap<FObjectKey, int32>& FFortAthenaLivingWorldDensityGrid::GetActorToDataIndexMap()
{
    static auto ActorToDataIndexMap = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::ActorToDataIndexMap"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov rcx, *; mov [rsp+0x*], rax")
        .Disp();

    return *(TMap<FObjectKey, int32>*)((uint8*)this + ActorToDataIndexMap);
}

TSparseArray<FFortAthenaLivingWorldDensityData>& FFortAthenaLivingWorldDensityGrid::GetActorDataArray()
{
    static auto ActorDataArray = Anchor::Offset(TEXT("FFortAthenaLivingWorldDensityGrid::ActorDataArray"))
        .Native(TEXT("/Script/LagerRuntime.FortAthenaLivingWorldManager:OnSpawnedActorDestroy"))
        .Find("lea rcx, [rbx+0x*]; mov rdx, rbp; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lea *, [rbx+0x*]; mov rax, [*]; mov rdi, [rax+rcx*8+0x8]")
        .Disp();

    return *(TSparseArray<FFortAthenaLivingWorldDensityData>*)((uint8*)this + ActorDataArray);
}
