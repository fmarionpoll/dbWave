#pragma once


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

