#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "dbTableView.h"
#include "dbWave_constants.h"

IMPLEMENT_DYNAMIC(dbTableView, CDaoRecordView)

dbTableView::dbTableView(LPCTSTR lpszTemplateName)
	: CDaoRecordView(lpszTemplateName)
{
	m_bEnableActiveAccessibility = FALSE;
}

dbTableView::dbTableView(UINT nIDTemplate)
	: CDaoRecordView(nIDTemplate)
{
	m_bEnableActiveAccessibility = FALSE;
}

dbTableView::~dbTableView()
= default;

BEGIN_MESSAGE_MAP(dbTableView, CDaoRecordView)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &dbTableView::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &dbTableView::OnTcnSelchangeTab1)

END_MESSAGE_MAP()

//  drawing
void dbTableView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

// diagnostics
#ifdef _DEBUG
void dbTableView::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

CdbWaveDoc* dbTableView::GetDocument()
{
	return static_cast<CdbWaveDoc*>(m_pDocument);
}

void dbTableView::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

#endif //_DEBUG

BOOL dbTableView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

CDaoRecordset* dbTableView::OnGetRecordset()
{
	return GetDocument()->DB_GetRecordset();
}

void dbTableView::OnSize(UINT nType, int cx, int cy)
{
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}
	CDaoRecordView::OnSize(nType, cx, cy);
}

BOOL dbTableView::OnMove(UINT nIDMoveCommand)
{
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	auto p_document = GetDocument();
	if (m_autoDetect && p_document->DB_GetCurrentSpkFileName(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
	}
	p_document->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void dbTableView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		AfxGetMainWnd()->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW,
		                             reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

BOOL dbTableView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	return TRUE;
}

void dbTableView::OnBeginPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void dbTableView::OnEndPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void dbTableView::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	// TODO: add customized printing code here
	if (pInfo->m_bDocObject)
		COleDocObjectItem::OnPrint(this, pInfo, TRUE);
	else
		CView::OnPrint(p_dc, pInfo);
}

void dbTableView::saveCurrentSpkFile()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		auto p_doc = GetDocument();
		auto currentlist = 0;
		if (m_tabCtrl.m_hWnd != nullptr) currentlist = m_tabCtrl.GetCurSel();
		m_pSpkList = m_pSpkDoc->set_spk_list_as_current(currentlist);
		if (m_pSpkList != nullptr && !m_pSpkList->IsClassListValid())
			m_pSpkList->UpdateClassList();

		auto spkfile_name = p_doc->DB_SetCurrentSpikeFileName();
		m_pSpkDoc->OnSaveDocument(spkfile_name);
		m_pSpkDoc->SetModifiedFlag(FALSE);

		auto n_spike_classes = 1;
		const auto n_spikes = (m_pSpkList != nullptr) ? m_pSpkList->get_spikes_count() : 0;
		if (n_spikes > 0)
		{
			n_spike_classes = m_pSpkList->get_classes_count();
			if (!m_pSpkList->IsClassListValid()) 
				n_spike_classes = m_pSpkList->UpdateClassList();				
		}
		p_doc->SetDB_n_spikes(n_spikes);
		p_doc->SetDB_n_spike_classes(n_spike_classes);

	}
}

void dbTableView::IncrementSpikeFlag()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		auto p_doc = GetDocument();
		// change flag is button is checked
		int flag = p_doc->DB_GetCurrentRecordFlag();
		flag++;
		p_doc->DB_SetCurrentRecordFlag(flag);
	}
}

void dbTableView::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto icursel = m_tabCtrl.GetCurSel();
	SendMessage(WM_MYMESSAGE, HINT_VIEWTABCHANGE, MAKELPARAM(icursel, 0));
	*pResult = 0;
}

void dbTableView::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	PostMessage(WM_MYMESSAGE, HINT_VIEWTABHASCHANGED, MAKELPARAM(icursel, 0));
	*pResult = 0;
}
