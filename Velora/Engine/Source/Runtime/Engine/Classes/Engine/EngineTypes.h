#pragma once

enum class ELevelCollectionType
{
    DynamicSourceLevels,
    DynamicDuplicatedLevels,
    StaticLevels
};

enum ENetRole
{
    ROLE_None,
    ROLE_SimulatedProxy,
    ROLE_AutonomousProxy,
    ROLE_Authority,
    ROLE_MAX,
};

enum ENetDormancy
{
    DORM_Never,
    DORM_Awake,
    DORM_DormantAll,
    DORM_DormantPartial,
    DORM_Initial,
    DORN_MAX,
};
