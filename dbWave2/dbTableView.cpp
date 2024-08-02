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
	return GetDocument()->db_get_recordset();
}

void dbTableView::OnSize(UINT nType, int cx, int cy)
{
	if (m_b_init_)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			m_stretch_.ResizeControls(nType, cx, cy);
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
	if (m_auto_detect && p_document->db_get_current_spk_file_name(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
	}
	p_document->update_all_views_db_wave(nullptr, HINT_DOC_MOVE_RECORD, nullptr);
	return flag;
}

void dbTableView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		AfxGetMainWnd()->PostMessage(WM_MYMESSAGE, HINT_ACTIVATE_VIEW,
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

void dbTableView::save_current_spk_file()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		const auto p_doc = GetDocument();
		auto current_list = 0;
		if (m_tabCtrl.m_hWnd != nullptr) current_list = m_tabCtrl.GetCurSel();
		m_pSpkList = m_pSpkDoc->set_spike_list_current_index(current_list);
		if (m_pSpkList != nullptr && !m_pSpkList->is_class_list_valid())
			m_pSpkList->update_class_list();

		const auto spk_file_name = p_doc->db_set_current_spike_file_name();
		m_pSpkDoc->OnSaveDocument(spk_file_name);
		m_pSpkDoc->SetModifiedFlag(FALSE);

		auto n_spike_classes = 1;
		const auto n_spikes = (m_pSpkList != nullptr) ? m_pSpkList->get_spikes_count() : 0;
		if (n_spikes > 0)
		{
			n_spike_classes = m_pSpkList->get_classes_count();
			if (!m_pSpkList->is_class_list_valid()) 
				n_spike_classes = m_pSpkList->update_class_list();				
		}
		p_doc->set_db_n_spikes(n_spikes);
		p_doc->set_db_n_spike_classes(n_spike_classes);
	}
}

void dbTableView::increment_spike_flag()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		const auto p_doc = GetDocument();
		// change flag is button is checked
		int flag = p_doc->db_get_current_record_flag();
		flag++;
		p_doc->db_set_current_record_flag(flag);
	}
}

void dbTableView::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto i_cur_sel = m_tabCtrl.GetCurSel();
	SendMessage(WM_MYMESSAGE, HINT_VIEW_TAB_CHANGE, MAKELPARAM(i_cur_sel, 0));
	*pResult = 0;
}

void dbTableView::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto i_cur_sel = m_tabCtrl.GetCurSel();
	PostMessage(WM_MYMESSAGE, HINT_VIEW_TAB_HAS_CHANGED, MAKELPARAM(i_cur_sel, 0));
	*pResult = 0;
}
