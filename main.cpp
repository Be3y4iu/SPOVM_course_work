#include "BootSector.h"
#include "FR.h"
#include <clocale>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

#define MAX_OPEN_COUNT 1

static int dirFiles = 0;
static int notBadFiles = 0;
static int BadFiles = 0;

void getFilesCount(string first) 
{
	vector<string> file_names;
	using iterator = fs::recursive_directory_iterator;
	for (iterator iter(first, fs::directory_options::skip_permission_denied); iter != iterator{}; ++iter) 
	{
		try 
		{
			file_names.push_back(iter->path().generic_u8string());
		}
		catch (std::exception & e) 
		{
			cout << e.what() << endl;
		}
	}
	for (auto& fn : file_names) 
	{
		try 
		{
			if (fs::is_directory(fn))
				continue;
		}
		catch (std::exception & e) 
		{
			cout << e.what() << endl;
		}
		dirFiles++;
	}
	dirFiles += 50;
}

INT32 openDisk(const string& logicDriverName, HANDLE& hDisk) 
{
	string add = R"(\\.\)" + logicDriverName + ":";
	getFilesCount(add);
	UINT32 index = 0;
	for (; index < MAX_OPEN_COUNT; index++) 
	{
		hDisk = CreateFileA( add.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr, OPEN_EXISTING, 0, nullptr);
		if (hDisk != INVALID_HANDLE_VALUE) 
		{
			break;
		}
	}
	if (index == MAX_OPEN_COUNT) 
	{
		fprintf(stderr, "open disk filed in openDisk(), have opened %lld times\nError ID: %lu\n", INT64(MAX_OPEN_COUNT), GetLastError());
		return -1;
	}
	return 0;
}

INT32 getData(HANDLE hDisk, UINT64 offset, BYTE* dest, DWORD length) 
{
	void* p = static_cast<void*>(&offset);
	SetFilePointer(hDisk, *PLONG(p), PLONG(PBYTE(p) + 4), FILE_BEGIN);
	DWORD dwReadLength = 0;
	INT32 res = 0;
	res = ReadFile(hDisk, dest, length, &dwReadLength, nullptr);
	if (res == 0) {
		fprintf(stderr, "read data failed in getData(), read %lu/%lu byte(s)\n", dwReadLength, length);
		return -2;
	}
	return 0;
}

INT32 searchFile(HANDLE hDisk, INT64& offset) {
	offset = 0;
	BYTE data[BYTES_PER_SECTOR * SECTORS_PER_FR];
	INT32 res = 0;
	res = getData(hDisk, 0, data, BYTES_PER_SECTOR);
	if (res != 0) {
		fprintf(stderr, "get boot sector data failed in searchFile()\n");
		return res;
	}
	bootSector bs(data);
	INT64 fileCount = 0;
	string sfsID(reinterpret_cast<char*>(bs.fileSystemID), 4);
	if (sfsID != "NTFS") {
		fprintf(stderr, "this disk is not ntfs format\n");
		return -1;
	}
	FR* fr = nullptr;
	INT64 offsetOfSector = bs.startOfMFT * 8;
	while (fileCount < dirFiles) {
		fileCount++;
		res = getData(hDisk, offsetOfSector * BYTES_PER_SECTOR, data, BYTES_PER_SECTOR * SECTORS_PER_FR);
		if (res != 0) {
			fprintf(stderr, "get FR data filed in searchFile()\n");
			return res;
		}
		if (string(reinterpret_cast<char*>(data), 4) == "BAAD") {
			FRHeader FRH(data);
			if (FRH.isExist && !FRH.isDIR) {
				fr = new FR(data);
				if (fr->aName != nullptr) {
					BadFiles++;
					string fnameUNICODE(reinterpret_cast<char*>(fr->aName->content->fileName), fr->aName->content->nameLenth * 2);
					string fnameASCII;
					string::size_type index = 0;
					for (; index < fnameUNICODE.length(); index++) {
						if (index % 2 == 0) {
							fnameASCII.push_back(fnameUNICODE[index]);
						}
					}
					fprintf(stdout, "found file %s\n", fnameASCII.c_str());
				}
			}
		}
		else if (string(reinterpret_cast<char*>(data), 4) == "FILE") {
			FRHeader FRH(data);
			if (FRH.isExist && !FRH.isDIR) {
				fr = new FR(data);
				if (fr->aName != nullptr) {
					notBadFiles++;
					string fnameUNICODE(reinterpret_cast<char*>(fr->aName->content->fileName), fr->aName->content->nameLenth * 2);
					string fnameASCII;
					string::size_type index = 0;
					for (; index < fnameUNICODE.length(); index++) {
						if (index % 2 == 0) {
							fnameASCII.push_back(fnameUNICODE[index]);
						}
					}
					fprintf(stdout, "found file %s\n", fnameASCII.c_str());
				}
			}
		}
		offsetOfSector += SECTORS_PER_FR;
	}
	offset = offsetOfSector;
	SAFE_RELEASE_SINGLE_POINTER(fr);
	return 0;
}

INT32 main() {	
	setlocale(LC_ALL, "");

	char* logicDriverName = new char[80];
	fprintf(stdout, "enter logical drive ");
	fscanf_s(stdin, "%s", logicDriverName, 80);

	INT64 sectorNum = 0;

	HANDLE hDisk = nullptr;
	INT32 res = openDisk(string(logicDriverName), hDisk);
	if (res != 0) {
		fprintf(stderr, "open disk failed\n");
		system("pause");
		return res;
	}

	searchFile(hDisk, sectorNum);
	const int fileScanned = notBadFiles + BadFiles;
	cout << "Files scanned " << fileScanned << ". Bad files " << BadFiles << ". Not bad files " << notBadFiles << endl;
	printf("\n");
	system("pause");

	return 0;
}
