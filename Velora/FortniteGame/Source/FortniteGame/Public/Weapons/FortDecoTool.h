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
    void ServerCreateBuildingAndSpawnDeco_Implementation(const FVector& BuildingLocation, const FRotator& BuildingRotation, const FVector& Location, const FRotator& Rotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const FVector& BuildingExtraPieceLocation);

    AFortPlayerController* GetPlayerController() const;

    UFortItemDefinition* GetDecoItemDefinition() const;
};
