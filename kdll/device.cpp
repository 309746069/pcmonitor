#include "device.h"
#include "debug.h"
#include <stdio.h>

HANDLE OpenDevice()
{

	HANDLE hDevice =           // �������� ������ � ��������
		CreateFile(KMON_WIN32_DEVICE_NAME_W,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hDevice == INVALID_HANDLE_VALUE) {
		DebugPrint(L"ERROR: can not access driver %ws, error %d\n", KMON_WIN32_DEVICE_NAME_W, GetLastError());
		return NULL;
	}

	DebugPrint(L"OpenDevice=%p", hDevice);

	return hDevice;
}

DWORD NTAPI ControlDevice(HANDLE hDevice, DWORD Ioctl, PVOID Input, DWORD InputSize, PVOID Output, DWORD OutputSize, DWORD *pBytesReturned)
{
	DWORD Error = ERROR_SUCCESS;

	if (!DeviceIoControl(hDevice,
		Ioctl,
		Input, InputSize,	// Input
		Output, OutputSize,	// Output
		pBytesReturned,
		NULL)) {
		Error = GetLastError();
	}

	DebugPrint(L"ControlDevice:ioctl=%d err=%d\n", Ioctl, Error);

	return Error;
}


HWINSTA	DeviceOpenWinsta(WCHAR *lpszWindowStation)
{
	OPEN_WINSTA Request, Result;
	DWORD ResultBytes;
	DWORD Error;
	HANDLE hDevice = NULL;

	HWINSTA hResult = NULL;

	hDevice = OpenDevice();
	if (hDevice == NULL)
		return NULL;

	memset(&Request, 0, sizeof(Request));
	memset(&Result, 0, sizeof(Result));

	_snwprintf_s((WCHAR *)Request.WinstaName, RTL_NUMBER_OF(Request.WinstaName), _TRUNCATE, L"%ws", lpszWindowStation);

	Error = ControlDevice(hDevice, IOCTL_KMON_OPEN_WINSTA, &Request, sizeof(Request), &Result, sizeof(Result), &ResultBytes);
	if (Error != ERROR_SUCCESS) {
		DebugPrint(L"ControlDevice error=%d\n", Error);
		goto cleanup;
	}

	if (sizeof(Result) != ResultBytes) {
		DebugPrint(L"mismatch result size\n");
		goto cleanup;
	}
	
	if (Result.Error != ERROR_SUCCESS) {
		DebugPrint(L"Result.error=%d\n", Result.Error);
		goto cleanup;
	}
	
	hResult = (HWINSTA)Result.hWinsta;

cleanup:
	if (hDevice != NULL)
		CloseHandle(hDevice);

	return hResult;
}

DWORD	DeviceScreenShot(char *data, unsigned long dataSz, unsigned long sessionId, int type)
{
	KMON_SCREENSHOT Request, Result;
	DWORD ResultBytes;
	DWORD Error;
	HANDLE hDevice = NULL;

	hDevice = OpenDevice();
	if (hDevice == NULL)
		return NULL;

	memset(&Request, 0, sizeof(Request));
	memset(&Result, 0, sizeof(Result));
	
	Request.data = data;
	Request.dataSz = dataSz;
	Request.sessionId = sessionId;
	Request.type = type;

	Error = ControlDevice(hDevice, IOCTL_KMON_SCREENSHOT, &Request, sizeof(Request), &Result, sizeof(Result), &ResultBytes);
	if (Error != ERROR_SUCCESS) {
		DebugPrint(L"ControlDevice error=%d\n", Error);
		goto cleanup;
	}

	Error = Result.Error;
	if (Error != ERROR_SUCCESS) {
		DebugPrint(L"Result.error=%d\n", Error);
		goto cleanup;
	}

cleanup:
	if (hDevice != NULL)
		CloseHandle(hDevice);

	return Error;
}

HDESK	DeviceOpenDesktop(HWINSTA hWinsta, WCHAR *lpszDesktopName)
{
	OPEN_DESKTOP Request, Result;
	DWORD ResultBytes;
	DWORD Error;
	HDESK hResult = NULL;
	HANDLE hDevice = NULL;

	hDevice = OpenDevice();
	if (hDevice == NULL)
		return NULL;

	memset(&Request, 0, sizeof(Request));
	memset(&Result, 0, sizeof(Result));

	_snwprintf_s((WCHAR *)Request.DesktopName, RTL_NUMBER_OF(Request.DesktopName), _TRUNCATE, L"%ws", lpszDesktopName);
	Request.hWinsta = hWinsta;

	Error = ControlDevice(hDevice, IOCTL_KMON_OPEN_DESKTOP, &Request, sizeof(Request), &Result, sizeof(Result), &ResultBytes);
	if (Error != ERROR_SUCCESS) {
		DebugPrint(L"ControlDevice error=%d\n", Error);
		goto cleanup;
	}

	if (sizeof(Result) != ResultBytes) {
		DebugPrint(L"mismatch result size\n");
		goto cleanup;
	}

	if (Result.Error != ERROR_SUCCESS) {
		DebugPrint(L"Result.error=%d\n", Result.Error);
		goto cleanup;
	}

	hResult = (HDESK)Result.hDesktop;

cleanup:
	if (hDevice != NULL)
		CloseHandle(hDevice);

	return hResult;
}
