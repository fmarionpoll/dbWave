#pragma once

// spike detection parameters

class SPKDETECTPARM : public CObject
{
	DECLARE_SERIAL(SPKDETECTPARM)
public:
	BOOL bChanged; // flag set TRUE if contents has changed
	WORD wversion; // version number

	CString comment; // free comment (for ex describing the method)

	int detectWhat; // 0=spikes, 1=stimulus
#define DETECT_SPIKES		0
#define DETECT_STIMULUS		1
	int detectMode; // stim: 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
#define MODE_ON_OFF		0
#define MODE_OFF_ON		1
#define MODE_ON_ON		2
#define MODE_OFF_OFF	3
	int detectFrom; // detection method - (stored as WORD): 0=chan; 1 = tag
	int detect_channel; // source channel
	int detect_transform; // detect from data transformed - i = transform method cf AcqDataDoc
	int detect_threshold; // value of threshold
	float detectThresholdmV; // mV value of threshold
	BOOL compensateBaseline; // compensate for baseline shift?

	int extract_channel; // channel number of data to extract
	int extract_transform; // transformed data extracted
	int extract_n_points; // spike length (n data pts)
	int detect_pre_threshold; // offset spike npts before threshold
	int detect_refractory_period; // re-start detection n pts after threshold

protected:
	void ReadVersionlessthan6(CArchive& ar, int version);
	void ReadVersion6(CArchive& ar);
	void ReadVersion7(CArchive& ar);

public:
	SPKDETECTPARM();
	~SPKDETECTPARM() override;
	SPKDETECTPARM& operator =(const SPKDETECTPARM& arg);
	void Serialize(CArchive& ar) override;
};
