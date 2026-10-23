#pragma once

#include "Weapons/FortDecoTool.h"

class AFortDecoTool_ContextTrap : public AFortDecoTool
{
public:
    void ServerCreateBuildingAndSpawnDeco_Implementation(const FVector& BuildingLocation, const FRotator& BuildingRotation, const FVector& Location, const FRotator& Rotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const FVector& BuildingExtraPieceLocation);

    UFortItemDefinition* GetContextTrapItemDefinition() const;
};
