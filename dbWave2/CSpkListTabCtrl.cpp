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
void CSpkListTabCtrl::InitctrlTabFromSpikeList(CdbWaveDoc* pDoc)
{
	// reset tab control
	DeleteAllItems();

	// load list of detection parameters
	auto j = 0;
	if (pDoc->OpenCurrentSpikeFile() != nullptr)
	{
		const auto curr_listsize = pDoc->GetcurrentSpkDocument()->GetSpkList_Size();
		for (auto i = 0; i < curr_listsize; i++)
		{
			auto p_spike_list = pDoc->m_pSpk->SetSpkList_AsCurrent(i);
			if (!p_spike_list)
				continue;
			CString cs;
			if (p_spike_list->GetdetectWhat() != 0)
				continue;
			cs.Format(_T("#%i %s"), i, static_cast<LPCTSTR>(p_spike_list->GetComment()));
			InsertItem(j, cs);
			//m_tabCtrl.SetItemData(j, i);		// save list item
			j++;
		}
	}
}

void CSpkListTabCtrl::SetCurSpkList(CdbWaveDoc* pDoc)
{
	int icur = pDoc->GetcurrentSpkDocument()->GetSpkList_CurrentIndex();
	SetCurSel(icur);
}



