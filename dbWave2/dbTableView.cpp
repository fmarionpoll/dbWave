#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "dbTableView.h"

#include "dbWave.h"
#include "dbWave_constants.h"

IMPLEMENT_DYNAMIC(CdbTableView, CDaoRecordView)

CdbTableView::CdbTableView(LPCTSTR lpszTemplateName)
	: CDaoRecordView(lpszTemplateName)
{
	m_bEnableActiveAccessibility = FALSE;
}

CdbTableView::CdbTableView(UINT nIDTemplate)
	: CDaoRecordView(nIDTemplate)
{
	m_bEnableActiveAccessibility = FALSE;
}

CdbTableView::~CdbTableView()
{
}

BEGIN_MESSAGE_MAP(CdbTableView, CDaoRecordView)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CdbTableView::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CdbTableView::OnTcnSelchangeTab1)

END_MESSAGE_MAP()

//  drawing
void CdbTableView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

// diagnostics
#ifdef _DEBUG
void CdbTableView::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

CdbWaveDoc* CdbTableView::GetDocument()
{
	return static_cast<CdbWaveDoc*>(m_pDocument);
}

#ifndef _WIN32_WCE
void CdbTableView::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}
#endif
#endif //_DEBUG

BOOL CdbTableView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

CDaoRecordset* CdbTableView::OnGetRecordset()
{
	return GetDocument()->GetDB_Recordset();
}

void CdbTableView::OnSize(UINT nType, int cx, int cy)
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

BOOL CdbTableView::OnMove(UINT nIDMoveCommand)
{
	//const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	//GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	//return flag;
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	auto p_document = GetDocument();
	if (m_autoDetect && p_document->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		//return false;
	}
	p_document->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CdbTableView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
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

void CdbTableView::OnDestroy()
{
	// Deactivate the item on destruction; this is important
	// when a splitter view is being used.
	CDaoRecordView::OnDestroy();
}

BOOL CdbTableView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	return TRUE;
}

void CdbTableView::OnBeginPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CdbTableView::OnEndPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CdbTableView::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	// TODO: add customized printing code here
	if (pInfo->m_bDocObject)
		COleDocObjectItem::OnPrint(this, pInfo, TRUE);
	else
		CView::OnPrint(p_dc, pInfo);
}

void CdbTableView::saveCurrentSpkFile()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		auto p_doc = GetDocument();
		auto currentlist = 0;
		if (m_tabCtrl.m_hWnd != nullptr) currentlist = m_tabCtrl.GetCurSel();
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
		if (m_pSpkList != nullptr && !m_pSpkList->IsClassListValid())
			m_pSpkList->UpdateClassList();

		auto spkfile_name = p_doc->SetDB_CurrentSpikeFileName();
		m_pSpkDoc->OnSaveDocument(spkfile_name);
		m_pSpkDoc->SetModifiedFlag(FALSE);

		auto nclasses = 1;
		const auto ntotalspikes = (m_pSpkList != nullptr) ? m_pSpkList->GetTotalSpikes() : 0;
		if (ntotalspikes > 0)
		{
			if (!m_pSpkList->IsClassListValid()) // if class list not valid:
				nclasses = m_pSpkList->UpdateClassList();
			else
				nclasses = m_pSpkList->GetNbclasses();
		}
		p_doc->SetDB_nbspikes(ntotalspikes);
		p_doc->SetDB_n_spike_classes(nclasses);

	}
}

void CdbTableView::IncrementSpikeFlag()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		auto p_doc = GetDocument();
		// change flag is button is checked
		int flag = p_doc->GetDB_CurrentRecordFlag();
		flag++;
		p_doc->SetDB_CurrentRecordFlag(flag);
	}
}

void CdbTableView::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	auto icursel = m_tabCtrl.GetCurSel();
	SendMessage(WM_MYMESSAGE, HINT_VIEWTABCHANGE, MAKELPARAM(icursel, 0));
	*pResult = 0;
}

void CdbTableView::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	PostMessage(WM_MYMESSAGE, HINT_VIEWTABHASCHANGED, MAKELPARAM(icursel, 0));
	*pResult = 0;
}
