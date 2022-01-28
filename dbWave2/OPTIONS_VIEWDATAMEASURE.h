#pragma once

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
	BOOL bExtrema;			// 1- measure max and min (value, time)
	BOOL bDiffExtrema;		// 2- difference between extrema (value, time)
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

