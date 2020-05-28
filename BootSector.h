#pragma once
#include<Windows.h>

#define BYTES_PER_SECTOR 512

class bootSector {
public:
	BYTE* data;

	BYTE fileSystemID[8]{};
	WORD bytesPerSector;
	BYTE sectorsPerCluster;
	UINT64 startOfMFT;

	bootSector(BYTE* _data);
};