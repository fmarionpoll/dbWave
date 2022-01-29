#pragma once

class OPTIONS_VIEWDATAMEASURE : public CObject
{
	DECLARE_SERIAL(OPTIONS_VIEWDATAMEASURE)
	OPTIONS_VIEWDATAMEASURE();
	~OPTIONS_VIEWDATAMEASURE() override;
	OPTIONS_VIEWDATAMEASURE& operator = (const OPTIONS_VIEWDATAMEASURE& arg);
	void Serialize(CArchive& ar) override;

public:
	BOOL b_changed{false};			// flag set TRUE if contents has changed
	WORD wversion{2};				// version number

	// parameters to measure
	BOOL bExtrema{true};			// 1- measure max and min (value, time)
	BOOL bDiffExtrema{true};		// 2- difference between extrema (value, time)
	BOOL bHalfrisetime{false};		// 3- time necessary to reach half of the amplitude
	BOOL bHalfrecovery{false};		// 4- time necessary to regain half of the initial amplitude
	BOOL bDatalimits{true};			// 5- data value at limits
	BOOL bDiffDatalimits{true};		// 6- diff between data limits
	BOOL btime{false};				// 7- time value of 1, 2, 5
	BOOL bKeepTags{true};			// 8- don't erase flags after measurement
	BOOL bSaveTags{true};			// 9- set flag "modified" into assoc doc
	BOOL bSetTagsforCompleteFile{false}; // 10- dialog box option

	// source data
	BOOL bAllChannels{false};		// measure on all chans (TRUE) or on single chan (FALSE)
	WORD wSourceChan{0};			// if bAllChans=FALSE, source data chan
	BOOL bAllFiles{false};			// measure on all files (default=FALSE)

	// limits
	WORD wOption{2};				// 0=vertic tags, 1=horiz lines{}; 2=rectangle{}; 3=relative to stimulus
	WORD wStimuluschan{0};			// if wOption=2: stimulus channel
	WORD wStimulusthresh{2048};		// if wOption=2: stimulus threshold
	float fStimulusoffset{0.f};		// if wOption=2: offset interval (default=0)
	WORD wLimitSup{0};				// rectangle limits
	WORD wLimitInf{1};
	long lLimitLeft{0};
	long lLimitRight{1};

	// HZ tags options
	float duration{.8f};			// on/OFF duration (sec)
	float period{3.f};				// period (sec)
	int	  nperiods{10};				// nb of duplicates
	float timeshift{0.f};			// shift tags
};

