#include "StdAfx.h"
#include "dbWave.h"
#include "resource.h"

#include "dbTableMain.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"

#include "ChartWnd.h"
#include "Editctrl.h"
#include "DataListCtrl.h"

#include "MainFrm.h"
#include "ChildFrm.h"

#include "ViewdbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewdbWave, ViewDbTable)

BEGIN_MESSAGE_MAP(ViewdbWave, ViewDbTable)
	//ON_WM_DESTROY()
	ON_WM_SIZE()

	ON_COMMAND(ID_RECORD_PAGE_UP, &ViewdbWave::on_record_page_up)
	ON_COMMAND(ID_RECORD_PAGE_DOWN, &ViewdbWave::on_record_page_down)
	ON_COMMAND(ID_FILE_PRINT, ViewDbTable::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, ViewDbTable::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, ViewDbTable::OnFilePrintPreview)

	ON_BN_CLICKED(IDC_DISPLAYDATA, &ViewdbWave::on_bn_clicked_data)
	ON_BN_CLICKED(IDC_DISPLAY_SPIKES, &ViewdbWave::on_bn_clicked_display_spikes)
	ON_BN_CLICKED(IDC_DISPLAY_NOTHING, &ViewdbWave::on_bn_clicked_display_nothing)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewdbWave::on_en_change_time_first)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewdbWave::on_en_change_time_last)
	ON_EN_CHANGE(IDC_AMPLITUDESPAN, &ViewdbWave::on_en_change_amplitude_span)
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewdbWave::on_en_change_spike_class)
	ON_BN_CLICKED(IDC_CHECKFILENAME, &ViewdbWave::on_bn_clicked_check_filename)
	ON_BN_CLICKED(IDC_FILTERCHECK, &ViewdbWave::on_click_median_filter)
	ON_BN_CLICKED(IDC_CHECK2, &ViewdbWave::on_bn_clicked_check2)
	ON_BN_CLICKED(IDC_CHECK1, &ViewdbWave::on_bn_clicked_check1)
	ON_BN_CLICKED(IDC_RADIOALLCLASSES, &ViewdbWave::on_bn_clicked_radio_all_classes)
	ON_BN_CLICKED(IDC_RADIOONECLASS, &ViewdbWave::on_bn_clicked_radio_one_class)
	

	ON_NOTIFY(HDN_ENDTRACK, 0, &ViewdbWave::on_hdn_end_track_list_ctrl)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTCTRL, &ViewdbWave::on_lvn_column_click_list_ctrl)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LISTCTRL, &ViewdbWave::on_item_activate_list_ctrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, &ViewdbWave::on_dbl_clk_list_ctrl)

	//ON_NOTIFY(NM_CLICK, IDC_TAB1, &dbTableView::OnNMClickTab1)
END_MESSAGE_MAP()

ViewdbWave::ViewdbWave() : ViewDbTable(IDD)
{
}

ViewdbWave::~ViewdbWave()
= default;

void ViewdbWave::DoDataExchange(CDataExchange * p_dx)
{
	ViewDbTable::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_TIMEFIRST, m_time_first_);
	DDX_Text(p_dx, IDC_TIMELAST, m_time_last_);
	DDX_Text(p_dx, IDC_AMPLITUDESPAN, m_amplitude_span_);
	DDX_Text(p_dx, IDC_SPIKECLASS, m_spike_class_);
	DDX_Control(p_dx, IDC_TAB1, spk_list_tab_ctrl);
}

void ViewdbWave::OnInitialUpdate()
{
	// init document and dbTableView
	const auto db_wave_doc = GetDocument();
	p_db_table_main = &db_wave_doc->db_table->m_main_table_set;
	ViewDbTable::OnInitialUpdate();

	subclass_dialog_controls();
	make_controls_stretchable();

	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	m_options_view_data_ = &p_app->options_view_data;
	m_data_list_ctrl.init_columns(&m_options_view_data_->i_column_width);

	// set how data are displayed
	m_amplitude_span_ = m_options_view_data_->mv_span;
	m_data_list_ctrl.set_amplitude_span(m_options_view_data_->mv_span);

	CheckDlgButton(IDC_CHECKFILENAME, m_options_view_data_->b_display_file_name);
	m_data_list_ctrl.set_display_file_name(m_options_view_data_->b_display_file_name);

	m_time_first_ = m_options_view_data_->t_first;
	m_time_last_ = m_options_view_data_->t_last;
	if (m_time_first_ != 0.f && m_time_last_ != 0.f)
		m_data_list_ctrl.set_time_intervals(m_time_first_, m_time_last_);

	CheckDlgButton(IDC_CHECK1, m_options_view_data_->b_set_time_span);
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(m_options_view_data_->b_set_time_span);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(m_options_view_data_->b_set_time_span);
	m_data_list_ctrl.set_timespan_adjust_mode(static_cast<boolean>(m_options_view_data_->b_set_time_span));

	CheckDlgButton(IDC_CHECK2, m_options_view_data_->b_set_mv_span);
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(m_options_view_data_->b_set_mv_span);
	m_data_list_ctrl.set_amplitude_adjust_mode(static_cast<boolean>(m_options_view_data_->b_set_mv_span));

	m_data_list_ctrl.SetExtendedStyle
	(m_data_list_ctrl.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT
		//| LVS_EX_GRIDLINES // when this style is set, data vertical bar is suppressed
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_SUBITEMIMAGES);

	// set display mode of m_dataListCtrl
	m_data_list_ctrl.set_display_mode(m_options_view_data_->display_mode);
	switch (m_options_view_data_->display_mode)
	{
	case 1:
		display_data();
		break;
	case 2:
		display_spikes();
		break;
	default:
		display_nothing();
		break;
	}

	// select the proper record
	fill_list_box();
	m_data_list_ctrl.update_cache(-3, -3);
	update_controls();

	// init display controls
	if (m_options_view_data_->display_mode == 2)
	{
		CSpikeDoc* p_spk_doc = GetDocument()->get_current_spike_file();
		if (p_spk_doc != nullptr)
		{
			spk_list_tab_ctrl.init_ctrl_tab_from_spike_doc(p_spk_doc);
			spk_list_tab_ctrl.SetCurSel(p_spk_doc->get_index_current_spike_list());
		}
		spk_list_tab_ctrl.ShowWindow(p_spk_doc != nullptr ? SW_SHOW : SW_HIDE);
	}
}

void ViewdbWave::subclass_dialog_controls()
{
	VERIFY(m_data_list_ctrl.SubclassDlgItem(IDC_LISTCTRL, this));
	VERIFY(mm_time_first_.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_time_last_.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_amplitude_span_.SubclassDlgItem(IDC_AMPLITUDESPAN, this));
	VERIFY(mm_spike_class_.SubclassDlgItem(IDC_SPIKECLASS, this));
}

void ViewdbWave::make_controls_stretchable()
{
	stretch_.attach_parent(this);
	stretch_.new_prop(IDC_LISTCTRL, XLEQ_XREQ, YTEQ_YBEQ);
	stretch_.new_prop(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	b_init_ = TRUE;
}

void ViewdbWave::display_data()
{
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYDATA))->SetCheck(BST_CHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAY_SPIKES))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAY_NOTHING))->SetCheck(BST_UNCHECKED);

	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(TRUE);
	m_options_view_data_->display_mode = 1;
	m_data_list_ctrl.set_display_mode(m_options_view_data_->display_mode);

	static_cast<CButton*>(GetDlgItem(IDC_FILTERCHECK))->SetCheck(m_options_view_data_->b_filter_dat);
	m_data_transform_ = m_options_view_data_->b_filter_dat ? 13 : 0;
	m_data_list_ctrl.set_transform_mode(m_data_transform_);
	spk_list_tab_ctrl.ShowWindow(SW_HIDE);
}

void ViewdbWave::display_spikes()
{
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAY_SPIKES))->SetCheck(BST_CHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYDATA))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAY_NOTHING))->SetCheck(BST_UNCHECKED);

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(TRUE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);

	// display all spike classes
	m_options_view_data_->display_mode = 2;
	m_data_list_ctrl.set_display_mode(m_options_view_data_->display_mode);
	if (m_options_view_data_->b_display_all_classes)
	{
		static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_CHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_UNCHECKED);
		m_data_list_ctrl.set_spike_plot_mode(PLOT_BLACK, m_spike_class_);
	}
	else
	{
		static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_UNCHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_CHECKED);
		m_spike_class_ = m_options_view_data_->spike_class;
		mm_spike_class_.EnableWindow(TRUE);
		m_data_list_ctrl.set_spike_plot_mode(PLOT_ONE_CLASS_ONLY, m_spike_class_);
	}
}

void ViewdbWave::display_nothing()
{
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYDATA))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAY_SPIKES))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAY_NOTHING))->SetCheck(BST_CHECKED);

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);

	m_options_view_data_->display_mode = 0;
	m_data_list_ctrl.set_display_mode(m_options_view_data_->display_mode);
	spk_list_tab_ctrl.ShowWindow(SW_HIDE);
}

void ViewdbWave::OnSize(const UINT n_type, const int cx, const int cy)
{
	ViewDbTable::OnSize(n_type, cx, cy);
	if (IsWindow(m_data_list_ctrl.m_hWnd))
	{
		CRect rect;
		m_data_list_ctrl.GetClientRect(&rect);
		m_data_list_ctrl.fit_columns_to_size(rect.Width());
	}
}

void ViewdbWave::update_controls()
{
	const auto db_wave_doc = GetDocument();
	CFileStatus status;

	b_valid_data_ = CFile::GetStatus(db_wave_doc->db_get_current_dat_file_name(), status);
	b_valid_spike_ = CFile::GetStatus(db_wave_doc->db_get_current_spk_file_name(TRUE), status);

	const int i_file = db_wave_doc->db_get_current_record_position();

	m_data_list_ctrl.set_current_selection(i_file);
	m_data_list_ctrl.EnsureVisible(i_file, FALSE);

	if (m_options_view_data_->display_mode == 2)
	{
		CSpikeDoc* p_spk_doc = GetDocument()->open_current_spike_file();
		if (p_spk_doc != nullptr)
		{
			const auto spk_list_size = p_spk_doc->get_spike_list_size();
			if (spk_list_tab_ctrl.GetItemCount() < spk_list_size)
				spk_list_tab_ctrl.init_ctrl_tab_from_spike_doc(p_spk_doc);
		}
	}
	//pdb_doc->SetModifiedFlag(true);
	//pdb_doc->UpdateAllViews(this, HINT_DOC_MOVE_RECORD, nullptr);
	db_wave_doc->update_all_views_db_wave(this, HINT_DOC_MOVE_RECORD, nullptr);

	//POSITION pos = pdb_doc->GetFirstViewPosition();
	//int n_views = 0;
	//while (pos != NULL)
	//{
	//	CView* pView = pdb_doc->GetNextView(pos);
	//	CMainFrame* frame = (CMainFrame*)pView->GetParentFrame();
	//	frame->OnUpdate(this, HINT_DOC_MOVE_RECORD, nullptr);

	//	//pView->UpdateWindow();
	//	n_views++;
	//}

}

void ViewdbWave::on_record_page_up() 
{
	m_data_list_ctrl.SendMessage(WM_VSCROLL, SB_PAGEUP, NULL);
}

void ViewdbWave::on_record_page_down()
{
	m_data_list_ctrl.SendMessage(WM_VSCROLL, SB_PAGEDOWN, NULL);
}

void ViewdbWave::on_click_median_filter()
{
	if (m_options_view_data_->b_filter_dat == static_cast<CButton*>(GetDlgItem(IDC_FILTERCHECK))->GetCheck())
		return;

	m_options_view_data_->b_filter_dat = static_cast<CButton*>(GetDlgItem(IDC_FILTERCHECK))->GetCheck();
	if (m_options_view_data_->b_filter_dat)
		m_data_transform_ = 13;
	else
		m_data_transform_ = 0;
	m_data_list_ctrl.set_transform_mode(m_data_transform_);
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::OnActivateView(const BOOL b_activate, CView * p_activate_view, CView * p_deactive_view)
{
	auto* p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
	if (b_activate)
	{
		// make sure the secondary toolbar is not visible (none is defined for the database)
		if (p_mainframe->m_p_second_tool_bar != nullptr)
			p_mainframe->ShowPane(p_mainframe->m_p_second_tool_bar, FALSE, FALSE, TRUE);
		// load status
		m_nStatus = static_cast<CChildFrame*>(p_mainframe->MDIGetActive())->m_n_status;
		p_mainframe->PostMessageW(WM_MYMESSAGE, HINT_ACTIVATE_VIEW, reinterpret_cast<LPARAM>(p_activate_view->GetDocument()));
	}
	else
	{
		ChartData* p_data_chart_wnd = m_data_list_ctrl.get_chart_data_of_current_record();
		if (p_data_chart_wnd != nullptr)
		{
			static_cast<CdbWaveApp*>(AfxGetApp())->options_view_data.view_data = *(p_data_chart_wnd->get_scope_parameters());
		}
		if (p_activate_view != nullptr)
			static_cast<CChildFrame*>(p_mainframe->MDIGetActive())->m_n_status = m_nStatus;
	}
	ViewDbTable::OnActivateView(b_activate, p_activate_view, p_deactive_view);
}

void ViewdbWave::fill_list_box()
{
	m_data_list_ctrl.DeleteAllItems();
	const int n_records = GetDocument()->db_get_records_count();
	m_data_list_ctrl.SetItemCountEx(n_records);
}

void ViewdbWave::on_item_activate_list_ctrl(NMHDR * p_nmhdr, LRESULT * p_result)
{
	// get item clicked and select it
	const auto p_item_activate = reinterpret_cast<NMITEMACTIVATE*>(p_nmhdr);
	if (p_item_activate->iItem >= 0) 
		GetDocument()->db_set_current_record_position(p_item_activate->iItem);
	
	ViewDbTable::OnInitialUpdate();
	GetDocument()->UpdateAllViews(nullptr, HINT_DOC_MOVE_RECORD, nullptr);

	*p_result = 0;
}

void ViewdbWave::on_dbl_clk_list_ctrl(NMHDR * p_nmhdr, LRESULT * p_result)
{
	*p_result = 0;
	// quit the current view and switch to spike detection view
	GetParent()->PostMessage(WM_COMMAND, static_cast<WPARAM>(ID_VIEW_SPIKE_DETECTION), static_cast<LPARAM>(NULL));
}

LRESULT ViewdbWave::on_my_message(const WPARAM w_param, const LPARAM l_param)
{
	const int threshold = LOWORD(l_param); // value associated
	//const int i_id = HIWORD(lParam);

	if (w_param == HINT_VIEW_TAB_HAS_CHANGED)
	{
		GetDocument()->get_current_spike_file()->set_index_current_spike_list(threshold);
		m_data_list_ctrl.refresh_display();
	}
	return 0L;
}

void ViewdbWave::OnUpdate(CView * p_sender, const LPARAM l_hint, CObject * p_hint)
{
	if (!b_init_ )
		return;

	switch (LOWORD(l_hint))
	{
	case HINT_GET_SELECTED_RECORDS:
	{
		const auto p_document = GetDocument();
		p_document->selected_records.RemoveAll();
		const int selected_count = static_cast<int>(m_data_list_ctrl.GetSelectedCount());

		// Update all selected items.
		if (selected_count > 0)
		{
			p_document->selected_records.SetSize(selected_count);
			auto n_item = -1;
			for (int i = 0; i < selected_count; i++)
			{
				n_item = m_data_list_ctrl.GetNextItem(n_item, LVNI_SELECTED);
				ASSERT(n_item != -1);
				p_document->selected_records.SetAt(i, n_item);
			}
		}
	}
	break;

	case HINT_SET_SELECTED_RECORDS:
	{
		const auto p_document = GetDocument();
		const UINT u_selected_count = p_document->selected_records.GetSize();

		// clear previous selection in the CListCtrl if any
		auto item = -1;
		item = m_data_list_ctrl.GetNextItem(item, LVNI_SELECTED);
		while (item != -1)
		{
			m_data_list_ctrl.SetItemState(item, 0, LVIS_SELECTED);
			item = m_data_list_ctrl.GetNextItem(item, LVNI_SELECTED);
		}

		// select items
		if (u_selected_count > 0)
		{
			for (UINT i = 0; i < u_selected_count; i++)
			{
				item = static_cast<int>(p_document->selected_records.GetAt(i));
				m_data_list_ctrl.SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
		m_data_list_ctrl.EnsureVisible(item, FALSE);
	}
	break;

	case HINT_REPLACE_VIEW:
		m_data_list_ctrl.update_cache(-2, -2);
		update_controls();
		break;

	case HINT_REQUERY:
		fill_list_box();
	case HINT_DOC_HAS_CHANGED:
		m_data_list_ctrl.update_cache(-1, -1);
	case HINT_DOC_MOVE_RECORD:
	default:
		update_controls();
		break;
	}
}

void ViewdbWave::delete_records()
{
	// save index current file
	auto current_index = GetDocument()->db_get_current_record_position() - 1;
	if (current_index < 0)
		current_index = 0;

	// loop on C_data_list_ctrl to delete all selected items
	const auto pdb_doc = GetDocument();
	auto pos = m_data_list_ctrl.GetFirstSelectedItemPosition();
	if (pos == nullptr)
	{
		AfxMessageBox(_T("No item selected: delete operation failed"));
		return;
	}
	// assume no one else accesses to the database at the same time
	auto n_files_to_delete = 0;
	while (pos)
	{
		const auto n_item = m_data_list_ctrl.GetNextSelectedItem(pos);
		if (pdb_doc->db_set_current_record_position(n_item - n_files_to_delete))
			pdb_doc->db_delete_current_record();
		n_files_to_delete++;
	}

	BOOL result = pdb_doc->db_set_current_record_position(current_index);
	pdb_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void ViewdbWave::on_lvn_column_click_list_ctrl(NMHDR * p_nmhdr, LRESULT * p_result)
{
	const auto p_nmlv = reinterpret_cast<LPNMLISTVIEW>(p_nmhdr);
	auto filter0 = p_db_table_main->GetSQL();
	CString cs;
	const auto pdb_doc = GetDocument();
	switch (p_nmlv->iSubItem)
	{
	case CTRL_COL_CURVE: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_DATALEN].header_name;
		break; 
	case CTRL_COL_INDEX: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_ID].header_name;
		break; 
	case CTRL_COL_SENSI: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_SENSILLUM_ID].header_name;
		break; 
	case CTRL_COL_STIM1: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_STIM_ID].header_name;
		break; 
	case CTRL_COL_CONC1: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_CONC_ID].header_name;
		break;
	case CTRL_COL_STIM2: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_STIM2_ID].header_name;
		break;
	case CTRL_COL_CONC2: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_CONC2_ID].header_name;
		break;
	case CTRL_COL_NBSPK: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_NSPIKES].header_name;
		break;
	case CTRL_COL_FLAG: cs = pdb_doc->db_table->m_main_table_set.m_desc[CH_FLAG].header_name;
		break;
	default:
		break;
	}
	p_db_table_main->m_strSort = cs;
	p_db_table_main->Requery();
	GetDocument()->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	*p_result = 0;
}

void ViewdbWave::on_bn_clicked_data()
{
	display_data();
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_bn_clicked_display_spikes()
{
	display_spikes();
	m_data_list_ctrl.refresh_display();

	// update tab control
	const int n_rows = m_data_list_ctrl.get_visible_rows_size();
	if (n_rows > 0)
	{
		const auto p_spk_doc = m_data_list_ctrl.get_visible_rows_spike_doc_at(0);
		if (p_spk_doc->get_spike_list_size() > 1)
		{
			spk_list_tab_ctrl.init_ctrl_tab_from_spike_doc(p_spk_doc);
			spk_list_tab_ctrl.ShowWindow(SW_SHOW);
			spk_list_tab_ctrl.SetCurSel(p_spk_doc->get_index_current_spike_list());
			spk_list_tab_ctrl.Invalidate();
		}
	}
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_bn_clicked_display_nothing()
{
	display_nothing();
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_en_change_time_first()
{
	if (!mm_time_first_.m_b_entry_done)
		return;
	mm_time_first_.on_en_change(this, m_time_first_, 1.f, -1.f);

	m_options_view_data_->t_first = m_time_first_;
	if (m_time_first_ > m_time_last_)
		m_time_first_ = 0.f;
	m_data_list_ctrl.set_time_intervals(m_time_first_, m_time_last_);
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_en_change_time_last()
{
	if (!mm_time_last_.m_b_entry_done)
		return;

	mm_time_last_.on_en_change(this, m_time_last_, 1.f, -1.f);
	m_options_view_data_->t_last = m_time_last_;
	m_data_list_ctrl.set_time_intervals(m_time_first_, m_time_last_);
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_en_change_amplitude_span()
{
	if (!mm_amplitude_span_.m_b_entry_done)
		return;

	mm_amplitude_span_.on_en_change(this, m_amplitude_span_, 1.f, -1.f);
	UpdateData(FALSE);
	m_options_view_data_->mv_span = m_amplitude_span_;
	m_data_list_ctrl.set_amplitude_span(m_amplitude_span_);
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_bn_clicked_check_filename()
{
	m_options_view_data_->b_display_file_name = (IsDlgButtonChecked(IDC_CHECKFILENAME) == BST_CHECKED);
	m_data_list_ctrl.set_display_file_name(m_options_view_data_->b_display_file_name);
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_hdn_end_track_list_ctrl(NMHDR * p_nmhdr, LRESULT * p_result)
{
	const auto p_hdr = reinterpret_cast<LPNMHEADER>(p_nmhdr);
	if (p_hdr->iItem == CTRL_COL_CURVE)
		m_data_list_ctrl.resize_signal_column(p_hdr->pitem->cxy);
	*p_result = 0;
}

void ViewdbWave::on_bn_clicked_check2()
{
	m_options_view_data_->b_set_mv_span = (IsDlgButtonChecked(IDC_CHECK2) == BST_CHECKED);
	m_data_list_ctrl.set_amplitude_adjust_mode(m_options_view_data_->b_set_mv_span);
	m_data_list_ctrl.refresh_display();
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(m_options_view_data_->b_set_mv_span);
}

void ViewdbWave::on_bn_clicked_check1()
{
	m_options_view_data_->b_set_time_span = (IsDlgButtonChecked(IDC_CHECK1) == BST_CHECKED);
	// check intervals to avoid crash
	if (m_options_view_data_->b_set_time_span)
	{
		if (m_time_first_ == m_time_last_)
		{
			m_time_last_++;
			m_options_view_data_->t_last = m_time_last_;
			UpdateData(FALSE);
		}
		m_data_list_ctrl.set_time_intervals(m_time_first_, m_time_last_);
	}
	// update data display
	m_data_list_ctrl.set_timespan_adjust_mode(m_options_view_data_->b_set_time_span);
	m_data_list_ctrl.refresh_display();
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(m_options_view_data_->b_set_time_span);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(m_options_view_data_->b_set_time_span);
}

void ViewdbWave::on_bn_clicked_radio_all_classes()
{
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
	m_options_view_data_->b_display_all_classes = TRUE;
	m_data_list_ctrl.set_spike_plot_mode(PLOT_BLACK, m_spike_class_);
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_bn_clicked_radio_one_class()
{
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);
	m_options_view_data_->b_display_all_classes = FALSE;
	m_data_list_ctrl.set_spike_plot_mode(PLOT_ONE_CLASS_ONLY, m_spike_class_);
	m_data_list_ctrl.refresh_display();
}

void ViewdbWave::on_en_change_spike_class()
{
	if (!mm_spike_class_.m_b_entry_done)
		return;

	mm_spike_class_.on_en_change(this, m_spike_class_, 1, -1);
	m_options_view_data_->spike_class = m_spike_class_;
	UpdateData(FALSE);
	m_data_list_ctrl.set_spike_plot_mode(PLOT_ONE_CLASS_ONLY, m_spike_class_);
	m_data_list_ctrl.refresh_display();
}
