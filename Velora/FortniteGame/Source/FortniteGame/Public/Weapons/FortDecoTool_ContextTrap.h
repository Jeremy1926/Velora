#pragma once

#include "Weapons/FortDecoTool.h"

class AFortDecoTool_ContextTrap : public AFortDecoTool
{
public:
    void ServerCreateBuildingAndSpawnDeco_Implementation(const TGameValue<FVector>& GameBuildingLocation, const TGameValue<FRotator>& GameBuildingRotation, const TGameValue<FVector>& GameLocation, const TGameValue<FRotator>& GameRotation, EBuildingAttachmentType InBuildingAttachmentType, bool bSpawnDecoOnExtraPiece, const TGameValue<FVector>& GameBuildingExtraPieceLocation);

    UFortItemDefinition* GetContextTrapItemDefinition() const;
};
