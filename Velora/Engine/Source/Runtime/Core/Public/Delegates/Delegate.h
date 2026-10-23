#pragma once

#include "Delegates/MulticastDelegateBase.h"
#include "Delegates/DelegateSignatureImpl.inl"

#define FUNC_DECLARE_MULTICAST_DELEGATE(MulticastDelegateName, ...) \
    typedef TMulticastDelegate<__VA_ARGS__> MulticastDelegateName;

#define DECLARE_DELEGATE_RetVal_ThreeParams(ReturnValueType, DelegateName, Param1Type, Param2Type, Param3Type) typedef TDelegate<ReturnValueType(Param1Type, Param2Type, Param3Type)> DelegateName;

#define DECLARE_MULTICAST_DELEGATE(DelegateName) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void)
#define DECLARE_MULTICAST_DELEGATE_OneParam(DelegateName, Param1Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type)
#define DECLARE_MULTICAST_DELEGATE_TwoParams(DelegateName, Param1Type, Param2Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type, Param2Type)
#define DECLARE_MULTICAST_DELEGATE_FourParams(DelegateName, Param1Type, Param2Type, Param3Type, Param4Type) FUNC_DECLARE_MULTICAST_DELEGATE(DelegateName, void, Param1Type, Param2Type, Param3Type, Param4Type)

DECLARE_MULTICAST_DELEGATE(FSimpleMulticastDelegate);
