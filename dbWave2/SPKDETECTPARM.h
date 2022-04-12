#pragma once

// spike detection parameters

class SPKDETECTPARM : public CObject
{
	DECLARE_SERIAL(SPKDETECTPARM)
public:
	BOOL b_changed; // flag set TRUE if contents has changed
	WORD w_version; // version number

	CString comment; // free comment (for ex describing the method)

	int detect_what; // 0=spikes, 1=stimulus
#define DETECT_SPIKES		0
#define DETECT_STIMULUS		1
	int detect_mode; // stim: 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
#define MODE_ON_OFF		0
#define MODE_OFF_ON		1
#define MODE_ON_ON		2
#define MODE_OFF_OFF	3
	int detect_from; // detection method - (stored as WORD): 0=chan; 1 = tag
	int detect_channel; // source channel
	int detect_transform; // detect from data transformed - i = transform method cf AcqDataDoc
	int detect_threshold_bin; // value of threshold
	float detect_threshold_mv; // mV value of threshold
	BOOL compensate_Baseline; // compensate for baseline shift?

	int extract_channel; // channel number of data to extract
	int extract_transform; // transformed data extracted
	int extract_n_points; // spike length (n data pts)
	int detect_pre_threshold; // offset spike npts before threshold
	int detect_refractory_period; // re-start detection n pts after threshold

protected:
	void Read_v5(CArchive& ar, int version);
	void Read_v6(CArchive& ar);
	void Serialize_v7(CArchive& ar);

public:
	SPKDETECTPARM();
	~SPKDETECTPARM() override;
	SPKDETECTPARM& operator =(const SPKDETECTPARM& arg);
	void Serialize(CArchive& ar) override;
};
