#ifndef  _KHOOK_H
#define _KHOOK_H
extern long g_PendingWrite;
NTSTATUS HookKeyboardDevice(PDRIVER_OBJECT pDriverObject);
NTSTATUS HookDevicePassThrough(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS  HookDeviceRead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS HookDeviceReadComplete(PDEVICE_OBJECT pDeviceObject, PIRP pIrp, PVOID Context);
#endif