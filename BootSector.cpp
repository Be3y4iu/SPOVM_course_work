#include"BootSector.h"

bootSector::bootSector(BYTE* _data) {
	data = _data;

	memcpy(fileSystemID, data + 0x3, 0x8);
	bytesPerSector = *(WORD*)(data + 0xB);
	sectorsPerCluster = *(BYTE*)(data + 0xD);
	startOfMFT = *(UINT64*)(data + 0x30);

}