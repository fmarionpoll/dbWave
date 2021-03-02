//  awavepar.h  acquisition parameters
//
#pragma once
class display : public CObject
{
	DECLARE_SERIAL(display)
	display();
	~display();
	display& operator = (const display& arg);
	void Serialize(CArchive& ar) override;

	int iextent = 2048;
	int izero = 0;
};

class SCOPESTRUCT : public CObject
{
	DECLARE_SERIAL(SCOPESTRUCT)
	SCOPESTRUCT();
	~SCOPESTRUCT();
	SCOPESTRUCT& operator = (const SCOPESTRUCT& arg);
	void Serialize(CArchive& ar) override;

public:
	WORD		wversion;
	int			iID;
	int			iXCells;
	int			iYCells;
	int			iXTicks;
	int			iYTicks;
	int			iXTickLine;
	int			iYTickLine;
	COLORREF	crScopeFill;
	COLORREF	crScopeGrid;
	BOOL		bDrawframe;
	BOOL		bClipRect;
	float		xScaleUnitValue;
	float		yScaleUnitValue;
	float		xScaleSpan_s;
	float		yScaleSpan_v;

	CArray <display, display> channels;
};

// view data options - version 1 (20-4-96)

class OPTIONS_VIEWDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_VIEWDATA)
	OPTIONS_VIEWDATA();
	~OPTIONS_VIEWDATA();
	OPTIONS_VIEWDATA& operator = (const OPTIONS_VIEWDATA& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL bChanged;			// flag set TRUE if contents has changed
	WORD m_wversion;		// version number

	// print options
	BOOL bAcqComment;		// print data acquisition comments
	BOOL bAcqDateTime;		// data acquisition date and time
	BOOL bChansComment;		// individual data acq channel description
	BOOL bChanSettings;		// amplifier's settings (CyberAmp conventions)
	BOOL bDocName;			// file name
	BOOL bFrameRect;		// draw a rectangle frame around the data
	BOOL bClipRect;			// clip data to the rectangle
	BOOL bTimeScaleBar;		// display time scale bar
	BOOL bVoltageScaleBar;	// display voltage scale bar
	BOOL bPrintSelection;	// print only selection or whole series
	BOOL bPrintSpkBars;		// for viewspkd view, print bars or not
	BOOL bFilterDataSource;	// display source data filtered

	// print parameters
	int	 horzRes;			// horizontal printer resolution
	int  vertRes;			// vertical printer resolution
	int	 HeightDoc;			// height of one document
	int	 WidthDoc;			// width of one document
	int	 leftPageMargin;	// page left margin (pixels)
	int	 bottomPageMargin;	// bottom
	int	 rightPageMargin;	// right
	int	 topPageMargin;		// top
	int	 heightSeparator;	// vertical space between consecutive displays
	int	 textseparator;		// space between left margin of display and text
	int  fontsize;			// font size (pixels)
	int	 spkheight;			//
	int	 spkwidth;			//

	// how data are displayed from file to file
	BOOL bAllChannels;		// display all channels
	BOOL bEntireRecord;		// display entire record
	BOOL bMultirowDisplay;	// display one record on consecutive lines
	BOOL bSplitCurves;		// arrange curves so that they are distributed vertically
	BOOL bMaximizeGain;		// maximize gain
	BOOL bCenterCurves;		// center curves
	BOOL bKeepForEachFile;	// keep parameters for each file
	BOOL bDispOldDetectP;	// spk detection parms: display old (true), keep same
	BOOL bFilterDat;		// for dbViewdata
	BOOL bDetectWhileBrowse;// for spkdetectionview
	BOOL bGotoRecordID;		// for dbWaveView
	BOOL bDisplayFileName;	// for dbViewData
	BOOL bsetTimeSpan;
	BOOL bsetmVSpan;
	BOOL bDisplayAllClasses;

	// export infos from data files
	BOOL bacqcomments;		// global comment
	BOOL bacqdate;			// acquisition date
	BOOL bacqtime;			// acquisition time
	BOOL bfilesize;			// file size
	BOOL bacqchcomment;		// acq channel indiv comment
	BOOL bacqchsetting;		// acq chan indiv settings (gain, filter, etc)
	BOOL bunits;			// TRUE=mV, FALSE=binary
	BOOL bcontours;			// TRUE=contours, FALSE=center
	BOOL bgraphics;			// TRUE=graphics (enh metafile), FALSE=ASCII
	BOOL bdatabasecols;		// TRUE= export content of database columns
	BOOL btoExcel;

	int	 hzResolution;		// int item #1: horizontal size of exported data (points)
	int  vtResolution;		// vertical size of exported data (points)
	int	 unitMode;			// provisional
	int	 nfilterindex;		// index list item in fileopen dialog m_nfilterindex

	// overview list parameters
#define NCOLS	11
	CUIntArray  icolwidth;
	int			irowheight;

	// save scopescreen item
	int			n_scope_items;
	SCOPESTRUCT	viewdata;
	SCOPESTRUCT viewspkdetectfiltered;
	SCOPESTRUCT	viewspkdetectdata;
	SCOPESTRUCT	viewspkdetectspk;
	SCOPESTRUCT	viewspkdetectbars;
	SCOPESTRUCT	viewadcontvi;
	SCOPESTRUCT	spkviewdata;
	SCOPESTRUCT	spkviewspk;
	SCOPESTRUCT	spkviewbars;
	SCOPESTRUCT	spksort1spk;
	SCOPESTRUCT	spksort1parms;
	SCOPESTRUCT spksort1bars;
	SCOPESTRUCT	spksort1hist;
	SCOPESTRUCT	dbviewdata;
	SCOPESTRUCT	dbviewbars;
	SCOPESTRUCT	dbviewshape;

	// splitter settings
	int		col0width;
	int		row0height;
	BOOL	bcol0visible;
	BOOL	brow0visible;

	// int parm - spike class
	int		spikeclass;

	// database list view settings
	float	tFirst;
	float	tLast;
	float	mVspan;
	int		displaymode;
};

// ---------------------------------------------------------------------
// view spikes options

#define EXPORT_PSTH			0
#define EXPORT_ISI			1
#define EXPORT_AUTOCORR		2
#define EXPORT_LATENCY		3
#define EXPORT_AVERAGE		4
#define EXPORT_INTERV		5
#define EXPORT_EXTREMA		6
#define EXPORT_AMPLIT		7
#define EXPORT_HISTAMPL		8
#define EXPORT_SPIKEPOINTS	9

class OPTIONS_VIEWSPIKES : public CObject
{
	DECLARE_SERIAL(OPTIONS_VIEWSPIKES)
	OPTIONS_VIEWSPIKES();
	~OPTIONS_VIEWSPIKES();
	OPTIONS_VIEWSPIKES& operator = (const OPTIONS_VIEWSPIKES& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL bChanged;				// flag set TRUE if contents has changed
	WORD wversion;				// version number
	int	 ichan;					// data channel from which we have detected spikes

	float timestart;			// interval definition
	float timeend;				//
	float timebin;				// bin size (sec)
	float histampl_vmax;
	float histampl_vmin;

	int  histampl_nbins;
	int	 nbins;					// number of bins
	int  classnb;				// class nb
	int  classnb2;
	int	 istimulusindex;
	int	 exportdatatype;		// export 0=psth, 1=isi, 2=autocorr, 3=intervals, 4=extrema, 5=max-min, 6=hist ampl
	int	 spikeclassoption;		// spike class: -1(one:selected); 0(all); 1(all:splitted)

	BOOL bacqcomments;			// source data comments
	BOOL bacqdate;				// source data time and date
	BOOL bacqchsettings;		// source data acq chan settings
	BOOL bspkcomments;			// spike file comments
	BOOL ballfiles;				// export number of spikes / interval
	BOOL bexportzero;			// when exporting histogram, export zero (true) or empty cell (false)
	BOOL bexportPivot;			// when exporting to excel, export pivot or not
	BOOL bexporttoExcel;		// export to Excel (1/0)
	BOOL bartefacts;			// eliminate artefacts (class < 0)
	BOOL bcolumnheader;			// column headers ON/OFF
	BOOL btotalspikes;			// total nb of spikes
	BOOL babsolutetime;			// TRUE= absolute spk time, FALSE=relative to first stim
	BOOL bincrflagonsave;		// increment database flag when spike file is saved
	BOOL bSpikeDetectThreshold;	// spike detection threshold

	int  ncommentcolumns;		// number of comment columns in the table exported to excel

	// print parameters
	WORD wIntParms;				// count nb of int parameters stored
	int  heightLine;			// height of one line
	int  heightSeparator;		// separator height betw classes
	WORD wBoolParms;
	BOOL bdisplayBars;			// default(TRUE)
	BOOL bdisplayShapes;		// default(TRUE)
	BOOL bsplitClasses;			// separate classes
	BOOL ballChannels;			// all spike channels (FALSE: only current)

	// histogram and dot display
	int		dotheight;
	int		dotlineheight;
	BOOL	bdotunderline;
	int		nbinsISI;
	float	binISI;
	BOOL	bYmaxAuto;
	float	Ymax;

	COLORREF crHistFill;
	COLORREF crHistBorder;
	COLORREF crStimFill;
	COLORREF crStimBorder;
	COLORREF crChartArea;
	COLORREF crScale[18]{};
	float	 fScalemax;
	int		bhistType;			// type of histogram requested (PSTH, ISI, ...)
	BOOL	bCycleHist;			// cycle histogram (TRUE/FALSE), default=FALSE
	int		nstipercycle;		// n stimuli per cycle
};

// --------------------------------------------------------------
// view data measure options

class OPTIONS_VIEWDATAMEASURE : public CObject
{
	DECLARE_SERIAL(OPTIONS_VIEWDATAMEASURE)
	OPTIONS_VIEWDATAMEASURE();
	~OPTIONS_VIEWDATAMEASURE();
	OPTIONS_VIEWDATAMEASURE& operator = (const OPTIONS_VIEWDATAMEASURE& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL bChanged;			// flag set TRUE if contents has changed
	WORD wversion;			// version number

	// parameters to measure
	BOOL bExtrema;			// 1- measure max and min (val, time)
	BOOL bDiffExtrema;		// 2- difference between extrema (val, time)
	BOOL bHalfrisetime;		// 3- time necessary to reach half of the amplitude
	BOOL bHalfrecovery;		// 4- time necessary to regain half of the initial amplitude
	BOOL bDatalimits;		// 5- data value at limits
	BOOL bDiffDatalimits;	// 6- diff between data limits
	BOOL btime;				// 7- time value of 1, 2, 5
	BOOL bKeepTags;			// 8- don't erase flags after measurement
	BOOL bSaveTags;			// 9- set flag "modified" into assoc doc
	BOOL bSetTagsforCompleteFile; // 10- dialog box option

	// source data
	BOOL bAllChannels;		// measure on all chans (TRUE) or on single chan (FALSE)
	WORD wSourceChan;		// if bAllChans=FALSE, source data chan
	BOOL bAllFiles;			// measure on all files (default=FALSE)

	// limits
	WORD wOption;			// 0=vertic tags, 1=horiz lines; 2=rectangle; 3=relative to stimulus
	WORD wStimuluschan;		// if wOption=2: stimulus channel
	WORD wStimulusthresh;	// if wOption=2: stimulus threshold
	float fStimulusoffset;	// if wOption=2: offset interval (default=0)
	WORD wLimitSup;			// rectangle limits
	WORD wLimitInf;
	long lLimitLeft;
	long lLimitRight;

	// HZ tags options
	float duration;			// on/OFF duration (sec)
	float period;			// period (sec)
	int	  nperiods;			// nb of duplicates
	float timeshift;		// shift tags
};

//-----------------------------------------------------------------------------------
// File importation parameters & options

class OPTIONS_IMPORT : public CObject
{
	DECLARE_SERIAL(OPTIONS_IMPORT);
	OPTIONS_IMPORT();
	virtual ~OPTIONS_IMPORT();
	OPTIONS_IMPORT& operator = (const OPTIONS_IMPORT& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL  bChanged;					// flag set TRUE if contents has changed
	WORD  wVersion;
	// Varian files
	float fGainFID;					// xgain in the FID channel
	float fGainEAD;					// xgain in the EAD channel
	CString pathWTOASCII;

	// generic files
	BOOL	bSingleRun;
	BOOL	bPreview;
	BOOL	bSapid3_5;
	BOOL	bDummy;
	BOOL	bImportDuplicateFiles = false;
	BOOL	bReadColumns = false;
	BOOL	bHeader = false;
	short	nbRuns;
	short	nbChannels;
	float	samplingRate;
	short	encodingMode;
	short	bitsPrecision;
	float	voltageMax;
	float	voltageMin;
	int		skipNbytes;
	CString	title;
	CWaveChanArray* pwaveChanArray;

	// last selected filter - index
	int		nSelectedFilter;
	// export options
	WORD	exportType;				// type of export (0=sapid 3.5, 1=txt, 2=excel, ...)
	BOOL	bAllchannels;			// export all channels or one
	BOOL	bSeparateComments;		// comments in a separate file
	BOOL	bentireFile;			// complete file / interval
	BOOL	bincludeTime;			// include time steps (excel, txt)
	int		selectedChannel;		// one channel
	float	fTimefirst;				// interval first
	float	fTimelast;				// interval last
	int		iundersample;			// undersample factor when exporting data

	CString path;					// path to files to import
};

// ---------------------------------------------------------------------
// acquisition data options - version 1 (17-2-98)

class OPTIONS_ACQDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_ACQDATA)
	OPTIONS_ACQDATA();
	~OPTIONS_ACQDATA();
	OPTIONS_ACQDATA& operator = (const OPTIONS_ACQDATA& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL			bChanged;						// flag set TRUE if contents has changed
	WORD			m_wversion;						// version number

	CString			csBasename;
	CString			csPathname;
	int				exptnumber{};
	int				insectnumber;
	int				iundersample;
	BOOL			baudiblesound;
	BOOL			bChannelType;					// OLx_CHNT_SINGLEENDED or OLx_CHNT_DIFFERENTIAL

	CStringArray	csA_stimulus;
	CStringArray	csA_concentration;
	CStringArray	csA_stimulus2;
	CStringArray	csA_concentration2;
	CStringArray	csA_insect;
	CStringArray	csA_location;
	CStringArray	csA_sensillum;
	CStringArray	csA_strain;
	CStringArray	csA_operatorname;
	CStringArray	csA_sex;
	CStringArray	csA_expt;

	int				icsA_stimulus;
	int				icsA_concentration;
	int				icsA_stimulus2;
	int				icsA_concentration2;
	int				icsA_insect;
	int				icsA_location;
	int				icsA_sensillum;
	int				icsA_strain;
	int				icsA_operatorname;
	int				icsA_sex;
	int				icsA_repeat;
	int				icsA_repeat2;
	int				icsA_expt;
	int				izoomCursel;					// display zoom factor (acq)

	CWaveFormat		waveFormat;						// wave format
	CWaveChanArray	chanArray;						// channels descriptors
	float			sweepduration;					// display parameter
};

// ---------------------------------------------------------------------
// output data options - version 1 (11/09/2011)

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

class OPTIONS_OUTPUTDATA : public CObject
{
	DECLARE_SERIAL(OPTIONS_OUTPUTDATA)
	OPTIONS_OUTPUTDATA();
	~OPTIONS_OUTPUTDATA();
	OPTIONS_OUTPUTDATA& operator = (const OPTIONS_OUTPUTDATA& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL	bChanged;								// flag set TRUE if contents has changed
	WORD	wversion;								// version number

	BOOL	bAllowDA;								// allow DA if present
	BOOL	bPresetWave;							// 0=preset, 1=from file
	CString	csOutputFile;							// output file

	CArray <OUTPUTPARMS, OUTPUTPARMS> outputparms_array;

	double	dDAFrequency_perchan;					// output frequency (per chan)
	int		iDATriggermode;							// 0=synchronous; 1=soft on start; 2=external trigger
	int		iDAnbuffers;							// number of DA buffers
	int		iDAbufferlength;						// length of each buffer
};
