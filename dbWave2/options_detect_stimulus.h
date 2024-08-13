#pragma once
#include "options_detect_spikes.h"

class options_detect_stimulus : public CObject
{
	DECLARE_SERIAL(options_detect_stimulus)
public:
	BOOL bChanged = FALSE; // flag set TRUE if contents has changed
	WORD wversion = 2; // version number

	int nItems = 0; // number of items/line (nb of chans/detection) ?
	int SourceChan = 0; // source channel
	int TransformMethod = 0; // detect from data transformed - i = transform method cf AcqDataDoc
	int DetectMethod = 0; // unused
	int Threshold1 = 0; // value of threshold 1
	int bMode = MODE_ON_OFF; // 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);

public:
	options_detect_stimulus();
	~options_detect_stimulus() override;
	options_detect_stimulus& operator =(const options_detect_stimulus& arg);
	void Serialize(CArchive& ar) override;
};
