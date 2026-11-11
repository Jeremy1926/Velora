#pragma once

#include "Weapons/FortWeapon.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"

class AFortPlayerController;
class ABuildingSMActor;
class UFortItemDefinition;
enum class EBuildingAttachmentType : uint8;

class AFortDecoTool : public AFortWeapon
{
public:
    static UClass* StaticClass();

    void SpawnDecoInternal(const TGameValue<FVector>& Location, const TGameValue<FRotator>& Rotation, ABuildingSMActor* AttachedActor, EBuildingAttachmentType AttachmentType, bool bFreeDeco);

    void FinishedSpawnAction(bool bFreeDeco);

    void ServerCreateBuildingAndSpawnDeco_Implementation(const TGameValue<FVector>& GameBuildingLocation, const TGameValue<FRotator>& GameBuildingRotation, const TGameValue<FVector>& GameLocation, const TGameValue<FRotator>& GameRotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const TGameValue<FVector>& GameBuildingExtraPieceLocation);

    AFortPlayerController* GetPlayerController() const;

    UFortItemDefinition* GetDecoItemDefinition() const;
};
