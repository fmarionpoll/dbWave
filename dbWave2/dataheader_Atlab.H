// Filename: AtlabHeader.h
// Author:   F. Marion-Poll August 1993
#pragma once

#define FORTRAN_ALIGN  1
// structures:
// ============================================================
/*
 DEV_FLAGS
 AL_CONFIGURATION
 CYBERAMP
 CYBERCHAN
 ATLABHEADER
 DATAPROP
*/
// ============================================================

// ATLAB data file acquisition information structure

//	unsigned	DMA_mode		: 2;
#define		PROGRAMMED_IO		0
#define		SINGLE_CHANNEL		1
#define		DUAL_CHANNEL		2

// bit definitions for the A/D timing source

#define 	INTERNAL_CLOCK		0
#define 	EXTERNAL_CLOCK		1
#define 	INTERNAL_TRIGGER	0
#define 	EXTERNAL_TRIGGER	2
#define 	TRIGGERED_SCAN		4

// device flags bit field definitions

#pragma pack (FORTRAN_ALIGN)
using DEV_FLAGS = struct
{
	short DMA_mode : 2;
	short SE_DI : 1;
	short unipolar : 1;
	short encoding : 2;
	short sign_extended : 1;
	//unsigned 				: 9;
};
#pragma pack ()

// definition of the information returned by AL_GET_CONFIGURATION

#pragma pack (FORTRAN_ALIGN)
using AL_CONFIGURATION = struct
{
	short base_address; // base I/O address of the device
	short device_id; // DT28xx device identifier
	DEV_FLAGS device_flags; // configuration options (bit definitions)
	short scan_count; // number of channels in scan list
	short channel_count; // number of A/D channels
	short reserved[11]; // reserved for future use
};
#pragma pack ()

// definitions for Axon Instrument amplifier settings

#pragma pack (FORTRAN_ALIGN)
using CYBERAMP = struct
{
	// CyberAmp common settings
	short ComPort; // communication port COM1,COM2 ...
	short ComSpeed; // serial port speed (bauds)
	short DevNumb; // device number (identifier)
	short nbChans; // nb of channels in the amplifier
	//    short overload;			// overload status (0=none)
};
#pragma pack ()

// atlab data file header
#pragma pack (FORTRAN_ALIGN)
using CYBERCHAN = struct
{
	// Cyber amplifier definition
	unsigned char acqchan;
	unsigned char chan;
	short gainpre; // pre-filter amplification
	short gainpost; // post-filter amplification
	short notchfilt; // notch filter ON/off
	short lowpass; // low pass filter 0=DC 4....30000
	float offset; // input offset
	short inputpos; // values:GND(-1) DC(0) .1 1 10 30 100 300
	short inputneg; // (*10)
	char probe[8]; // probe type (ASCII)
	short gainprobe; // amplification factor at probe level
};
#pragma pack ()

#pragma pack (FORTRAN_ALIGN)
using INSTRUMPARM = struct
{
	char amplifier[12];
	CYBERCHAN cyber;
};
#pragma pack ()

// offset within header (C convention, assuming 1 byte)

constexpr auto DEVID = 4; // device_id.........AD card id;
constexpr auto DEVFLAGS = 6; // device_flags......type of acquisition;
constexpr auto SCNCNT = 8; // scan_count .......number of acq channels;
constexpr auto CHANLST = 10; // channel_list[8] ..input channel list;
constexpr auto TRIGGER_MODE = 26; // trigger_mode......AD trigger mode [0..];
constexpr auto TRIGGER_CHAN = 28; // trigger_chan......AD trigger channel [0..7];
constexpr auto TRIGGER_THRESHOLD = 30; // trigger_treshold..AD trig threshold value [binary];
constexpr auto TRIGGER_POLARITY = 32; // trigger_polarity;
constexpr auto GAINLST = 42; // gain_list[8]...... gain channel list;
constexpr auto CHANCOM = 74; // channel_comment[8][40];
constexpr auto ACQDATE = 714; // acq_date[10];
constexpr auto ACQTIME = 724; // acq_time[10];
constexpr auto TIMING = 736; // timing_source;
constexpr auto CLKPER = 738; // clock_period;
constexpr auto SAMCNT = 742; // sample_count;
constexpr auto RDCNT = 746; // read_count;
constexpr auto ACQCOM = 750; // acq_comment[80];
constexpr auto VERSION = 830; // version;
constexpr auto CYBERA320 = 832; // CyberA320;
constexpr auto CYBER_1 = 840; // cyber1
constexpr auto CYBER_2 = 868; // cyber2
constexpr auto XGAIN = 914; // xgain_list[8]

constexpr auto ACQCOM_LEN = 80;
