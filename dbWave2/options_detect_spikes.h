#pragma once

// spike detection parameters

class options_detect_spikes final : public CObject
{
	DECLARE_SERIAL(options_detect_spikes)
public:
	BOOL b_changed{0}; // flag set TRUE if contents has changed
	WORD w_version {7}; // version number

	CString comment; // free comment (for ex describing the method)
#define DETECT_SPIKES		0
#define DETECT_STIMULUS		1
	
#define MODE_ON_OFF		0
#define MODE_OFF_ON		1
#define MODE_ON_ON		2
#define MODE_OFF_OFF	3

	int detect_what{ DETECT_SPIKES }; // 0=spikes, 1=stimulus
	int detect_mode{MODE_ON_OFF}; // stim: 0: ON/OFF (up/down); 1: OFF/ON (down/up); 2: ON/ON (up/up); 3: OFF/OFF (down, down);
	int detect_from{0}; // detection method - (stored as WORD): 0=chan; 1 = tag
	int detect_channel {0}; // source channel
	int detect_transform{13}; // detect from data transformed - i = transform method cf AcqDataDoc
	int detect_threshold_bin{0}; // value of threshold
	float detect_threshold_mv{0.5f}; // mV value of threshold
	BOOL compensate_baseline{FALSE}; // compensate for baseline shift?

	int extract_channel{0}; // channel number of data to extract
	int extract_transform{13}; // transformed data extracted
	int extract_n_points{60}; // spike length (n data pts)
	int detect_pre_threshold{20}; // offset spike npts before threshold
	int detect_refractory_period{20}; // re-start detection n pts after threshold

protected:
	void read_v5(CArchive& ar, int version);
	void read_v6(CArchive& ar);
	void serialize_v7(CArchive& ar);

public:
	options_detect_spikes();
	~options_detect_spikes() override;
	options_detect_spikes& operator =(const options_detect_spikes& arg);
	void Serialize(CArchive& ar) override;
};
