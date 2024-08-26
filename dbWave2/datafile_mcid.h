#pragma once
#include "datafile_X.h"

#define MAX_MCID_CHANNELS	10				// Hopefully gross overkill!
#define MAX_PROG_NAME		20				// Ditto

enum SPIKE_MODE
{
	HISTO_MODE,
	// just count the spikes into bins
	SINC_MODE,
	// convolve each spike with sin(x)/x
	MECH_MODE,
	// Use MCID info to make pulses
	RATE_MODE,
	// Convert spikes to spikes/second
	TIMING_MODE,
	// Output just spike times
	FILTER_MODE,
	// Filter the whole thing
	MAX_SPIKE_MODE
};

#define	SPIKE_MODES	{"Histogram",	"Sin(x)/x",	"Mechanical", "Spikes/sec",	"Spike timing",	"Filter"}

enum IO_MODE { IO_UPPER_LOWER, IO_UPPER_SPIKES, IO_LOWER_SPIKES, MAX_IO_MODE };

#define IO_MODES	{"Input upper, Output lower",	"Input upper, Output spikes",	"Input lower, Output spikes"};

using MCID_HEADER = struct
{
	char program_name[MAX_PROG_NAME]; // Zero-terminated
	UINT program_version; // ((V1*10+V2)*10+V3)*10+V4
	UINT mc_id_version; // ((V1*10+V2)*10+V3)*10+V4
	double sample_interval; // In seconds
	double sensitivity[MAX_MCID_CHANNELS];
	UINT number_of_channels; // Number of channels recorded
	UINT number_of_samples; // Total samples/channel
	size_t application_header; // sizeof the application header
	double initial_time; // Time starts at this value
};

// MCID files have an application header
// that must be defined first and then defined in the main header
// The application header must follow the main header
// This is the first one defined for Sinc1

using SINC1_MCID_HEADER_1 = struct
{
	double original_interval; // sample interval of original file (s)
	UINT original_number; // number of samples in original file
	UINT original_fields; // number of fields in original file
	double new_interval; // resampled sample interval (s)
	UINT new_number; // resampled number of points
	UINT block_size; // viewer block size
	double ap_height; // action potential height
	double ap_width; // action potential width
	SPIKE_MODE spike_mode; // histogram, sin(x)/x, mechanical, spikes/s
	IO_MODE io_mode; // input-output modes
	double count_time; // spikes/s count interval (s)
};

class CDataFileMCID :
	public CDataFileX
{
public:
	CDataFileMCID();
	DECLARE_DYNCREATE(CDataFileMCID)
	~CDataFileMCID() override;

public:
	BOOL read_data_infos(CWaveBuf* pBuf) override;
	int check_file_type(CString& cs_filename) override;

public:
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
