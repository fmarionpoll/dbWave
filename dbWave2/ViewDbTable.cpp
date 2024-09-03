#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "ViewDbTable.h"
#include "dbWave_constants.h"

IMPLEMENT_DYNAMIC(ViewDbTable, CDaoRecordView)

ViewDbTable::ViewDbTable(LPCTSTR lpsz_template_name)
	: CDaoRecordView(lpsz_template_name)
{
	m_bEnableActiveAccessibility = FALSE;
}

ViewDbTable::ViewDbTable(UINT n_id_template)
	: CDaoRecordView(n_id_template)
{
	m_bEnableActiveAccessibility = FALSE;
}

ViewDbTable::~ViewDbTable()
= default;

BEGIN_MESSAGE_MAP(ViewDbTable, CDaoRecordView)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &ViewDbTable::on_nm_click_tab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &ViewDbTable::on_tcn_sel_change_tab1)

END_MESSAGE_MAP()

//  drawing
void ViewDbTable::OnDraw(CDC* p_dc)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

// diagnostics
#ifdef _DEBUG
void ViewDbTable::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

CdbWaveDoc* ViewDbTable::GetDocument()
{
	return static_cast<CdbWaveDoc*>(m_pDocument);
}

void ViewDbTable::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

#endif //_DEBUG

BOOL ViewDbTable::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

CDaoRecordset* ViewDbTable::OnGetRecordset()
{
	return GetDocument()->db_get_recordset();
}

void ViewDbTable::OnSize(UINT n_type, int cx, int cy)
{
	if (b_init_)
	{
		switch (n_type)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			stretch_.resize_controls(n_type, cx, cy);
			break;
		default:
			break;
		}
	}
	CDaoRecordView::OnSize(n_type, cx, cy);
}

BOOL ViewDbTable::OnMove(const UINT n_id_move_command)
{
	const auto flag = CDaoRecordView::OnMove(n_id_move_command);
	auto p_document = GetDocument();
	if (m_auto_detect && p_document->db_get_current_spk_file_name(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKE_DETECTION, NULL);
	}
	p_document->update_all_views_db_wave(nullptr, HINT_DOC_MOVE_RECORD, nullptr);
	return flag;
}

void ViewDbTable::OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactive_view)
{
	if (b_activate)
	{
		AfxGetMainWnd()->PostMessage(WM_MYMESSAGE, HINT_ACTIVATE_VIEW,
		                             reinterpret_cast<LPARAM>(p_activate_view->GetDocument()));
	}
	else
	{
	}
	CDaoRecordView::OnActivateView(b_activate, p_activate_view, p_deactive_view);
}

BOOL ViewDbTable::OnPreparePrinting(CPrintInfo* p_info)
{
	if (!DoPreparePrinting(p_info))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, p_info))
		return FALSE;

	return TRUE;
}

void ViewDbTable::OnBeginPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void ViewDbTable::OnEndPrinting(CDC* /*p_dc*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void ViewDbTable::OnPrint(CDC* p_dc, CPrintInfo* p_info)
{
	// TODO: add customized printing code here
	if (p_info->m_bDocObject)
		COleDocObjectItem::OnPrint(this, p_info, TRUE);
	else
		CView::OnPrint(p_dc, p_info);
}

void ViewDbTable::save_current_spk_file()
{
	const auto p_doc = GetDocument();
	const long current_position = p_doc->db_get_current_record_position();
	const long record_id = p_doc->db_get_current_record_id();

	if (p_spk_doc != nullptr && p_spk_doc->IsModified())
	{
		auto current_list = 0;
		if (spk_list_tab_ctrl.m_hWnd != nullptr) current_list = spk_list_tab_ctrl.GetCurSel();
		p_spk_list = p_spk_doc->set_index_current_spike_list(current_list);
		if (p_spk_list != nullptr && !p_spk_list->is_class_list_valid())
			p_spk_list->update_class_list();

		const auto spk_file_name = p_doc->db_set_current_spike_file_name();
		p_spk_doc->OnSaveDocument(spk_file_name);
		p_spk_doc->SetModifiedFlag(FALSE);

		auto n_spike_classes = 1;
		const auto n_spikes = (p_spk_list != nullptr) ? p_spk_list->get_spikes_count() : 0;
		if (n_spikes > 0)
		{
			n_spike_classes = p_spk_list->get_classes_count();
			if (!p_spk_list->is_class_list_valid()) 
				n_spike_classes = p_spk_list->update_class_list();				
		}
		p_doc->set_db_n_spikes(n_spikes);
		p_doc->set_db_n_spike_classes(n_spike_classes);
	}

	BOOL success = p_doc->db_move_to_id(record_id);
}

void ViewDbTable::increment_spike_flag()
{
	if (p_spk_doc != nullptr && p_spk_doc->IsModified())
	{
		const auto p_doc = GetDocument();
		// change flag is button is checked
		int flag = p_doc->db_get_current_record_flag();
		flag++;
		p_doc->db_set_current_record_flag(flag);
	}
}

void ViewDbTable::on_nm_click_tab1(NMHDR* p_nmhdr, LRESULT* p_result)
{
	const auto i_cur_sel = spk_list_tab_ctrl.GetCurSel();
	SendMessage(WM_MYMESSAGE, HINT_VIEW_TAB_CHANGE, MAKELPARAM(i_cur_sel, 0));
	*p_result = 0;
}

void ViewDbTable::on_tcn_sel_change_tab1(NMHDR* p_nmhdr, LRESULT* p_result)
{
	const auto i_cur_sel = spk_list_tab_ctrl.GetCurSel();
	PostMessage(WM_MYMESSAGE, HINT_VIEW_TAB_HAS_CHANGED, MAKELPARAM(i_cur_sel, 0));
	*p_result = 0;
}
