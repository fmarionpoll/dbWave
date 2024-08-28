#pragma once
#include "Intervals.h"
#include "IntervalPoints.h"

class output_parameters final : public CObject
{
	DECLARE_SERIAL(output_parameters)
	output_parameters();
	~output_parameters() override;
	output_parameters& operator =(const output_parameters& arg);
	void Serialize(CArchive& ar) override;

	BOOL b_changed{false}; // flag set TRUE if contents has changed
	WORD w_version{1}; // version number

	BOOL b_on{false}; // channel authorized or not
	int i_chan{0}; // chan
	BOOL b_digital{false}; // digital or Analog
	double d_amplitude_max_v{1.}; // maximum amplitude of the signal
	double d_amplitude_min_v{-1.}; // minimum amplitude of the signal
	double d_frequency{1000.}; // frequency of the output signal
	int i_waveform{0}; // 0=sinusoid, 1=square, 2=triangle, 3=m_sequence, 4=noise
	CString csFilename{}; // filename to output
	CIntervals stimulus_sequence{};
	CIntervalPoints sti{};

	int m_seq_i_ratio{1000}; // m_mSeqRatio{};		// Shifts/sample interval
	int m_seq_i_delay{0}; // m_mSeqDelay{};		// Delay M-sequence and offset by this number of samples
	int m_seq_i_seed{0}; // m_mSeqSeed{};		// Sequence seed or zero=random seed
	double dummy1{0.}; // m_mSeqAmplitude{}; // M-sequence amplitude
	double dummy2{0.}; // m_mSeqOffset{};	// Add this offset before multiplication
	BOOL b_start{false};
	int num{512};
	UINT bit33{1};
	UINT count{1};
	UINT bit1{0};
	double amp_up{0.};
	double amp_low{0.};

	double noise_d_amplitude_v{1.};
	double noise_d_factor{1.};
	double noise_d_offset_v{0.};
	int noise_i_delay{0};
	BOOL noise_b_external{false};
	double last_amp{0.};
	double last_phase{0.};
	double value{0.};
};
