#include "StdAfx.h"
#include "AcqWaveFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CWaveFormat, CObject, 0 /* schema number*/)

CWaveFormat::CWaveFormat()
{
	acquisition_time = CTime::GetCurrentTime();
}

CWaveFormat::~CWaveFormat()
= default;

void CWaveFormat::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		constexpr WORD version = 10;
		ar << version; // 1
		ar << acquisition_time; // 2
		ar << full_scale_volts; // 3
		ar << bin_span; // 4
		ar << bin_zero; // 5
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
		ar << b_online_display;
		ar << b_ad_write_to_file;
		ar << buffer_n_items;
		ar << buffer_size;

		constexpr int n_comments = 14;
		ar << n_comments;
		// save "CString"
		ar << cs_comment;
		ar << cs_stimulus;
		ar << cs_concentration;
		ar << cs_insect_name;
		ar << cs_operator;
		ar << cs_location;
		ar << cs_sensillum;
		ar << cs_strain;
		ar << cs_more_comment;
		ar << cs_ad_card_name;
		ar << cs_stimulus2;
		ar << cs_concentration2;
		ar << cs_sex;

		constexpr int n_items = 4;
		ar << n_items;
		// save "long"
		ar << insect_id;
		ar << sensillum_id;
		ar << repeat;
		ar << repeat2;
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
			ar >> acquisition_time; // 2
			ar >> full_scale_volts; // 3
			ar >> bin_span; // 4
			ar >> bin_zero; // 5
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
			ar >> b_online_display;
			ar >> b_ad_write_to_file;
			ar >> buffer_n_items >> buffer_size;

			int n_comments;
			ar >> n_comments;
			ASSERT(n_comments == 14);
			ar >> cs_comment;
			ar >> cs_stimulus;
			ar >> cs_concentration;
			ar >> cs_insect_name;
			ar >> cs_operator;
			ar >> cs_location;
			ar >> cs_sensillum;
			ar >> cs_strain;
			ar >> cs_more_comment;
			ar >> cs_ad_card_name;
			ar >> cs_stimulus2;
			ar >> cs_concentration2;
			ar >> cs_sex;

			int n_items; ar >> n_items;
			n_items--; ar >> insect_id;		// 4
			n_items--; ar >> sensillum_id;	// 3
			n_items--; ar >> repeat;		// 2
			n_items--; ar >> repeat2;		// 1
			float dummy;
			n_items--; if (n_items >= 0) ar >> dummy;
		}
	}
}

void CWaveFormat::read_v8_and_before(CArchive& ar, WORD version)
{
	ASSERT(!ar.IsStoring());
	WORD w;
	ar >> acquisition_time; // 2
	ar >> full_scale_volts; // 3
	if (version < 6)
	{
		ar >> w;
		bin_span = w; // 4
		ar >> w;
		bin_zero = w; // 5
	}
	else
	{
		ar >> bin_span; // 4
		ar >> bin_zero; // 5
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
	ar >> b_online_display;
	ar >> b_ad_write_to_file;
	ar >> buffer_n_items >> buffer_size;

	if (version >= 7)
	{
		ar >> cs_comment;
		ar >> cs_stimulus;
		ar >> cs_concentration;
		ar >> cs_insect_name;
		ar >> cs_operator;
		ar >> cs_location;
		ar >> cs_sensillum;
		ar >> cs_strain;
		ar >> cs_more_comment;
		ar >> cs_ad_card_name;
		ar >> cs_stimulus2;
		ar >> cs_concentration2;
		if (version == 8)
			ar >> cs_sex;
	}
	else
	{
		ar >> cs_comment;
		ar >> cs_stimulus;
		ar >> cs_concentration;
		ar >> cs_insect_name;
		ar >> cs_operator;
		ar >> cs_location;
		ar >> cs_sensillum;
		ar >> cs_strain;
		ar >> cs_more_comment;
		if (version >= 3)
		{
			ar >> cs_ad_card_name;
		}
	}

	if (version > 4)
	{
		int n_items;
		ar >> n_items;
		ar >> insect_id;
		n_items--;
		ar >> sensillum_id;
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
		insect_id = 0;
		sensillum_id = 0;
	}
}

void CWaveFormat::copy(const CWaveFormat* arg)
{
	acquisition_time = arg->acquisition_time;
	w_version = arg->w_version;
	full_scale_volts = arg->full_scale_volts;
	bin_span = arg->bin_span;
	bin_zero = arg->bin_zero;

	mode_encoding = arg->mode_encoding;
	mode_clock = arg->mode_clock;
	mode_trigger = arg->mode_trigger;

	sampling_rate_per_channel = arg->sampling_rate_per_channel;
	scan_count = arg->scan_count;
	sample_count = arg->sample_count;
	duration = arg->duration;

	trig_mode = arg->trig_mode;
	trig_chan = arg->trig_chan;
	trig_threshold = arg->trig_threshold;

	data_flow = arg->data_flow;
	b_online_display = arg->b_online_display;
	b_ad_write_to_file = arg->b_ad_write_to_file;
	buffer_n_items = arg->buffer_n_items;
	buffer_size = arg->buffer_size;

	cs_comment = arg->cs_comment;
	cs_stimulus = arg->cs_stimulus;
	cs_concentration = arg->cs_concentration;
	cs_stimulus2 = arg->cs_stimulus2;
	cs_concentration2 = arg->cs_concentration2;

	cs_insect_name = arg->cs_insect_name;
	cs_location = arg->cs_location;
	cs_sensillum = arg->cs_sensillum;
	cs_strain = arg->cs_strain;
	cs_sex = arg->cs_sex;
	cs_more_comment = arg->cs_more_comment;
	cs_ad_card_name = arg->cs_ad_card_name;
	cs_operator = arg->cs_operator;
	insect_id = arg->insect_id;
	sensillum_id = arg->sensillum_id;
	repeat = arg->repeat;
	repeat2 = arg->repeat2;
}

long CWaveFormat::write(CFile* datafile)
{
	const auto p1 = datafile->GetPosition();
	CArchive ar(datafile, CArchive::store);
	Serialize(ar);
	ar.Close();
	const auto p2 = datafile->GetPosition();
	return static_cast<long>(p2 - p1);
}

BOOL CWaveFormat::read(CFile* datafile)
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

CString CWaveFormat::get_comments(const CString& p_separator, const BOOL b_explanation) const
{
	CString cs_out;
	cs_out += add_comments(p_separator, b_explanation, _T("comment1="), cs_comment);
	cs_out += add_comments(p_separator, b_explanation, _T("stim1="), cs_stimulus);
	cs_out += add_comments(p_separator, b_explanation, _T("conc1= #"), cs_concentration);
	cs_out += add_comments(p_separator, b_explanation, _T("stim2="), cs_stimulus2);
	cs_out += add_comments(p_separator, b_explanation, _T("com2= #"), cs_concentration2);

	cs_out += add_comments(p_separator, b_explanation, _T("insect="), cs_insect_name);
	cs_out += add_comments(p_separator, b_explanation, _T("location="), cs_location);
	cs_out += add_comments(p_separator, b_explanation, _T("sensillum="), cs_sensillum);
	cs_out += add_comments(p_separator, b_explanation, _T("strain="), cs_strain);
	cs_out += add_comments(p_separator, b_explanation, _T("sex="), cs_sex);
	cs_out += add_comments(p_separator, b_explanation, _T("operator="), cs_operator);
	cs_out += add_comments(p_separator, b_explanation, _T("comment2="), cs_more_comment);

	return cs_out;
}

CString CWaveFormat::add_comments(const CString& p_separator, const BOOL b_explanation, const CString& cs_explanation,
                                 const CString& cs_comment)
{
	auto cs_out = p_separator;
	if (b_explanation)
		cs_out += cs_explanation;
	if (!cs_comment.IsEmpty())
		cs_out += cs_comment;
	return cs_out;
}
