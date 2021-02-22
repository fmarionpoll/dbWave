#pragma once

////////////////////////////////////////////////////////////////////////
// CWaveFormat
// data acquisition definitions
// comments, sampling rate, etc
// these parameters are stored in the header of data acquisition files
// stored either under Asyst or Atlab
// Separate routines decode the headers from such files and store the
// parameters in CWaveFormat structure
////////////////////////////////////////////////////////////////////////

class CWaveFormat : public CObject
{
	DECLARE_SERIAL(CWaveFormat);

public:
	WORD	wversion;			// header version number

	CTime	acqtime;
	float	fullscale_Volts;	// volts full scale, gain 1
	long	binspan;			// 2^12, 2^16
	long	binzero;			// 2048, 32768 (0 if 2's complement)

	short	mode_encoding;		// 0=offset binary/1=straight/2=2scomplement
	short	mode_clock;			// 0=internal, 1=external
	short	mode_trigger;		// 0=internal, 1=external, 2=trigg_scan
	short	scan_count;			// number of channels in scan list

	float	chrate;				// channel sampling rate (Hz)
	long	sample_count;		// sample count
	float 	duration;			// file duration (s)

	short	trig_mode;			// 0=soft, 1=keyboard, 2=trig_ext(TTL), 3=input+, 4=input-
	short	trig_chan;			// (trig_mode=input) A/D channel input
	short	trig_threshold;		// (trig_mode=input) A/D channel input threshold value
	short 	data_flow;			// 0 = continuous, 1 = burst
	short 	bufferNitems;		// n buffers declared
	WORD 	buffersize;			// total size of one buffer (nb points [short])
	BOOL 	bOnlineDisplay;		// TRUE = online data display, FALSE=no display during acquisition
	BOOL 	bADwritetofile;		// TRUE = write data to disk,  FALSE=no saving
	BOOL	bUnicode;			// TRUE if unicode

	// comments
	CString csADcardName;		// 1 I/O card name
	CString cs_comment;			// 2 file annotation - used now as "csExpt"
	CString csStimulus;			// 3 stimulation (chemical)
	CString csConcentration;	// 4 stimulus concentration
	CString csInsectname;		// 5 sensillum type
	CString csLocation;			// 6 location
	CString csSensillum;		// 7 sensillum nb
	CString csStrain;			// 8 name of the strain
	CString csOperator;			// 9 name of the operator
	CString csMoreComment;		// 10 free comment space
	CString csStimulus2;		// 11 stimulus 2
	CString csConcentration2;	// 12 concentration of stimulus 2
	CString csSex;				// 13 sex
	long	insectID;
	long	sensillumID;
	long	repeat;
	long	repeat2;

	//------------------- Total : 33 variable members
protected:
	static CString	addComments(const CString& p_separator, BOOL bExpl, const CString& csExpl, const CString& cscomment);
	void 			read_v8_and_before(CArchive& ar, WORD version);

public:
	CWaveFormat();	// constructor
	~CWaveFormat();	// destructor
	CWaveFormat& operator = (const CWaveFormat& arg);	// operator redefinition
	long Write(CFile* datafile);
	BOOL Read(CFile* datafile);
	void Serialize(CArchive& ar) override;
	CString GetComments(const CString& separator, BOOL bExplanations = FALSE) const;
};
