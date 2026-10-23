#include "pch.h"
#include "Athena/Vehicles/FortAthenaExitCraft.h"
#include "Scanner.h"

TMulticastDelegate<void, AFortAthenaExitCraft*, float>& AFortAthenaExitCraft::GetOnExitVehicleCullDistChange()
{
    static auto OnExitVehicleCullDistChange = Anchor::Data<TMulticastDelegate<void, AFortAthenaExitCraft*, float>*>(TEXT("AFortAthenaExitCraft::OnExitVehicleCullDistChange"))
        .String(TEXT("ReceiveBeginPlay"))
        .Find("lea *, [0x*]")
        .Target()
        .Reference("mov ?dx, *")
        .Start()
        .Caller()
        .SlotIn(TEXT("/Script/Engine.Default__Actor"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortAthenaExitCraft"))
        .Find("call 0x*")
        .Target()
        .Find("mov rax, [0x*]")
        .Target()
        .Get();

    return *OnExitVehicleCullDistChange;
}
