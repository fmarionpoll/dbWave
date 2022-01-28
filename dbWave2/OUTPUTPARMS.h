#pragma once

class OUTPUTPARMS : public CObject
{
	DECLARE_SERIAL(OUTPUTPARMS)
	OUTPUTPARMS();
	~OUTPUTPARMS();
	OUTPUTPARMS& operator = (const OUTPUTPARMS& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL	bChanged{};								// flag set TRUE if contents has changed
	WORD	wversion{};								// version number

	BOOL	bON;									// channel authorized or not
	int		iChan;									// chan
	BOOL	bDigital;								// digital or Analog
	double	dAmplitudeMaxV;							// maximum amplitude of the signal
	double	dAmplitudeMinV;							// minimum amplitude of the signal
	double	dFrequency;								// frequency of the output signal
	int		iWaveform;								// 0=sinusoid, 1=square, 2=triangle, 3=m_sequence, 4=noise
	CString	csFilename;								// filename to output
	CIntervalsAndLevels	stimulussequence;
	CIntervalsAndWordsSeries sti;

	int		mseq_iRatio;							// m_mSeqRatio;		// Shifts/sample interval
	int		mseq_iDelay;							// m_mSeqDelay;		// Delay M-sequence and offset by this number of samples
	int		mseq_iSeed;								// m_mSeqSeed;		// Sequence seed or zero=random seed
	double	dummy1;									// m_mSeqAmplitude; // M-sequence amplitude
	double	dummy2;									// m_mSeqOffset;	// Add this offset before multiplication
	BOOL	bStart;
	int		num;
	UINT	bit33;
	UINT	count;
	UINT	bit1{};
	double	ampUp{};
	double	ampLow{};

	double	noise_dAmplitV;
	double	noise_dFactor;
	double	noise_dOffsetV;
	int		noise_iDelay;
	BOOL	noise_bExternal;
	double	lastamp{};
	double	lastphase{};
	double	value;
};
