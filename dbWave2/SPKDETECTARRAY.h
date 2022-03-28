﻿#pragma once
#include "SpikeDetectArray.h"

// array of spike detection parameters
// 2 dimensions

class SPKDETECTARRAY : public CObject
{
	DECLARE_SERIAL(SPKDETECTARRAY)
public:
	BOOL bChanged; // flag set TRUE if contents has changed
	void SetChanArray(int acqchan, SpikeDetectArray* pspk);
	SpikeDetectArray* GetChanArray(int acqchan);

protected:
	WORD wversion; // version number
	CMapWordToPtr chanArrayMap; // array of CPtrArray
	void DeleteAll(); // erase all arrays of parmItems (and all parmItems within them)

public:
	SPKDETECTARRAY();
	~SPKDETECTARRAY() override;
	void Serialize(CArchive& ar) override;
};

