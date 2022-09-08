#pragma once
#include "AcqDataDoc.h"
class ADAcqDataDoc :
    public AcqDataDoc
{
	public:
	BOOL AcqDoc_DataAppendStart();
	BOOL AcqDoc_DataAppend(short* pBU, UINT uilbytesLength) const;
	BOOL AcqDoc_DataAppendStop() const;

};

