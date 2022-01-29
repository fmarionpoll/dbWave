#pragma once

#include <OLERRORS.H>
#include <Olxdaapi.h>
#include <OLTYPES.H>
#include <Olxdadefs.h>
//#include "WinUser.h"

// Registration structure
using REG_CODE = struct _REG_CODE
{
	char user_name[100]; // information for registering the user
	UINT code; // code # the user must know
	BOOL registered; // is it?
	UINT scramble_key; // random key for scrambling (encrypting) user name
};

#define STRLEN	100				// general string length

//---------------- messages information -----------------------------------------
#define	MAX_MESSAGES	3

enum ASF_PACK_MESSAGE { MESS_FATAL, MESS_INFORM, MESS_INFORM_CANCEL, MESS_QUERY, MESS_QUERY_CANCEL };

#define MESSAGE(x,y) CASF_Pack_Data::Message(x,y)


///////////////////////////////////////////////////////////////////////
// DATA VALUES

#define MAX_DATA			50000000	// maximum of 50 million points
#define	MAX_EXTERNAL_NOISE	5000000		// maximum five million points
#define MAX_GAIN_VALUES		10			// maximum number of pre-amplifier gains
#define	MAX_AD_BUFFERS		2			// Number of A/D buffers avaialble
#define	MAX_DA_BUFFERS		2			// Number of D/A buffers avaialble

///////////////////////////////////////////////////////////////////////
// modes of operation
enum MODE { SINGLE_CHANNEL, DOUBLE_CHANNEL, TRIPLE_CHANNEL, NUMBER_OF_MODES };

#define			MODE_NAMES	{_T("Single"), _T("Double"), _T("Triple")};
#define			CHANNELS	{1,					2,					3}

///////////////////////////////////////////////////////////////////////
// ADC pre-amplifier gains
#define MAX_ADC_GAINS	4
#define ADC_GAIN_VALUE	{1.0,	2.0,	4.0,	8.0}
#define ADC_GAIN_TEXT	{_T("1"), _T("2"), _T("4"), _T("8")}

///////////////////////////////////////////////////////////////////////
// ANALOG encoding modes
enum A_ENCODING { ENC_BINARY, ENC_2SCOMP };

///////////////////////////////////////////////////////////////////////
// Windows messages for boards and other functions
enum WM_CODES
{
	WM_UPDATE_RATIO = WM_USER + 102,
	// Update the ratio display
	WM_CLOSE_RATIO,
	DT_ADC_EVENT,
	DT_DAC_EVENT
}; // Close the ratio display

///////////////////////////////////////////////////////////////////////
// Available sample intervals
#define NUMBER_OF_INTERVALS	10
// sample intervals in decimal seconds
#define	D_INTERVALS		{0.05,		0.02,		0.01,		0.005,	0.002,	0.001,	0.0005,	0.0002,	0.0001,	0.00005}
// and in text
#define S_INTERVALS		{_T("50.0"), _T("20.0"), _T("10.0"), _T("5.0"), _T("2.0"), _T("1.0"), _T("0.5"), _T("0.2"), _T("0.1"), _T("0.05")}

///////////////////////////////////////////////////////////////////////
// Data storage can be shown as samples or time
enum STORE_MODE { SAMPLES_STORED, TIME_STORED, NUMBER_OF_STORE_MODES };

///////////////////////////////////////////////////////////////////////
// Ratio display information
enum RATIO_CHOICE { RATIO_CHOICE_1, RATIO_CHOICE_2, RATIO_CHOICE_3, RATIO_CHOICE_4, MAX_RATIO_CHOICES };

#define			RATIO_CHOICE_TEXT	{_T("Prepulse/First pulse"), _T("First pulse/Prepulse"), _T("Prepulse"), _T("First pulse")};


///////////////////////////////////////////////////////////////////////
// Stimulus scaling
#define NUMBER_OF_STIMULUS_SCALINGS 7
#define	STIMULUS_SCALING		{ -100.0,	 -50.0,		0.0,	 1.0,		 10.0,	 50.0,	100.0}
#define STIMULUS_SCALING_TEXT	{_T("-0.1"), _T("-0.05"), _T("0"), _T("0.001"), _T("0.01"), _T("0.05"), _T("0.1")}

///////////////////////////////////////////////////////////////////////
// Options that get stored in registry

using TWO_STEP = struct _TWO_STEP
{
	BOOL ts_enable; // Enable two steps
	double prepulse_amplitude; // Amplitude of prepulse
	double prepulse_duration; // Duration of prepulse
	UINT prepulse_number; // Number of prepulses
	double step_0_amplitude; // Amplitude of first step in volts
	double step_1_amplitude; // Amplitude of second step in volts
	double step_0_duration; // Duration of first step (s)
	double step_1_duration; // Duration of second step (s)
	double step_delay; // Delay between two steps (s)
	double step_interval; // Interval from first step to next pair (s)
	BOOL ratio; // Show ratio window
	double max_ratio; // For ratio display
	double min_ratio; // For ratio display
	RATIO_CHOICE ratio_choice; // What to divide by what?
	UINT ratio_channel; // Channel=0,1 or 2
};

using IV = struct _IV
{
	// IV Curve info
	double ivBaseline; // Baseline voltage
	UINT ivDelay; // Delay IV by the number of samples
	BOOL ivEnable; // Generate IV curve
	double ivStart; // Start voltage (mV)
	double ivStep; // Step voltage (mV)
	double ivFinal; // Final voltage
	double ivDuration; // Duration of each step (ms)
	UINT ivStimulusScaling; // Patch clamp amplifier setting
	BOOL ivMark; // Mark epochs with zero current
};

///////////////////////////////////////////////////////////////////////
// Data storage in MCID (Multi-channel integer data) file

// MCID files have an optional application header that must be defined first
// The application header must follow the main header
// This is the sixth one defined for this program

using DAQ_MCID_HEADER_9 = struct _DAQ_MCID_HEADER_9
{
	// M-sequence info
	double mSeqAmplitude; // M-sequence amplitude
	BOOL mSeqEnable; // Generate M-sequence
	double mSeqOffset; // Add this offset before multiplication
	UINT mSeqDelay; // Delay M-sequence and offset by this number of samples
	UINT mSeqRatio; // Shifts/sample interval
	// Noise info
	double noiseAmplitude; // Noise amplitude
	UINT noiseEnable; // Generate noise
	double noiseFactor; // Multiply noise by this factor
	double noiseOffset; // Add this offset before multiplication
	UINT noiseDelay; // Delay noise and offset at start and end
	// Stimulus info
	double stimulusAmplitude; // Delay before stimulus
	double stimulusDuration; // Duration of stimulus
	BOOL stimulateEnable; // enable stimulation
	// Two-step
	TWO_STEP ts;
};

#define MAX_MCID_CHANNELS	10				// Hopefully gross overkill!
#define MAX_PROG_NAME		20				// Ditto

using MCID_HEADER = struct _MCID_HEADER
{
	char program_name[MAX_PROG_NAME]; // Zero-terminated
	UINT program_version; // ((V1*10+V2)*10+V3)*10+V4
	UINT mcid_version; // ((V1*10+V2)*10+V3)*10+V4
	double sample_interval; // In seconds
	double sensitivity[MAX_MCID_CHANNELS];
	UINT number_of_channels; // Number of channels recorded
	UINT number_of_samples; // Total samples/channel
	size_t application_header; // sizeof the application header
	double initial_time; // Time starts at this value
};


///////////////////////////////////////////////////////////////////////
// Data Translation board information

using BOARD = struct _BOARD
{
	HDEV hdrvr; // Driver
	HDASS hdass_ad; // A/D Subsystem
	HDASS hdass_da; // D/A Subsystem
	ECODE status;
	CString name; // [MAX_BOARD_NAME_LENGTH];	// string for board name
	CString entry; // [MAX_BOARD_NAME_LENGTH];	// string for board name
	A_ENCODING adc_enc; // Binary or 2's Complement
	A_ENCODING dac_enc; // Binary or 2's Complement
};

#define MAX_SS_NAME_LENGTH 100

using SUBSYS = struct _SUBSYS
{
	char name[MAX_SS_NAME_LENGTH];
	OLSS ss_type;
	UINT element;
};

// Board capabilities
#define CAPABILITY_LIST_SIZE	7
#define	CAPABILITY_LIST		{OLDC_ADELEMENTS,	OLDC_DAELEMENTS,	OLDC_DINELEMENTS,	OLDC_DOUTELEMENTS,	OLDC_CTELEMENTS,	OLDC_SRLELEMENTS,	OLDC_TOTALELEMENTS}
#define CAPABILITY_TEXT		{_T("A/D systems:"), _T("D/A systems:"), _T("Digital inputs:"), _T("Digital outputs:"), _T("Counter/Timers:"),_T("Serial ports:"), _T("Total subsystems:")}

// Possible subsystems on a board
#define SUBSYSTEM_COUNT	7
#define	SUBSYSTEM_LIST	{OLSS_AD,			OLSS_DA,			OLSS_DIN,			OLSS_DOUT,			OLSS_CT,			OLSS_SRL}
#define SUBSYSTEM_TYPE	{_T("Analog input"), _T("Analog output"), _T("Digital input"), _T("Digital output"), _T("Counter-timer"), _T("Serial port"), _T("UNKNOWN")}

// Possible subsystem capabilities
#define	SUB_COUNT	36
#define	SUB_CAP		{OLSSC_SUP_AUTO_CALIBRATE,	OLSSC_SUP_SINGLEVALUE,	OLSSC_SUP_CONTINUOUS,	OLSSC_SUP_CONTINUOUS_PRETRIG,	OLSSC_SUP_CONTINUOUS_ABOUTTRIG,	OLSSC_SUP_RANDOM_CGL,		OLSSC_SUP_SEQUENTIAL_CGL,		OLSSC_SUP_ZEROSEQUENTIAL_CGL,	OLSSC_SUP_SIMULTANEOUS_SH,	OLSSC_SUP_SIMULTANEOUS_START,	OLSSC_SUP_SYNCHRONIZATION,	OLSSC_SUP_SIMULTANEOUS_CLOCKING,	OLSSC_SUP_PAUSE,	OLSSC_SUP_POSTMESSAGE,	OLSSC_SUP_INPROCESSFLUSH,	OLSSC_SUP_BUFFERING,	OLSSC_SUP_WRPSINGLE,	OLSSC_SUP_WRPMULTIPLE,	OLSSC_SUP_WRPWAVEFORM,	OLSSC_SUP_WRPWAVEFORM_ONLY,	OLSSC_SUP_GAPFREE_NODMA,	OLSSC_SUP_GAPFREE_SINGLEDMA,	OLSSC_SUP_GAPFREE_DUALDMA,	OLSSC_SUP_TRIGSCAN,	OLSSC_MAXMULTISCAN,	OLSS_SUP_RETRIGGER_SCAN_PER_TRIGGER,	OLSS_SUP_RETRIGGER_INTERNAL,	OLSSC_SUP_RETRIGGER_EXTRA,	OLSSC_SUP_INTERRUPT,	OLSSC_SUP_SINGLEENDED,	OLSSC_SUP_DIFFERENTIAL,	OLSSC_SUP_BINARY,	OLSSC_SUP_2SCOMP,	OLSSC_RETURNS_FLOATS,	OLSSC_NUMRANGES}
#define	SUB_TEXT	{_T("Automatic calibration"),	_T("Single sampling"),	_T("Continuous sampling"), _T("Continuous pretriggered"), _T("Continuous about triggered"),_T("Random channels allowed"), _T("Sequential channels allowed"), _T("Zero first channel allowed"), _T("Hold channel required"), _T("Simultaneous start"), _T("Prog. synchronization"), _T("Simultaneous clocking"), _T("Pausing"), _T("Post messages"), _T("In-process buffering"), _T("Buffering"), _T("Single-buffer wrap"),_T("Multi-buffer wrap"), _T("Waveform generation"), _T("FIFI Waveform only"), _T("Gap-free without DMA"), _T("Gap-free single DMA"), _T("Gap-free dual DMA"), _T("Triggered scans"), _T("Multiple scans"), _T("Scan-per-trigger"), _T("Internal retriggering"), _T("Extra retriggering"), _T("Interrupt-driven I/O"),_T("Single-ended"), _T("Differential"), _T("Binary encoding"), _T("Twos-Complement"), _T("Floating point"), _T("Multiple voltage ranges")}

// Numerical subsystem capabilities
#define SS_NUM_COUNT 10
#define SS_NUM_CAP	{OLSSC_MAXSECHANS,			OLSSC_MAXDICHANS,			OLSSC_NUMGAINS,		OLSSC_CGLDEPTH,		OLSSC_NUMDMACHANS,		OLSSC_NUMFILTERS,	OLSSC_NUMRANGES,	OLSSC_NUMRESOLUTIONS,	OLSSC_FIFO_SIZE_IN_K,	OLSSC_NUMCHANNELS}
#define SS_NUM_TEXT	{_T("Single-ended channels"), _T("Differential channels"), _T("Gain selections"), _T("Channel gain list"),_T("DMA channels"), _T("Filters"), _T("Ranges"), _T("Resolutions"), _T("FIFO (KB)"), _T("I/O Channels")}

#define FATAL_ERROR(ecode,message) if ((m_board.status=(ecode))!=OLNOERROR) { MESSAGE(MESS_FATAL,message); return;}
#define FATAL_ERROR_0(ecode,message) if ((m_board.status=(ecode))!=OLNOERROR) { MESSAGE(MESS_FATAL,message); return 0;}

#define DAC_TIMER			1
#define DAC_INTERVAL		2
