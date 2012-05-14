#ifndef _KLOG_H
#define _KLONG_H
#include "common.h"
#include "Klog.h"
NTSTATUS OpenLogFile(PDRIVER_OBJECT pDriverObject);
NTSTATUS HookDeviceUnload(PDRIVER_OBJECT pDriverObject);
extern "C" DriverEntry(PDRIVER_OBJECT pDriverObject, UNICODE_STRING RegisterPath);
#endif