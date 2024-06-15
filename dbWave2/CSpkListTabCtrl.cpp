// CSpkListTabCtrl.cpp : implementation file
//
#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "CSpkListTabCtrl.h"

// CSpkListTabCtrl

IMPLEMENT_DYNAMIC(CSpkListTabCtrl, CTabCtrl)

CSpkListTabCtrl::CSpkListTabCtrl()
{
}

CSpkListTabCtrl::~CSpkListTabCtrl()
{
}

BEGIN_MESSAGE_MAP(CSpkListTabCtrl, CTabCtrl)
END_MESSAGE_MAP()

// CSpkListTabCtrl message handlers
void CSpkListTabCtrl::InitctrlTabFromSpikeDoc(CSpikeDoc* pSpkDoc)
{
	DeleteAllItems();
	auto j = 0;
	if (pSpkDoc == nullptr) return;

	const auto curr_listsize = pSpkDoc->get_spk_list_size();
	for (auto i = 0; i < curr_listsize; i++)
	{
		auto p_spike_list = pSpkDoc->get_spk_list_at(i);
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
