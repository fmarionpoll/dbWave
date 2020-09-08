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
void CSpkListTabCtrl::InitctrlTabFromSpikeList(CSpikeDoc* pSpkDoc)
{
	DeleteAllItems();
	auto j = 0;
	
		const auto curr_listsize = pSpkDoc->GetSpkList_Size();
		for (auto i = 0; i < curr_listsize; i++)
		{
			auto p_spike_list = pSpkDoc->GetSpkList_At(i);
			if (!p_spike_list)
				continue;
			CString cs;
			if (p_spike_list->GetdetectWhat() != 0)
				continue;
			cs.Format(_T("#%i %s"), i, static_cast<LPCTSTR>(p_spike_list->GetComment()));
			InsertItem(j, cs);
			j++;
		}
	
}





