#pragma once

class OPTIONS_ACQDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_ACQDATA)
	OPTIONS_ACQDATA();
	~OPTIONS_ACQDATA();
	OPTIONS_ACQDATA& operator = (const OPTIONS_ACQDATA& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL			bChanged;						// flag set TRUE if contents has changed
	WORD			m_wversion;						// version number

	CString			csBasename;
	CString			csPathname;
	int				exptnumber{};
	int				insectnumber;
	int				iundersample;
	BOOL			baudiblesound;
	BOOL			bChannelType;					// OLx_CHNT_SINGLEENDED or OLx_CHNT_DIFFERENTIAL

	CStringArray	csA_stimulus;
	CStringArray	csA_concentration;
	CStringArray	csA_stimulus2;
	CStringArray	csA_concentration2;
	CStringArray	csA_insect;
	CStringArray	csA_location;
	CStringArray	csA_sensillum;
	CStringArray	csA_strain;
	CStringArray	csA_operatorname;
	CStringArray	csA_sex;
	CStringArray	csA_expt;

	int				icsA_stimulus;
	int				icsA_concentration;
	int				icsA_stimulus2;
	int				icsA_concentration2;
	int				icsA_insect;
	int				icsA_location;
	int				icsA_sensillum;
	int				icsA_strain;
	int				icsA_operatorname;
	int				icsA_sex;
	int				icsA_repeat;
	int				icsA_repeat2;
	int				icsA_expt;
	int				izoomCursel;					// display zoom factor (acq)

	CWaveFormat		waveFormat;						// wave format
	CWaveChanArray	chanArray;						// channels descriptors
	float			sweepduration;					// display parameter
};
