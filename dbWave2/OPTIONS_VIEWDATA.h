#pragma once


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
