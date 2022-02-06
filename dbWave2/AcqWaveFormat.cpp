// aqcparam.cpp    implementation file

#include "StdAfx.h"
#include "dataheader_Atlab.H"
#include "AcqWaveFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CWaveFormat, CObject, 0 /* schema number*/)

CWaveFormat::CWaveFormat()
{
	acqtime = CTime::GetCurrentTime();
}

CWaveFormat::~CWaveFormat()
= default;

void CWaveFormat::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		constexpr WORD version = 10;
		ar << version; // 1
		ar << acqtime; // 2
		ar << fullscale_volts; // 3
		ar << binspan; // 4
		ar << binzero; // 5
		ar << static_cast<WORD>(mode_encoding); // 6
		ar << static_cast<WORD>(mode_clock); // 7
		ar << static_cast<WORD>(mode_trigger); // 8
		ar << sampling_rate_per_channel; // 9
		ar << static_cast<WORD>(scan_count); // 10
		ar << sample_count;
		ar << duration;
		ar << static_cast<WORD>(trig_mode);
		ar << static_cast<WORD>(trig_chan);
		ar << static_cast<WORD>(trig_threshold);
		ar << static_cast<WORD>(data_flow);
		ar << bOnlineDisplay;
		ar << bADwritetofile;
		ar << bufferNitems;
		ar << buffersize;

		constexpr int n_comments = 14;
		ar << n_comments;
		// save "CString"
		ar << cs_comment;
		ar << csStimulus;
		ar << csConcentration;
		ar << csInsectname;
		ar << csOperator;
		ar << csLocation;
		ar << csSensillum;
		ar << csStrain;
		ar << csMoreComment;
		ar << csADcardName;
		ar << csStimulus2;
		ar << csConcentration2;
		ar << csSex;

		constexpr int n_items = 5;
		ar << n_items;
		// save "long"
		ar << insectID;
		ar << sensillumID;
		ar << repeat;
		ar << repeat2;
		ar << duration_to_acquire; 
	}
	else
	{
		WORD version;
		ar >> version; // 1 version number
		ASSERT(version >= 3); // assume higher version than 3
		if (version < 9)
			read_v8_and_before(ar, version);
		else
		{
			WORD w;
			ar >> acqtime; // 2
			ar >> fullscale_volts; // 3
			ar >> binspan; // 4
			ar >> binzero; // 5
			ar >> w;
			mode_encoding = static_cast<short>(w); // 6
			ar >> w;
			mode_clock = static_cast<short>(w); // 7
			ar >> w;
			mode_trigger = static_cast<short>(w); // 8
			ar >> sampling_rate_per_channel; // 9
			ar >> w;
			scan_count = static_cast<short>(w); // 10
			ar >> sample_count;
			ar >> duration;
			ar >> w;
			trig_mode = static_cast<short>(w);
			ar >> w;
			trig_chan = static_cast<short>(w);
			ar >> w;
			trig_threshold = static_cast<short>(w);
			ar >> w;
			data_flow = static_cast<short>(w);
			ar >> bOnlineDisplay;
			ar >> bADwritetofile;
			ar >> bufferNitems >> buffersize;

			int n_comments;
			ar >> n_comments;
			ASSERT(n_comments == 14);
			ar >> cs_comment;
			ar >> csStimulus;
			ar >> csConcentration;
			ar >> csInsectname;
			ar >> csOperator;
			ar >> csLocation;
			ar >> csSensillum;
			ar >> csStrain;
			ar >> csMoreComment;
			ar >> csADcardName;
			ar >> csStimulus2;
			ar >> csConcentration2;
			ar >> csSex;

			int n_items; ar >> n_items;
			n_items--; ar >> insectID;		// 4
			n_items--; ar >> sensillumID;	// 3
			n_items--; ar >> repeat;		// 2
			n_items--; ar >> repeat2;		// 1
			n_items--; if (n_items >= 0) ar >> duration_to_acquire;
		}
	}
}

void CWaveFormat::read_v8_and_before(CArchive& ar, WORD version)
{
	ASSERT(!ar.IsStoring());
	WORD w;
	ar >> acqtime; // 2
	ar >> fullscale_volts; // 3
	if (version < 6)
	{
		ar >> w;
		binspan = w; // 4
		ar >> w;
		binzero = w; // 5
	}
	else
	{
		ar >> binspan; // 4
		ar >> binzero; // 5
	}
	ar >> w;
	mode_encoding = static_cast<short>(w); // 6
	ar >> w;
	mode_clock = static_cast<short>(w); // 7
	ar >> w;
	mode_trigger = static_cast<short>(w); // 8
	ar >> sampling_rate_per_channel; // 9
	ar >> w;
	scan_count = static_cast<short>(w); // 10
	ar >> sample_count;
	ar >> duration;
	ar >> w;
	trig_mode = static_cast<short>(w);
	ar >> w;
	trig_chan = static_cast<short>(w);
	ar >> w;
	trig_threshold = static_cast<short>(w);
	ar >> w;
	data_flow = static_cast<short>(w);
	ar >> bOnlineDisplay;
	ar >> bADwritetofile;
	ar >> bufferNitems >> buffersize;

	if (version >= 7)
	{
		ar >> cs_comment;
		ar >> csStimulus;
		ar >> csConcentration;
		ar >> csInsectname;
		ar >> csOperator;
		ar >> csLocation;
		ar >> csSensillum;
		ar >> csStrain;
		ar >> csMoreComment;
		ar >> csADcardName;
		ar >> csStimulus2;
		ar >> csConcentration2;
		if (version == 8)
			ar >> csSex;
	}
	else
	{
		ar >> cs_comment;
		ar >> csStimulus;
		ar >> csConcentration;
		ar >> csInsectname;
		ar >> csOperator;
		ar >> csLocation;
		ar >> csSensillum;
		ar >> csStrain;
		ar >> csMoreComment;
		if (version >= 3)
		{
			ar >> csADcardName;
		}
	}

	if (version > 4)
	{
		int n_items;
		ar >> n_items;
		ar >> insectID;
		n_items--;
		ar >> sensillumID;
		n_items--;
		repeat = 0;
		repeat2 = 0;
		if (n_items > 0) ar >> repeat;
		n_items--;
		if (n_items > 0) ar >> repeat2;
		n_items--;
	}
	else
	{
		insectID = 0;
		sensillumID = 0;
	}
}

void CWaveFormat::Copy(const CWaveFormat* arg)
{
	acqtime = arg->acqtime;
	wversion = arg->wversion;
	fullscale_volts = arg->fullscale_volts;
	binspan = arg->binspan;
	binzero = arg->binzero;

	mode_encoding = arg->mode_encoding;
	mode_clock = arg->mode_clock;
	mode_trigger = arg->mode_trigger;

	sampling_rate_per_channel = arg->sampling_rate_per_channel;
	scan_count = arg->scan_count;
	sample_count = arg->sample_count;
	duration = arg->duration;
	duration_to_acquire = arg->duration_to_acquire;

	trig_mode = arg->trig_mode;
	trig_chan = arg->trig_chan;
	trig_threshold = arg->trig_threshold;

	data_flow = arg->data_flow;
	bOnlineDisplay = arg->bOnlineDisplay;
	bADwritetofile = arg->bADwritetofile;
	bufferNitems = arg->bufferNitems;
	buffersize = arg->buffersize;

	cs_comment = arg->cs_comment;
	csStimulus = arg->csStimulus;
	csConcentration = arg->csConcentration;
	csStimulus2 = arg->csStimulus2;
	csConcentration2 = arg->csConcentration2;

	csInsectname = arg->csInsectname;
	csLocation = arg->csLocation;
	csSensillum = arg->csSensillum;
	csStrain = arg->csStrain;
	csSex = arg->csSex;
	csMoreComment = arg->csMoreComment;
	csADcardName = arg->csADcardName;
	csOperator = arg->csOperator;
	insectID = arg->insectID;
	sensillumID = arg->sensillumID;
	repeat = arg->repeat;
	repeat2 = arg->repeat2;
}

// Write CWaveFormat in a binary file
long CWaveFormat::Write(CFile* datafile)
{
	const auto p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

// Read CWaveFormat from a binary file
BOOL CWaveFormat::Read(CFile* datafile)
{
	CArchive ar(datafile, CArchive::load);
	auto flag = TRUE;
	try
	{
		Serialize(ar);
	}
	catch (CException* e)
	{
		e->Delete();
		flag = FALSE;
	}
	ar.Close();
	return flag;
}

CString CWaveFormat::GetComments(const CString& p_separator, const BOOL b_explanation) const
{
	CString cs_out;
	cs_out += addComments(p_separator, b_explanation, _T("comment1="), cs_comment);
	cs_out += addComments(p_separator, b_explanation, _T("stim1="), csStimulus);
	cs_out += addComments(p_separator, b_explanation, _T("conc1= #"), csConcentration);
	cs_out += addComments(p_separator, b_explanation, _T("stim2="), csStimulus2);
	cs_out += addComments(p_separator, b_explanation, _T("com2= #"), csConcentration2);

	cs_out += addComments(p_separator, b_explanation, _T("insect="), csInsectname);
	cs_out += addComments(p_separator, b_explanation, _T("location="), csLocation);
	cs_out += addComments(p_separator, b_explanation, _T("sensillum="), csSensillum);
	cs_out += addComments(p_separator, b_explanation, _T("strain="), csStrain);
	cs_out += addComments(p_separator, b_explanation, _T("sex="), csSex);
	cs_out += addComments(p_separator, b_explanation, _T("operator="), csOperator);
	cs_out += addComments(p_separator, b_explanation, _T("comment2="), csMoreComment);

	return cs_out;
}

CString CWaveFormat::addComments(const CString& p_separator, const BOOL b_explanation, const CString& cs_explanation,
                                 const CString& cs_comment)
{
	auto cs_out = p_separator;
	if (b_explanation)
		cs_out += cs_explanation;
	if (!cs_comment.IsEmpty())
		cs_out += cs_comment;
	return cs_out;
}
