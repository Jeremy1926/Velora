#pragma once

class FOutputDeviceRedirector;

extern bool& GIsClient;
extern bool& GIsServer;

FOutputDeviceRedirector* GetGlobalLogSingleton();

#define GLog GetGlobalLogSingleton()
