#pragma once

#include "Spikedoc.h"
#include "ChartData.h"
#include "ChartSpikeBar.h"


class CDataListCtrl_Row : public CObject
{
	DECLARE_SERIAL(CDataListCtrl_Row)
public:
	CDataListCtrl_Row();
	CDataListCtrl_Row(int i);
	~CDataListCtrl_Row() override;

	BOOL b_changed = false;
	WORD w_version = 0;
	BOOL b_init = false;
	int index = 0;
	long insect_id = 0;

	CString cs_comment{};
	CString cs_datafile_name{};
	CString cs_spike_file_name{};
	CString cs_sensillum_name{};
	CString cs_stimulus1{};
	CString cs_concentration1{};
	CString cs_stimulus2{};
	CString cs_concentration2{};
	CString cs_n_spikes{};
	CString cs_flag{};
	CString cs_date{};

	AcqDataDoc* p_data_doc = nullptr;
	ChartData* p_data_chart_wnd = nullptr;
	CSpikeDoc* p_spike_doc = nullptr;
	ChartSpikeBar* p_spike_chart_wnd = nullptr;

	CDataListCtrl_Row& operator =(const CDataListCtrl_Row& arg);
	void Serialize(CArchive& ar) override;
};
