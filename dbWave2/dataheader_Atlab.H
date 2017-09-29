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
typedef struct {
	short DMA_mode		: 2;
	short SE_DI			: 1;												  
	short unipolar		: 1;
	short encoding		: 2;
	short sign_extended	: 1;
	//unsigned 				: 9;
	} DEV_FLAGS;
#pragma pack ()

// definition of the information returned by AL_GET_CONFIGURATION

#pragma pack (FORTRAN_ALIGN)
typedef struct {
	short	base_address;		// base I/O address of the device 
	short	device_id;			// DT28xx device identifier 
	DEV_FLAGS device_flags;		// configuration options (bit definitions)							
	short	scan_count;			// number of channels in scan list 
	short	channel_count;		// number of A/D channels 
	short	reserved[11];		// reserved for future use 
	} AL_CONFIGURATION;
#pragma pack ()
																	 
// definitions for Axon Instrument amplifier settings

#pragma pack (FORTRAN_ALIGN)
typedef struct {           // CyberAmp common settings
	short ComPort;         // communication port COM1,COM2 ...
	short ComSpeed;        // serial port speed (bauds)
	short DevNumb;         // device number (identifier)
	short nbChans;         // nb of channels in the amplifier
//    short overload;        // overload status (0=none)
	} CYBERAMP;
#pragma pack ()

// atlab data file header
#pragma pack (FORTRAN_ALIGN)
typedef struct {      	// Cyber amplifier definition
	unsigned char	acqchan;
	unsigned char	chan;
	short	gainpre;    // pre-filter amplification
	short	gainpost;   // post-filter amplification
	short	notchfilt;  // notch filter ON/off
	short	lowpass;    // low pass filter 0=DC 4....30000
	float	offset;     // input offset
	short	inputpos;   // values:GND(-1) DC(0) .1 1 10 30 100 300
	short	inputneg;   // (*10)
	char	probe[8];   // probe type (ASCII)
	short	gainprobe;	// amplification factor at probe level      
	} CYBERCHAN;
#pragma pack ()

#pragma pack (FORTRAN_ALIGN)
typedef struct {
	char	amplifier[12];
	CYBERCHAN cyber;
	} INSTRUMPARM;
#pragma pack ()

/*                  
// data file header definitions - ATLAB

typedef struct tagATLABHEADER {		// index
	short		reserved_0[2];		// 0  .... Header version
	short		device_id;			// 2  DT28xx device identifier
	DEV_FLAGS 	device_flags;		// 3  configuration options(defs below)
	short		scan_count;			// 4  number of channels in scan list
	short		channel_list[8];	// 5  channel scan list	
	short		trigger_mode;		// 13 soft, keyboard, trig_ext, input+, input-
	short		trigger_chan;		// 14 (trig_mode=input) A/D channel input
	short		trigger_threshold;	// 15 (trig_mode=input) A/D channel input threshold value
	short		trigger_polarity;	// 16 trigger polarity
	char 		free_1[8];			// ...................................
	short		gain_list[8];		// 21  channel gain list
	char 		free_2[16];			// ...................................
	char		channel_comment[8][40];// 37 channel annotation
	//INSTRUMPARM channel_instrum[8];	//     channel instumental parameters
	char		free_3[320];			//
	char		acq_date[10];		// 357 acquisition date
	char		acq_time[10];		// 362 acquisition time
	short		timing_source;		// 368 A/D timing source
	long		clock_period;		// 369 clock period in ticks
	long		sample_count;		// 371 sample count
	long		read_count;			// 373 samples read so far
	char		acq_comment[80];	// 375 file annotation
	short		version;			// 415 header version number
	CYBERAMP	cyberA320;			// 416 Cyber amp general settings	
	CYBERCHAN	cyber1;				// 421 Cyber channel 1
	CYBERCHAN	cyber2;             // 435
	char 		free_4[14];			// ...................................
	float		xgain_list[8];		// 457 instrumental gain list
	char 		free_5[82];			// ...................................
	} ATLABHEADER;
//
*/

// offset within header (C convention, assuming 1 byte)

#define DEVID 				4	// device_id.........AD card id
#define DEVFLAGS 			6	// device_flags......type of acquisition
#define SCNCNT				8	// scan_count .......number of acq channels
#define CHANLST				10	// channel_list[8] ..input channel list
#define TRIGGER_MODE		26	// trigger_mode......AD trigger mode [0..]
#define TRIGGER_CHAN		28	// trigger_chan......AD trigger channel [0..7]
#define TRIGGER_THRESHOLD 	30	// trigger_treshold..AD trig threshold value [binary]
#define TRIGGER_POLARITY	32	// trigger_polarity
#define GAINLST				42	// gain_list[8]...... gain channel list
#define CHANCOM				74	// channel_comment[8][40]
#define ACQDATE				714	// acq_date[10]
#define ACQTIME				724	// acq_time[10]
#define TIMING				736	// timing_source
#define CLKPER				738	// clock_period
#define SAMCNT				742	// sample_count
#define RDCNT				746	// read_count
#define ACQCOM				750	// acq_comment[80]
#define VERSION				830	// version
#define CYBERA320			832	// CyberA320
#define CYBER_1				840	// cyber1
#define CYBER_2				868	// cyber2
#define XGAIN				914	// xgain_list[8]

#define	ACQCOM_LEN			80


/* A/D trigger property             */

#define OLx_TRG_SOFT                 0
#define OLx_TRG_EXTERN               1
#define OLx_TRG_THRESHPOS            2
#define OLx_TRG_THRESHNEG            3
#define OLx_TRG_ANALOGEVENT          4
#define OLx_TRG_DIGITALEVENT         5
#define OLx_TRG_TIMEREVENT           6
#define OLx_TRG_EXTRA                7

/* A/D Encoding property            */

#define OLx_ENC_BINARY               0
#define OLx_ENC_2SCOMP               1


