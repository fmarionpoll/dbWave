
#pragma once


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
public:
	CString		am_csComment;		// channel annotation
	short		am_adchannel{};		// channel A/D
	short		am_gainAD{};		// channel A/D gain

	CString		am_csheadstage;   	// headstage type (ASCII)
	short		am_gainheadstage{};	// amplification factor at headstage level

	CString		am_csamplifier;		// instrumental amplifier
	short 		am_amplifierchan{};	// amplifier channel
	short		am_gainpre{};		// pre-filter amplification
	short		am_gainpost{};		// post-filter amplification
	short		am_notchfilt{};		// notch filter ON/off
	short		am_lowpass{};		// low pass filter 0=DC 4....30000
	float		am_offset{};		// input offset
	CString		am_csInputpos;		// in+ filter settings GND= inactive input
	CString		am_csInputneg;		// in- filter settings GND= inactive input

	double		am_gainamplifier{};	// double value to allow fractional gain
	double		am_gaintotal{};		// total gain as double
	double		am_resolutionV{};	// resolution

	int			am_CEDchanID{};		// id used for CED
	double		am_CEDscale{};		// scale used by CED
	double		am_CEDoffset{};		// offset used by CED
	long long	am_CEDticksPerSample{};
	long long	am_CEDmaxTimeInTicks{};

public:
	CWaveChan();
	CWaveChan(CWaveChan& arg);
	~CWaveChan();
	virtual long Write(CFile* datafile);
	virtual BOOL Read(CFile* datafile);
	CWaveChan& operator = (const CWaveChan& arg); 
	void Serialize(CArchive& ar) override;

protected:
	WORD		am_version;
	CString		am_csversion;
	short		am_inputpos;	// values:GND(-1) DC(0) .1 1 10 30 100 300 Hz (*10)
	short		am_inputneg;	// ibidem.
	static CString	GetCyberAmpInput(int value);
};
