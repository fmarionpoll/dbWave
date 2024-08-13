#include "StdAfx.h"
#include "DataListCtrl_Row.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(CDataListCtrl_Row, CObject, 0)

CDataListCtrl_Row::CDataListCtrl_Row()
= default;

CDataListCtrl_Row::CDataListCtrl_Row(const int i)
{
	index = i;
}

CDataListCtrl_Row::~CDataListCtrl_Row()
{
	delete p_data_chart_wnd;
	delete p_spike_chart_wnd;
	
	SAFE_DELETE(p_data_doc)
	SAFE_DELETE(p_spike_doc)
	cs_comment.Empty();
	cs_datafile_name.Empty();
	cs_spike_file_name.Empty();
	cs_sensillum_name.Empty();
	cs_stimulus1.Empty();
	cs_concentration1.Empty();
	cs_stimulus2.Empty();
	cs_concentration2.Empty();
	cs_n_spikes.Empty();
	cs_flag.Empty();
	cs_date.Empty();
}

CDataListCtrl_Row& CDataListCtrl_Row::operator =(const CDataListCtrl_Row& arg)
{
	if (this != &arg)
	{
		b_init = arg.b_init;
		index = arg.index;
		insect_id = arg.insect_id;
		cs_comment = arg.cs_comment;
		cs_datafile_name = arg.cs_datafile_name;
		cs_spike_file_name = arg.cs_spike_file_name;
		cs_sensillum_name = arg.cs_sensillum_name;
		cs_stimulus1 = arg.cs_stimulus1;
		cs_concentration1 = arg.cs_concentration1;
		cs_stimulus2 = arg.cs_stimulus2;
		cs_concentration2 = arg.cs_concentration2;
		cs_n_spikes = arg.cs_n_spikes;
		cs_flag = arg.cs_flag;
		cs_date = arg.cs_date;
		p_data_chart_wnd = arg.p_data_chart_wnd;
		p_spike_chart_wnd = arg.p_spike_chart_wnd;
		p_data_doc = arg.p_data_doc;
		p_spike_doc = arg.p_spike_doc;
	}
	return *this;
}

void CDataListCtrl_Row::Serialize(CArchive& ar)
{
	// not serialized:
	// AcqDataDoc*	p_data_Doc;
	// CSpikeDoc*	p_spike_Doc;

	if (ar.IsStoring())
	{
		w_version = 2;
		ar << w_version;
		ar << index;

		constexpr auto string_count = 8;
		ar << string_count;
		ar << cs_comment;
		ar << cs_datafile_name;
		ar << cs_sensillum_name;
		ar << cs_stimulus1;
		ar << cs_concentration1;
		ar << cs_stimulus2;
		ar << cs_concentration2;
		ar << cs_n_spikes;
		ar << cs_flag;
		ar << cs_date;

		constexpr auto object_count = 3;
		ar << object_count;
		p_data_chart_wnd->Serialize(ar);
		p_spike_chart_wnd->Serialize(ar);
		ar << insect_id;
	}
	else
	{
		ar >> w_version;
		ar >> index;

		int string_count;
		ar >> string_count;
		ar >> cs_comment;
		string_count--;
		ar >> cs_datafile_name;
		string_count--;
		ar >> cs_sensillum_name;
		string_count--;
		ar >> cs_stimulus1;
		string_count--;
		ar >> cs_concentration1;
		string_count--;
		if (w_version > 1)
		{
			ar >> cs_stimulus2;
			string_count--;
			ar >> cs_concentration2;
			string_count--;
		}
		ar >> cs_n_spikes;
		string_count--;
		ar >> cs_flag;
		string_count--;
		ar >> cs_date;
		string_count--;

		int object_count;
		ar >> object_count;
		ASSERT(object_count >= 2);
		p_data_chart_wnd->Serialize(ar);
		object_count--;
		p_spike_chart_wnd->Serialize(ar);;
		object_count--;
		if (object_count > 0) 
			ar >> insect_id;
	}
}
