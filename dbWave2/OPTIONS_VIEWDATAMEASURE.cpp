#include "StdAfx.h"
#include "OPTIONS_VIEWDATAMEASURE.h"

IMPLEMENT_SERIAL(OPTIONS_VIEWDATAMEASURE, CObject, 0 /* schema number*/)

OPTIONS_VIEWDATAMEASURE::OPTIONS_VIEWDATAMEASURE()
= default;

OPTIONS_VIEWDATAMEASURE::~OPTIONS_VIEWDATAMEASURE()
= default;

OPTIONS_VIEWDATAMEASURE& OPTIONS_VIEWDATAMEASURE::operator =(const OPTIONS_VIEWDATAMEASURE& arg)
{
	if (this != &arg)
	{
		b_changed = arg.b_changed;

		// parameters to measure
		bExtrema = arg.bExtrema; // measure max and min (value, time)
		bDiffExtrema = arg.bDiffExtrema; // difference between extrema (value, time)
		bDiffDatalimits = arg.bDiffDatalimits; // diff between tags/cursors
		bHalfrisetime = arg.bHalfrisetime; // time necessary to reach half of the amplitude
		bHalfrecovery = arg.bHalfrecovery; // time necessary to regain half of the initial amplitude
		bDatalimits = arg.bDatalimits; // data value at limits
		btime = arg.btime;
		bKeepTags = arg.bKeepTags; // keep flags after measurement
		bSaveTags = arg.bSaveTags;
		bSetTagsforCompleteFile = arg.bSetTagsforCompleteFile;

		// source data
		bAllChannels = arg.bAllChannels; // measure on all chans (TRUE) or on single chan (FALSE)
		wSourceChan = arg.wSourceChan; // if bAllChans=FALSE, source data chan
		bAllFiles = arg.bAllFiles; // measure on all files (default=FALSE)

		// limits
		wOption = arg.wOption; // 0=between vertic tags, 1=between horiz lines, 2=relative to stimulus
		wStimuluschan = arg.wStimuluschan; // if wOption=2: stimulus channel
		wStimulusthresh = arg.wStimulusthresh; // if wOption=2: stimulus threshold
		fStimulusoffset = arg.fStimulusoffset; // if wOption=2: offset interval (default=0)
		wLimitSup = arg.wLimitSup; // rectangle limits
		wLimitInf = arg.wLimitInf;
		lLimitLeft = arg.lLimitLeft;
		lLimitRight = arg.lLimitRight;

		// dlg edit VT
		duration = arg.duration; // on/OFF duration (sec)
		period = arg.period; // period (sec)
		nperiods = arg.nperiods; // nb of duplicates
		timeshift = arg.timeshift; // shift tags
	}
	return *this;
}

void OPTIONS_VIEWDATAMEASURE::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) // save data .....................
	{
		ar << wversion;

		// measure options
		WORD wMeasureFlags;
		WORD wMult;
		wMult = 1;
		wMeasureFlags = bExtrema * wMult;
		wMult += wMult;
		wMeasureFlags += bDiffExtrema * wMult;
		wMult += wMult;
		wMeasureFlags += bHalfrisetime * wMult;
		wMult += wMult;
		wMeasureFlags += bHalfrecovery * wMult;
		wMult += wMult;
		wMeasureFlags += bDatalimits * wMult;
		wMult += wMult;
		wMeasureFlags += bDiffDatalimits * wMult;
		wMult += wMult;
		wMeasureFlags += btime * wMult;
		wMult += wMult;
		wMeasureFlags += bKeepTags * wMult;
		wMult += wMult;
		wMeasureFlags += bSaveTags * wMult;
		wMult += wMult;
		wMeasureFlags += bSetTagsforCompleteFile * wMult;
		ar << wMeasureFlags;

		// source data
		ar << static_cast<WORD>(bAllChannels);
		ar << wSourceChan;
		ar << static_cast<WORD>(bAllFiles);

		// limits
		ar << wOption;
		ar << wStimuluschan << wStimulusthresh << fStimulusoffset;
		ar << wLimitSup << wLimitInf << lLimitLeft << lLimitRight;

		// dlg edit VT tags
		ar << duration; // on/OFF duration (sec)
		ar << period; // period (sec)
		ar << nperiods; // nb of duplicates
		ar << timeshift; // shift tags
	}
	else // read data ...................
	{
		WORD version;
		ar >> version;
		// measure options version 1
		WORD wMeasureFlags;
		WORD wMult;
		ar >> wMeasureFlags;
		wMult = 1;
		bExtrema = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bDiffExtrema = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bHalfrisetime = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bHalfrecovery = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bDatalimits = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bDiffDatalimits = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		btime = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bKeepTags = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bSaveTags = ((wMeasureFlags & wMult) > 0);
		wMult += wMult;
		bSetTagsforCompleteFile = ((wMeasureFlags & wMult) > 0);

		// source data
		WORD w1;
		ar >> w1;
		bAllChannels = w1;
		ar >> wSourceChan;
		ar >> w1;
		bAllFiles = w1;

		// limits
		ar >> wOption;
		ar >> wStimuluschan >> wStimulusthresh >> fStimulusoffset;
		ar >> wLimitSup >> wLimitInf >> lLimitLeft >> lLimitRight;

		// dlg edit VT tags - parameters
		if (version > 1)
		{
			ar >> duration; // on/OFF duration (sec)
			ar >> period; // period (sec)
			ar >> nperiods; // nb of duplicates
			ar >> timeshift; // shift tags
		}
	}
}
