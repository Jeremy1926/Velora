#include "pch.h"
#include "Player/FortPlayerControllerFrontEnd.h"
#include "Scanner.h"
#include "Detour.h"

void AFortPlayerControllerFrontEnd::OnReportAppEnvironmentSecurity(const UACScannerFrontEnd::FReport& ReportData)
{
    DETOUR(AFortPlayerControllerFrontEnd, OnReportAppEnvironmentSecurity, Anchor::Hook()
        .String(TEXT("LoadThreshold"))
        .Start()
        .Find("mov dword ptr [*], 0xff4effff")
        .Start()
        .Optional()
        .Inline());
}
