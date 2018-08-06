#include <windows.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN // We don"t need lots of extra headers

// All device names from winioctl.h
LPSTR lpszDeviceNames[] = { "UNKNOWN", "FILE_DEVICE_BEEP", "FILE_DEVICE_CD_ROM", "FILE_DEVICE_CD_ROM_FILE_SYSTEM", "FILE_DEVICE_CONTROLLER",
						"FILE_DEVICE_DATALINK", "FILE_DEVICE_DFS", "FILE_DEVICE_DISK", "FILE_DEVICE_DISK_FILE_SYSTEM",
						"FILE_DEVICE_FILE_SYSTEM", "FILE_DEVICE_INPORT_PORT", "FILE_DEVICE_KEYBOARD", "FILE_DEVICE_MAILSLOT",
						"FILE_DEVICE_MIDI_IN", "FILE_DEVICE_MIDI_OUT", "FILE_DEVICE_MOUSE", "FILE_DEVICE_MULTI_UNC_PROVIDER",
						"FILE_DEVICE_NAMED_PIPE", "FILE_DEVICE_NETWORK", "FILE_DEVICE_NETWORK_BROWSER", "FILE_DEVICE_NETWORK_FILE_SYSTEM",
						"FILE_DEVICE_NULL", "FILE_DEVICE_PARALLEL_PORT", "FILE_DEVICE_PHYSICAL_NETCARD", "FILE_DEVICE_PRINTER",
						"FILE_DEVICE_SCANNER", "FILE_DEVICE_SERIAL_MOUSE_PORT", "FILE_DEVICE_SERIAL_PORT", "FILE_DEVICE_SCREEN",
						"FILE_DEVICE_SOUND", "FILE_DEVICE_STREAMS", "FILE_DEVICE_TAPE", "FILE_DEVICE_TAPE_FILE_SYSTEM",
						"FILE_DEVICE_TRANSPORT", "FILE_DEVICE_UNKNOWN", "FILE_DEVICE_VIDEO", "FILE_DEVICE_VIRTUAL_DISK", "FILE_DEVICE_WAVE_IN",
						"FILE_DEVICE_WAVE_OUT", "FILE_DEVICE_8042_PORT", "FILE_DEVICE_NETWORK_REDIRECTOR", "FILE_DEVICE_BATTERY",
						"FILE_DEVICE_BUS_EXTENDER", "FILE_DEVICE_MODEM", "FILE_DEVICE_VDM", "FILE_DEVICE_MASS_STORAGE", "FILE_DEVICE_SMB",
						"FILE_DEVICE_KS", "FILE_DEVICE_CHANGER", "FILE_DEVICE_SMARTCARD", "FILE_DEVICE_ACPI", "FILE_DEVICE_DVD",
						"FILE_DEVICE_FULLSCREEN_VIDEO", "FILE_DEVICE_DFS_FILE_SYSTEM", "FILE_DEVICE_DFS_VOLUME", "FILE_DEVICE_SERENUM",
						"FILE_DEVICE_TERMSRV", "FILE_DEVICE_KSEC", "FILE_DEVICE_FIPS", "FILE_DEVICE_INFINIBAND", "UNKNOWN", "UNKNOWN",
						"FILE_DEVICE_VMBUS", "FILE_DEVICE_CRYPT_PROVIDER", "FILE_DEVICE_WPD", "FILE_DEVICE_BLUETOOTH",
						"FILE_DEVICE_MT_COMPOSITE", "FILE_DEVICE_MT_TRANSPORT", "FILE_DEVICE_BIOMETRIC", "FILE_DEVICE_PMI", "FILE_DEVICE_EHSTOR",
						"FILE_DEVICE_DEVAPI", "FILE_DEVICE_GPIO", "FILE_DEVICE_USBEX", "FILE_DEVICE_CONSOLE", "FILE_DEVICE_NFP",
						"FILE_DEVICE_SYSENV", "FILE_DEVICE_VIRTUAL_BLOCK", "FILE_DEVICE_POINT_OF_SERVICE", "FILE_DEVICE_STORAGE_REPLICATION",
						"FILE_DEVICE_TRUST_ENV", "FILE_DEVICE_UCM", "FILE_DEVICE_UCMTCPCI", "FILE_DEVICE_PERSISTENT_MEMORY",
						"FILE_DEVICE_NVDIMM", "FILE_DEVICE_HOLOGRAPHIC", "FILE_DEVICE_SDFXHCI" };
LPSTR lpszAccessTypes[] = { "FILE_ANY_ACCESS", "FILE_READ_ACCESS", "FILE_WRITE_ACCESS", "FILE_READ_ACCESS + FILE_WRITE_ACCESS" };
LPSTR lpszMethods[] = { "METHOD_BUFFERED", "METHOD_IN_DIRECT", "METHOD_OUT_DIRECT", "METHOD_NEITHER" };

void decodeIOCTLWithPrint(DWORD dwIoctl) {
	// Thanks to: https://github.com/tandasat/WinIoCtlDecoder/blob/master/plugins/WinIoCtlDecoder.py
	DWORD dwDeviceType = DEVICE_TYPE_FROM_CTL_CODE(dwIoctl);
	DWORD dwAccess = ((DWORD)(dwIoctl >> 14)) & 3;
	DWORD dwControlCode = ((DWORD)(dwIoctl >> 2)) & 0xFFF;
	DWORD dwMethod = METHOD_FROM_CTL_CODE(dwIoctl);
	LPSTR lpszDeviceName = "UNKNOWN";
	LPSTR lpszAccessType = "UNKNOWN";
	LPSTR lpszMethod = "UNKNOWN";
	if (dwDeviceType <= 87) { // Check array out of bounds 
		lpszDeviceName = lpszDeviceNames[dwDeviceType];
	}
	if (dwAccess <= 4) {
		lpszAccessType = lpszAccessTypes[dwAccess];
	}
	if (dwMethod <= 4) {
		lpszMethod = lpszMethods[dwMethod];
	}
	printf("\tDevice Type = 0x%X ~ %s\n\tAccess = 0x%X ~ %s\n\tControl Code = 0x%X\n\tTransfer Type = 0x%X ~ %s\n", dwDeviceType, 
			lpszDeviceName, dwAccess, lpszAccessType, dwControlCode, dwMethod, lpszMethod);
	return(0);
}

int main(int argc, char* argv[]) {
	printf("%d\n", argc);
	if (argc > 2 || argc < 2) {
		printf("Usage: %s [Target Device]\n", argv[0]);
		printf("[?] Target device format: BASIC_DEVICE\n[?] This will be interpreted as: \\\\.\\BASIC_DEVICE\n");
		printf("[?] WinObj (Windows Sysinternals) may help you find the write device name if it is unknown\n");
		return(-1);
	}
	if (strlen(argv[1]) >= 0x900) { // Try to prevent heap overflow
		printf("[!] Input string too large");
		return(-1);
	}
	else {
		LPVOID lpWorkArea = malloc(0x1000);
		memcpy((BYTE*)lpWorkArea + 0x4, argv[1], strlen(argv[1])); // Leave room for front chars
		memset(lpWorkArea, '\\\\', 2); // Risky manual string addition 
		memset((BYTE*)lpWorkArea + 0x2, '\.', 1);
		memset((BYTE*)lpWorkArea + 0x3, '\\', 1);
		argv[1] = lpWorkArea; // argv[1] should now have the corrent syntax
	}
	printf("[i] Attemping to get a handle on: %s\n", argv[1]);
	HANDLE hDriver = CreateFileA(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	DWORD dwBytesOut = 0; // For output from DeviceIoControl
	LPVOID lpFakeBuffer = malloc(0x1000); // Fake buffer
	memset(lpFakeBuffer, 0x00, 0x1000 - 1);
	DWORD dwCurrentIoctl = 0;
	DWORD dwErrorsNotSupported = 0;
	DWORD dwLastError = 0; // For result checking of the flood process
	if (hDriver == INVALID_HANDLE_VALUE) { // Make sure we have a valid handle
		printf("[!] Unable to get a handle on the device\n");
		return(-1);
	}
	else {
		printf("[i] Starting IOCTL flooding process (This could take some time)\n");
		for (int i = 0; i <= 0xFFFFFFFF; i++) {
			dwCurrentIoctl = i;
			DeviceIoControl(hDriver, dwCurrentIoctl, lpFakeBuffer, 4, lpFakeBuffer, 4, &dwBytesOut, FALSE);
			dwLastError = GetLastError();
			switch (dwLastError) {
				case ERROR_ACCESS_DENIED: // 0x5
					printf("[!!!] Access is denied. Can not continue\n"); // Could really continue, but it would be pointless
					printf("Last Error: 0x%X\n", dwLastError);
					return(-1);
				case ERROR_NOT_READY: // 0x15
					printf("[!!!] Driver lock violation. Can not continue\n");
					printf("Last Error: 0x%X\n", dwLastError);
					return(-1);
				/*
				It is possible that there are more fatal error codes that would make looking for IOCTLs pointless.
				The ones in the above case statements are the only ones I"ve encountered.
				Remain wary...
				*/
				case ERROR_NOT_SUPPORTED: // 0x32
					dwErrorsNotSupported++; // Usually returned because the IOCTL is just not supported
					break;
				default: 
					printf("------------------------------\n"); // For style!
					printf("[i] Possible IOCTL: 0x%X\n", dwCurrentIoctl);
					decodeIOCTLWithPrint(dwCurrentIoctl); // Decode the IOCTL
					printf("[i] With last error status: 0x%X (See MSDN)\n", dwLastError);	
					printf("------------------------------\nPress [ENTER] to continue flooding");
					getchar(); // Pause so the user can read the data
					printf("[i] Resuming IOCTL flooding process\n");
			}
		}
		printf("[i] Number of ERROR_NOT_SUPPORTED (s): %d\n", dwErrorsNotSupported);
		printf("[i] ERROR_NOT_SUPPORTED are usually from non-valid IOCTLs");
	}
	return(0);
}
