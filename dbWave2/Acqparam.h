//  acqparam.h  acquisition parameters
//

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
	
public :
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
	CString csComment;			// 2 file annotation - used now as "csExpt"
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
	CString AddComments(CString psep, BOOL bExpl, CString csExpl, CString cscomment);
	void Read_v8_and_before(CArchive& ar, WORD version);

public :
	CWaveFormat();	// constructor
	~CWaveFormat();	// destructor
	CWaveFormat& operator = (const CWaveFormat& arg);	// operator redefinition
	long Write(CFile* datafile);	
	BOOL Read(CFile* datafile);
	virtual void Serialize(CArchive& ar);
	CString GetComments(CString separator, BOOL bExplanations=FALSE);
};


////////////////////////////////////////////////////////////////////////
// CWaveChan
// data acquisition definitions
// parameters describing one data acquisition channel
// these parameters are stored in the header of data acquisition files
// stored either under Asyst or Atlab
// Separate routines decode the headers from such files and store the
// parameters in CWaveFormat structure
////////////////////////////////////////////////////////////////////////
class CWaveChan : public CObject
{     
	DECLARE_SERIAL(CWaveChan);
public :
	CString		am_csComment;		// channel annotation
	short		am_adchannel;		// channel A/D 
	short		am_gainAD;			// channel A/D gain 

	CString		am_csheadstage;   	// headstage type (ASCII)
	short		am_gainheadstage;	// amplification factor at headstage level

	CString		am_csamplifier;		// instrumental amplifier
	short 		am_amplifierchan;	// amplifier channel
	short		am_gainpre;			// pre-filter amplification
	short		am_gainpost;		// post-filter amplification
	short		am_notchfilt;		// notch filter ON/off
	short		am_lowpass;			// low pass filter 0=DC 4....30000
	float		am_offset;			// input offset	
	CString		am_csInputpos;		// in+ filter settings GND= inactive input
	CString		am_csInputneg;		// in- filter settings GND= inactive input

	double		am_gainamplifier;	// double value to allow fractional gain 
	double		am_gaintotal;		// total gain as double
	double		am_resolutionV;		// resolution

public :
	CWaveChan();					//constructor
	CWaveChan(CWaveChan& arg);
	~CWaveChan();					//destructor
	virtual long Write(CFile *datafile);
	virtual BOOL Read(CFile *datafile);
	CWaveChan& operator = (const CWaveChan& arg); // redefinition operator = 	
	virtual void Serialize(CArchive & ar);

protected:
	WORD		am_version;
	CString		am_csversion;
	short		am_inputpos;	// values:GND(-1) DC(0) .1 1 10 30 100 300 Hz (*10)
	short		am_inputneg;	// ibidem.
	CString		GetCyberAmpInput(int val);
};

//////////////////////////////////////////////////////////////////////
// Class  CWaveChanArray                                            //
// table of CWaveChan                                               //
//////////////////////////////////////////////////////////////////////
class CWaveChanArray : public CObject
{
	DECLARE_SERIAL(CWaveChanArray);
protected:
	CArray <CWaveChan*, CWaveChan*>	m_chanArray;		// array of channels

public:
	CWaveChanArray();				// create array
	~CWaveChanArray();				// delete array
	long write(CFile *datafile);	// binary write
	BOOL read(CFile *datafile);		// binary read

	CWaveChanArray& operator = (const CWaveChanArray& arg);
	CWaveChan* get_p_channel(int i) const;

	int		channel_add();
	int		channel_add(CWaveChan* arg);
	int		channel_set_number(int i);
	void	channel_insert(int i);
	void	channel_remove(int i);
	void	channel_remove_all();
	int		channel_get_number() const;
	void Serialize(CArchive & ar) override;
};
