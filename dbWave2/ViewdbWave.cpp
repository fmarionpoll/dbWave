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

IMPLEMENT_DYNCREATE(ViewdbWave, dbTableView)

BEGIN_MESSAGE_MAP(ViewdbWave, dbTableView)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_RECORD_PAGEUP, &ViewdbWave::OnRecordPageup)
	ON_COMMAND(ID_RECORD_PAGEDOWN, &ViewdbWave::OnRecordPagedown)
	ON_COMMAND(ID_FILE_PRINT, dbTableView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, dbTableView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, dbTableView::OnFilePrintPreview)
	ON_BN_CLICKED(IDC_DISPLAYDATA, &ViewdbWave::OnBnClickedData)
	ON_BN_CLICKED(IDC_DISPLAYNOTHING, &ViewdbWave::OnBnClickedDisplayNothing)
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewdbWave::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewdbWave::OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_AMPLITUDESPAN, &ViewdbWave::OnEnChangeAmplitudespan)
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewdbWave::OnEnChangeSpikeclass)
	ON_BN_CLICKED(IDC_CHECKFILENAME, &ViewdbWave::OnBnClickedCheckfilename)
	ON_BN_CLICKED(IDC_FILTERCHECK, &ViewdbWave::OnClickMedianFilter)
	ON_BN_CLICKED(IDC_CHECK2, &ViewdbWave::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK1, &ViewdbWave::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_RADIOALLCLASSES, &ViewdbWave::OnBnClickedRadioallclasses)
	ON_BN_CLICKED(IDC_RADIOONECLASS, &ViewdbWave::OnBnClickedRadiooneclass)
	ON_BN_CLICKED(IDC_DISPLAYSPIKES, &ViewdbWave::OnBnClickedDisplaySpikes)
	ON_NOTIFY(HDN_ENDTRACK, 0, &ViewdbWave::OnHdnEndtrackListctrl)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LISTCTRL, &ViewdbWave::OnLvnColumnclickListctrl)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LISTCTRL, &ViewdbWave::OnItemActivateListctrl)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTCTRL, &ViewdbWave::OnDblclkListctrl)

	//ON_NOTIFY(NM_CLICK, IDC_TAB1, &dbTableView::OnNMClickTab1)
END_MESSAGE_MAP()

ViewdbWave::ViewdbWave() : dbTableView(IDD)
{
}

ViewdbWave::~ViewdbWave()
= default;

void ViewdbWave::DoDataExchange(CDataExchange * pDX)
{
	dbTableView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_AMPLITUDESPAN, m_amplitudespan);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikeclass);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

void ViewdbWave::OnInitialUpdate()
{
	// init document and dbTableView
	const auto dbwavedoc = GetDocument();
	m_pSet = &dbwavedoc->db_table->m_mainTableSet;
	dbTableView::OnInitialUpdate();

	subclass_dialog_controls();
	make_controls_stretchable();

	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	m_options_viewdata = &p_app->options_view_data;
	m_dataListCtrl.init_columns(&m_options_viewdata->icolwidth);

	// set how data are displayed
	m_amplitudespan = m_options_viewdata->mVspan;
	m_dataListCtrl.set_amplitude_span(m_options_viewdata->mVspan);

	CheckDlgButton(IDC_CHECKFILENAME, m_options_viewdata->bDisplayFileName);
	m_dataListCtrl.set_display_file_name(m_options_viewdata->bDisplayFileName);

	m_timefirst = m_options_viewdata->tFirst;
	m_timelast = m_options_viewdata->tLast;
	if (m_timefirst != 0.f && m_timelast != 0.f)
		m_dataListCtrl.set_time_intervals(m_timefirst, m_timelast);

	CheckDlgButton(IDC_CHECK1, m_options_viewdata->bsetTimeSpan);
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
	m_dataListCtrl.set_timespan_adjust_mode(m_options_viewdata->bsetTimeSpan);

	CheckDlgButton(IDC_CHECK2, m_options_viewdata->bsetmVSpan);
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(m_options_viewdata->bsetmVSpan);
	m_dataListCtrl.set_amplitude_adjust_mode(m_options_viewdata->bsetmVSpan);

	m_dataListCtrl.SetExtendedStyle
	(m_dataListCtrl.GetExtendedStyle()
		| LVS_EX_FULLROWSELECT
		| LVS_EX_GRIDLINES
		| LVS_EX_ONECLICKACTIVATE
		| LVS_EX_SUBITEMIMAGES);

	// set display mode of m_dataListCtrl
	m_dataListCtrl.set_display_mode(m_options_viewdata->displaymode);
	switch (m_options_viewdata->displaymode)
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
	fillListBox();
	m_dataListCtrl.update_cache(-3, -3);
	updateControls();

	// init display controls
	if (m_options_viewdata->displaymode == 2)
	{
		CSpikeDoc* p_spk_doc = GetDocument()->get_current_spike_file();
		if (p_spk_doc != nullptr)
		{
			m_tabCtrl.InitctrlTabFromSpikeDoc(p_spk_doc);
			m_tabCtrl.SetCurSel(p_spk_doc->get_spike_list_current_index());
		}
		m_tabCtrl.ShowWindow(p_spk_doc != nullptr ? SW_SHOW : SW_HIDE);
	}
}

void ViewdbWave::subclass_dialog_controls()
{
	VERIFY(m_dataListCtrl.SubclassDlgItem(IDC_LISTCTRL, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_amplitudespan.SubclassDlgItem(IDC_AMPLITUDESPAN, this));
	VERIFY(mm_spikeclass.SubclassDlgItem(IDC_SPIKECLASS, this));
}

void ViewdbWave::make_controls_stretchable()
{
	m_stretch_.AttachParent(this);
	m_stretch_.newProp(IDC_LISTCTRL, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch_.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	m_b_init_ = TRUE;
}

void ViewdbWave::display_data()
{
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYDATA))->SetCheck(BST_CHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYSPIKES))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYNOTHING))->SetCheck(BST_UNCHECKED);

	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(TRUE);
	m_options_viewdata->displaymode = 1;
	m_dataListCtrl.set_display_mode(m_options_viewdata->displaymode);

	static_cast<CButton*>(GetDlgItem(IDC_FILTERCHECK))->SetCheck(m_options_viewdata->bFilterDat);
	m_dattransform = m_options_viewdata->bFilterDat ? 13 : 0;
	m_dataListCtrl.set_transform_mode(m_dattransform);
	m_tabCtrl.ShowWindow(SW_HIDE);
}

void ViewdbWave::display_spikes()
{
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYSPIKES))->SetCheck(BST_CHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYDATA))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYNOTHING))->SetCheck(BST_UNCHECKED);

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(TRUE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);

	// display all spike classes
	m_options_viewdata->displaymode = 2;
	m_dataListCtrl.set_display_mode(m_options_viewdata->displaymode);
	if (m_options_viewdata->bDisplayAllClasses)
	{
		static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_CHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_UNCHECKED);
		m_dataListCtrl.set_spike_plot_mode(PLOT_BLACK, m_spikeclass);
	}
	else
	{
		static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(BST_UNCHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_RADIOONECLASS))->SetCheck(BST_CHECKED);
		m_spikeclass = m_options_viewdata->spikeclass;
		mm_spikeclass.EnableWindow(TRUE);
		m_dataListCtrl.set_spike_plot_mode(PLOT_ONE_CLASS_ONLY, m_spikeclass);
	}
}

void ViewdbWave::display_nothing()
{
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYDATA))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYSPIKES))->SetCheck(BST_UNCHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_DISPLAYNOTHING))->SetCheck(BST_CHECKED);

	GetDlgItem(IDC_FILTERCHECK)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOALLCLASSES)->EnableWindow(FALSE);
	GetDlgItem(IDC_RADIOONECLASS)->EnableWindow(FALSE);
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);

	m_options_viewdata->displaymode = 0;
	m_dataListCtrl.set_display_mode(m_options_viewdata->displaymode);
	m_tabCtrl.ShowWindow(SW_HIDE);
}

void ViewdbWave::OnSize(const UINT n_type, const int cx, const int cy)
{
	dbTableView::OnSize(n_type, cx, cy);
	if (IsWindow(m_dataListCtrl.m_hWnd))
	{
		CRect rect;
		m_dataListCtrl.GetClientRect(&rect);
		m_dataListCtrl.fit_columns_to_size(rect.Width());
	}
}

void ViewdbWave::updateControls()
{
	const auto dbWaveDoc = GetDocument();
	CFileStatus status;

	m_bvalidDat = CFile::GetStatus(dbWaveDoc->db_get_current_dat_file_name(), status);
	m_bvalidSpk = CFile::GetStatus(dbWaveDoc->db_get_current_spk_file_name(TRUE), status);

	const int i_file = dbWaveDoc->db_get_current_record_position();

	m_dataListCtrl.set_cur_sel(i_file);
	m_dataListCtrl.EnsureVisible(i_file, FALSE);

	if (m_options_viewdata->displaymode == 2)
	{
		CSpikeDoc* p_spk_doc = GetDocument()->open_current_spike_file();
		if (p_spk_doc != nullptr)
		{
			const auto spklist_size = p_spk_doc->get_spike_list_size();
			if (m_tabCtrl.GetItemCount() < spklist_size)
				m_tabCtrl.InitctrlTabFromSpikeDoc(p_spk_doc);
		}
	}
	//pdb_doc->SetModifiedFlag(true);
	//pdb_doc->UpdateAllViews(this, HINT_DOCMOVERECORD, nullptr);
	dbWaveDoc->update_all_views_db_wave(this, HINT_DOCMOVERECORD, nullptr);

	//POSITION pos = pdb_doc->GetFirstViewPosition();
	//int nviews = 0;
	//while (pos != NULL)
	//{
	//	CView* pView = pdb_doc->GetNextView(pos);
	//	CMainFrame* frame = (CMainFrame*)pView->GetParentFrame();
	//	frame->OnUpdate(this, HINT_DOCMOVERECORD, nullptr);

	//	//pView->UpdateWindow();
	//	nviews++;
	//}
	//TRACE("nviews", nviews);
}

void ViewdbWave::OnRecordPageup()
{
	m_dataListCtrl.SendMessage(WM_VSCROLL, SB_PAGEUP, NULL);
}

void ViewdbWave::OnRecordPagedown()
{
	m_dataListCtrl.SendMessage(WM_VSCROLL, SB_PAGEDOWN, NULL);
}

void ViewdbWave::OnClickMedianFilter()
{
	if (m_options_viewdata->bFilterDat == static_cast<CButton*>(GetDlgItem(IDC_FILTERCHECK))->GetCheck())
		return;

	m_options_viewdata->bFilterDat = static_cast<CButton*>(GetDlgItem(IDC_FILTERCHECK))->GetCheck();
	if (m_options_viewdata->bFilterDat)
		m_dattransform = 13;
	else
		m_dattransform = 0;
	m_dataListCtrl.set_transform_mode(m_dattransform);
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnActivateView(BOOL bActivate, CView * pActivateView, CView * pDeactiveView)
{
	auto* p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
	if (bActivate)
	{
		// make sure the secondary toolbar is not visible (none is defined for the database)
		if (p_mainframe->m_pSecondToolBar != nullptr)
			p_mainframe->ShowPane(p_mainframe->m_pSecondToolBar, FALSE, FALSE, TRUE);
		// load status
		m_nStatus = static_cast<CChildFrame*>(p_mainframe->MDIGetActive())->m_n_status;
		p_mainframe->PostMessageW(WM_MYMESSAGE, HINT_ACTIVATEVIEW, reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		ChartData* pDataChartWnd = m_dataListCtrl.get_chart_data_of_current_record();
		if (pDataChartWnd != nullptr)
		{
			static_cast<CdbWaveApp*>(AfxGetApp())->options_view_data.viewdata = *(pDataChartWnd->get_scope_parameters());
		}
		if (pActivateView != nullptr)
			static_cast<CChildFrame*>(p_mainframe->MDIGetActive())->m_n_status = m_nStatus;
	}
	dbTableView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void ViewdbWave::fillListBox()
{
	m_dataListCtrl.DeleteAllItems();
	const int n_records = GetDocument()->db_get_n_records();
	m_dataListCtrl.SetItemCountEx(n_records);
}

void ViewdbWave::OnItemActivateListctrl(NMHDR * pNMHDR, LRESULT * pResult)
{
	// get item clicked and select it
	const auto p_item_activate = reinterpret_cast<NMITEMACTIVATE*>(pNMHDR);
	if (p_item_activate->iItem >= 0) 
		GetDocument()->db_set_current_record_position(p_item_activate->iItem);
	
	dbTableView::OnInitialUpdate();
	GetDocument()->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);

	*pResult = 0;
}

void ViewdbWave::OnDblclkListctrl(NMHDR * pNMHDR, LRESULT * pResult)
{
	*pResult = 0;
	// quit the current view and switch to spike detection view
	GetParent()->PostMessage(WM_COMMAND, static_cast<WPARAM>(ID_VIEW_SPIKEDETECTION), static_cast<LPARAM>(NULL));
}

LRESULT ViewdbWave::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	const int threshold = LOWORD(lParam); // value associated
	//const int i_id = HIWORD(lParam);

	switch (wParam)
	{
	case HINT_VIEWTABHASCHANGED:
		GetDocument()->get_current_spike_file()->set_spike_list_current_index(threshold);
		m_dataListCtrl.refresh_display();
		break;

	default:
		break;
	}
	return 0L;
}

void ViewdbWave::OnUpdate(CView * pSender, LPARAM lHint, CObject * pHint)
{
	if (!m_b_init_ )
		return;

	switch (LOWORD(lHint))
	{
	case HINT_GETSELECTEDRECORDS:
	{
		const auto p_document = GetDocument();
		p_document->selected_records.RemoveAll();
		const int selected_count = static_cast<int>(m_dataListCtrl.GetSelectedCount());

		// Update all of the selected items.
		if (selected_count > 0)
		{
			p_document->selected_records.SetSize(selected_count);
			auto n_item = -1;
			for (int i = 0; i < selected_count; i++)
			{
				n_item = m_dataListCtrl.GetNextItem(n_item, LVNI_SELECTED);
				ASSERT(n_item != -1);
				p_document->selected_records.SetAt(i, n_item);
			}
		}
	}
	break;

	case HINT_SETSELECTEDRECORDS:
	{
		const auto p_document = GetDocument();
		const UINT u_selected_count = p_document->selected_records.GetSize();

		// clear previous selection in the CListCtrl if any
		auto item = -1;
		item = m_dataListCtrl.GetNextItem(item, LVNI_SELECTED);
		while (item != -1)
		{
			m_dataListCtrl.SetItemState(item, 0, LVIS_SELECTED);
			item = m_dataListCtrl.GetNextItem(item, LVNI_SELECTED);
		}

		// select items
		if (u_selected_count > 0)
		{
			for (UINT i = 0; i < u_selected_count; i++)
			{
				item = static_cast<int>(p_document->selected_records.GetAt(i));
				m_dataListCtrl.SetItemState(item, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
		m_dataListCtrl.EnsureVisible(item, FALSE);
	}
	break;

	case HINT_REPLACEVIEW:
		m_dataListCtrl.update_cache(-2, -2);
		updateControls();
		break;

	case HINT_REQUERY:
		fillListBox();
	case HINT_DOCHASCHANGED:
		m_dataListCtrl.update_cache(-1, -1);
	case HINT_DOCMOVERECORD:
	default:
		updateControls();
		break;
	}
}

void ViewdbWave::DeleteRecords()
{
	// save index current file
	auto current_index = GetDocument()->db_get_current_record_position() - 1;
	if (current_index < 0)
		current_index = 0;

	// loop on Cdatalistctrl to delete all selected items
	const auto pdb_doc = GetDocument();
	auto pos = m_dataListCtrl.GetFirstSelectedItemPosition();
	if (pos == nullptr)
	{
		AfxMessageBox(_T("No item selected: delete operation failed"));
		return;
	}
	// assume no one else accesses to the database at the same time
	auto n_files_to_delete = 0;
	while (pos)
	{
		const auto n_item = m_dataListCtrl.GetNextSelectedItem(pos);
		pdb_doc->db_set_current_record_position(n_item - n_files_to_delete);
		pdb_doc->db_delete_current_record();
		n_files_to_delete++;
	}

	pdb_doc->db_set_current_record_position(current_index);
	pdb_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void ViewdbWave::OnLvnColumnclickListctrl(NMHDR * pNMHDR, LRESULT * pResult)
{
	const auto pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	auto filter0 = m_pSet->GetSQL();
	CString cs;
	const auto pdb_doc = GetDocument();
	switch (pNMLV->iSubItem)
	{
	case CTRL_COL_CURVE: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_DATALEN].header_name;
		break; // datalen
	case CTRL_COL_INDEX: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_ID].header_name;
		break; // ID
	case CTRL_COL_SENSI: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_SENSILLUM_ID].header_name;
		break; // sensillum_ID
	case CTRL_COL_STIM1: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_STIM_ID].header_name;
		break; // stim_ID
	case CTRL_COL_CONC1: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_CONC_ID].header_name;
		break; // conc_ID
	case CTRL_COL_STIM2: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_STIM2_ID].header_name;
		break; // stim2_ID
	case CTRL_COL_CONC2: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_CONC2_ID].header_name;
		break; // conc2_ID
	case CTRL_COL_NBSPK: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_NSPIKES].header_name;
		break; // nspikes
	case CTRL_COL_FLAG: cs = pdb_doc->db_table->m_mainTableSet.m_desc[CH_FLAG].header_name;
		break; // flag
	default:
		break;
	}
	m_pSet->m_strSort = cs;
	m_pSet->Requery();
	GetDocument()->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	*pResult = 0;
}

void ViewdbWave::OnBnClickedData()
{
	display_data();
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnBnClickedDisplaySpikes()
{
	display_spikes();
	m_dataListCtrl.refresh_display();

	// update tab control
	const int nrows = m_dataListCtrl.get_visible_rows_size();
	if (nrows > 0)
	{
		const auto pSpkDoc = m_dataListCtrl.get_visible_rows_spike_doc_at(0);
		if (pSpkDoc->get_spike_list_size() > 1)
		{
			m_tabCtrl.InitctrlTabFromSpikeDoc(pSpkDoc);
			m_tabCtrl.ShowWindow(SW_SHOW);
			m_tabCtrl.SetCurSel(pSpkDoc->get_spike_list_current_index());
			m_tabCtrl.Invalidate();
		}
	}
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnBnClickedDisplayNothing()
{
	display_nothing();
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnEnChangeTimefirst()
{
	if (!mm_timefirst.m_bEntryDone)
		return;
	mm_timefirst.OnEnChange(this, m_timefirst, 1.f, -1.f);

	m_options_viewdata->tFirst = m_timefirst;
	if (m_timefirst > m_timelast)
		m_timefirst = 0.f;
	m_dataListCtrl.set_time_intervals(m_timefirst, m_timelast);
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnEnChangeTimelast()
{
	if (!mm_timelast.m_bEntryDone)
		return;

	mm_timelast.OnEnChange(this, m_timelast, 1.f, -1.f);
	m_options_viewdata->tLast = m_timelast;
	m_dataListCtrl.set_time_intervals(m_timefirst, m_timelast);
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnEnChangeAmplitudespan()
{
	if (!mm_amplitudespan.m_bEntryDone)
		return;

	mm_amplitudespan.OnEnChange(this, m_amplitudespan, 1.f, -1.f);
	UpdateData(FALSE);
	m_options_viewdata->mVspan = m_amplitudespan;
	m_dataListCtrl.set_amplitude_span(m_amplitudespan);
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnBnClickedCheckfilename()
{
	m_options_viewdata->bDisplayFileName = IsDlgButtonChecked(IDC_CHECKFILENAME);
	m_dataListCtrl.set_display_file_name(m_options_viewdata->bDisplayFileName);
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnHdnEndtrackListctrl(NMHDR * pNMHDR, LRESULT * pResult)
{
	const auto phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	if (phdr->iItem == CTRL_COL_CURVE)
		m_dataListCtrl.resize_signal_column(phdr->pitem->cxy);
	*pResult = 0;
}

void ViewdbWave::OnBnClickedCheck2()
{
	m_options_viewdata->bsetmVSpan = IsDlgButtonChecked(IDC_CHECK2);
	m_dataListCtrl.set_amplitude_adjust_mode(m_options_viewdata->bsetmVSpan);
	m_dataListCtrl.refresh_display();
	GetDlgItem(IDC_AMPLITUDESPAN)->EnableWindow(m_options_viewdata->bsetmVSpan);
}

void ViewdbWave::OnBnClickedCheck1()
{
	m_options_viewdata->bsetTimeSpan = IsDlgButtonChecked(IDC_CHECK1);
	// check intervals to avoid crash
	if (m_options_viewdata->bsetTimeSpan)
	{
		if (m_timefirst == m_timelast)
		{
			m_timelast++;
			m_options_viewdata->tLast = m_timelast;
			UpdateData(FALSE);
		}
		m_dataListCtrl.set_time_intervals(m_timefirst, m_timelast);
	}
	// update data display
	m_dataListCtrl.set_timespan_adjust_mode(m_options_viewdata->bsetTimeSpan);
	m_dataListCtrl.refresh_display();
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(m_options_viewdata->bsetTimeSpan);
}

void ViewdbWave::OnBnClickedRadioallclasses()
{
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
	m_options_viewdata->bDisplayAllClasses = TRUE;
	m_dataListCtrl.set_spike_plot_mode(PLOT_BLACK, m_spikeclass);
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnBnClickedRadiooneclass()
{
	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);
	m_options_viewdata->bDisplayAllClasses = FALSE;
	m_dataListCtrl.set_spike_plot_mode(PLOT_ONE_CLASS_ONLY, m_spikeclass);
	m_dataListCtrl.refresh_display();
}

void ViewdbWave::OnEnChangeSpikeclass()
{
	if (!mm_spikeclass.m_bEntryDone)
		return;

	mm_spikeclass.OnEnChange(this, m_spikeclass, 1, -1);
	m_options_viewdata->spikeclass = m_spikeclass;
	UpdateData(FALSE);
	m_dataListCtrl.set_spike_plot_mode(PLOT_ONE_CLASS_ONLY, m_spikeclass);
	m_dataListCtrl.refresh_display();
}
