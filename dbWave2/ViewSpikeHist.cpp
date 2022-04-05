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

IMPLEMENT_DYNCREATE(ViewSpikeHist, dbTableView)

ViewSpikeHist::ViewSpikeHist()
	: dbTableView(IDD)
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
	return dbTableView::PreCreateWindow(cs);
}

void ViewSpikeHist::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);
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

BEGIN_MESSAGE_MAP(ViewSpikeHist, dbTableView)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_TIMEFIRST, &ViewSpikeHist::OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, &ViewSpikeHist::OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_EDIT4, &ViewSpikeHist::OnEnChangeTimebin)
	ON_EN_CHANGE(IDC_BINSIZE, &ViewSpikeHist::OnEnChangebinISI)
	ON_BN_CLICKED(IDC_CHECK1, &ViewSpikeHist::OnClickAllfiles)
	ON_BN_CLICKED(IDC_RADIOABSOLUTE, &ViewSpikeHist::OnabsoluteTime)
	ON_BN_CLICKED(IDC_RADIORELATIVE, &ViewSpikeHist::OnrelativeTime)
	ON_BN_CLICKED(IDC_RADIOONECLASS, &ViewSpikeHist::OnClickOneclass)
	ON_BN_CLICKED(IDC_RADIOALLCLASSES, &ViewSpikeHist::OnClickAllclasses)
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewSpikeHist::OnEnChangeSpikeclass)
	ON_EN_CHANGE(IDC_EDIT1, &ViewSpikeHist::OnEnChangenbins)
	ON_EN_CHANGE(IDC_EDIT3, &ViewSpikeHist::OnEnChangerowheight)
	ON_EN_CHANGE(IDC_EDIT2, &ViewSpikeHist::OnEnChangeDotheight)
	ON_COMMAND(ID_FORMAT_HISTOGRAM, &ViewSpikeHist::OnFormatHistogram)
	ON_BN_CLICKED(IDC_CHECK2, &ViewSpikeHist::OnClickCycleHist)
	ON_COMMAND(ID_EDIT_COPY, &ViewSpikeHist::OnEditCopy)
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
	m_pvdS = &(p_app->options_viewspikes); // get address of spike display options
	mdPM = &(p_app->options_viewdata); // printing options

	// create local fonts
	memset(&m_logFontDisp, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_logFontDisp.lfFaceName, _T("Arial")); // Arial font
	m_logFontDisp.lfHeight = 15; // font height
	/*BOOL flag = */
	m_fontDisp.CreateFontIndirect(&m_logFontDisp);

	// fill controls with initial values
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(m_pvdS->ballfiles);
	if (m_pvdS->ballfiles)
		m_nfiles = GetDocument()->GetDB_NRecords();
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
	showControls(m_pvdS->bhistType);

	// attach controls to stretch
	m_stretch.AttachParent(this); // attach formview pointer
	m_stretch.newProp(IDC_STATIC12, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	m_binit = TRUE;

	// init database and load documents
	dbTableView::OnInitialUpdate();

	const auto p_dbwave_doc = GetDocument();
	if (p_dbwave_doc->m_pSpk == nullptr)
	{
		p_dbwave_doc->m_pSpk = new CSpikeDoc;
		ASSERT(p_dbwave_doc->m_pSpk != NULL);
	}
	p_spike_doc_ = p_dbwave_doc->m_pSpk;
	p_spike_doc_->GetSpkList_Current();
	buildDataAndDisplay();
	selectSpkList(p_spike_doc_->GetSpkList_CurrentIndex(), TRUE);
}

void ViewSpikeHist::OnSize(UINT nType, int cx, int cy)
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
	dbTableView::OnSize(nType, cx, cy);
}

void ViewSpikeHist::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		auto p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW,
		                         reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		p_app->options_viewspikes.ballfiles = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	}
	dbTableView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void ViewSpikeHist::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_REQUERY:
		if (m_pvdS->ballfiles)
			m_nfiles = GetDocument()->GetDB_NRecords();
		else
			m_nfiles = 1;
		break;
	case HINT_DOCHASCHANGED: // file has changed?
	case HINT_DOCMOVERECORD:
		selectSpkList(GetDocument()->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex(), TRUE);
		buildDataAndDisplay();
		break;

	default:
		break;
	}
}

BOOL ViewSpikeHist::OnMove(UINT nIDMoveCommand)
{
	const auto flag = dbTableView::OnMove(nIDMoveCommand);
	auto p_document = GetDocument();
	if (p_document->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		return false;
	}

	p_document->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	if (!m_pvdS->ballfiles)
		buildDataAndDisplay();
	selectSpkList(GetDocument()->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex(), TRUE);
	return flag;
}

void ViewSpikeHist::OnDestroy()
{
	dbTableView::OnDestroy();
}

#ifdef _DEBUG
void ViewSpikeHist::AssertValid() const
{
	dbTableView::AssertValid();
}

void ViewSpikeHist::Dump(CDumpContext& dc) const
{
	dbTableView::Dump(dc);
}
#endif //_DEBUG


void ViewSpikeHist::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
		const auto timefirst = m_timefirst;
		switch (mm_timefirst.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_timefirst++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_timefirst--;
			break;
		default: ;
		}
		mm_timefirst.m_bEntryDone = FALSE;
		mm_timefirst.m_nChar = 0;
		mm_timefirst.SetSel(0, -1); //select all text
		if (m_timefirst > m_timelast)
		{
			m_timefirst = timefirst;
			MessageBeep(-1);
		}
		m_pvdS->timestart = m_timefirst;
		UpdateData(FALSE);
		if (timefirst != m_timefirst)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone)
	{
		const auto timelast = m_timelast;
		switch (mm_timelast.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR:
			m_timelast++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_timelast--;
			break;
		default: ;
		}
		mm_timelast.m_bEntryDone = FALSE;
		mm_timelast.m_nChar = 0;
		mm_timelast.SetSel(0, -1);
		if (m_timelast < m_timefirst)
		{
			m_timelast = timelast;
			MessageBeep(-1);
		}
		m_pvdS->timeend = m_timelast;
		UpdateData(FALSE);
		if (timelast != m_timelast)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnEnChangeTimebin()
{
	if (mm_timebinms.m_bEntryDone)
	{
		const auto binms = m_timebinms;
		switch (mm_timebinms.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_timebinms++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_timebinms--;
			break;
		default: ;
		}
		mm_timebinms.m_bEntryDone = FALSE;
		mm_timebinms.m_nChar = 0;
		mm_timebinms.SetSel(0, -1);
		m_pvdS->timebin = m_timebinms / t1000;
		UpdateData(FALSE);
		if (binms != m_timebinms)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnEnChangebinISI()
{
	if (mm_binISIms.m_bEntryDone)
	{
		const auto binms = m_binISIms;
		switch (mm_binISIms.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_binISIms++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_binISIms--;
			break;
		default: ;
		}
		mm_binISIms.m_bEntryDone = FALSE;
		mm_binISIms.m_nChar = 0;
		mm_binISIms.SetSel(0, -1);
		m_pvdS->binISI = m_binISIms / t1000;
		UpdateData(FALSE);
		if (binms != m_binISIms)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnEnChangeSpikeclass()
{
	if (mm_spikeclass.m_bEntryDone)
	{
		const auto spikeclassoption = m_spikeclass;
		switch (mm_spikeclass.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_spikeclass++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_spikeclass--;
			break;
		default: ;
		}
		mm_spikeclass.m_bEntryDone = FALSE;
		mm_spikeclass.m_nChar = 0;
		mm_spikeclass.SetSel(0, -1); //select all text
		m_pvdS->classnb = m_spikeclass;
		UpdateData(FALSE);
		if (spikeclassoption != m_spikeclass)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::buildDataAndDisplay()
{
	if (!m_binit)
		return;
	buildData(); // build histogram(s)
	m_bmodified = TRUE; // force display
	InvalidateRect(&m_displayRect); // display
}

void ViewSpikeHist::OnDraw(CDC* p_dc)
{
	CRect rect; // get coordinates of display area
	auto p_wnd = GetDlgItem(IDC_STATIC12); // get pointer to display static control
	p_wnd->GetClientRect(&rect); // get the final rect

	CDC dcMem; // prepare device context
	dcMem.CreateCompatibleDC(p_dc);

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
	auto poldbitmap = dcMem.SelectObject(m_pbitmap);

	if (m_bmodified) // replot only if flag is set
	{
		const auto isaved_dc = dcMem.SaveDC(); // save DC
		dcMem.Rectangle(rect); // erase window background
		CFont* pold_font = nullptr;
		if (!m_bPrint)
			pold_font = dcMem.SelectObject(&m_fontDisp);

		// call display routine according to selection
		switch (m_bhistType)
		{
		case 0:
		case 1:
		case 2:
			displayHistogram(&dcMem, &rect);
			break;
		case 3:
			displayDot(&dcMem, &rect);
			break;
		case 4:
			displayPSTHAutoc(&dcMem, &rect);
			break;
		default:
			break;
		}

		// restore parameters
		if (pold_font != nullptr)
			dcMem.SelectObject(pold_font);

		dcMem.RestoreDC(isaved_dc); // restore DC
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
	             &dcMem,
	             0,
	             0,
	             SRCCOPY);
	dcMem.SelectObject(poldbitmap); // release bitmap
}

void ViewSpikeHist::getFileInfos(CString& str_comment)
{
	if (m_nfiles == 1)
	{
		const CString tab("    "); // use 4 spaces as tabulation character
		const CString rc("\n"); // next line
		if (m_bPrint)
		{
			if (mdPM->bDocName || mdPM->bAcqDateTime) // print doc infos?
			{
				if (mdPM->bDocName) // print file name
				{
					const auto filename = GetDocument()->GetDB_CurrentSpkFileName(FALSE);
					str_comment += filename + tab;
				}
				if (mdPM->bAcqDateTime) // print data acquisition date & time
				{
					const auto date = (p_spike_doc_->GetAcqTime()).Format("%#d %m %Y %X"); //("%c");
					str_comment += date;
				}
				str_comment += rc;
			}
		}
	}
}

void ViewSpikeHist::OnClickAllfiles()
{
	if (static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck())
	{
		m_pvdS->ballfiles = TRUE;
		m_nfiles = GetDocument()->GetDB_NRecords();
	}
	else
	{
		m_pvdS->ballfiles = FALSE;
		m_nfiles = 1;
	}
	buildDataAndDisplay();
}

void ViewSpikeHist::OnClickCycleHist()
{
	if (static_cast<CButton*>(GetDlgItem(IDC_CHECK2))->GetCheck())
	{
		m_pvdS->bCycleHist = TRUE;
		m_pvdS->nstipercycle = GetDlgItemInt(IDC_EDITNSTIPERCYCLE);
		m_pvdS->istimulusindex = GetDlgItemInt(IDC_EDITLOCKONSTIM);
	}
	else
		m_pvdS->bCycleHist = FALSE;
	buildDataAndDisplay();
}

void ViewSpikeHist::OnClickOneclass()
{
	if (!m_pvdS->spikeclassoption)
	{
		m_pvdS->spikeclassoption = TRUE;
		(CWnd*)GetDlgItem(IDC_SPIKECLASS)->EnableWindow(TRUE);
		buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnClickAllclasses()
{
	if (m_pvdS->spikeclassoption)
	{
		m_pvdS->spikeclassoption = FALSE;
		(CWnd*)GetDlgItem(IDC_SPIKECLASS)->EnableWindow(FALSE);
		buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnabsoluteTime()
{
	if (!m_pvdS->babsolutetime)
	{
		m_pvdS->babsolutetime = TRUE;
		buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnrelativeTime()
{
	if (m_pvdS->babsolutetime)
	{
		m_pvdS->babsolutetime = FALSE;
		buildDataAndDisplay();
	}
}

void ViewSpikeHist::showControls(int iselect)
{
	m_bhistType = iselect;
	switch (iselect)
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

	int bsettings[5][11] = //1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	{
		{SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_HIDE},
		{SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW,SW_HIDE},
		{SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_SHOW},
		{SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE, SW_HIDE},
		{SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_SHOW}
	};

	int i = 0;
	GetDlgItem(IDC_STATIC7)->ShowWindow(bsettings[iselect][i]);
	i++; // 1  PSTH bin size (ms)
	GetDlgItem(IDC_EDIT4)->ShowWindow(bsettings[iselect][i]);
	i++; // 2  edit:  PSTH bin size (ms)

	GetDlgItem(IDC_BINSIZE)->ShowWindow(bsettings[iselect][i]);
	i++; // 3  Edit: ISI &bin size (ms) or
	GetDlgItem(IDC_STATIC3)->ShowWindow(bsettings[iselect][i]);
	i++; // 4  n bins
	GetDlgItem(IDC_EDIT1)->ShowWindow(bsettings[iselect][i]);
	i++; // 5  Edit: nbins

	GetDlgItem(IDC_STATIC5)->ShowWindow(bsettings[iselect][i]);
	i++; // 6  dot height
	GetDlgItem(IDC_EDIT2)->ShowWindow(bsettings[iselect][i]);
	i++; // 7  Edit: dot height
	GetDlgItem(IDC_STATIC6)->ShowWindow(bsettings[iselect][i]);
	i++; // 8  separator height
	GetDlgItem(IDC_EDIT3)->ShowWindow(bsettings[iselect][i]);
	i++; // 9  Edit: separator height

	GetDlgItem(IDC_STATIC2)->ShowWindow(bsettings[iselect][i]);
	i++; // 10 ISI &bin size (ms)
	GetDlgItem(IDC_STATIC13)->ShowWindow(bsettings[iselect][i]); /*i++;*/ // 11 autoc. &bin size (ms)
}

void ViewSpikeHist::OnEnChangenbins()
{
	if (mm_nbinsISI.m_bEntryDone)
	{
		const auto nbins = m_nbinsISI;
		switch (mm_nbinsISI.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_nbinsISI++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_nbinsISI--;
			break;
		default: ;
		}
		mm_nbinsISI.m_bEntryDone = FALSE;
		mm_nbinsISI.m_nChar = 0;
		mm_nbinsISI.SetSel(0, -1); //select all text
		m_pvdS->nbinsISI = m_nbinsISI;
		UpdateData(FALSE);
		if (nbins != m_nbinsISI)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnEnChangerowheight()
{
	if (mm_rowheight.m_bEntryDone)
	{
		const auto rowheight = m_rowheight;
		switch (mm_rowheight.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_rowheight++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_rowheight--;
			break;
		default: ;
		}
		mm_rowheight.m_bEntryDone = FALSE;
		mm_rowheight.m_nChar = 0;
		mm_rowheight.SetSel(0, -1); //select all text
		m_pvdS->dotlineheight = m_rowheight + m_pvdS->dotheight;
		UpdateData(FALSE);
		if (rowheight != m_rowheight)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnEnChangeDotheight()
{
	if (mm_dotheight.m_bEntryDone)
	{
		const auto dotheight = m_dotheight;
		switch (mm_dotheight.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); // load data from edit controls
			break;
		case VK_UP:
		case VK_PRIOR:
			m_dotheight++;
			break;
		case VK_DOWN:
		case VK_NEXT:
			m_dotheight--;
			break;
		default: ;
		}
		mm_dotheight.m_bEntryDone = FALSE;
		mm_dotheight.m_nChar = 0;
		mm_dotheight.SetSel(0, -1); //select all text
		m_pvdS->dotheight = m_dotheight;
		m_pvdS->dotlineheight = m_rowheight + m_dotheight;
		UpdateData(FALSE);
		if (dotheight != m_dotheight)
			buildDataAndDisplay();
	}
}

void ViewSpikeHist::OnEditCopy()
{
	// create metafile
	CMetaFileDC m_dc;

	// size of the window
	CRect rect_bound, rect;
	auto p_wnd = GetDlgItem(IDC_STATIC12); // get pointer to display static control
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
	CClientDC attrib_dc(this); // Create and attach attribute DC
	m_dc.SetAttribDC(attrib_dc.GetSafeHdc()); // from current screen

	// display curves
	// call display routine according to selection
	switch (m_bhistType)
	{
	case 0:
	case 1:
	case 2:
		displayHistogram(&m_dc, &rect);
		break;
	case 3:
		displayDot(&m_dc, &rect);
		break;
	case 4:
		displayPSTHAutoc(&m_dc, &rect);
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

BOOL ViewSpikeHist::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	// printing margins
	if (mdPM->vertRes <= 0 || mdPM->horzRes <= 0
		|| mdPM->horzRes != pInfo->m_rectDraw.Width()
		|| mdPM->vertRes != pInfo->m_rectDraw.Height())
	{
		// compute printer's page dot resolution
		CPrintDialog dlg(FALSE); // borrowed from VC++ sample\drawcli\drawdoc.cpp
		VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));
		CDC dc; // GetPrinterDC returns a HDC so attach it
		const auto h_dc = dlg.CreatePrinterDC();
		ASSERT(h_dc != NULL);
		dc.Attach(h_dc);
		// Get the size of the page in pixels
		mdPM->horzRes = dc.GetDeviceCaps(HORZRES);
		mdPM->vertRes = dc.GetDeviceCaps(VERTRES);
	}

	// how many rows per page?
	const auto size_row = mdPM->HeightDoc + mdPM->heightSeparator;
	auto nbrowsperpage = (mdPM->vertRes - 2 * mdPM->topPageMargin) / size_row;
	auto nfiles = 1;
	if (m_nfiles == 1)
		nfiles = GetDocument()->GetDB_NRecords();

	if (nbrowsperpage == 0) // prevent zero pages
		nbrowsperpage = 1;
	auto npages = nfiles / nbrowsperpage;
	if (nfiles > nbrowsperpage * npages)
		npages++;

	pInfo->SetMaxPage(npages); // one page printing/preview
	pInfo->m_nNumPreviewPages = 1; // preview 1 pages at a time
	// allow print only selection
	if (mdPM->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION;
	else
		pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;

	// call dialog box
	const auto flag = DoPreparePrinting(pInfo);
	// set max nb of pages according to selection
	mdPM->bPrintSelection = pInfo->m_pPD->PrintSelection();
	if (mdPM->bPrintSelection)
		pInfo->SetMaxPage(1);
	return flag;
}

void ViewSpikeHist::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	// select font, set print flag, save current file index
	const auto p_old_font = p_dc->SelectObject(&m_fontPrint);

	m_bPrint = TRUE;
	const int file0 = GetDocument()->GetDB_CurrentRecordPosition();

	// print page footer: file path, page number/total pages, date
	auto t = CTime::GetCurrentTime(); // current date & time
	CString cs_footer; // first string to receive
	cs_footer.Format(_T("  page %d:%d %d-%d-%d"), // page and time infos
	                 pInfo->m_nCurPage, pInfo->GetMaxPage(),
	                 t.GetDay(), t.GetMonth(), t.GetYear());
	CString ch_date = GetDocument()->GetDB_CurrentSpkFileName(FALSE);
	ch_date = ch_date.Left(ch_date.GetLength() - 1) + cs_footer;
	p_dc->SetTextAlign(TA_CENTER); // and print the footer
	p_dc->TextOut(mdPM->horzRes / 2, mdPM->vertRes - 57, ch_date);

	// define page rectangle (where data and comments are plotted)
	CRect rect_page; // = pInfo->m_rectDraw;
	//rect_page.right = mdPM->horzRes-mdPM->rightPageMargin;
	//rect_page.bottom = mdPM->vertRes-mdPM->bottomPageMargin;
	rect_page.left = mdPM->leftPageMargin;
	rect_page.top = mdPM->topPageMargin;

	// define data file rectangle - position of the first file
	const auto r_width = mdPM->WidthDoc; // margins
	const auto r_height = mdPM->HeightDoc; // margins
	CRect r_where(rect_page.left, rect_page.top,
	              rect_page.left + r_width, rect_page.top + r_height);

	// prepare file loop
	auto p_dbwave_doc = GetDocument();
	/*int nfiles = */
	p_dbwave_doc->GetDB_NRecords();
	const auto size_row = mdPM->HeightDoc + mdPM->heightSeparator; // size of one row
	auto nbrowsperpage = pInfo->m_rectDraw.Height() / size_row; // nb of rows per page
	if (nbrowsperpage == 0)
		nbrowsperpage = 1;
	const int file1 = (pInfo->m_nCurPage - 1) * nbrowsperpage; // index first file
	auto file2 = file1 + nbrowsperpage; // index last file
	if (m_nfiles != 1) // special case: all together
		file2 = file1 + 1;
	if (file2 > p_dbwave_doc->GetDB_NRecords())
		file2 = p_dbwave_doc->GetDB_NRecords();

	// loop through all files
	for (auto ifile = file1; ifile < file2; ifile++)
	{
		if (mdPM->bFrameRect) // print data rect if necessary
		{
			p_dc->MoveTo(r_where.left, r_where.top);
			p_dc->LineTo(r_where.right, r_where.top);
			p_dc->LineTo(r_where.right, r_where.bottom);
			p_dc->LineTo(r_where.left, r_where.bottom);
			p_dc->LineTo(r_where.left, r_where.top);
		}
		m_commentRect = r_where; // calculate where the comments will be printed
		m_commentRect.OffsetRect(mdPM->textseparator + m_commentRect.Width(), 0);
		m_commentRect.right = pInfo->m_rectDraw.right;
		// refresh data if necessary
		if (m_nfiles == 1) //??? (m_nfiles > 1)
		{
			p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
			buildData();
		}
		// print the histogram
		switch (m_bhistType)
		{
		case 0:
		case 1:
		case 2:
			displayHistogram(p_dc, &r_where);
			break;
		case 3:
			displayDot(p_dc, &r_where);
			break;
		case 4:
			displayPSTHAutoc(p_dc, &r_where);
			break;
		default:
			break;
		}
		// update display rectangle for next row
		r_where.OffsetRect(0, r_height + mdPM->heightSeparator);
	}

	// restore parameters
	if (p_old_font != nullptr)
		p_dc->SelectObject(p_old_font);

	p_dbwave_doc->SetDB_CurrentRecordPosition(file0);
	p_spike_doc_ = p_dbwave_doc->OpenCurrentSpikeFile();
}

void ViewSpikeHist::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bPrint = FALSE;
	dbTableView::OnEndPrinting(p_dc, pInfo);
}

void ViewSpikeHist::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	memset(&m_logFont, 0, sizeof(LOGFONT)); // prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial")); // Arial font
	m_logFont.lfHeight = mdPM->fontsize; // font height
	m_fontPrint.CreateFontIndirect(&m_logFont);
	p_dc->SetBkMode(TRANSPARENT);
}

void ViewSpikeHist::OnFormatHistogram()
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
		buildDataAndDisplay();
	}
}

void ViewSpikeHist::buildData()
{
	auto nbins = static_cast<int>((m_timelast - m_timefirst) * t1000 / m_timebinms);
	if (nbins <= 0)
		nbins = 1;

	switch (m_bhistType)
	{
	case 0:
		m_nPSTH = 0;
		if (m_sizepPSTH != nbins) // make space for side PSTH
		{
			SAFE_DELETE_ARRAY(m_pPSTH);
			m_pPSTH = new long[nbins + 2];
			ASSERT(m_pPSTH != NULL);
			m_sizepPSTH = nbins;
			m_pvdS->nbins = nbins;
		}
		break;

	case 1:
	case 2:
		m_nISI = 0;
		if (m_sizepISI != m_nbinsISI) // make space for side autocorr
		{
			SAFE_DELETE_ARRAY(m_pISI);
			m_pISI = new long[m_nbinsISI + 2];
			ASSERT(m_pISI != NULL);
			m_sizepISI = m_nbinsISI;
			m_pvdS->nbinsISI = m_nbinsISI;
		}
		break;

	case 4:
		m_nPSTH = 0;
		if (m_sizepPSTH != nbins) // make space for side PSTH
		{
			SAFE_DELETE_ARRAY(m_pPSTH);
			m_pPSTH = new long[nbins + 2];
			ASSERT(m_pPSTH != NULL);
			m_sizepPSTH = nbins;
			m_pvdS->nbins = nbins;
		}

		if (m_sizepISI != m_nbinsISI) // make space for side autocorr
		{
			SAFE_DELETE_ARRAY(m_pISI);
			m_pISI = new long[m_nbinsISI + 2];
			ASSERT(m_pISI != NULL);
			m_sizepISI = m_nbinsISI;
			m_pvdS->nbinsISI = m_nbinsISI;
		}

		nbins = m_sizepPSTH * m_sizepISI; // total bins for main histogram array
		if (m_sizeparrayISI != nbins) // make space for side autocorr
		{
			SAFE_DELETE_ARRAY(m_parrayISI);
			m_parrayISI = new long[nbins + 2];
			ASSERT(m_parrayISI != NULL);
			m_sizeparrayISI = nbins;
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

	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
	auto firstfile = currentfile; // index first file in the series
	auto lastfile = currentfile; // index last file in the series

	DlgProgress* pdlg = nullptr;
	auto istep = 0;
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");

	if (m_nfiles > 1)
	{
		pdlg = new DlgProgress;
		pdlg->Create();
		pdlg->SetStep(1);
		firstfile = 0;
		lastfile = m_nfiles - 1;
	}

	auto currentlist_index = p_dbwave_doc->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();

	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		if (m_nfiles > 1)
		{
			// check if user wants to stop
			if (pdlg->CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cscomment.Format(_T("Processing file [%i / %i]"), ifile + 1, m_nfiles);
			pdlg->SetStatus(cscomment);
			if (MulDiv(ifile, 100, m_nfiles) > istep)
			{
				pdlg->StepIt();
				istep = MulDiv(ifile, 100, m_nfiles);
			}
		}

		// select spike file
		p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
		p_spike_doc_ = p_dbwave_doc->OpenCurrentSpikeFile();
		if (nullptr == p_spike_doc_)
			continue;

		p_spike_doc_->SetSpkList_AsCurrent(currentlist_index);

		switch (m_bhistType)
		{
		// -------------------------------------------------------------
		// PERIS-STIMULUS TIME HISTOGRAM
		// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
		// m_pvdS->timestart, m_pvdS->timeend
		// m_pvdS->timebin, m_pvdS->nbins
		case 0:
			m_nPSTH += p_spike_doc_->BuildPSTH(m_pvdS, m_pPSTH, m_spikeclass);
			break;
		// -------------------------------------------------------------
		// INTER-SPIKES INTERVALS HISTOGRAM
		// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
		// m_pvdS->timestart, m_pvdS->timeend
		// m_pvdS->nbinsISI, m_pvdS->binISI
		case 1:
			m_nISI += p_spike_doc_->BuildISI(m_pvdS, m_pISI, m_spikeclass);
			break;
		// -------------------------------------------------------------
		// AUTOCORRELATION
		// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
		// m_pvdS->timestart, m_pvdS->timeend
		// m_pvdS->nbinsISI, m_pvdS->binISI
		case 2:
			m_nISI += p_spike_doc_->BuildAUTOCORR(m_pvdS, m_pISI, m_spikeclass);
			break;
		// -------------------------------------------------------------
		// PSTH-AUTOCORRELATION DENSITY
		// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
		// m_pvdS->nbinsISI, m_pvdS->binISI
		// m_pvdS->timestart, m_pvdS->timeend
		// m_pvdS->timebin, m_pvdS->nbins
		case 4:
			{
				m_nPSTH += p_spike_doc_->BuildPSTH(m_pvdS, m_pPSTH, m_spikeclass);
				p_spike_doc_->BuildAUTOCORR(m_pvdS, m_pISI, m_spikeclass);
				p_spike_doc_->BuildPSTHAUTOCORR(m_pvdS, m_parrayISI, m_spikeclass);
			}
			break;
		default:
			break;
		}
	}

	if (currentfile != p_dbwave_doc->GetDB_CurrentRecordPosition())
	{
		p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
		p_spike_doc_ = p_dbwave_doc->OpenCurrentSpikeFile();
		p_spike_doc_->SetSpkList_AsCurrent(currentlist_index);
	}
	SAFE_DELETE(pdlg)
}

// plot histogram (generic)
//
// input (passed parameters):
//		p_dc			pointer to DC context
//		dispRect	display rectangle (plotting area)
//		nbinshistog	number of bins within the histogram
//		phistog		pointer to an array of long with the values of the histogram
//		orientation	0=0 deg, +1=+90 deg; -1=-90 deg; +2,-2=-180 degrees
//		btype	type of display histogram (0=PSTH, 1=ISI, 2=autoc)

// input (internal parameters):
//		p_option_viewspikes		array with spike display parameters OPTIONS_VIEWSPIKES
//		m_pvdS->crHistFill	histogram COLORREF fill color
//		m_pvdS->crHistBorder histogram border color
//		m_pvdS->crChartArea background plane color
//		m_pvdS->bYmaxAuto	flag:on=search max off=imposed max
//		m_timelast
//		m_timefirst
//		m_timebinms
//		t1000
//		m_xfirst
//		m_xlast
//		m_pvdS->babsolutetime
//		m_pSpkDoc->m_stimulus_intervals.n_items
//		m_pvdS->crStimFill
//		m_pvdS->crStimBorder
//		m_pSpkDoc->m_spklist
//		p_spk_list->GetAcqSampRate()
//
// output (passed parameter):
//		return max value of histogram
// output (modified internal parameters)
//		m_rectratio	 % of histogram height versus plotting area

long ViewSpikeHist::plotHistog(CDC* p_dc, CRect* pdispRect, int nbinshistog, long* phistog0, int orientation,
                                int btype)
{
	CPen penbars;
	penbars.CreatePen(PS_SOLID, 0, m_pvdS->crHistBorder);
	CBrush brushbars;
	brushbars.CreateSolidBrush(m_pvdS->crHistFill);

	// update chart area
	pdispRect->InflateRect(1, 1);
	p_dc->FillSolidRect(pdispRect, m_pvdS->crChartArea);

	const auto pold_pen = p_dc->SelectObject(&penbars); // rect fill
	p_dc->MoveTo(pdispRect->left, pdispRect->top);
	p_dc->LineTo(pdispRect->right, pdispRect->top);
	p_dc->LineTo(pdispRect->right, pdispRect->bottom);
	p_dc->LineTo(pdispRect->left, pdispRect->bottom);
	p_dc->LineTo(pdispRect->left, pdispRect->top);
	pdispRect->DeflateRect(1, 1);

	// display XY axis
	p_dc->SelectStockObject(BLACK_PEN);
	int xzero = pdispRect->left;
	CRect rect_hz;
	CRect rect_vert;

	switch (orientation)
	{
	case -1: // -90
		xzero = pdispRect->bottom;
		if (btype == 2)
			xzero = (pdispRect->bottom + pdispRect->top) / 2;
		rect_hz = CRect(pdispRect->left, xzero, pdispRect->right, xzero);
		rect_vert = CRect(pdispRect->right, pdispRect->top, pdispRect->right, pdispRect->bottom);
		break;
	case 1: // +90
		xzero = pdispRect->top;
		if (btype == 2)
			xzero = (pdispRect->bottom + pdispRect->top) / 2;
		rect_hz = CRect(pdispRect->left, xzero, pdispRect->right, xzero);
		rect_vert = CRect(pdispRect->left, pdispRect->top, pdispRect->left, pdispRect->bottom);

		break;
	case 2: // -180 deg
		xzero = pdispRect->left;
		if (btype == 2)
			xzero = (pdispRect->left + pdispRect->right) / 2;
		rect_hz = CRect(pdispRect->left, pdispRect->top, pdispRect->right, pdispRect->top);
		rect_vert = CRect(xzero, pdispRect->top, xzero, pdispRect->bottom);
		break;
	default: //	normal
		xzero = pdispRect->left;
		if (btype == 2)
			xzero = (pdispRect->left + pdispRect->right) / 2;
		rect_hz = CRect(pdispRect->left, pdispRect->bottom, pdispRect->right, pdispRect->bottom);
		rect_vert = CRect(xzero, pdispRect->top, xzero, pdispRect->bottom);
		break;
	}
	p_dc->MoveTo(rect_hz.left, rect_hz.top); // hz line
	p_dc->LineTo(rect_hz.right, rect_hz.bottom);
	p_dc->MoveTo(rect_vert.left, rect_vert.top); // vert line
	p_dc->LineTo(rect_vert.right, rect_vert.bottom);

	// set position of max (assuming hardwired 80% of max: rectratio=80)
	long max = 0;
	if (m_pvdS->bYmaxAuto)
	{
		auto phistog = phistog0;
		for (auto i = 0; i < nbinshistog; i++, phistog++)
		{
			const int val = *phistog; // load value in a temp variable
			if (val > max) // search max
				max = val;
		}
		m_rectratio = 80; // span only 80%
	}
	else
	{
		auto divisor = m_timebinms / t1000;
		if (btype > 0)
			divisor = m_timelast - m_timefirst;
		max = static_cast<long>(m_pvdS->Ymax * divisor * m_nfiles);
		m_rectratio = 100; // 100% span for data
	}
	auto rectmax = MulDiv(pdispRect->Height(), m_rectratio, 100);

	// prepare pen and brush
	p_dc->SelectObject(&penbars); // rect outer line
	const auto pold_brush = p_dc->SelectObject(&brushbars); // rect fill
	auto rect_bar = *pdispRect; // rectangle to plot the bars

	int displen;
	int ui;
	switch (orientation)
	{
	case -1: // rotation -90 deg
		displen = pdispRect->Height();
		rectmax = MulDiv(pdispRect->Width(), m_rectratio, 100);
		for (ui = 0; ui < m_sizepISI; ui++)
		{
			rect_bar.bottom = MulDiv(displen, ui + 1, nbinshistog) + pdispRect->top;
			rect_bar.left = rect_bar.right - MulDiv(*(phistog0 + ui), rectmax, max);
			p_dc->MoveTo(rect_bar.left, rect_bar.bottom);
			if (rect_bar.top != rect_bar.bottom)
				p_dc->Rectangle(rect_bar);
			else
				p_dc->LineTo(rect_bar.left, rect_bar.top);
			rect_bar.top = rect_bar.bottom;
		}
		break;

	case 1: // rotation +90 deg
		displen = pdispRect->Height();
		rectmax = MulDiv(pdispRect->Width(), m_rectratio, 100);
		for (ui = 0; ui < m_sizepISI; ui++)
		{
			rect_bar.bottom = MulDiv(displen, ui + 1, nbinshistog) + pdispRect->top;
			rect_bar.right = rect_bar.left + MulDiv(*(phistog0 + ui), rectmax, max);
			p_dc->MoveTo(rect_bar.left, rect_bar.bottom);
			if (rect_bar.top != rect_bar.bottom)
				p_dc->Rectangle(rect_bar);
			else
				p_dc->LineTo(rect_bar.left, rect_bar.top);
			rect_bar.top = rect_bar.bottom;
		}
		break;

	case 2:
		rectmax = MulDiv(pdispRect->Height(), m_rectratio, 100);
		displen = pdispRect->Width();
		for (ui = 0; ui < nbinshistog; ui++)
		{
			rect_bar.right = MulDiv(displen, ui + 1, nbinshistog) + pdispRect->left;
			rect_bar.bottom = rect_bar.top + MulDiv(*(phistog0 + ui), rectmax, max);
			p_dc->MoveTo(rect_bar.left, rect_bar.bottom);
			if (rect_bar.left != rect_bar.right)
				p_dc->Rectangle(rect_bar);
			else
				p_dc->LineTo(rect_bar.left, rect_bar.top);
			rect_bar.left = rect_bar.right;
		}
		break;

	default:
		displen = pdispRect->Width();
		rectmax = MulDiv(pdispRect->Height(), m_rectratio, 100);

	// display stimulus
		if (btype == 0 && p_spike_doc_->m_stimulus_intervals.n_items > 0)
		{
			const auto p_spk_list = p_spike_doc_->GetSpkList_Current();
			const auto samprate = p_spk_list->GetAcqSampRate();
			int iioffset0 = p_spike_doc_->m_stimulus_intervals.GetAt(m_pvdS->istimulusindex);
			if (m_pvdS->babsolutetime)
				iioffset0 = 0;

			// search first stimulus transition within interval
			auto iistart = static_cast<long>(m_pvdS->timestart * samprate) + iioffset0;
			auto iiend = static_cast<long>(m_pvdS->timeend * samprate) + iioffset0;

			TEXTMETRIC tm; // load characteristics of the font
			p_dc->GetTextMetrics(&tm);
			auto rect = *pdispRect;
			rect.bottom = rect.bottom + tm.tmHeight + tm.tmDescent - 2;
			rect.top = pdispRect->bottom + 2 * tm.tmDescent + 2;
			displayStim(p_dc, &rect, &iistart, &iiend);
		}

	// display histogram
		for (ui = 0; ui < nbinshistog; ui++)
		{
			rect_bar.right = MulDiv(displen, ui + 1, nbinshistog) + pdispRect->left;
			rect_bar.top = rect_bar.bottom - MulDiv(*(phistog0 + ui), rectmax, max);
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
	p_dc->SelectObject(pold_pen);
	p_dc->SelectObject(pold_brush);
	return max;
}

void ViewSpikeHist::displayDot(CDC* p_dc, CRect* pRect)
{
	CWaitCursor wait;

	// save old pen and brush / restore on exit
	CPen penbars;
	penbars.CreatePen(PS_SOLID, 0, RGB(0x80, 0x80, 0x80));
	const auto pold_pen = p_dc->SelectObject(&penbars);
	CBrush brushbars;
	brushbars.CreateSolidBrush(RGB(0x80, 0x80, 0x80));

	const auto pold_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));
	auto disp_rect = *pRect; // this will be the display rect for histogram
	disp_rect.left++;
	disp_rect.right--;

	// print comments
	TEXTMETRIC tm; // load characteristics of the font
	p_dc->GetTextMetrics(&tm);
	auto comment_rect = disp_rect;
	comment_rect.left += 4;
	comment_rect.top++;
	if (m_bPrint)
		comment_rect = m_commentRect;

	// output legends
	CString str_comment;
	getFileInfos(str_comment);

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
	disp_rect.top += tm.tmHeight * (line + 1) + tm.tmDescent;
	disp_rect.bottom -= tm.tmHeight + tm.tmDescent;
	disp_rect.left += tm.tmDescent;
	disp_rect.right -= tm.tmDescent;

	// print left abscissa value
	CString cs_xleft;
	cs_xleft.Format(_T("%1.3f"), m_xfirst);
	p_dc->TextOut(disp_rect.left, disp_rect.bottom, cs_xleft);

	// print right abscissa value + unit
	CString cs_xright;
	cs_xright.Format(_T("%1.3f s"), m_xlast);
	const auto left = p_dc->GetTextExtent(cs_xright);
	p_dc->TextOut(disp_rect.right - left.cx - tm.tmDescent, disp_rect.bottom, cs_xright);

	// display XY axis
	p_dc->SelectStockObject(BLACK_PEN);
	// abscissa
	p_dc->MoveTo(disp_rect.left, disp_rect.bottom);
	p_dc->LineTo(disp_rect.right, disp_rect.bottom);

	// set temp variables
	const int rcleft = disp_rect.left;
	const auto span = m_xlast - m_xfirst;
	const auto rectlen = disp_rect.Width();
	int row = disp_rect.top;
	const auto dotheight = MulDiv(pRect->Height(), m_pvdS->dotheight, 1000);
	const auto dotlineheight = MulDiv(pRect->Height(), m_pvdS->dotlineheight, 1000);
	const auto vt_top = dotheight + 1;
	const auto vt_bottom = dotlineheight - 2;

	// prepare loop / files (stop when no more space is available)
	auto p_dbwave_doc = GetDocument();
	const int currentfile = p_dbwave_doc->GetDB_CurrentRecordPosition(); // index current file
	auto firstfile = currentfile; // index first file in the series
	auto lastfile = firstfile; // index last file in the series
	if (m_nfiles > 1)
	{
		firstfile = 0;
		lastfile = m_nfiles - 1;
	}

	// external loop: browse from file to file
	auto currentlist_index = p_dbwave_doc->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
	for (auto ifile = firstfile;
	     ifile <= lastfile && row < disp_rect.bottom;
	     ifile++)
	{
		p_dbwave_doc->SetDB_CurrentRecordPosition(ifile);
		p_spike_doc_ = p_dbwave_doc->OpenCurrentSpikeFile();
		p_spike_doc_->SetSpkList_AsCurrent(currentlist_index);

		// load pointers to spike file and spike list
		const auto p_spk_list = p_spike_doc_->GetSpkList_Current();
		const auto samprate = p_spk_list->GetAcqSampRate();
		const auto ii_frame_first = static_cast<long>(m_timefirst * samprate);
		const auto ii_frame_last = static_cast<long>(m_timelast * samprate);
		const auto ii_frame_length = ii_frame_last - ii_frame_first;
		const auto nspikes = p_spk_list->GetTotalSpikes();

		// display spikes and stimuli either on one line or on multiple lines
		if (m_pvdS->babsolutetime)
		{
			auto nrows = 1;
			if (mdPM->bMultirowDisplay)
			{
				nrows = p_spike_doc_->GetAcqSize() / ii_frame_length;
				if (nrows * ii_frame_length < p_spike_doc_->GetAcqSize())
					nrows++;
			}
			auto ii_first = ii_frame_first;
			auto ii_last = ii_frame_last;
			auto ispikefirst = 0;
			for (auto irow = 0; irow < nrows; irow++)
			{
				// display stimuli
				if (p_spike_doc_->m_stimulus_intervals.n_items > 0)
				{
					CRect rect(rcleft, row + vt_bottom,
					           rectlen + rcleft, row + vt_top);
					displayStim(p_dc, &rect, &ii_first, &ii_last);
				}
				// display spikes
				auto iitime0 = -1;
				for (auto i = ispikefirst; i < nspikes; i++)
				{
					auto ii_time = p_spk_list->GetSpike(i)->get_time() - ii_first;
					// check if this spike should be processed
					// assume that spikes occurence times are ordered
					if (ii_time < 0)
						continue;
					if (ii_time > ii_frame_length)
					{
						ispikefirst = i;
						break;
					}
					// check spike class
					if (m_pvdS->spikeclassoption
						&& p_spk_list->GetSpike(i)->get_class() != m_spikeclass)
						continue;
					// convert interval into a pixel bin
					const auto spktime = ii_time / samprate;
					ii_time = static_cast<int>(spktime * rectlen / span) + rcleft;
					if (ii_time != iitime0) // avoid multiple drawing of the same dots
					{
						p_dc->MoveTo(ii_time, row);
						p_dc->LineTo(ii_time, row + dotheight);
						iitime0 = ii_time;
					}
				}

				// end loop - update parameters for next row
				ii_first += ii_frame_length;
				ii_last += ii_frame_length;
				row += dotlineheight;
			}
		}

		// display spikes in time locked to the stimuli
		else
		{
			// if !bCycleHist - only one pass is called
			const auto first_stim = m_pvdS->istimulusindex;
			auto last_stim = first_stim + 1;
			auto increment = 2;
			// if bCycleHist - one line per stimulus (or group of stimuli)
			if (m_pvdS->bCycleHist)
			{
				last_stim = p_spike_doc_->m_stimulus_intervals.GetSize();
				if (last_stim == 0)
					last_stim = 1;
				increment = m_pvdS->nstipercycle;
				if (p_spike_doc_->m_stimulus_intervals.n_per_cycle > 1
					&& increment > p_spike_doc_->m_stimulus_intervals.n_per_cycle)
					increment = p_spike_doc_->m_stimulus_intervals.n_per_cycle;
				increment *= 2;
			}

			// loop over stimuli
			for (auto istim = first_stim; istim < last_stim; istim += increment)
			{
				// compute temp parameters
				long istart;
				if (p_spike_doc_->m_stimulus_intervals.n_items > 0)
					istart = p_spike_doc_->m_stimulus_intervals.GetAt(istim);
				else
					istart = static_cast<long>(-(m_pvdS->timestart * samprate));

				istart += ii_frame_first;

				// draw dots -- look at all spikes...
				auto iitime0 = -1;
				for (auto i = 0; i < nspikes; i++)
				{
					auto ii_time = p_spk_list->GetSpike(i)->get_time() - istart;
					// check if this spike should be processed
					// assume that spikes occurence times are ordered
					if (ii_time < 0)
						continue;
					if (ii_time > ii_frame_length)
						break;
					// check spike class
					if (m_pvdS->spikeclassoption
						&& p_spk_list->GetSpike(i)->get_class() != m_spikeclass)
						continue;
					// convert interval into a pixel bin
					const auto spktime = ii_time / samprate;
					ii_time = static_cast<int>(spktime * rectlen / span) + rcleft;
					if (ii_time != iitime0) // avoid multiple drawing of the same dots
					{
						p_dc->MoveTo(ii_time, row);
						p_dc->LineTo(ii_time, row + dotheight);
						iitime0 = ii_time;
					}
				}
				// next stim set -- jump to next line
				row += dotlineheight;
			}

			if (p_spike_doc_->m_stimulus_intervals.n_items > 1)
			{
				// stimulus
				auto istart = p_spike_doc_->m_stimulus_intervals.GetAt(m_pvdS->istimulusindex);
				auto iend = ii_frame_last + istart;
				istart = ii_frame_first + istart;

				CRect rect(rcleft, row + dotlineheight - dotheight,
				           rectlen + rcleft, row + vt_bottom);
				displayStim(p_dc, &rect, &istart, &iend);
			}
		}

		// next file -- jump to next line..
		row += dotlineheight;
	}

	p_dbwave_doc->SetDB_CurrentRecordPosition(currentfile);
	p_spike_doc_ = p_dbwave_doc->OpenCurrentSpikeFile();
	p_spike_doc_->SetSpkList_AsCurrent(currentlist_index);

	p_dc->SelectObject(pold_pen);
	p_dc->SelectObject(pold_brush);
}

void ViewSpikeHist::displayHistogram(CDC* p_dc, CRect* pRect)
{
	auto disp_rect = *pRect; // this will be the display rect for histogram

	// print text on horizontal lines
	TEXTMETRIC tm; // load characteristics of the font
	p_dc->GetTextMetrics(&tm);
	p_dc->SetBkMode(TRANSPARENT);

	// define display rectangle and plot data within display area
	disp_rect.top += tm.tmHeight * 2 + tm.tmDescent;
	disp_rect.bottom -= tm.tmHeight + 2 * tm.tmDescent;
	disp_rect.left += 2 * tm.tmHeight;
	disp_rect.right -= 2 * tm.tmHeight;

	// get data pointer and size
	long* phistog0 = nullptr; // pointer to first element
	auto nbinshistog = 0; // nelements
	switch (m_bhistType)
	{
	case 0: // PSTH
		phistog0 = m_pPSTH; // pointer to first element
		nbinshistog = m_sizepPSTH;
		break;
	case 1: // ISI
	case 2: // AUTOCORR
		phistog0 = m_pISI; // pointer to first element
		nbinshistog = m_sizepISI;
		break;
	default:
		break;
	}
	// plot data
	const int histogmax = plotHistog(p_dc, &disp_rect, nbinshistog, phistog0, 0, m_bhistType);

	// output legends: title of the graph and data description
	auto comment_rect = disp_rect; // comment rectangle
	if (m_bPrint) // adjust comment position if printing
		comment_rect = m_commentRect;
	else
		comment_rect.top -= tm.tmHeight * 2; // + tm.tmDescent;

	CString str_comment; // scratch pad
	getFileInfos(str_comment); // file comments

	// histogram type and bin value
	auto divisor = 1.f; // factor to normalize histograms
	m_xfirst = m_timefirst; // abscissa first
	m_xlast = m_timelast; // abscissa last
	auto binms = 1.f;
	switch (m_bhistType)
	{
	case 0: // PSTH
		str_comment += _T("PSTH (");
		divisor = m_nPSTH * m_timebinms / t1000;
		binms = m_timebinms;
		break;
	case 1: // ISI
		str_comment += _T("ISI (");
		divisor = m_timelast - m_timefirst;
		m_xfirst = 0;
		m_xlast = m_binISIms / t1000 * m_nbinsISI;
		binms = m_binISIms;
		break;
	case 2: // AUTOCORR
		str_comment += _T("AUTOCORR (");
		divisor = m_timelast - m_timefirst;
		m_xfirst = -m_binISIms / t1000 * m_nbinsISI / 2;
		m_xlast = -m_xfirst;
		binms = m_binISIms;
		break;
	default:
		break;
	}
	CString cs_t2;
	cs_t2.Format(_T("bin:%1.1f ms)"), binms);
	str_comment += cs_t2;

	// display title (on 2 lines)
	const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	p_dc->DrawText(str_comment,
	               str_comment.GetLength(),
	               comment_rect,
	               DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	p_dc->SetTextAlign(ui_flag);

	// print abscissa first and last values
	CString cs_xleft;
	CString cs_xright;
	switch (m_bhistType)
	{
	case 1: // ISI
		cs_xleft = _T("0");
		cs_xright.Format(_T("%1.0f ms"), static_cast<double>(m_xlast) * t1000);
		break;
	case 2: // AUTOCORR
		cs_xleft.Format(_T("%1.0f"), static_cast<double>(m_xfirst) * t1000);
		cs_xright.Format(_T("%1.0f ms"), static_cast<double>(m_xlast) * t1000);
		break;
	default: // PSTH (default)
		cs_xleft.Format(_T("%1.3f"), m_xfirst);
		cs_xright.Format(_T("%1.3f s"), m_xlast);
		break;
	}

	// origin
	p_dc->TextOut(disp_rect.left, disp_rect.bottom + tm.tmDescent, cs_xleft);
	// end (align on the right)
	auto left = p_dc->GetTextExtent(cs_xright);
	p_dc->TextOut(disp_rect.right - left.cx - tm.tmDescent, disp_rect.bottom + tm.tmDescent, cs_xright);

	// print value of ordinate max vertically
	CFont v_font;
	LOGFONT logv_font;

	memset(&logv_font, 0, sizeof(LOGFONT)); // prepare font
	p_dc->GetCurrentFont()->GetLogFont(&logv_font); // fill it from CDC
	logv_font.lfEscapement = +900;
	v_font.CreateFontIndirect(&logv_font);
	const auto p_old_font = p_dc->SelectObject(&v_font);

	// compute histog max
	float ymax;
	if (m_pvdS->bYmaxAuto)
		ymax = (MulDiv(histogmax, m_rectratio, 100) / divisor) / m_nfiles;
	else
		ymax = m_pvdS->Ymax;

	CString csYmax;
	csYmax.Format(_T("%1.3f spk/s"), ymax);
	p_dc->GetTextMetrics(&tm);
	left = p_dc->GetTextExtent(csYmax);
	left.cy += tm.tmDescent;
	p_dc->TextOut(disp_rect.left - left.cy, disp_rect.top + left.cx, csYmax);
	p_dc->TextOut(disp_rect.left - left.cy, disp_rect.bottom, _T("0"));

	// restore normal font
	p_dc->SetBkMode(OPAQUE);
	p_dc->SelectObject(p_old_font);
	v_font.DeleteObject();
}

void ViewSpikeHist::displayPSTHAutoc(CDC* p_dc, CRect* pRect)
{
	// print text on horizontal lines
	TEXTMETRIC tm; // load characteristics of the font
	p_dc->GetTextMetrics(&tm);
	p_dc->SetBkMode(TRANSPARENT);

	// define display rectangle
	auto disp_rect = *pRect; // this will be the display rect for histogram
	disp_rect.top += tm.tmHeight * 2 + tm.tmDescent;
	disp_rect.bottom -= tm.tmHeight + 2 * tm.tmDescent;
	disp_rect.left += 2 * tm.tmHeight;
	disp_rect.right -= 2 * tm.tmHeight;

	// save old pen and brush / restore on exit
	CPen penbars;
	penbars.CreatePen(PS_SOLID, 0, m_pvdS->crHistBorder);
	CBrush brushbars;
	brushbars.CreateSolidBrush(m_pvdS->crHistFill);

	const auto pold_pen = p_dc->SelectObject(&penbars);
	const auto pold_brush = static_cast<CBrush*>(p_dc->SelectStockObject(BLACK_BRUSH));

	// histogram type and bin value
	m_xfirst = m_timefirst; // abscissa first
	m_xlast = m_timelast; // abscissa last

	// update Peristimulus-Autocorrelation histogram rectangle
	auto ipsheight = MulDiv(disp_rect.Width(), 25, 100);
	auto iautocheight = MulDiv(disp_rect.Height(), 25, 100);
	if (ipsheight < iautocheight)
		iautocheight = ipsheight;
	else
		ipsheight = iautocheight;
	const auto separator = MulDiv(ipsheight, 1, 15);

	const auto histog1_width = disp_rect.Width() - iautocheight - separator;
	const auto histog1_height = disp_rect.Height() - ipsheight - separator;
	auto rect_histog = disp_rect;
	rect_histog.right = rect_histog.left + histog1_width;
	rect_histog.bottom = rect_histog.top + histog1_height;

	// search for max to adapt the scale
	auto maxval = 0;
	for (auto i = 0; i < m_sizeparrayISI; i++)
	{
		if (*(m_parrayISI + i) > maxval)
			maxval = *(m_parrayISI + i);
	}

	// display rectangle around the area with the intensity
	p_dc->MoveTo(rect_histog.left - 1, rect_histog.top - 1);
	p_dc->LineTo(rect_histog.right + 1, rect_histog.top - 1);
	p_dc->LineTo(rect_histog.right + 1, rect_histog.bottom + 1);
	p_dc->LineTo(rect_histog.left - 1, rect_histog.bottom + 1);
	p_dc->LineTo(rect_histog.left - 1, rect_histog.top - 1);
	const int ymiddle = (rect_histog.top + rect_histog.bottom) / 2;
	p_dc->MoveTo(rect_histog.left - 1, ymiddle);
	p_dc->LineTo(rect_histog.right + 1, ymiddle);

	// display color as small rectangles
	auto drect = rect_histog;
	const auto drectheight = rect_histog.Height();
	const auto drectwidth = rect_histog.Width();

	// loop over columns to pass over the different autoc
	for (auto i_psth = 0; i_psth < m_sizepPSTH; i_psth++)
	{
		auto parray = m_parrayISI + (i_psth * m_nbinsISI);
		drect.right = rect_histog.left + MulDiv((i_psth + 1), drectwidth, m_sizepPSTH);
		drect.bottom = rect_histog.bottom;
		// loop over all time increments of the local autocorrelation histogram
		for (auto iautoc = 0; iautoc < m_sizepISI; iautoc++)
		{
			drect.top = rect_histog.bottom - MulDiv((iautoc + 1), drectheight, m_sizepISI);
			const int val = *parray;
			parray++;
			auto icolor = MulDiv(val, NB_COLORS, maxval);
			if (icolor > NB_COLORS) icolor = NB_COLORS;
			if (icolor > 0)
				p_dc->FillSolidRect(&drect, m_pvdS->crScale[icolor]);
			drect.bottom = drect.top;
		}
		drect.left = drect.right;
	}

	// display peristimulus histogram -----------------------------------
	auto peri_rect_histog = rect_histog;
	peri_rect_histog.right = peri_rect_histog.left + rect_histog.Width();
	peri_rect_histog.top = rect_histog.bottom + separator;
	peri_rect_histog.bottom = disp_rect.bottom;

	plotHistog(p_dc, &peri_rect_histog, m_sizepPSTH, m_pPSTH, 0, 0);

	// display autocorrelation -----------------------------------
	auto auto_rect_histog = rect_histog;
	auto_rect_histog.left = rect_histog.right + separator;
	auto_rect_histog.right = auto_rect_histog.left + peri_rect_histog.Height();

	plotHistog(p_dc, &auto_rect_histog, m_sizepISI, m_pISI, -1, 2);

	// display colour scale
	const auto delta_xpix = separator;
	const auto delta_ypix = MulDiv(peri_rect_histog.Height(), 1, 18);
	drect.left = peri_rect_histog.right + delta_xpix;
	drect.right = drect.left + 2 * delta_xpix;
	drect.top = peri_rect_histog.top;
	const CPoint scaletop(drect.right, drect.top);

	for (auto j = 17; j >= 0; j--)
	{
		drect.bottom = drect.top + delta_ypix; // update rectangle coordinates
		p_dc->FillSolidRect(&drect, m_pvdS->crScale[j]); // fill rectangle with color
		p_dc->MoveTo(drect.left - 1, drect.top - 1); // draw a horizontal bar
		p_dc->LineTo(drect.right, drect.top - 1); // at the top of the rectangle
		drect.top = drect.bottom; // update rectangle coordinates
	}

	p_dc->MoveTo(drect.left - 1, drect.bottom); // draw last bar on the bottom
	p_dc->LineTo(drect.right + 1, drect.bottom);

	p_dc->MoveTo(drect.left - 1, scaletop.y - 1); // draw left line
	p_dc->LineTo(drect.left - 1, drect.bottom);

	p_dc->MoveTo(drect.right, scaletop.y - 1); // draw right line
	p_dc->LineTo(drect.right, drect.bottom);
	// draw ticks
	p_dc->MoveTo(drect.right, scaletop.y + delta_ypix - 1);
	p_dc->LineTo(drect.right + delta_xpix, scaletop.y + delta_ypix - 1);
	p_dc->MoveTo(drect.right, drect.bottom);
	p_dc->LineTo(drect.right + delta_xpix, drect.bottom);

	// display comments
	auto comment_rect = disp_rect; // comment rectangle
	if (m_bPrint) // adjust comment position if printing
		comment_rect = m_commentRect;
	else
		comment_rect.top -= tm.tmHeight * 2;

	CString str_comment; // scratch pad
	getFileInfos(str_comment); // file comments
	str_comment += _T("Peristimulus-Autocorrelation");
	auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	p_dc->DrawText(str_comment,
	               str_comment.GetLength(),
	               comment_rect,
	               DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	p_dc->SetTextAlign(ui_flag);

	// display abscissa of PSH
	CString cs_xleft;
	CString cs_xright;
	cs_xleft.Format(_T("%1.3f"), m_timefirst);
	cs_xright.Format(_T("%1.3f s"), m_timelast);
	p_dc->TextOut(peri_rect_histog.left, peri_rect_histog.bottom + tm.tmDescent, cs_xleft);
	const auto left = p_dc->GetTextExtent(cs_xright);
	p_dc->TextOut(peri_rect_histog.right - left.cx - tm.tmDescent,
	              peri_rect_histog.bottom + tm.tmDescent, cs_xright);

	// display scale max value
	CString cs_spkpers;
	cs_spkpers.Format(_T("%i spk / bin"), maxval);

	p_dc->SetTextAlign(TA_TOP | TA_LEFT | TA_NOUPDATECP);
	p_dc->TextOut(scaletop.x + delta_xpix, scaletop.y + delta_ypix - tm.tmDescent, cs_spkpers);

	ui_flag = p_dc->SetTextAlign(TA_BOTTOM | TA_LEFT | TA_NOUPDATECP);
	p_dc->TextOut(scaletop.x + delta_xpix, drect.top + tm.tmDescent, _T("0"));
	p_dc->SetTextAlign(ui_flag);

	// display abscissa of autocorrelation
	// print value of ordinate max vertically
	CFont v_font; // vertical font
	LOGFONT logv_font; // array describing font parameters
	memset(&logv_font, 0, sizeof(LOGFONT)); // prepare font
	p_dc->GetCurrentFont()->GetLogFont(&logv_font); // fill it from CDC
	logv_font.lfEscapement = +900; // angle
	v_font.CreateFontIndirect(&logv_font); // create font
	const auto p_old_font = p_dc->SelectObject(&v_font); // select font (now we can display txt)

	const auto bin_s = m_pvdS->binISI * 1000.f * m_nbinsISI / 2.f;
	const auto leftx = auto_rect_histog.right + tm.tmDescent;
	CString cs_xautoc;
	cs_xautoc.Format(_T("%1.0f ms"), bin_s);

	p_dc->SetTextAlign(TA_TOP | TA_RIGHT | TA_NOUPDATECP); // max autoc abscissa
	p_dc->TextOut(leftx, auto_rect_histog.top, cs_xautoc);

	cs_xautoc.Format(_T("-%1.0f"), bin_s);
	p_dc->SetTextAlign(TA_TOP | TA_LEFT | TA_NOUPDATECP); // min autoc abscissa
	p_dc->TextOut(leftx, auto_rect_histog.bottom, cs_xautoc);

	p_dc->SetTextAlign(TA_TOP | TA_CENTER | TA_NOUPDATECP); // center autoc abscissa
	p_dc->TextOut(leftx, (auto_rect_histog.bottom + auto_rect_histog.top) / 2, _T("0"));

	// end of vertical font...
	p_dc->SelectObject(p_old_font); // reselect old font
	v_font.DeleteObject(); // delete vertical font

	p_dc->SelectObject(pold_pen);
	p_dc->SelectObject(pold_brush);
	p_dc->SetBkMode(OPAQUE);
}

void ViewSpikeHist::displayStim(CDC* p_dc, CRect* pRect, long* l_first, long* l_last)
{
	// draw rectangle for stimulus
	if (p_spike_doc_->m_stimulus_intervals.n_items <= 0)
		return;

	CPen bluepen;
	bluepen.CreatePen(PS_SOLID, 0, m_pvdS->crStimBorder);
	const auto pold_p = p_dc->SelectObject(&bluepen);

	// search first stimulus transition within interval
	const auto iistart = *l_first;
	const auto iiend = *l_last;
	const auto iilen = iiend - iistart;
	auto i0 = 0;
	while (i0 < p_spike_doc_->m_stimulus_intervals.GetSize()
		&& p_spike_doc_->m_stimulus_intervals.GetAt(i0) < iistart)
		i0++; // loop until found

	const auto displen = pRect->Width();
	int top = pRect->top;
	int bottom = pRect->bottom;
	if (top > bottom)
	{
		top = bottom;
		bottom = pRect->top;
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
	p_dc->MoveTo(pRect->left, istate);

	for (ii; ii < p_spike_doc_->m_stimulus_intervals.GetSize(); ii++, ii++)
	{
		// stim starts here
		int iix0 = p_spike_doc_->m_stimulus_intervals.GetAt(ii) - iistart;
		if (iix0 >= iilen) // first transition ON after last graph pt?
			break; // yes = exit loop

		if (iix0 < 0) // first transition off graph?
			iix0 = 0; // yes = clip

		iix0 = MulDiv(displen, iix0, iilen) + pRect->left;
		p_dc->LineTo(iix0, istate); // draw line up to the first point of the pulse
		p_dc->LineTo(iix0, top); // draw vertical line to top of pulse

		// stim ends here
		istate = bottom; // after pulse, descend to bottom level
		int iix1 = iilen;
		if (ii < p_spike_doc_->m_stimulus_intervals.GetSize() - 1)
			iix1 = p_spike_doc_->m_stimulus_intervals.GetAt(ii + 1) - iistart;
		if (iix1 > iilen) // last transition off graph?
		{
			iix1 = iilen; // yes = clip
			istate = top; // do not descend..
		}

		iix1 = MulDiv(displen, iix1, iilen) + pRect->left + 1;

		p_dc->LineTo(iix1, top); // draw top of pulse
		p_dc->LineTo(iix1, istate); // draw descent to bottom line
	}

	// end of loop - draw the rest
	p_dc->LineTo(pRect->right, istate);
	p_dc->SelectObject(pold_p);
}

void ViewSpikeHist::OnSelchangeHistogramtype()
{
	const auto i = static_cast<CListBox*>(GetDlgItem(IDC_LIST1))->GetCurSel();
	if (m_bhistType == i)
		return;
	showControls(i);
	buildDataAndDisplay();
}

void ViewSpikeHist::OnEnChangeEditnstipercycle()
{
	m_pvdS->nstipercycle = GetDlgItemInt(IDC_EDITNSTIPERCYCLE);
	buildDataAndDisplay();
}

void ViewSpikeHist::OnEnChangeEditlockonstim()
{
	if (p_spike_doc_ == nullptr)
		return;
	int ilock = GetDlgItemInt(IDC_EDITLOCKONSTIM);
	if (ilock != m_pvdS->istimulusindex)
	{
		if (ilock >= p_spike_doc_->m_stimulus_intervals.GetSize())
			ilock = p_spike_doc_->m_stimulus_intervals.GetSize() - 1;
		if (ilock < 0)
			ilock = 0;
		m_pvdS->istimulusindex = ilock;
		SetDlgItemInt(IDC_EDITLOCKONSTIM, m_pvdS->istimulusindex);
	}
	buildDataAndDisplay();
}

void ViewSpikeHist::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (static_cast<CScrollBar*>(GetDlgItem(IDC_SCROLLBAR1)) != pScrollBar)
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);

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
	buildDataAndDisplay();
	UpdateData(FALSE);
}

void ViewSpikeHist::selectSpkList(int icur, BOOL bRefreshInterface)
{
	if (bRefreshInterface)
	{
		// reset tab control
		m_tabCtrl.DeleteAllItems();
		// load list of detection parameters
		auto j = 0;
		for (auto i = 0; i < p_spike_doc_->GetSpkList_Size(); i++)
		{
			const auto p_spike_list = p_spike_doc_->SetSpkList_AsCurrent(i);
			CString cs;
			if (p_spike_list->GetDetectParms()->detectWhat != DETECT_SPIKES)
				continue;
			cs.Format(_T("#%i %s"), i, (LPCTSTR)p_spike_list->GetDetectParms()->comment);
			m_tabCtrl.InsertItem(j, cs);
			j++;
		}
	}

	// select spike list
	GetDocument()->GetCurrent_Spk_Document()->SetSpkList_AsCurrent(icur);
	m_tabCtrl.SetCurSel(icur);
}

void ViewSpikeHist::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	selectSpkList(icursel);
	buildDataAndDisplay();
	*pResult = 0;
}

void ViewSpikeHist::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	selectSpkList(icursel);
	buildDataAndDisplay();
	*pResult = 0;
}
