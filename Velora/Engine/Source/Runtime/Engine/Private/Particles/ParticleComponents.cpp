#include "pch.h"
#include "Scanner.h"

bool& GIsAllowingParticles = *(bool*)Scanner::String(TEXT("CreateRenderState_Concurrent @ %fs %s")).Find("cmp byte ptr [0x*], 0x0").Target(TEXT("GIsAllowingParticles"));
