#pragma once

#include "NavMesh/RecastNavMesh.h"

class AFortNavMesh : public ARecastNavMesh
{
public:
    void SetUseGenerationBudgets(bool bUseGenerationBudgets);
};
