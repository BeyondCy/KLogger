#ifndef _KLOG_H
#define _KLONG_H
#include "common.h"

NTSTATUS OpenLogFile(PDRIVER_OBJECT pDriverObject);
VOID HookDeviceUnload(PDRIVER_OBJECT pDriverObject);
extern "C" NTSTATUS DriverEntry( IN PDRIVER_OBJECT  pDriverObject, IN PUNICODE_STRING RegistryPath );
#endif