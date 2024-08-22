#include "StdAfx.h"
#include "dbWave.h"
#include "ChartWnd.h"
#include "Editctrl.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "MainFrm.h"
#include "ViewSpikeHist.h"

#include "DlgFormatHistogram.h"
#include "DlgProgress.h"
#include "StretchControls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(ViewSpikeHist, ViewDbTable)

ViewSpikeHist::ViewSpikeHist()
	: ViewDbTable(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

ViewSpikeHist::~ViewSpikeHist()
{
	delete[] m_pPSTH;
	delete[] m_pISI;
	delete[] m_parrayISI;
	delete m_pbitmap;
	m_fontDisp.DeleteObject();
}

BOOL ViewSpikeHist::PreCreateWindow(CREATESTRUCT& cs)
{
	return ViewDbTable::PreCreateWindow(cs);
}

void ViewSpikeHist::DoDataExchange(CDataExchange* pDX)
{
	ViewDbTable::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikeclass);
	DDX_Text(pDX, IDC_EDIT2, m_dotheight);
	DDX_Text(pDX, IDC_EDIT3, m_rowheight);
	DDX_Text(pDX, IDC_BINSIZE, m_binISIms);
	DDX_Text(pDX, IDC_EDIT1, m_nbinsISI);
	DDX_Text(pDX, IDC_EDIT4, m_timebinms);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

BEGIN_MESSAGE_MAP(ViewSpikeHist, ViewDbTable)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_HSCROLL()

	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikeHist::on_en_change_time_first)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikeHist::on_en_change_time_last)
	ON_EN_CHANGE(IDC_EDIT4, &ViewSpikeHist::on_en_change_time_bin)
	ON_EN_CHANGE(IDC_BINSIZE, &ViewSpikeHist::on_en_change_bin_isi)
	ON_BN_CLICKED(IDC_CHECK1, &ViewSpikeHist::on_click_all_files)
	ON_BN_CLICKED(IDC_RADIOABSOLUTE, &ViewSpikeHist::on_absolute_time)
	ON_BN_CLICKED(IDC_RADIORELATIVE, &ViewSpikeHist::on_relative_time)
	ON_BN_CLICKED(IDC_RADIOONECLASS, &ViewSpikeHist::on_click_one_class)
	ON_BN_CLICKED(IDC_RADIOALLCLASSES, &ViewSpikeHist::on_click_all_classes)
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewSpikeHist::on_en_change_spike_class)
	ON_EN_CHANGE(IDC_EDIT1, &ViewSpikeHist::on_en_change_n_bins)
	ON_EN_CHANGE(IDC_EDIT3, &ViewSpikeHist::on_en_change_row_height)
	ON_EN_CHANGE(IDC_EDIT2, &ViewSpikeHist::on_en_change_dot_height)
	ON_COMMAND(ID_FORMAT_HISTOGRAM, &ViewSpikeHist::on_format_histogram)
	ON_BN_CLICKED(IDC_CHECK2, &ViewSpikeHist::on_click_cycle_hist)
	ON_COMMAND(ID_EDIT_COPY, &ViewSpikeHist::on_edit_copy)
	ON_LBN_SELCHANGE(IDC_LIST1, &ViewSpikeHist::OnSelchangeHistogramtype)
	ON_EN_CHANGE(IDC_EDITNSTIPERCYCLE, &ViewSpikeHist::OnEnChangeEditnstipercycle)
	ON_EN_CHANGE(IDC_EDITLOCKONSTIM, &ViewSpikeHist::OnEnChangeEditlockonstim)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &ViewSpikeHist::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &ViewSpikeHist::OnTcnSelchangeTab1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ViewSpikeHist message handlers

void ViewSpikeHist::OnInitialUpdate()
{
	VERIFY(mm_binISIms.SubclassDlgItem(IDC_BINSIZE, this));
	VERIFY(mm_timebinms.SubclassDlgItem(IDC_EDIT4, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_spikeclass.SubclassDlgItem(IDC_SPIKECLASS, this));
	VERIFY(mm_nbinsISI.SubclassDlgItem(IDC_EDIT1, this));
	VERIFY(mm_dotheight.SubclassDlgItem(IDC_EDIT2, this));
	VERIFY(mm_rowheight.SubclassDlgItem(IDC_EDIT3, this));

	static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100);
	static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1))->SetScrollPos(50);

	// load stored parameters
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	m_pvdS = &(p_app->options_view_spikes); // get address of spike display options
	mdPM = &(p_app->options_view_data); // printing options

	// create local fonts
	memset(&m_logFontDisp, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_logFontDisp.lfFaceName, _T("Arial")); // Arial font
	m_logFontDisp.lfHeight = 15; // font height
	/*BOOL flag = */
	m_fontDisp.CreateFontIndirect(&m_logFontDisp);

	// fill controls with initial values
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_pvdS->ballfiles);
	if (m_pvdS->ballfiles)
		m_nfiles = GetDocument()->db_get_records_count();
	else
		m_nfiles = 1;
	SetDlgItemInt(IDC_EDITNSTIPERCYCLE, m_pvdS->nstipercycle);
	SetDlgItemInt(IDC_EDITLOCKONSTIM, m_pvdS->istimulusindex);
	static_cast<CButton*>(GetDlgItem(IDC_CHECK2))->SetCheck(m_pvdS->bCycleHist);
	m_timefirst = m_pvdS->timestart;
	m_timelast = m_pvdS->timeend;
	static_cast<CButton*>(GetDlgItem(IDC_RADIOABSOLUTE))->SetCheck(m_pvdS->babsolutetime);
	static_cast<CButton*>(GetDlgItem(IDC_RADIORELATIVE))->SetCheck(!m_pvdS->babsolutetime);
	static_cast<CButton*>(GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(!m_pvdS->spikeclassoption);
	static_cast<CButton*>(GetDlgItem(IDC_RADIOONECLASS))->SetCheck(m_pvdS->spikeclassoption);
	m_spikeclass = m_pvdS->classnb;

	static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->AddString(_T("Peristimulus histogram (PS)"));
	static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->AddString(_T("Spike intervals histogram (ISI)"));
	static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->AddString(_T("Autocorrelation histogram (Autoc)"));
	static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->AddString(_T("Raster display"));
	static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->AddString(_T("Peristimulus-Autocorrelation (PS-Autoc)"));

	GetDlgItem(IDC_SPIKECLASS)->EnableWindow(m_pvdS->spikeclassoption);
	m_timebinms = m_pvdS->timebin * t1000;
	m_dotheight = m_pvdS->dotheight;
	m_rowheight = m_pvdS->dotlineheight - m_pvdS->dotheight;

	// init coordinates of display area
	CRect rect0, rect1;
	GetWindowRect(&rect0);
	const auto p_wnd = GetDlgItem(IDC_STATIC12); // get pointer to display area
	p_wnd->GetWindowRect(&rect1);
	m_topleft.x = rect1.left - rect0.left + 1;
	m_topleft.y = rect1.top - rect0.top + 1;
	m_initiated = TRUE;
	show_controls(m_pvdS->bhistType);

	// attach controls to stretch
	m_stretch.AttachParent(this); // attach form_view pointer
	m_stretch.newProp(IDC_STATIC12, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	m_binit = TRUE;

	// init database and load documents
	ViewDbTable::OnInitialUpdate();

	const auto p_dbwave_doc = GetDocument();
	if (p_dbwave_doc->m_p_spk_doc == nullptr)
	{
		p_dbwave_doc->m_p_spk_doc = new CSpikeDoc;
		ASSERT(p_dbwave_doc->m_p_spk_doc != NULL);
	}
	p_spike_doc_ = p_dbwave_doc->m_p_spk_doc;
	p_spike_doc_->get_spike_list_current();
	build_data_and_display();
	select_spk_list(p_spike_doc_->get_spike_list_current_index(), TRUE);
}

void ViewSpikeHist::OnSize(const UINT n_type, const int cx, const int cy)
{
	if (m_binit)
	{
		switch (n_type)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx <= 0 || cy <= 0)
				break;
			m_stretch.ResizeControls(n_type, cx, cy);
			break;
		default:
			break;
		}
	}
	ViewDbTable::OnSize(n_type, cx, cy);
}

void ViewSpikeHist::OnActivateView(const BOOL b_activate, CView* p_activate_view, CView* p_deactive_view)
{
	if (b_activate)
	{
		auto p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATE_VIEW,
		                         reinterpret_cast<LPARAM>(p_activate_view->GetDocument()));
	}
	else
	{
		auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		p_app->options_view_spikes.ballfiles = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	}
	ViewDbTable::OnActivateView(b_activate, p_activate_view, p_deactive_view);
}

void ViewSpikeHist::OnUpdate(CView* p_sender, const LPARAM l_hint, CObject* p_hint)
{
	if (!m_binit)
		return;

	switch (LOWORD(l_hint))
	{
	case HINT_REQUERY:
		if (m_pvdS->ballfiles)
			m_nfiles = GetDocument()->db_get_records_count();
		else
			m_nfiles = 1;
		break;
	case HINT_DOC_HAS_CHANGED:
	case HINT_DOC_MOVE_RECORD:
		select_spk_list(GetDocument()->get_current_spike_file()->get_spike_list_current_index(), TRUE);
		build_data_and_display();
		break;

	default:
		break;
	}
}

BOOL ViewSpikeHist::OnMove(const UINT n_id_move_command)
{
	const auto flag = ViewDbTable::OnMove(n_id_move_command);
	const auto p_document = GetDocument();
	if (p_document->db_get_current_spk_file_name(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKE_DETECTION, NULL);
		return false;
	}

	p_document->update_all_views_db_wave(nullptr, HINT_DOC_MOVE_RECORD, nullptr);
	if (!m_pvdS->ballfiles)
		build_data_and_display();
	select_spk_list(GetDocument()->get_current_spike_file()->get_spike_list_current_index(), TRUE);
	return flag;
}

void ViewSpikeHist::OnDestroy()
{
	ViewDbTable::OnDestroy();
}

#ifdef _DEBUG
void ViewSpikeHist::AssertValid() const
{
	ViewDbTable::AssertValid();
}

void ViewSpikeHist::Dump(CDumpContext& dc) const
{
	ViewDbTable::Dump(dc);
}
#endif //_DEBUG


void ViewSpikeHist::on_en_change_time_first()
{
	if (mm_timefirst.m_bEntryDone)
	{
		const auto time_first = m_timefirst;
		mm_timefirst.OnEnChange(this, m_timefirst, 1.f, -1.f);

		if (m_timefirst > m_timelast)
		{
			m_timefirst = time_first;
			MessageBeep(-1);
		}
		m_pvdS->timestart = m_timefirst;
		UpdateData(FALSE);
		if (time_first != m_timefirst)
			build_data_and_display();
	}
}

void ViewSpikeHist::on_en_change_time_last()
{
	if (mm_timelast.m_bEntryDone)
	{
		const auto time_last = m_timelast;
		mm_timelast.OnEnChange(this, m_timelast, 1.f, -1.f);
		if (m_timelast < m_timefirst)
		{
			m_timelast = time_last;
			MessageBeep(-1);
		}
		m_pvdS->timeend = m_timelast;
		UpdateData(FALSE);
		if (time_last != m_timelast)
			build_data_and_display();
	}
}

void ViewSpikeHist::on_en_change_time_bin()
{
	if (mm_timebinms.m_bEntryDone)
	{
		const auto bin_ms = m_timebinms;
		mm_timebinms.OnEnChange(this, m_timebinms, 1.f, -1.f);
		m_pvdS->timebin = m_timebinms / t1000;
		UpdateData(FALSE);
		if (bin_ms != m_timebinms)
			build_data_and_display();
	}
}

void ViewSpikeHist::on_en_change_bin_isi()
{
	if (mm_binISIms.m_bEntryDone)
	{
		const auto bin_ms = m_binISIms;
		mm_binISIms.OnEnChange(this, m_binISIms, 1.f, -1.f);
		m_pvdS->binISI = m_binISIms / t1000;
		UpdateData(FALSE);
		if (bin_ms != m_binISIms)
			build_data_and_display();
	}
}

void ViewSpikeHist::on_en_change_spike_class()
{
	if (mm_spikeclass.m_bEntryDone)
	{
		const auto spike_class_option = m_spikeclass;
		mm_spikeclass.OnEnChange(this, m_spikeclass, 1, -1);
		m_pvdS->classnb = m_spikeclass;
		UpdateData(FALSE);
		if (spike_class_option != m_spikeclass)
			build_data_and_display();
	}
}

void ViewSpikeHist::build_data_and_display()
{
	if (!m_binit)
		return;
	build_data(); 
	m_bmodified = TRUE; 
	InvalidateRect(&m_displayRect); 
}

void ViewSpikeHist::OnDraw(CDC* p_dc)
{
	CRect rect;
	const auto p_wnd = GetDlgItem(IDC_STATIC12); 
	p_wnd->GetClientRect(&rect);

	CDC dc_mem; 
	dc_mem.CreateCompatibleDC(p_dc);

	// adjust size of the bitmap (eventually)
	if (m_displayRect.Width() != rect.right || m_displayRect.Height() != rect.bottom)
	{
		delete m_pbitmap;
		m_pbitmap = new CBitmap;
		ASSERT(m_pbitmap != NULL);
		m_displayRect = rect;
		m_displayRect.OffsetRect(m_topleft);
		m_pbitmap->CreateBitmap(rect.Width(), rect.Height(),
		                        p_dc->GetDeviceCaps(PLANES),
		                        p_dc->GetDeviceCaps(BITSPIXEL),
		                        nullptr);
		m_bmodified = TRUE;
	}

	// select bitmap into device context
	const auto p_old_bitmap = dc_mem.SelectObject(m_pbitmap);

	if (m_bmodified) 
	{
		const auto saved_dc = dc_mem.SaveDC(); // save DC
		dc_mem.Rectangle(rect); // erase window background
		CFont* p_old_font = nullptr;
		if (!m_bPrint)
			p_old_font = dc_mem.SelectObject(&m_fontDisp);

		// call display routine according to selection
		switch (m_bhistType)
		{
		case 0:
		case 1:
		case 2:
			display_histogram(&dc_mem, &rect);
			break;
		case 3:
			display_dot(&dc_mem, &rect);
			break;
		case 4:
			display_psth_autocorrelation(&dc_mem, &rect);
			break;
		default:
			break;
		}

		// restore parameters
		if (p_old_font != nullptr)
			dc_mem.SelectObject(p_old_font);

		dc_mem.RestoreDC(saved_dc); // restore DC
		p_dc->SetMapMode(MM_TEXT); // reset mapping mode to text
		p_dc->SetWindowOrg(0, 0);
		p_dc->SetViewportOrg(0, 0);
		m_bmodified = FALSE; // set flag to FALSE (job done)
	}

	// transfer to the screen
	p_dc->BitBlt(m_topleft.x,
	             m_topleft.y,
	             rect.right,
	             rect.bottom,
	             &dc_mem,
	             0,
	             0,
	             SRCCOPY);
	dc_mem.SelectObject(p_old_bitmap); // release bitmap
}

void ViewSpikeHist::get_file_infos(CString& str_comment)
{
	if (m_nfiles == 1)
	{
		const CString tab("    "); // use 4 spaces as tabulation character
		const CString rc("\n"); 
		if (m_bPrint)
		{
			if (mdPM->b_doc_name || mdPM->b_acq_date_time) // print doc infos?
			{
				if (mdPM->b_doc_name) // print file name
				{
					const auto filename = GetDocument()->db_get_current_spk_file_name(FALSE);
					str_comment += filename + tab;
				}
				if (mdPM->b_acq_date_time) // print data acquisition date & time
				{
					const auto date = (p_spike_doc_->get_acq_time()).Format("%#d %m %Y %X"); //("%c");
					str_comment += date;
				}
				str_comment += rc;
			}
		}
	}
}

void ViewSpikeHist::on_click_all_files()
{
	if (static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck())
	{
		m_pvdS->ballfiles = TRUE;
		m_nfiles = GetDocument()->db_get_records_count();
	}
	else
	{
		m_pvdS->ballfiles = FALSE;
		m_nfiles = 1;
	}
	build_data_and_display();
}

void ViewSpikeHist::on_click_cycle_hist()
{
	if (static_cast<CButton*>(GetDlgItem(IDC_CHECK2))->GetCheck())
	{
		m_pvdS->bCycleHist = TRUE;
		m_pvdS->nstipercycle = static_cast<int>(GetDlgItemInt(IDC_EDITNSTIPERCYCLE));
		m_pvdS->istimulusindex = static_cast<int>(GetDlgItemInt(IDC_EDITLOCKONSTIM));
	}
	else
		m_pvdS->bCycleHist = FALSE;
	build_data_and_display();
}

void ViewSpikeHist::on_click_one_class()
{
	if (!m_pvdS->spikeclassoption)
	{
		m_pvdS->spikeclassoption = TRUE;
		(CWnd*)GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);
		build_data_and_display();
	}
}

void ViewSpikeHist::on_click_all_classes()
{
	if (m_pvdS->spikeclassoption)
	{
		m_pvdS->spikeclassoption = FALSE;
		(CWnd*)GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
		build_data_and_display();
	}
}

void ViewSpikeHist::on_absolute_time()
{
	if (!m_pvdS->babsolutetime)
	{
		m_pvdS->babsolutetime = TRUE;
		build_data_and_display();
	}
}

void ViewSpikeHist::on_relative_time()
{
	if (m_pvdS->babsolutetime)
	{
		m_pvdS->babsolutetime = FALSE;
		build_data_and_display();
	}
}

void ViewSpikeHist::show_controls(const int i_select)
{
	m_bhistType = i_select;
	switch (i_select)
	{
	case 0:
		m_timebinms = m_pvdS->timebin * t1000;
		m_pvdS->exportdatatype = EXPORT_PSTH;
		break;
	case 1:
		m_nbinsISI = m_pvdS->nbinsISI;
		m_binISIms = m_pvdS->binISI * t1000;
		m_pvdS->exportdatatype = EXPORT_ISI;
		break;
	case 2:
		m_nbinsISI = m_pvdS->nbinsISI;
		m_binISIms = m_pvdS->binISI * t1000;
		m_pvdS->exportdatatype = EXPORT_AUTOCORR;
		break;
	case 3:
		break;
	case 4:
		m_nbinsISI = m_pvdS->nbinsISI;
		m_binISIms = m_pvdS->binISI * t1000;
		m_timebinms = m_pvdS->timebin * t1000;
		break;
	default:
		m_bhistType = 0;
		m_timebinms = m_pvdS->timebin * t1000;
		break;
	}
	m_pvdS->bhistType = m_bhistType;

	static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->SetCurSel(m_bhistType);
	UpdateData(FALSE);

	const int b_settings[5][11] = //1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	{
		{SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_HIDE},
		{SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW,SW_HIDE},
		{SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_SHOW},
		{SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE, SW_HIDE},
		{SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_SHOW}
	};

	int i = 0;
	GetDlgItem(IDC_STATIC7)->ShowWindow(b_settings[i_select][i]);
	i++; // 1  PSTH bin size (ms)
	GetDlgItem(IDC_EDIT4)->ShowWindow(b_settings[i_select][i]);
	i++; // 2  edit:  PSTH bin size (ms)

	GetDlgItem(IDC_BINSIZE)->ShowWindow(b_settings[i_select][i]);
	i++; // 3  Edit: ISI &bin size (ms) or
	GetDlgItem(IDC_STATIC3)->ShowWindow(b_settings[i_select][i]);
	i++; // 4  n bins
	GetDlgItem(IDC_EDIT1)->ShowWindow(b_settings[i_select][i]);
	i++; // 5  Edit: nbins

	GetDlgItem(IDC_STATIC5)->ShowWindow(b_settings[i_select][i]);
	i++; // 6  dot height
	GetDlgItem(IDC_EDIT2)->ShowWindow(b_settings[i_select][i]);
	i++; // 7  Edit: dot height
	GetDlgItem(IDC_STATIC6)->ShowWindow(b_settings[i_select][i]);
	i++; // 8  separator height
	GetDlgItem(IDC_EDIT3)->ShowWindow(b_settings[i_select][i]);
	i++; // 9  Edit: separator height

	GetDlgItem(IDC_STATIC2)->ShowWindow(b_settings[i_select][i]);
	i++; // 10 ISI &bin size (ms)
	GetDlgItem(IDC_STATIC13)->ShowWindow(b_settings[i_select][i]); /*i++;*/ // 11 autocorrelation &bin size (ms)
}

void ViewSpikeHist::on_en_change_n_bins()
{
	if (mm_nbinsISI.m_bEntryDone)
	{
		const auto n_bins = m_nbinsISI;
		mm_nbinsISI.OnEnChange(this, m_nbinsISI, 1, -1);
		m_pvdS->nbinsISI = m_nbinsISI;
		UpdateData(FALSE);
		if (n_bins != m_nbinsISI)
			build_data_and_display();
	}
}

void ViewSpikeHist::on_en_change_row_height()
{
	if (mm_rowheight.m_bEntryDone)
	{
		const auto row_height = m_rowheight;
		mm_rowheight.OnEnChange(this, m_rowheight, 1, -1);
		m_pvdS->dotlineheight = m_rowheight + m_pvdS->dotheight;
		UpdateData(FALSE);
		if (row_height != m_rowheight)
			build_data_and_display();
	}
}

void ViewSpikeHist::on_en_change_dot_height()
{
	if (mm_dotheight.m_bEntryDone)
	{
		const auto dot_height = m_dotheight;
		mm_dotheight.OnEnChange(this, m_dotheight, 1, -1);
		m_pvdS->dotheight = m_dotheight;
		m_pvdS->dotlineheight = m_rowheight + m_dotheight;
		UpdateData(FALSE);
		if (dot_height != m_dotheight)
			build_data_and_display();
	}
}

void ViewSpikeHist::on_edit_copy()
{
	// create metafile
	CMetaFileDC m_dc;

	// size of the window
	CRect rect_bound, rect;
	const auto p_wnd = GetDlgItem(IDC_STATIC12); // get pointer to display static control
	p_wnd->GetClientRect(&rect); // get the final rect
	rect_bound = rect;
	rect_bound.right *= 32; // HIMETRIC UNIT (0.01 mm increments)
	rect_bound.bottom *= 30; // HIMETRIC UNIT (0.01 mm increments)

	// DC for output and objects
	const auto p_dc_ref = p_wnd->GetDC();
	auto cs_title = _T("dbWave\0") + (GetDocument())->GetTitle();
	cs_title += _T("\0\0");
	const auto hm_dc = m_dc.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
	ASSERT(hm_dc != NULL);

	// Draw document in metafile.
	const CClientDC attrib_dc(this); // Create and attach attribute DC
	m_dc.SetAttribDC(attrib_dc.GetSafeHdc()); // from current screen

	// display curves
	// call display routine according to selection
	switch (m_bhistType)
	{
	case 0:
	case 1:
	case 2:
		display_histogram(&m_dc, &rect);
		break;
	case 3:
		display_dot(&m_dc, &rect);
		break;
	case 4:
		display_psth_autocorrelation(&m_dc, &rect);
		break;
	default:
		break;
	}
	ReleaseDC(p_dc_ref);

	// Close metafile
	const auto h_emf_tmp = m_dc.CloseEnhanced();
	ASSERT(h_emf_tmp != NULL);
	if (OpenClipboard())
	{
		EmptyClipboard(); // prepare clipboard
		SetClipboardData(CF_ENHMETAFILE, h_emf_tmp); // put data
		CloseClipboard(); // close clipboard
	}
	else
	{
		// Someone else has the Clipboard open...
		DeleteEnhMetaFile(h_emf_tmp); // delete data
		MessageBeep(0); // tell user something is wrong!
		AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
	}
}

BOOL ViewSpikeHist::OnPreparePrinting(CPrintInfo* p_info)
{
	if (!DoPreparePrinting(p_info))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, p_info))
		return FALSE;

	// printing margins
	if (mdPM->vertical_resolution <= 0 || mdPM->horizontal_resolution <= 0
		|| mdPM->horizontal_resolution != p_info->m_rectDraw.Width()
		|| mdPM->vertical_resolution != p_info->m_rectDraw.Height())
	{
		// compute printer's page dot resolution
		CPrintDialog dlg(FALSE); // borrowed from VC++ sample\drawcli\drawdoc.cpp
		VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));
		CDC dc; // GetPrinterDC returns a HDC so attach it
		const auto h_dc = dlg.CreatePrinterDC();
		ASSERT(h_dc != NULL);
		dc.Attach(h_dc);
		// Get the size of the page in pixels
		mdPM->horizontal_resolution = dc.GetDeviceCaps(HORZRES);
		mdPM->vertical_resolution = dc.GetDeviceCaps(VERTRES);
	}

	// how many rows per page?
	const auto size_row = mdPM->height_doc + mdPM->height_separator;
	auto n_rows_per_page = (mdPM->vertical_resolution - 2 * mdPM->top_page_margin) / size_row;
	auto n_files = 1;
	if (m_nfiles == 1)
		n_files = GetDocument()->db_get_records_count();

	if (n_rows_per_page == 0) // prevent zero pages
		n_rows_per_page = 1;
	auto n_pages = n_files / n_rows_per_page;
	if (n_files > n_rows_per_page * n_pages)
		n_pages++;

	p_info->SetMaxPage(n_pages); // one-page printing/preview
	p_info->m_nNumPreviewPages = 1; // preview 1 pages at a time
	// allow print only selection
	if (mdPM->b_print_selection)
		p_info->m_pPD->m_pd.Flags |= PD_SELECTION;
	else
		p_info->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;

	// call dialog box
	const auto flag = DoPreparePrinting(p_info);
	// set max nb of pages according to selection
	mdPM->b_print_selection = p_info->m_pPD->PrintSelection();
	if (mdPM->b_print_selection)
		p_info->SetMaxPage(1);
	return flag;
}

void ViewSpikeHist::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	// select font, set print flag, save current file index
	const auto p_old_font = p_dc->SelectObject(&m_fontPrint);

	m_bPrint = TRUE;
	const int file0 = GetDocument()->db_get_current_record_position();

	// print page footer: file path, page number/total pages, date
	const auto t = CTime::GetCurrentTime(); // current date & time
	CString cs_footer; // first string to receive
	cs_footer.Format(_T("  page %d:%d %d-%d-%d"), // page and time infos
	                 pInfo->m_nCurPage, pInfo->GetMaxPage(),
	                 t.GetDay(), t.GetMonth(), t.GetYear());
	CString ch_date = GetDocument()->db_get_current_spk_file_name(FALSE);
	ch_date = ch_date.Left(ch_date.GetLength() - 1) + cs_footer;
	p_dc->SetTextAlign(TA_CENTER); // and print the footer
	p_dc->TextOut(mdPM->horizontal_resolution / 2, mdPM->vertical_resolution - 57, ch_date);

	// define page rectangle (where data and comments are plotted)
	CRect rect_page; // = pInfo->m_rectDraw;
	//rect_page.right = mdPM->horzRes-mdPM->rightPageMargin;
	//rect_page.bottom = mdPM->vertRes-mdPM->bottomPageMargin;
	rect_page.left = mdPM->left_page_margin;
	rect_page.top = mdPM->top_page_margin;

	// define data file rectangle - position of the first file
	const auto r_width = mdPM->width_doc; // margins
	const auto r_height = mdPM->height_doc; // margins
	CRect r_where(rect_page.left, rect_page.top,
	              rect_page.left + r_width, rect_page.top + r_height);

	// prepare file loop
	auto p_dbwave_doc = GetDocument();
	/*int nfiles = */
	long n_records = p_dbwave_doc->db_get_records_count();
	const auto size_row = mdPM->height_doc + mdPM->height_separator; // size of one row
	auto n_rows_per_page = pInfo->m_rectDraw.Height() / size_row; // nb of rows per page
	if (n_rows_per_page == 0)
		n_rows_per_page = 1;
	const int file1 = (pInfo->m_nCurPage - 1) * n_rows_per_page; // index first file
	auto file2 = file1 + n_rows_per_page; // index last file
	if (m_nfiles != 1) // special case: all together
		file2 = file1 + 1;
	if (file2 > p_dbwave_doc->db_get_records_count())
		file2 = p_dbwave_doc->db_get_records_count();

	// loop through all files
	for (auto i_file = file1; i_file < file2; i_file++)
	{
		if (mdPM->b_frame_rect) // print data rect if necessary
		{
			p_dc->MoveTo(r_where.left, r_where.top);
			p_dc->LineTo(r_where.right, r_where.top);
			p_dc->LineTo(r_where.right, r_where.bottom);
			p_dc->LineTo(r_where.left, r_where.bottom);
			p_dc->LineTo(r_where.left, r_where.top);
		}
		m_commentRect = r_where; // calculate where the comments will be printed
		m_commentRect.OffsetRect(mdPM->text_separator + m_commentRect.Width(), 0);
		m_commentRect.right = pInfo->m_rectDraw.right;
		// refresh data if necessary
		if (m_nfiles == 1) //??? (m_nfiles > 1)
		{
			BOOL success = p_dbwave_doc->db_set_current_record_position(i_file);
			build_data();
		}
		// print the histogram
		switch (m_bhistType)
		{
		case 0:
		case 1:
		case 2:
			display_histogram(p_dc, &r_where);
			break;
		case 3:
			display_dot(p_dc, &r_where);
			break;
		case 4:
			display_psth_autocorrelation(p_dc, &r_where);
			break;
		default:
			break;
		}
		// update display rectangle for next row
		r_where.OffsetRect(0, r_height + mdPM->height_separator);
	}

	// restore parameters
	if (p_old_font != nullptr)
		p_dc->SelectObject(p_old_font);

	BOOL success = p_dbwave_doc->db_set_current_record_position(file0);
	p_spike_doc_ = p_dbwave_doc->open_current_spike_file();
}

void ViewSpikeHist::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bPrint = FALSE;
	ViewDbTable::OnEndPrinting(p_dc, pInfo);
}

void ViewSpikeHist::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	memset(&m_logFont, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial")); // Arial font
	m_logFont.lfHeight = mdPM->font_size; // font height
	m_fontPrint.CreateFontIndirect(&m_logFont);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikeHist::on_format_histogram()
{
	DlgFormatHistogram dlg;
	dlg.m_bYmaxAuto = m_pvdS->bYmaxAuto;
	dlg.m_Ymax = m_pvdS->Ymax;
	dlg.m_xfirst = m_pvdS->timestart;
	dlg.m_xlast = m_pvdS->timeend;
	dlg.m_crHistFill = m_pvdS->crHistFill;
	dlg.m_crHistBorder = m_pvdS->crHistBorder;
	dlg.m_crStimFill = m_pvdS->crStimFill;
	dlg.m_crStimBorder = m_pvdS->crStimBorder;
	dlg.m_crChartArea = m_pvdS->crChartArea;

	if (IDOK == dlg.DoModal())
	{
		m_pvdS->bYmaxAuto = dlg.m_bYmaxAuto;
		m_pvdS->Ymax = dlg.m_Ymax;
		m_pvdS->timestart = dlg.m_xfirst;
		m_pvdS->timeend = dlg.m_xlast;
		m_timefirst = m_pvdS->timestart;
		m_timelast = m_pvdS->timeend;
		m_pvdS->crHistFill = dlg.m_crHistFill;
		m_pvdS->crHistBorder = dlg.m_crHistBorder;
		m_pvdS->crStimFill = dlg.m_crStimFill;
		m_pvdS->crStimBorder = dlg.m_crStimBorder;
		m_pvdS->crChartArea = dlg.m_crChartArea;
		build_data_and_display();
	}
}

void ViewSpikeHist::build_data()
{
	auto n_bins = static_cast<int>((m_timelast - m_timefirst) * t1000 / m_timebinms);
	if (n_bins <= 0)
		n_bins = 1;

	switch (m_bhistType)
	{
	case 0:
		m_nPSTH = 0;
		if (m_sizepPSTH != n_bins) // make space for side PSTH
		{
			SAFE_DELETE_ARRAY(m_pPSTH)
			m_pPSTH = new long[n_bins + 2];
			ASSERT(m_pPSTH != NULL);
			m_sizepPSTH = n_bins;
			m_pvdS->nbins = n_bins;
		}
		break;

	case 1:
	case 2:
		m_nISI = 0;
		if (m_sizepISI != m_nbinsISI) // make space for side autocorrelation
		{
			SAFE_DELETE_ARRAY(m_pISI)
			m_pISI = new long[m_nbinsISI + 2];
			ASSERT(m_pISI != NULL);
			m_sizepISI = m_nbinsISI;
			m_pvdS->nbinsISI = m_nbinsISI;
		}
		break;

	case 4:
		m_nPSTH = 0;
		if (m_sizepPSTH != n_bins) // make space for side PSTH
		{
			SAFE_DELETE_ARRAY(m_pPSTH)
			m_pPSTH = new long[n_bins + 2];
			ASSERT(m_pPSTH != NULL);
			m_sizepPSTH = n_bins;
			m_pvdS->nbins = n_bins;
		}

		if (m_sizepISI != m_nbinsISI) // make space for side autocorrelation
		{
			SAFE_DELETE_ARRAY(m_pISI)
			m_pISI = new long[m_nbinsISI + 2];
			ASSERT(m_pISI != NULL);
			m_sizepISI = m_nbinsISI;
			m_pvdS->nbinsISI = m_nbinsISI;
		}

		n_bins = m_sizepPSTH * m_sizepISI; // total bins for main histogram array
		if (m_sizeparrayISI != n_bins) // make space for side autocorrelation
		{
			SAFE_DELETE_ARRAY(m_parrayISI)
			m_parrayISI = new long[n_bins + 2];
			ASSERT(m_parrayISI != NULL);
			m_sizeparrayISI = n_bins;
		}
		break;

	default:
		break;
	}

	// erase the content of the arrays
	long* p_long;
	if (m_sizepPSTH != NULL)
	{
		p_long = m_pPSTH;
		for (auto ui = 0; ui <= m_sizepPSTH; ui++, p_long++)
			*p_long = 0;
	}
	if (m_sizepISI != 0)
	{
		p_long = m_pISI;
		for (auto ui = 0; ui <= m_sizepISI; ui++, p_long++)
			*p_long = 0;
	}
	if (m_sizeparrayISI != 0)
	{
		p_long = m_parrayISI;
		for (auto ui = 0; ui <= m_sizeparrayISI; ui++, p_long++)
			*p_long = 0;
	}

	const auto p_dbwave_doc = GetDocument();
	const int current_file = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto first_file = current_file; // index first file in the series
	auto last_file = current_file; // index last file in the series

	DlgProgress* p_dlg = nullptr;
	auto i_step = 0;
	CString cs_comment;
	CString cs_file_comment = _T("Analyze file: ");

	if (m_nfiles > 1)
	{
		p_dlg = new DlgProgress;
		p_dlg->Create();
		p_dlg->SetStep(1);
		first_file = 0;
		last_file = m_nfiles - 1;
	}

	const auto current_list_index = p_dbwave_doc->get_current_spike_file()->get_spike_list_current_index();

	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		if (m_nfiles > 1)
		{
			// check if user wants to stop
			if (p_dlg->CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cs_comment.Format(_T("Processing file [%i / %i]"), i_file + 1, m_nfiles);
			p_dlg->SetStatus(cs_comment);
			if (MulDiv(i_file, 100, m_nfiles) > i_step)
			{
				p_dlg->StepIt();
				i_step = MulDiv(i_file, 100, m_nfiles);
			}
		}

		// select spike file
		BOOL success = p_dbwave_doc->db_set_current_record_position(i_file);
		p_spike_doc_ = p_dbwave_doc->open_current_spike_file();
		if (nullptr == p_spike_doc_)
			continue;

		p_spike_doc_->set_spike_list_current_index(current_list_index);

		switch (m_bhistType)
		{
		// PERI-STIMULUS TIME HISTOGRAM
		case 0:
			m_nPSTH += p_spike_doc_->build_psth(m_pvdS, m_pPSTH, m_spikeclass);
			break;
		// INTER-SPIKES INTERVALS HISTOGRAM
		case 1:
			m_nISI += p_spike_doc_->build_isi(m_pvdS, m_pISI, m_spikeclass);
			break;
		// AUTOCORRELATION
		case 2:
			m_nISI += p_spike_doc_->build_autocorrelation(m_pvdS, m_pISI, m_spikeclass);
			break;
		// PSTH-AUTOCORRELATION DENSITY
		case 4:
			{
				m_nPSTH += p_spike_doc_->build_psth(m_pvdS, m_pPSTH, m_spikeclass);
				p_spike_doc_->build_autocorrelation(m_pvdS, m_pISI, m_spikeclass);
				p_spike_doc_->build_psth_autocorrelation(m_pvdS, m_parrayISI, m_spikeclass);
			}
			break;
		default:
			break;
		}
	}

	if (current_file != p_dbwave_doc->db_get_current_record_position())
	{
		BOOL success = p_dbwave_doc->db_set_current_record_position(current_file);
		p_spike_doc_ = p_dbwave_doc->open_current_spike_file();
		p_spike_doc_->set_spike_list_current_index(current_list_index);
	}
	SAFE_DELETE(p_dlg)
}

// plot histogram (generic)
//
// input (passed parameters):
//		p_dc			pointer to DC context
//		display_Rect	display rectangle (plotting area)
//		n_bins_hist	number of bins within the histogram
//		p_hist		pointer to an array of long with the values of the histogram
//		orientation	0=0 deg, +1=+90 deg; -1=-90 deg; +2,-2=-180 degrees
//		b_type	type of display histogram (0=PSTH, 1=ISI, 2=autocorrelation)

// input (internal parameters):
//		p_option_view_spikes array with spike display parameters OPTIONS_VIEW_SPIKES
//		m_pvdS->crHistFill	histogram COLOR_REF fill color
//		m_pvdS->crHistBorder histogram border color
//		m_pvdS->crChartArea background plane color
//		m_pvdS->b_Y_max_Auto	flag:on=search max off=imposed max
//		m_time_last
//		m_time_first
//		m_time_bin_ms
//		t1000
//		m_x_first
//		m_x_last
//		m_pvdS->b_absolute_time
//		m_pSpkDoc->m_stimulus_intervals.n_items
//		m_pvdS->cr_Stimulus_Fill
//		m_pvdS->cr_Stimulus_Border
//		m_pSpkDoc->m_spk_list
//		p_spk_list->Get_Acq_Sampling_Rate()
//
// output (passed parameter):
//		return max value of histogram
// output (modified internal parameters)
//		m_rect_ratio	 % of histogram height versus plotting area

long ViewSpikeHist::plot_histogram(CDC* p_dc, CRect* p_display_rect, int n_bins, long* p_hist0, int orientation, int b_type)
{
	CPen pen_bars;
	pen_bars.CreatePen(PS_SOLID, 0, m_pvdS->crHistBorder);
	CBrush brush_bars;
	brush_bars.CreateSolidBrush(m_pvdS->crHistFill);

	// update chart area
	p_display_rect->InflateRect(1, 1);
	p_dc->FillSolidRect(p_display_rect, m_pvdS->crChartArea);

	const auto p_old_pen = p_dc->SelectObject(&pen_bars); 
	p_dc->MoveTo(p_display_rect->left, p_display_rect->top);
	p_dc->LineTo(p_display_rect->right, p_display_rect->top);
	p_dc->LineTo(p_display_rect->right, p_display_rect->bottom);
	p_dc->LineTo(p_display_rect->left, p_display_rect->bottom);
	p_dc->LineTo(p_display_rect->left, p_display_rect->top);
	p_display_rect->DeflateRect(1, 1);

	// display XY axis
	p_dc->SelectStockObject(BLACK_PEN);
	int x_zero = p_display_rect->left;
	CRect rect_hz;
	CRect rect_vert;

	switch (orientation)
	{
	case -1: // -90
		x_zero = p_display_rect->bottom;
		if (b_type == 2)
			x_zero = (p_display_rect->bottom + p_display_rect->top) / 2;
		rect_hz = CRect(p_display_rect->left, x_zero, p_display_rect->right, x_zero);
		rect_vert = CRect(p_display_rect->right, p_display_rect->top, p_display_rect->right, p_display_rect->bottom);
		break;
	case 1: // +90
		x_zero = p_display_rect->top;
		if (b_type == 2)
			x_zero = (p_display_rect->bottom + p_display_rect->top) / 2;
		rect_hz = CRect(p_display_rect->left, x_zero, p_display_rect->right, x_zero);
		rect_vert = CRect(p_display_rect->left, p_display_rect->top, p_display_rect->left, p_display_rect->bottom);

		break;
	case 2: // -180 deg
		x_zero = p_display_rect->left;
		if (b_type == 2)
			x_zero = (p_display_rect->left + p_display_rect->right) / 2;
		rect_hz = CRect(p_display_rect->left, p_display_rect->top, p_display_rect->right, p_display_rect->top);
		rect_vert = CRect(x_zero, p_display_rect->top, x_zero, p_display_rect->bottom);
		break;
	default: //	normal
		x_zero = p_display_rect->left;
		if (b_type == 2)
			x_zero = (p_display_rect->left + p_display_rect->right) / 2;
		rect_hz = CRect(p_display_rect->left, p_display_rect->bottom, p_display_rect->right, p_display_rect->bottom);
		rect_vert = CRect(x_zero, p_display_rect->top, x_zero, p_display_rect->bottom);
		break;
	}
	p_dc->MoveTo(rect_hz.left, rect_hz.top); // hz line
	p_dc->LineTo(rect_hz.right, rect_hz.bottom);
	p_dc->MoveTo(rect_vert.left, rect_vert.top); // vert line
	p_dc->LineTo(rect_vert.right, rect_vert.bottom);

	// set position of max (assuming hardwired 80% of max: rect_ratio=80)
	long max = 0;
	if (m_pvdS->bYmaxAuto)
	{
		auto p_hist = p_hist0;
		for (auto i = 0; i < n_bins; i++, p_hist++)
		{
			const int val = *p_hist; // load value in a temp variable
			if (val > max) // search max
				max = val;
		}
		m_rectratio = 80; // span only 80%
	}
	else
	{
		auto divisor = m_timebinms / t1000;
		if (b_type > 0)
			divisor = m_timelast - m_timefirst;
		max = static_cast<long>(m_pvdS->Ymax * divisor * static_cast<float>(m_nfiles));
		m_rectratio = 100; // 100% span for data
	}
	auto rect_max = MulDiv(p_display_rect->Height(), m_rectratio, 100);

	// prepare pen and brush
	p_dc->SelectObject(&pen_bars); // rect outer line
	const auto p_old_brush = p_dc->SelectObject(&brush_bars); // rect fill
	auto rect_bar = *p_display_rect; // rectangle to plot the bars

	int display_length;
	int ui;
	switch (orientation)
	{
	case -1: // rotation -90 deg
		display_length = p_display_rect->Height();
		rect_max = MulDiv(p_display_rect->Width(), m_rectratio, 100);
		for (ui = 0; ui < m_sizepISI; ui++)
		{
			rect_bar.bottom = MulDiv(display_length, ui + 1, n_bins) + p_display_rect->top;
			rect_bar.left = rect_bar.right - MulDiv(*(p_hist0 + ui), rect_max, max);
			p_dc->MoveTo(rect_bar.left, rect_bar.bottom);
			if (rect_bar.top != rect_bar.bottom)
				p_dc->Rectangle(rect_bar);
			else
				p_dc->LineTo(rect_bar.left, rect_bar.top);
			rect_bar.top = rect_bar.bottom;
		}
		break;

	case 1: // rotation +90 deg
		display_length = p_display_rect->Height();
		rect_max = MulDiv(p_display_rect->Width(), m_rectratio, 100);
		for (ui = 0; ui < m_sizepISI; ui++)
		{
			rect_bar.bottom = MulDiv(display_length, ui + 1, n_bins) + p_display_rect->top;
			rect_bar.right = rect_bar.left + MulDiv(*(p_hist0 + ui), rect_max, max);
			p_dc->MoveTo(rect_bar.left, rect_bar.bottom);
			if (rect_bar.top != rect_bar.bottom)
				p_dc->Rectangle(rect_bar);
			else
				p_dc->LineTo(rect_bar.left, rect_bar.top);
			rect_bar.top = rect_bar.bottom;
		}
		break;

	case 2:
		rect_max = MulDiv(p_display_rect->Height(), m_rectratio, 100);
		display_length = p_display_rect->Width();
		for (ui = 0; ui < n_bins; ui++)
		{
			rect_bar.right = MulDiv(display_length, ui + 1, n_bins) + p_display_rect->left;
			rect_bar.bottom = rect_bar.top + MulDiv(*(p_hist0 + ui), rect_max, max);
			p_dc->MoveTo(rect_bar.left, rect_bar.bottom);
			if (rect_bar.left != rect_bar.right)
				p_dc->Rectangle(rect_bar);
			else
				p_dc->LineTo(rect_bar.left, rect_bar.top);
			rect_bar.left = rect_bar.right;
		}
		break;

	default:
		display_length = p_display_rect->Width();
		rect_max = MulDiv(p_display_rect->Height(), m_rectratio, 100);

	// display stimulus
		if (b_type == 0 && p_spike_doc_->m_stimulus_intervals.n_items > 0)
		{
			const auto p_spk_list = p_spike_doc_->get_spike_list_current();
			const auto sampling_rate = p_spk_list->get_acq_sampling_rate();
			int ii_offset0 = p_spike_doc_->m_stimulus_intervals.get_at(m_pvdS->istimulusindex);
			if (m_pvdS->babsolutetime)
				ii_offset0 = 0;

			// search first stimulus transition within interval
			auto ii_start = static_cast<long>(m_pvdS->timestart * sampling_rate) + ii_offset0;
			auto ii_end = static_cast<long>(m_pvdS->timeend * sampling_rate) + ii_offset0;

			TEXTMETRIC tm; // load characteristics of the font
			p_dc->GetTextMetrics(&tm);
			auto rect = *p_display_rect;
			rect.bottom = rect.bottom + tm.tmHeight + tm.tmDescent - 2;
			rect.top = p_display_rect->bottom + 2 * tm.tmDescent + 2;
			display_stimulus(p_dc, &rect, &ii_start, &ii_end);
		}

	// display histogram
		for (ui = 0; ui < n_bins; ui++)
		{
			rect_bar.right = MulDiv(display_length, ui + 1, n_bins) + p_display_rect->left;
			rect_bar.top = rect_bar.bottom - MulDiv(*(p_hist0 + ui), rect_max, max);
			p_dc->MoveTo(rect_bar.left, rect_bar.bottom);
			if (rect_bar.left != rect_bar.right)
				p_dc->Rectangle(rect_bar);
			else
				p_dc->LineTo(rect_bar.left, rect_bar.top);
			rect_bar.left = rect_bar.right;
		}
		break;
	}

	// restore objects
	p_dc->SelectObject(p_old_pen);
	p_dc->SelectObject(p_old_brush);
	return max;
}

void ViewSpikeHist::display_dot(CDC* p_dc, CRect* p_rect)
{
	CWaitCursor wait;

	// save old pen and brush / restore on exit
	CPen pen_bars;
	pen_bars.CreatePen(PS_SOLID, 0, col_black);
	const auto p_old_pen = p_dc->SelectObject(&pen_bars);
	CBrush brush_bars;
	brush_bars.CreateSolidBrush(col_black);

	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	auto display_rect = *p_rect; // this will be the display rect for histogram
	display_rect.left++;
	display_rect.right--;

	// print comments
	TEXTMETRIC tm; // load characteristics of the font
	p_dc->GetTextMetrics(&tm);
	auto comment_rect = display_rect;
	comment_rect.left += 4;
	comment_rect.top++;
	if (m_bPrint)
		comment_rect = m_commentRect;

	// output legends
	CString str_comment;
	get_file_infos(str_comment);

	// histogram type and bin value
	m_xfirst = m_timefirst; // abscissa first
	m_xlast = m_timelast; // abscissa last
	str_comment += _T("Dot Display"); // Dot display
	const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	auto line = p_dc->DrawText(str_comment, str_comment.GetLength(), comment_rect,
	                           DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	p_dc->SetTextAlign(ui_flag);
	line = line / tm.tmHeight;
	if (m_bPrint)
		line = 0;

	// define display rectangle and plot axis
	display_rect.top += tm.tmHeight * (line + 1) + tm.tmDescent;
	display_rect.bottom -= tm.tmHeight + tm.tmDescent;
	display_rect.left += tm.tmDescent;
	display_rect.right -= tm.tmDescent;

	// print left abscissa value
	CString cs_x_left;
	cs_x_left.Format(_T("%1.3f"), m_xfirst);
	p_dc->TextOut(display_rect.left, display_rect.bottom, cs_x_left);

	// print right abscissa value + unit
	CString cs_x_right;
	cs_x_right.Format(_T("%1.3f s"), m_xlast);
	const auto left = p_dc->GetTextExtent(cs_x_right);
	p_dc->TextOut(display_rect.right - left.cx - tm.tmDescent, display_rect.bottom, cs_x_right);

	// display XY axis
	p_dc->SelectStockObject(BLACK_PEN);
	// abscissa
	p_dc->MoveTo(display_rect.left, display_rect.bottom);
	p_dc->LineTo(display_rect.right, display_rect.bottom);

	// set temp variables
	const int rc_left = display_rect.left;
	const auto span = m_xlast - m_xfirst;
	const auto rect_len = display_rect.Width();
	int row = display_rect.top;
	const auto dot_height = MulDiv(p_rect->Height(), m_pvdS->dotheight, 1000);
	const auto dot_line_height = MulDiv(p_rect->Height(), m_pvdS->dotlineheight, 1000);
	const auto vt_top = dot_height + 1;
	const auto vt_bottom = dot_line_height - 2;

	// prepare loop / files (stop when no more space is available)
	auto p_dbwave_doc = GetDocument();
	const int current_file = p_dbwave_doc->db_get_current_record_position(); // index current file
	auto first_file = current_file; // index first file in the series
	auto last_file = first_file; // index last file in the series
	if (m_nfiles > 1)
	{
		first_file = 0;
		last_file = m_nfiles - 1;
	}

	// external loop: browse from file to file
	auto current_list_index = p_dbwave_doc->get_current_spike_file()->get_spike_list_current_index();
	for (auto i_file = first_file;
	     i_file <= last_file && row < display_rect.bottom;
	     i_file++)
	{
		BOOL success = p_dbwave_doc->db_set_current_record_position(i_file);
		p_spike_doc_ = p_dbwave_doc->open_current_spike_file();
		p_spike_doc_->set_spike_list_current_index(current_list_index);

		// load pointers to spike file and spike list
		const auto p_spk_list = p_spike_doc_->get_spike_list_current();
		const auto sampling_rate = p_spk_list->get_acq_sampling_rate();
		const auto ii_frame_first = static_cast<long>(m_timefirst * sampling_rate);
		const auto ii_frame_last = static_cast<long>(m_timelast * sampling_rate);
		const auto ii_frame_length = ii_frame_last - ii_frame_first;
		const auto n_spikes = p_spk_list->get_spikes_count();

		// display spikes and stimuli either on one line or on multiple lines
		if (m_pvdS->babsolutetime)
		{
			auto n_rows = 1;
			if (mdPM->b_multiple_rows)
			{
				n_rows = p_spike_doc_->get_acq_size() / ii_frame_length;
				if (n_rows * ii_frame_length < p_spike_doc_->get_acq_size())
					n_rows++;
			}
			auto ii_first = ii_frame_first;
			auto ii_last = ii_frame_last;
			auto i_spike_first = 0;
			for (auto i_row = 0; i_row < n_rows; i_row++)
			{
				// display stimuli
				if (p_spike_doc_->m_stimulus_intervals.n_items > 0)
				{
					CRect rect(rc_left, row + vt_bottom,
					           rect_len + rc_left, row + vt_top);
					display_stimulus(p_dc, &rect, &ii_first, &ii_last);
				}
				// display spikes
				auto ii_time0 = -1;
				for (auto i = i_spike_first; i < n_spikes; i++)
				{
					auto ii_time = p_spk_list->get_spike(i)->get_time() - ii_first;
					// check if this spike should be processed
					// assume that spikes occurence times are ordered
					if (ii_time < 0)
						continue;
					if (ii_time > ii_frame_length)
					{
						i_spike_first = i;
						break;
					}
					// check spike class
					if (m_pvdS->spikeclassoption
						&& p_spk_list->get_spike(i)->get_class_id() != m_spikeclass)
						continue;
					// convert interval into a pixel bin
					const auto spk_time = static_cast<float>(ii_time) / sampling_rate;
					ii_time = static_cast<int>(spk_time * static_cast<float>(rect_len) / span) + rc_left;
					if (ii_time != ii_time0) // avoid multiple drawing of the same dots
					{
						p_dc->MoveTo(ii_time, row);
						p_dc->LineTo(ii_time, row + dot_height);
						ii_time0 = ii_time;
					}
				}

				// end loop - update parameters for next row
				ii_first += ii_frame_length;
				ii_last += ii_frame_length;
				row += dot_line_height;
			}
		}

		// display spikes in time locked to the stimuli
		else
		{
			// if !bCycleHist - only one pass is called
			const auto stimulus_first = m_pvdS->istimulusindex;
			auto stimulus_last = stimulus_first + 1;
			auto increment = 2;
			// if bCycleHist - one line per stimulus (or group of stimuli)
			if (m_pvdS->bCycleHist)
			{
				stimulus_last = p_spike_doc_->m_stimulus_intervals.get_size();
				if (stimulus_last == 0)
					stimulus_last = 1;
				increment = m_pvdS->nstipercycle;
				if (p_spike_doc_->m_stimulus_intervals.n_per_cycle > 1
					&& increment > p_spike_doc_->m_stimulus_intervals.n_per_cycle)
					increment = p_spike_doc_->m_stimulus_intervals.n_per_cycle;
				increment *= 2;
			}

			// loop over stimuli
			for (auto i_stimulus = stimulus_first; i_stimulus < stimulus_last; i_stimulus += increment)
			{
				// compute temp parameters
				long i_start;
				if (p_spike_doc_->m_stimulus_intervals.n_items > 0)
					i_start = p_spike_doc_->m_stimulus_intervals.get_at(i_stimulus);
				else
					i_start = static_cast<long>(-(m_pvdS->timestart * sampling_rate));

				i_start += ii_frame_first;

				// draw dots -- look at all spikes...
				auto ii_time0 = -1;
				for (auto i = 0; i < n_spikes; i++)
				{
					auto ii_time = p_spk_list->get_spike(i)->get_time() - i_start;
					// check if this spike should be processed
					// assume that spikes occurence times are ordered
					if (ii_time < 0)
						continue;
					if (ii_time > ii_frame_length)
						break;
					// check spike class
					if (m_pvdS->spikeclassoption
						&& p_spk_list->get_spike(i)->get_class_id() != m_spikeclass)
						continue;
					// convert interval into a pixel bin
					const auto spk_time = static_cast<float>(ii_time) / sampling_rate;
					ii_time = static_cast<int>(spk_time * static_cast<float>(rect_len) / span) + rc_left;
					if (ii_time != ii_time0) // avoid multiple drawing of the same dots
					{
						p_dc->MoveTo(ii_time, row);
						p_dc->LineTo(ii_time, row + dot_height);
						ii_time0 = ii_time;
					}
				}
				// next stimulus set -- jump to next line
				row += dot_line_height;
			}

			if (p_spike_doc_->m_stimulus_intervals.n_items > 1)
			{
				// stimulus
				auto i_start = p_spike_doc_->m_stimulus_intervals.get_at(m_pvdS->istimulusindex);
				auto i_end = ii_frame_last + i_start;
				i_start = ii_frame_first + i_start;

				CRect rect(rc_left, row + dot_line_height - dot_height,
				           rect_len + rc_left, row + vt_bottom);
				display_stimulus(p_dc, &rect, &i_start, &i_end);
			}
		}

		// next file -- jump to next line..
		row += dot_line_height;
	}

	BOOL success = p_dbwave_doc->db_set_current_record_position(current_file);
	p_spike_doc_ = p_dbwave_doc->open_current_spike_file();
	p_spike_doc_->set_spike_list_current_index(current_list_index);

	p_dc->SelectObject(p_old_pen);
	p_dc->SelectObject(p_old_brush);
}

void ViewSpikeHist::display_histogram(CDC* p_dc, const CRect* p_rect)
{
	auto display_rect = *p_rect; // this will be the display rect for histogram

	// print text on horizontal lines
	TEXTMETRIC tm; // load characteristics of the font
	p_dc->GetTextMetrics(&tm);
	p_dc->SetBkMode(TRANSPARENT);

	// define display rectangle and plot data within display area
	display_rect.top += tm.tmHeight * 2 + tm.tmDescent;
	display_rect.bottom -= tm.tmHeight + 2 * tm.tmDescent;
	display_rect.left += 2 * tm.tmHeight;
	display_rect.right -= 2 * tm.tmHeight;

	// get data pointer and size
	long* p_hist0 = nullptr; // pointer to first element
	auto n_bins_hist = 0; 
	switch (m_bhistType)
	{
	case 0: // PSTH
		p_hist0 = m_pPSTH; // pointer to first element
		n_bins_hist = m_sizepPSTH;
		break;
	case 1: // ISI
	case 2: // AUTOCORRELATION
		p_hist0 = m_pISI; // pointer to first element
		n_bins_hist = m_sizepISI;
		break;
	default:
		break;
	}
	// plot data
	const int hist_max = plot_histogram(p_dc, &display_rect, n_bins_hist, p_hist0, 0, m_bhistType);

	// output legends: title of the graph and data description
	auto comment_rect = display_rect; // comment rectangle
	if (m_bPrint) // adjust comment position if printing
		comment_rect = m_commentRect;
	else
		comment_rect.top -= tm.tmHeight * 2; // + tm.tmDescent;

	CString str_comment; // scratch pad
	get_file_infos(str_comment); // file comments

	// histogram type and bin value
	auto divisor = 1.f; // factor to normalize histograms
	m_xfirst = m_timefirst; // abscissa first
	m_xlast = m_timelast; // abscissa last
	auto bin_ms = 1.f;
	switch (m_bhistType)
	{
	case 0: // PSTH
		str_comment += _T("PSTH (");
		divisor = static_cast<float>(m_nPSTH) * m_timebinms / t1000;
		bin_ms = m_timebinms;
		break;
	case 1: // ISI
		str_comment += _T("ISI (");
		divisor = m_timelast - m_timefirst;
		m_xfirst = 0;
		m_xlast = m_binISIms / t1000 * static_cast<float>(m_nbinsISI);
		bin_ms = m_binISIms;
		break;
	case 2: // AUTOCORRELATION
		str_comment += _T("AUTOCORR (");
		divisor = m_timelast - m_timefirst;
		m_xfirst = -m_binISIms / t1000 * static_cast<float>(m_nbinsISI) / 2;
		m_xlast = -m_xfirst;
		bin_ms = m_binISIms;
		break;
	default:
		break;
	}
	CString cs_t2;
	cs_t2.Format(_T("bin:%1.1f ms)"), bin_ms);
	str_comment += cs_t2;

	// display title (on 2 lines)
	const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	p_dc->DrawText(str_comment,
	               str_comment.GetLength(),
	               comment_rect,
	               DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	p_dc->SetTextAlign(ui_flag);

	// print abscissa first and last values
	CString cs_x_left;
	CString cs_x_right;
	switch (m_bhistType)
	{
	case 1: // ISI
		cs_x_left = _T("0");
		cs_x_right.Format(_T("%1.0f ms"), static_cast<double>(m_xlast) * t1000);
		break;
	case 2: // AUTOCORRELATION
		cs_x_left.Format(_T("%1.0f"), static_cast<double>(m_xfirst) * t1000);
		cs_x_right.Format(_T("%1.0f ms"), static_cast<double>(m_xlast) * t1000);
		break;
	default: // PSTH (default)
		cs_x_left.Format(_T("%1.3f"), m_xfirst);
		cs_x_right.Format(_T("%1.3f s"), m_xlast);
		break;
	}

	// origin
	p_dc->TextOut(display_rect.left, display_rect.bottom + tm.tmDescent, cs_x_left);
	// end (align on the right)
	auto left = p_dc->GetTextExtent(cs_x_right);
	p_dc->TextOut(display_rect.right - left.cx - tm.tmDescent, display_rect.bottom + tm.tmDescent, cs_x_right);

	// print value of ordinate max vertically
	CFont v_font;
	LOGFONT log_font;

	memset(&log_font, 0, sizeof(LOGFONT)); // prepare font
	p_dc->GetCurrentFont()->GetLogFont(&log_font); // fill it from CDC
	log_font.lfEscapement = +900;
	v_font.CreateFontIndirect(&log_font);
	const auto p_old_font = p_dc->SelectObject(&v_font);

	// compute hist max
	float y_max;
	if (m_pvdS->bYmaxAuto)
		y_max = (static_cast<float>(MulDiv(hist_max, m_rectratio, 100)) / divisor) / static_cast<float>(m_nfiles);
	else
		y_max = m_pvdS->Ymax;

	CString cs_y_max;
	cs_y_max.Format(_T("%1.3f spk/s"), y_max);
	p_dc->GetTextMetrics(&tm);
	left = p_dc->GetTextExtent(cs_y_max);
	left.cy += tm.tmDescent;
	p_dc->TextOut(display_rect.left - left.cy, display_rect.top + left.cx, cs_y_max);
	p_dc->TextOut(display_rect.left - left.cy, display_rect.bottom, _T("0"));

	// restore normal font
	p_dc->SetBkMode(OPAQUE);
	p_dc->SelectObject(p_old_font);
	v_font.DeleteObject();
}

void ViewSpikeHist::display_psth_autocorrelation(CDC* p_dc, CRect* p_rect)
{
	// print text on horizontal lines
	TEXTMETRIC tm; // load characteristics of the font
	p_dc->GetTextMetrics(&tm);
	p_dc->SetBkMode(TRANSPARENT);

	// define display rectangle
	auto display_rect = *p_rect; // this will be the display rect for histogram
	display_rect.top += tm.tmHeight * 2 + tm.tmDescent;
	display_rect.bottom -= tm.tmHeight + 2 * tm.tmDescent;
	display_rect.left += 2 * tm.tmHeight;
	display_rect.right -= 2 * tm.tmHeight;

	// save old pen and brush / restore on exit
	CPen pen_bars;
	pen_bars.CreatePen(PS_SOLID, 0, m_pvdS->crHistBorder);
	CBrush brush_bars;
	brush_bars.CreateSolidBrush(m_pvdS->crHistFill);

	const auto p_old_pen = p_dc->SelectObject(&pen_bars);
	const auto p_old_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));

	// histogram type and bin value
	m_xfirst = m_timefirst; // abscissa first
	m_xlast = m_timelast; // abscissa last

	// update Peri-stimulus-Autocorrelation histogram rectangle
	auto i_ps_height = MulDiv(display_rect.Width(), 25, 100);
	auto i_autocorrelation_height = MulDiv(display_rect.Height(), 25, 100);
	if (i_ps_height < i_autocorrelation_height)
		i_autocorrelation_height = i_ps_height;
	else
		i_ps_height = i_autocorrelation_height;
	const auto separator = MulDiv(i_ps_height, 1, 15);

	const auto hist1_width = display_rect.Width() - i_autocorrelation_height - separator;
	const auto hist1_height = display_rect.Height() - i_ps_height - separator;
	auto rect_hist = display_rect;
	rect_hist.right = rect_hist.left + hist1_width;
	rect_hist.bottom = rect_hist.top + hist1_height;

	// search for max to adapt the scale
	auto max_val = 0;
	for (auto i = 0; i < m_sizeparrayISI; i++)
	{
		if (*(m_parrayISI + i) > max_val)
			max_val = *(m_parrayISI + i);
	}

	// display rectangle around the area with the intensity
	p_dc->MoveTo(rect_hist.left - 1, rect_hist.top - 1);
	p_dc->LineTo(rect_hist.right + 1, rect_hist.top - 1);
	p_dc->LineTo(rect_hist.right + 1, rect_hist.bottom + 1);
	p_dc->LineTo(rect_hist.left - 1, rect_hist.bottom + 1);
	p_dc->LineTo(rect_hist.left - 1, rect_hist.top - 1);
	const int y_middle = (rect_hist.top + rect_hist.bottom) / 2;
	p_dc->MoveTo(rect_hist.left - 1, y_middle);
	p_dc->LineTo(rect_hist.right + 1, y_middle);

	// display color as small rectangles
	auto d_rect = rect_hist;
	const auto d_rect_height = rect_hist.Height();
	const auto d_rect_width = rect_hist.Width();

	// loop over columns to pass over the different autocorrelation
	for (auto i_psth = 0; i_psth < m_sizepPSTH; i_psth++)
	{
		auto p_array = m_parrayISI + (i_psth * m_nbinsISI);
		d_rect.right = rect_hist.left + MulDiv((i_psth + 1), d_rect_width, m_sizepPSTH);
		d_rect.bottom = rect_hist.bottom;
		// loop over all time increments of the local autocorrelation histogram
		for (auto i_autocorrelation = 0; i_autocorrelation < m_sizepISI; i_autocorrelation++)
		{
			d_rect.top = rect_hist.bottom - MulDiv((i_autocorrelation + 1), d_rect_height, m_sizepISI);
			const int val = *p_array;
			p_array++;
			auto i_color = MulDiv(val, NB_COLORS, max_val);
			if (i_color > NB_COLORS) i_color = NB_COLORS;
			if (i_color > 0)
				p_dc->FillSolidRect(&d_rect, m_pvdS->crScale[i_color]);
			d_rect.bottom = d_rect.top;
		}
		d_rect.left = d_rect.right;
	}

	// display peri-stimulus histogram -----------------------------------
	auto peri_rect_hist = rect_hist;
	peri_rect_hist.right = peri_rect_hist.left + rect_hist.Width();
	peri_rect_hist.top = rect_hist.bottom + separator;
	peri_rect_hist.bottom = display_rect.bottom;

	plot_histogram(p_dc, &peri_rect_hist, m_sizepPSTH, m_pPSTH, 0, 0);

	// display autocorrelation -----------------------------------
	auto auto_rect_hist = rect_hist;
	auto_rect_hist.left = rect_hist.right + separator;
	auto_rect_hist.right = auto_rect_hist.left + peri_rect_hist.Height();

	plot_histogram(p_dc, &auto_rect_hist, m_sizepISI, m_pISI, -1, 2);

	// display colour scale
	const auto delta_x_pix = separator;
	const auto delta_y_pix = MulDiv(peri_rect_hist.Height(), 1, 18);
	d_rect.left = peri_rect_hist.right + delta_x_pix;
	d_rect.right = d_rect.left + 2 * delta_x_pix;
	d_rect.top = peri_rect_hist.top;
	const CPoint scale_top(d_rect.right, d_rect.top);

	for (auto j = 17; j >= 0; j--)
	{
		d_rect.bottom = d_rect.top + delta_y_pix; // update rectangle coordinates
		p_dc->FillSolidRect(&d_rect, m_pvdS->crScale[j]); // fill rectangle with color
		p_dc->MoveTo(d_rect.left - 1, d_rect.top - 1); // draw a horizontal bar
		p_dc->LineTo(d_rect.right, d_rect.top - 1); // at the top of the rectangle
		d_rect.top = d_rect.bottom; // update rectangle coordinates
	}

	p_dc->MoveTo(d_rect.left - 1, d_rect.bottom); // draw last bar on the bottom
	p_dc->LineTo(d_rect.right + 1, d_rect.bottom);

	p_dc->MoveTo(d_rect.left - 1, scale_top.y - 1); // draw left line
	p_dc->LineTo(d_rect.left - 1, d_rect.bottom);

	p_dc->MoveTo(d_rect.right, scale_top.y - 1); // draw right line
	p_dc->LineTo(d_rect.right, d_rect.bottom);
	// draw ticks
	p_dc->MoveTo(d_rect.right, scale_top.y + delta_y_pix - 1);
	p_dc->LineTo(d_rect.right + delta_x_pix, scale_top.y + delta_y_pix - 1);
	p_dc->MoveTo(d_rect.right, d_rect.bottom);
	p_dc->LineTo(d_rect.right + delta_x_pix, d_rect.bottom);

	// display comments
	auto comment_rect = display_rect; // comment rectangle
	if (m_bPrint) // adjust comment position if printing
		comment_rect = m_commentRect;
	else
		comment_rect.top -= tm.tmHeight * 2;

	CString str_comment; // scratch pad
	get_file_infos(str_comment); // file comments
	str_comment += _T("Peri-stimulus-Autocorrelation");
	auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	p_dc->DrawText(str_comment,
	               str_comment.GetLength(),
	               comment_rect,
	               DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	p_dc->SetTextAlign(ui_flag);

	// display abscissa of PSH
	CString cs_x_left;
	CString cs_x_right;
	cs_x_left.Format(_T("%1.3f"), m_timefirst);
	cs_x_right.Format(_T("%1.3f s"), m_timelast);
	p_dc->TextOut(peri_rect_hist.left, peri_rect_hist.bottom + tm.tmDescent, cs_x_left);
	const auto left = p_dc->GetTextExtent(cs_x_right);
	p_dc->TextOut(peri_rect_hist.right - left.cx - tm.tmDescent,
	              peri_rect_hist.bottom + tm.tmDescent, cs_x_right);

	// display scale max value
	CString cs_spk_per_s;
	cs_spk_per_s.Format(_T("%i spk / bin"), max_val);

	p_dc->SetTextAlign(TA_TOP | TA_LEFT | TA_NOUPDATECP);
	p_dc->TextOut(scale_top.x + delta_x_pix, scale_top.y + delta_y_pix - tm.tmDescent, cs_spk_per_s);

	ui_flag = p_dc->SetTextAlign(TA_BOTTOM | TA_LEFT | TA_NOUPDATECP);
	p_dc->TextOut(scale_top.x + delta_x_pix, d_rect.top + tm.tmDescent, _T("0"));
	p_dc->SetTextAlign(ui_flag);

	// display abscissa of autocorrelation
	// print value of ordinate max vertically
	CFont v_font; // vertical font
	LOGFONT log_v_font; // array describing font parameters
	memset(&log_v_font, 0, sizeof(LOGFONT)); // prepare font
	p_dc->GetCurrentFont()->GetLogFont(&log_v_font); // fill it from CDC
	log_v_font.lfEscapement = +900; // angle
	v_font.CreateFontIndirect(&log_v_font); // create font
	const auto p_old_font = p_dc->SelectObject(&v_font); // select font (now we can display txt)

	const auto bin_s = m_pvdS->binISI * 1000.f *static_cast<float>(m_nbinsISI) / 2.f;
	const auto left_x = auto_rect_hist.right + tm.tmDescent;
	CString cs_x_autocorrelation;
	cs_x_autocorrelation.Format(_T("%1.0f ms"), bin_s);

	p_dc->SetTextAlign(TA_TOP | TA_RIGHT | TA_NOUPDATECP); // max autocorrelation abscissa
	p_dc->TextOut(left_x, auto_rect_hist.top, cs_x_autocorrelation);

	cs_x_autocorrelation.Format(_T("-%1.0f"), bin_s);
	p_dc->SetTextAlign(TA_TOP | TA_LEFT | TA_NOUPDATECP); // min autocorrelation abscissa
	p_dc->TextOut(left_x, auto_rect_hist.bottom, cs_x_autocorrelation);

	p_dc->SetTextAlign(TA_TOP | TA_CENTER | TA_NOUPDATECP); // center autocorrelation abscissa
	p_dc->TextOut(left_x, (auto_rect_hist.bottom + auto_rect_hist.top) / 2, _T("0"));

	// end of vertical font...
	p_dc->SelectObject(p_old_font); // reselect old font
	v_font.DeleteObject(); // delete vertical font

	p_dc->SelectObject(p_old_pen);
	p_dc->SelectObject(p_old_brush);
	p_dc->SetBkMode(OPAQUE);
}

void ViewSpikeHist::display_stimulus(CDC* p_dc, const CRect* p_rect, const long* l_first, const long* l_last) const
{
	// draw rectangle for stimulus
	if (p_spike_doc_->m_stimulus_intervals.n_items <= 0)
		return;

	CPen blue_pen;
	blue_pen.CreatePen(PS_SOLID, 0, m_pvdS->crStimBorder);
	const auto p_old_p = p_dc->SelectObject(&blue_pen);

	// search first stimulus transition within interval
	const auto ii_start = *l_first;
	const auto ii_end = *l_last;
	const auto ii_len = ii_end - ii_start;
	auto i0 = 0;
	while (i0 < p_spike_doc_->m_stimulus_intervals.get_size()
		&& p_spike_doc_->m_stimulus_intervals.get_at(i0) < ii_start)
		i0++; // loop until found

	const auto display_len = p_rect->Width();
	int top = p_rect->top;
	int bottom = p_rect->bottom;
	if (top > bottom)
	{
		top = bottom;
		bottom = p_rect->top;
	}
	if ((bottom - top) < 2)
		bottom = top + 2;
	bottom++;

	// start looping from the first interval that meet the criteria
	// set baseline according to the interval (broken pulse?)
	auto istate = bottom; // use this variable to keep track of pulse broken by display limits
	auto ii = (i0 / 2) * 2; // keep index of the ON transition
	if (ii != i0)
		istate = top;
	p_dc->MoveTo(p_rect->left, istate);

	for (ii; ii < p_spike_doc_->m_stimulus_intervals.get_size(); ii++, ii++)
	{
		// stim starts here
		int iix0 = p_spike_doc_->m_stimulus_intervals.get_at(ii) - ii_start;
		if (iix0 >= ii_len) // first transition ON after last graph pt?
			break; // yes = exit loop

		if (iix0 < 0) // first transition off graph?
			iix0 = 0; // yes = clip

		iix0 = MulDiv(display_len, iix0, ii_len) + p_rect->left;
		p_dc->LineTo(iix0, istate); // draw line up to the first point of the pulse
		p_dc->LineTo(iix0, top); // draw vertical line to top of pulse

		// stim ends here
		istate = bottom; // after pulse, descend to bottom level
		int iix1 = ii_len;
		if (ii < p_spike_doc_->m_stimulus_intervals.get_size() - 1)
			iix1 = p_spike_doc_->m_stimulus_intervals.get_at(ii + 1) - ii_start;
		if (iix1 > ii_len) // last transition off graph?
		{
			iix1 = ii_len; // yes = clip
			istate = top; // do not descend..
		}

		iix1 = MulDiv(display_len, iix1, ii_len) + p_rect->left + 1;

		p_dc->LineTo(iix1, top); // draw top of pulse
		p_dc->LineTo(iix1, istate); // draw descent to bottom line
	}

	// end of loop - draw the rest
	p_dc->LineTo(p_rect->right, istate);
	p_dc->SelectObject(p_old_p);
}

void ViewSpikeHist::OnSelchangeHistogramtype()
{
	const auto i = static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->GetCurSel();
	if (m_bhistType == i)
		return;
	show_controls(i);
	build_data_and_display();
}

void ViewSpikeHist::OnEnChangeEditnstipercycle()
{
	m_pvdS->nstipercycle = GetDlgItemInt(IDC_EDITNSTIPERCYCLE);
	build_data_and_display();
}

void ViewSpikeHist::OnEnChangeEditlockonstim()
{
	if (p_spike_doc_ == nullptr)
		return;
	int ilock = GetDlgItemInt(IDC_EDITLOCKONSTIM);
	if (ilock != m_pvdS->istimulusindex)
	{
		if (ilock >= p_spike_doc_->m_stimulus_intervals.get_size())
			ilock = p_spike_doc_->m_stimulus_intervals.get_size() - 1;
		if (ilock < 0)
			ilock = 0;
		m_pvdS->istimulusindex = ilock;
		SetDlgItemInt(IDC_EDITLOCKONSTIM, m_pvdS->istimulusindex);
	}
	build_data_and_display();
}

void ViewSpikeHist::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1)) != pScrollBar)
		ViewDbTable::OnHScroll(nSBCode, nPos, pScrollBar);

	// Get the current position of scroll box.
	auto curpos = pScrollBar->GetScrollPos();
	float delta;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT: // Scroll to far left - next frame forwards
		delta = m_timelast - m_timefirst;
		break;
	case SB_RIGHT: // Scroll to far right.
		delta = m_timefirst - m_timelast;
		break;
	case SB_ENDSCROLL: // End scroll.
		delta = (m_timelast - m_timefirst) * (curpos - 50) / 100;
		break;
	case SB_LINELEFT: // Scroll left.
		delta = -2 * m_timebinms / t1000;
		break;
	case SB_LINERIGHT: // Scroll right.
		delta = 2 * m_timebinms / t1000;
		break;
	case SB_PAGELEFT: // Scroll one page left.
		delta = m_timefirst - m_timelast;
		break;
	case SB_PAGERIGHT: // Scroll one page right.
		delta = m_timelast - m_timefirst;
		break;
	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		curpos = nPos; // of the scroll box at the end of the drag operation.
		return; // no action
	case SB_THUMBTRACK: // Drag scroll box to specified position. nPos is the
		curpos = nPos; // position that the scroll box has been dragged to.
		return; // no action
	default:
		return;
	}

	// Set the new position of the thumb (scroll box).
	pScrollBar->SetScrollPos(50);
	const auto nbins = static_cast<int>(delta * t1000 / m_timebinms);
	delta = m_timebinms * nbins / t1000;
	m_timefirst += delta;
	m_timelast += delta;
	m_pvdS->timestart = m_timefirst;
	m_pvdS->timeend = m_timelast;
	build_data_and_display();
	UpdateData(FALSE);
}

void ViewSpikeHist::select_spk_list(int icur, BOOL b_refresh_interface)
{
	if (b_refresh_interface)
	{
		// reset tab control
		m_tabCtrl.DeleteAllItems();
		// load list of detection parameters
		auto j = 0;
		for (auto i = 0; i < p_spike_doc_->get_spike_list_size(); i++)
		{
			const auto p_spike_list = p_spike_doc_->set_spike_list_current_index(i);
			CString cs;
			if (p_spike_list->get_detection_parameters()->detect_what != DETECT_SPIKES)
				continue;
			cs.Format(_T("#%i %s"), i, (LPCTSTR)p_spike_list->get_detection_parameters()->comment);
			m_tabCtrl.InsertItem(j, cs);
			j++;
		}
	}

	// select spike list
	GetDocument()->get_current_spike_file()->set_spike_list_current_index(icur);
	m_tabCtrl.SetCurSel(icur);
}

void ViewSpikeHist::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	select_spk_list(icursel);
	build_data_and_display();
	*pResult = 0;
}

void ViewSpikeHist::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	select_spk_list(icursel);
	build_data_and_display();
	*pResult = 0;
}
