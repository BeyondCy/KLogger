#ifndef  _KHOOK_H
#define _KHOOK_H
extern long g_PendingWrite;
NTSTATUS HookKeyboardDevice(PDRIVER_OBJECT pDriverObject);
<<<<<<< HEAD
NTSTATUS  HookDevicePassThrough(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
=======
NTSTATUS HookDevicePassThrough(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
>>>>>>> 37fbd77ca42a7799a32fd6019fa8c0bcf5541923
NTSTATUS  HookDeviceRead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS HookDeviceReadComplete(PDEVICE_OBJECT pDeviceObject, PIRP pIrp, PVOID Context);
#endif