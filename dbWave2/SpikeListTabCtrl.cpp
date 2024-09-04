// CSpkListTabCtrl.cpp : implementation file
//
#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "SpikeListTabCtrl.h"

// CSpkListTabCtrl

IMPLEMENT_DYNAMIC(SpikeListTabCtrl, CTabCtrl)

SpikeListTabCtrl::SpikeListTabCtrl()
{
}

SpikeListTabCtrl::~SpikeListTabCtrl()
{
}

BEGIN_MESSAGE_MAP(SpikeListTabCtrl, CTabCtrl)
END_MESSAGE_MAP()

// CSpkListTabCtrl message handlers
void SpikeListTabCtrl::init_ctrl_tab_from_spike_doc(CSpikeDoc* p_spk_doc)
{
	DeleteAllItems();
	auto j = 0;
	if (p_spk_doc == nullptr) return;

	const auto curr_list_size = p_spk_doc->get_spike_list_size();
	for (auto i = 0; i < curr_list_size; i++)
	{
		auto p_spike_list = p_spk_doc->get_spike_list_at(i);
		if (!p_spike_list)
			continue;
		CString cs;
		if (p_spike_list->get_detection_parameters()->detect_what != DETECT_SPIKES)
			continue;
		cs.Format(_T("#%i %s"), i, (LPCTSTR) p_spike_list->get_detection_parameters()->comment);
		InsertItem(j, cs);
		j++;
	}
}
