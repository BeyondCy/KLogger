#ifndef _KWRITE_H
#define _KWRITE_H
NTSTATUS InitWriteFileThread(PDRIVER_OBJECT pDriverObject);
VOID WriteFileThread(PVOID Context);
#endif