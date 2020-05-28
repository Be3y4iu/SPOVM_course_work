#pragma once
#include<Windows.h>

#include<vector>

#define SECTORS_PER_FR 2

#define SAFE_RELEASE_SINGLE_POINTER(pointer)\
    if((pointer) != NULL)\
    {\
        delete (pointer);\
    }

#define SAFE_RELEASE_VECTOR_POINTER(pointer)\
    if(pointer != NULL)\
    {\
        delete[] pointer;\
    }

class FRHeader {
public:
	BYTE* data;

	BYTE magicNumber[4];
	WORD offsetToTheSequencceOfAttributesPart;
	DWORD realSizeOfFR;
	WORD flags;
	UINT64 fileReferenceToTheBaseFileRecord;

	bool isMainFR;
	bool isDIR;
	bool isExist;

	FRHeader(BYTE* _data);
};

class clasterFragments {
public:
	UINT64 begin;
	UINT64 lenth;

	clasterFragments();
};

class runList {
public:
	BYTE* data;
	std::vector<clasterFragments> cf;
	runList(BYTE* _data);
};

class attributeHeader {
public:
	BYTE* data;

	DWORD type;
	DWORD lenth;
	BYTE nonResidentFlag;
	WORD offsetToTheContentPart;

	DWORD lenthOfTheStream;

	UINT64 startingVCN;
	UINT64 lastVCN;
	WORD offsetToTheRunlist;
	UINT64 realSizeOfTheStream;
	runList* run;

	DWORD realOffsetToTheContentPart;
	bool isResident;

	attributeHeader(BYTE* _data);
	~attributeHeader();
};

class attributeNAMEContent {
public:
	BYTE* data;

	BYTE nameLenth;
	BYTE* fileName;

	attributeNAMEContent(BYTE* _data);
	~attributeNAMEContent();
};

class attributeDATAContent {
public:
	BYTE* data;
	attributeDATAContent(BYTE* _data);
};

class attributeIndexRootContent {
public:
	BYTE* data;
	attributeIndexRootContent(BYTE* _data);

};

class attributeIndexAllocationContent {
public:
	BYTE* data;

	attributeIndexAllocationContent(BYTE* _data);
};

class attributeBitmapContent {
public:
	BYTE* data;

	attributeBitmapContent(BYTE* _data);
};

template<typename T>
class attribute {
public:
	BYTE* data;
	attributeHeader* header;
	T* content;

	attribute(BYTE* _data);
	~attribute();
};


class FR {
public:
	BYTE* data;

	FRHeader* FRH;

	attribute<attributeNAMEContent>* aName;
	attribute<attributeDATAContent>* aData;
	attribute<attributeIndexRootContent>* aIndexRoot;
	attribute<attributeIndexAllocationContent>* aIndexAllocationAttribute;
	attribute<attributeBitmapContent>* aBitmap;

	FR(BYTE* _data);
	~FR();
};
