#pragma once


// CWaveFormat
// data acquisition definitions
// comments, sampling rate, etc
// these parameters are stored in the header of data acquisition files
// stored either under Asyst or Atlab
// Separate routines decode the headers from such files and store the
// parameters in CWaveFormat structure

class CWaveFormat : public CObject
{
	DECLARE_SERIAL(CWaveFormat);

public:
	WORD	wversion = 8;				// header version number

	CTime	acqtime;
	float	fullscale_volts = 20.0f;	// volts full scale, gain 1
	long	binspan = 4096;				// 2^12, 2^16
	long	binzero = 2048;				// 2048, 32768 (0 if 2's complement)

	short	mode_encoding = 0;			// 0=offset binary/1=straight/2=2scomplement
	short	mode_clock = 0;				// 0=internal, 1=external
	short	mode_trigger = 0;			// 0=internal, 1=external, 2=trigg_scan
	short	scan_count = 1;				// number of channels in scan list

	float	chrate = 10000.;			// channel sampling rate (Hz)
	long	sample_count = 1000;		// sample count
	float 	duration = 0.;				// file duration (s)

	short	trig_mode = 0;				// 0=soft, 1=keyboard, 2=trig_ext(TTL), 3=input+, 4=input-
	short	trig_chan = 0;				// (trig_mode=input) A/D channel input
	short	trig_threshold = 2048;		// (trig_mode=input) A/D channel input threshold value
	short 	data_flow = 0;				// 0 = continuous, 1 = burst
	short 	bufferNitems = 10;			// n buffers declared
	WORD 	buffersize = 1000;			// total size of one buffer (nb points [short])
	BOOL 	bOnlineDisplay = true;		// TRUE = online data display, FALSE=no display during acquisition
	BOOL 	bADwritetofile = false;		// TRUE = write data to disk,  FALSE=no saving
	BOOL	bUnicode = true;			// TRUE if unicode

	// comments
	CString csADcardName = _T("undefined");	// 1 I/O card name
	CString cs_comment = _T("");		// 2 file annotation - used now as "csExpt"
	CString csStimulus = _T("");		// 3 stimulation (chemical)
	CString csConcentration = _T("");	// 4 stimulus concentration
	CString csInsectname = _T("");		// 5 sensillum type
	CString csLocation = _T("");		// 6 location
	CString csSensillum = _T("");		// 7 sensillum nb
	CString csStrain = _T("");			// 8 name of the strain
	CString csOperator = _T("");		// 9 name of the operator
	CString csMoreComment = _T("");		// 10 free comment space
	CString csStimulus2 = _T("");		// 11 stimulus 2
	CString csConcentration2 = _T("");	// 12 concentration of stimulus 2
	CString csSex = _T("");				// 13 sex
	long	insectID = 0;				// 14
	long	sensillumID = 0;			// 15
	long	repeat = 0;					// 16
	long	repeat2 = 0;				// 17
	int		flag = 0;					// 18

	//------------------- Total : 33 variable members
protected:
	static CString	addComments(const CString& p_separator, const BOOL b_explanation, const CString& cs_explanation, const CString& cs_comment);
	void 			read_v8_and_before(CArchive& ar, WORD version);

public:
	CWaveFormat();
	~CWaveFormat();
	CWaveFormat& operator = (const CWaveFormat& arg);
	long Write(CFile* datafile);
	BOOL Read(CFile* datafile);
	void Serialize(CArchive& ar) override;
	CString GetComments(const CString& p_separator, BOOL b_explanations = FALSE) const;
};
