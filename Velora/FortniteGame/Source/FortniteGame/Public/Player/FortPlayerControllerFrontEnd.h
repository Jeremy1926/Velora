#pragma once

#include "Player/FortPlayerController.h"

namespace UACScannerFrontEnd
{
    struct FReport;
}

class AFortPlayerControllerFrontEnd : public AFortPlayerController
{
public:
    typedef AFortPlayerController Super;

    void OnReportAppEnvironmentSecurity(const UACScannerFrontEnd::FReport& ReportData);
};
