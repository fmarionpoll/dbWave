#pragma once
#include "StimLevelArray.h"

class OUTPUTPARMS : public CObject
{
	DECLARE_SERIAL(OUTPUTPARMS)
	OUTPUTPARMS();
	~OUTPUTPARMS();
	OUTPUTPARMS& operator = (const OUTPUTPARMS& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL	b_changed{false};					// flag set TRUE if contents has changed
	WORD	wversion{1};						// version number

	BOOL	bON{false};							// channel authorized or not
	int		iChan{0};							// chan
	BOOL	bDigital{false};					// digital or Analog
	double	dAmplitudeMaxV{1.};					// maximum amplitude of the signal
	double	dAmplitudeMinV{-1.};				// minimum amplitude of the signal
	double	dFrequency{1000.};					// frequency of the output signal
	int		iWaveform{0};						// 0=sinusoid, 1=square, 2=triangle, 3=m_sequence, 4=noise
	CString	csFilename{};						// filename to output
	CIntervalsAndLevels	stimulussequence{};
	CIntervalsAndWordsSeries sti{};

	int		mseq_iRatio{1000};					// m_mSeqRatio{};		// Shifts/sample interval
	int		mseq_iDelay{0};						// m_mSeqDelay{};		// Delay M-sequence and offset by this number of samples
	int		mseq_iSeed{0};						// m_mSeqSeed{};		// Sequence seed or zero=random seed
	double	dummy1{0.};							// m_mSeqAmplitude{}; // M-sequence amplitude
	double	dummy2{0.};							// m_mSeqOffset{};	// Add this offset before multiplication
	BOOL	bStart{false};
	int		num{512};
	UINT	bit33{1};
	UINT	count{1};
	UINT	bit1{0};
	double	ampUp{0.};
	double	ampLow{0.};

	double	noise_dAmplitV{1.};
	double	noise_dFactor{1.};
	double	noise_dOffsetV{0.};
	int		noise_iDelay{0};
	BOOL	noise_bExternal{false};
	double	lastamp{0.};
	double	lastphase{0.};
	double	value{0.};
};
