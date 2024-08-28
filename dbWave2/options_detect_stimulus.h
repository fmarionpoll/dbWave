#pragma once
#include "options_detect_spikes.h"

class options_detect_stimulus : public CObject
{
	DECLARE_SERIAL(options_detect_stimulus)
public:
	BOOL b_changed  {FALSE}; // flag set TRUE if contents has changed
	WORD w_version  {2}; // version number

	int n_items  {0}; // number of items/line (nb of chans/detection) ?
	int source_chan { 0}; // source channel
	int transform_method  {0}; // detect from data transformed - i  transform method cf AcqDataDoc
	int detect_method  {0}; // unused
	int threshold1  {0}; // value of threshold 1
	int b_mode  {MODE_ON_OFF}; // 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);

public:
	options_detect_stimulus();
	~options_detect_stimulus() override;
	options_detect_stimulus& operator =(const options_detect_stimulus& arg);
	void Serialize(CArchive& ar) override;
};
