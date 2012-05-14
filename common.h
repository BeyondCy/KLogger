#ifndef _COMMON_H
#define _COMMON_H
extern "C"{
#include <ntddk.h>
}
#include <ntddkbd.h>
typedef struct _KEY_DATA
{
	LIST_ENTRY ListEntry;
	char KeyData;
	char KeyFlags;
}KEY_DATA, *PKEY_DATA;

struct KEY_STATE 
{
	bool kSHIFT; //if the shift key is pressed 
	bool kCAPSLOCK; //if the caps lock key is pressed down
	bool kCTRL; //if the control key is pressed down
	bool kALT; //if the alt key is pressed down
};

typedef struct _DEVICE_EXTENSION{
	PDEVICE_OBJECT pKeyboardDevice;//保存attach后的设备
	HANDLE hFile;
	bool bThreadTerminate;
	PETHREAD pThreadObj;
	KSEMAPHORE sem;
	LIST_ENTRY MessageList;
	KSPIN_LOCK kLock;//insert taillist use
	struct KEY_STATE kState;//scancode use
}DEVICE_EXTENSION, *PDEVICE_EXTIONSION;
typedef PDEVICE_EXTIONSION PDEVICE_EXTENSION;

#endif