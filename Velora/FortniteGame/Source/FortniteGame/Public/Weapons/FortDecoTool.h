#pragma once

#include "Weapons/FortWeapon.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"

class AFortPlayerController;
class UFortItemDefinition;
enum class EBuildingAttachmentType : uint8;

class AFortDecoTool : public AFortWeapon
{
public:
    void ServerCreateBuildingAndSpawnDeco_Implementation(const TGameValue<FVector>& GameBuildingLocation, const TGameValue<FRotator>& GameBuildingRotation, const TGameValue<FVector>& GameLocation, const TGameValue<FRotator>& GameRotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const TGameValue<FVector>& GameBuildingExtraPieceLocation);

    AFortPlayerController* GetPlayerController() const;

    UFortItemDefinition* GetDecoItemDefinition() const;
};
