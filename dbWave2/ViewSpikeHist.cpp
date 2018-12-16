// SpikeHistView.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave.h"
#include "resource.h"

#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"

#include "Spikedoc.h"
#include "FormatHistogram.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ProgDlg.h"
#include ".\ViewSpikeHist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeHist

IMPLEMENT_DYNCREATE(CViewSpikeHist, CDaoRecordView)

// -------------------------------------------------------------------------

CViewSpikeHist::CViewSpikeHist()
	: CDaoRecordView(CViewSpikeHist::IDD), m_pvdS(nullptr), mdPM(nullptr), m_bhistType(0), m_scrollFilePos_infos(),
	  m_nPSTH(0), m_nISI(0), m_logFont(), m_xfirst(0), m_xlast(0), m_logFontDisp()
{
	m_timefirst = 0.0f;
	m_timelast = 2.5f;
	m_spikeclass = 0;
	m_dotheight = 0;
	m_rowheight = 0;
	m_binISIms = 0.0f;
	m_nbinsISI = 0;
	m_timebinms = 0.0f;
	m_parrayISI = nullptr;
	m_pPSTH = nullptr;
	m_pISI = nullptr;
	m_sizepPSTH = 0;
	m_sizepISI = 0;
	m_sizeparrayISI = 0;
	m_initiated = FALSE;
	m_pbitmap = nullptr;
	m_displayRect = CRect(0, 0, 0, 0);
	m_bmodified = TRUE;
	m_nfiles = 1;
	t1000 = 1000.f;
	m_bPrint = FALSE;
	m_rectratio = 100;
	m_pSpkDoc = nullptr;
	m_binit = FALSE;
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

CViewSpikeHist::~CViewSpikeHist()
{
	if (m_pPSTH != nullptr && m_sizepPSTH != 0)
		delete [] m_pPSTH;
	if (m_pISI != nullptr && m_sizepISI != 0)
		delete [] m_pISI;
	if (m_parrayISI != nullptr && m_sizeparrayISI != NULL)
		delete [] m_parrayISI;
	if (m_pbitmap != nullptr)
		delete m_pbitmap;
	m_fontDisp.DeleteObject();	
}

BOOL CViewSpikeHist::PreCreateWindow(CREATESTRUCT &cs)
{
// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

// -------------------------------------------------------------------------

void CViewSpikeHist::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);
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

// -------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CViewSpikeHist, CDaoRecordView)
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_TIMEFIRST, OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_EDIT4, OnEnChangeTimebin)
	ON_EN_CHANGE(IDC_BINSIZE, OnEnChangebinISI)
	ON_BN_CLICKED(IDC_CHECK1, OnClickAllfiles)
	ON_BN_CLICKED(IDC_RADIOABSOLUTE, OnabsoluteTime)
	ON_BN_CLICKED(IDC_RADIORELATIVE, OnrelativeTime)
	ON_BN_CLICKED(IDC_RADIOONECLASS, OnClickOneclass)
	ON_BN_CLICKED(IDC_RADIOALLCLASSES, OnClickAllclasses)
	ON_EN_CHANGE(IDC_SPIKECLASS, OnEnChangeSpikeclass)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangenbins)
	ON_EN_CHANGE(IDC_EDIT3, OnEnChangerowheight)
	ON_EN_CHANGE(IDC_EDIT2, OnEnChangeDotheight)
	ON_COMMAND(ID_FORMAT_HISTOGRAM, OnFormatHistogram)
	ON_BN_CLICKED(IDC_CHECK2, OnClickCycleHist)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeHistogramtype)
	ON_EN_CHANGE(IDC_EDITNSTIPERCYCLE, OnEnChangeEditnstipercycle)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDITLOCKONSTIM, OnEnChangeEditlockonstim)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CViewSpikeHist::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CViewSpikeHist::OnTcnSelchangeTab1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeHist message handlers

void CViewSpikeHist::OnInitialUpdate() 
{	
	VERIFY(mm_binISIms.SubclassDlgItem(IDC_BINSIZE, this));
	VERIFY(mm_timebinms.SubclassDlgItem(IDC_EDIT4, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	VERIFY(mm_spikeclass.SubclassDlgItem(IDC_SPIKECLASS, this));
	VERIFY(mm_nbinsISI.SubclassDlgItem(IDC_EDIT1, this));
	VERIFY(mm_dotheight.SubclassDlgItem(IDC_EDIT2, this));
	VERIFY(mm_rowheight.SubclassDlgItem(IDC_EDIT3, this));

	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100);
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollPos(50);

	// load stored parameters
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();    
	m_pvdS= &(pApp->vdS);	// get address of spike display options
	mdPM = &(pApp->vdP);	// printing options

	// create local fonts
	memset(&m_logFontDisp, 0, sizeof(LOGFONT));		// prepare font
	lstrcpy(m_logFontDisp.lfFaceName, _T("Arial"));		// Arial font
	m_logFontDisp.lfHeight = 15;					// font height
	BOOL flag = m_fontDisp.CreateFontIndirect(&m_logFontDisp);

	// fill controls with initial values
	((CButton*) GetDlgItem(IDC_CHECK1))->SetCheck(m_pvdS->ballfiles);
	if (m_pvdS->ballfiles)
		m_nfiles = GetDocument()->DBGetNRecords();
	else
		m_nfiles = 1;
	SetDlgItemInt(IDC_EDITNSTIPERCYCLE, m_pvdS->nstipercycle);
	SetDlgItemInt(IDC_EDITLOCKONSTIM, m_pvdS->istimulusindex);
	((CButton*) GetDlgItem(IDC_CHECK2))->SetCheck(m_pvdS->bCycleHist);
	m_timefirst = m_pvdS->timestart;
	m_timelast = m_pvdS->timeend;
	((CButton*) GetDlgItem(IDC_RADIOABSOLUTE))->SetCheck(m_pvdS->babsolutetime);
	((CButton*) GetDlgItem(IDC_RADIORELATIVE))->SetCheck(!m_pvdS->babsolutetime);
	((CButton*) GetDlgItem(IDC_RADIOALLCLASSES))->SetCheck(!m_pvdS->spikeclassoption);
	((CButton*) GetDlgItem(IDC_RADIOONECLASS))->SetCheck(m_pvdS->spikeclassoption);
	m_spikeclass=m_pvdS->classnb;

	((CListBox*) GetDlgItem(IDC_LIST1))->AddString(_T("Peristimulus histogram (PS)"));
	((CListBox*) GetDlgItem(IDC_LIST1))->AddString(_T("Spike intervals histogram (ISI)"));
	((CListBox*) GetDlgItem(IDC_LIST1))->AddString(_T("Autocorrelation histogram (Autoc)"));
	((CListBox*) GetDlgItem(IDC_LIST1))->AddString(_T("Raster display"));
	((CListBox*) GetDlgItem(IDC_LIST1))->AddString(_T("Peristimulus-Autocorrelation (PS-Autoc)"));

	(CWnd*)(GetDlgItem(IDC_SPIKECLASS))->EnableWindow(m_pvdS->spikeclassoption);
	m_timebinms = m_pvdS->timebin*t1000;
	m_dotheight = m_pvdS->dotheight;
	m_rowheight = m_pvdS->dotlineheight-m_pvdS->dotheight;

	// init coordinates of display area
	CRect rect0, rect1;
	GetWindowRect(&rect0);
	CWnd* pWnd= GetDlgItem(IDC_STATIC12);	// get pointer to display area
	pWnd->GetWindowRect(&rect1);
	m_topleft.x = rect1.left - rect0.left +1;
	m_topleft.y = rect1.top - rect0.top + 1;
	m_initiated=TRUE;
	ShowControls(m_pvdS->bhistType);

	// attach controls to stretch
	m_stretch.AttachParent(this);		// attach formview pointer
	m_stretch.newProp(IDC_STATIC12,		XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1,			XLEQ_XREQ, SZEQ_YBEQ);
	m_binit = TRUE;

	// init database and load documents
	CDaoRecordView::OnInitialUpdate();

	CdbWaveDoc* pdbDoc = GetDocument();
	if (pdbDoc->m_pSpk == nullptr)
	{
		pdbDoc->m_pSpk = new CSpikeDoc;
		ASSERT(pdbDoc->m_pSpk != NULL);
	}
	m_pSpkDoc = pdbDoc->m_pSpk;
	m_pSpkDoc->SetSpkListCurrent(pdbDoc->GetcurrentSpkListIndex());
	OnDisplay();
	SelectSpkList(m_pSpkDoc->GetSpkListCurrentIndex(), TRUE);
}


// -------------------------------------------------------------

void CViewSpikeHist::OnSize(UINT nType, int cx, int cy) 
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

// -------------------------------------------------------------

void CViewSpikeHist::OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		CMainFrame* pmF = (CMainFrame*) AfxGetMainWnd();
		pmF->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	else
	{
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();    
		pApp->vdS.ballfiles = ((CButton*) GetDlgItem(IDC_CHECK1))->GetCheck();
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// -------------------------------------------------------------

void CViewSpikeHist::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_REQUERY:
		if (m_pvdS->ballfiles)
			m_nfiles = GetDocument()->DBGetNRecords();
		else
			m_nfiles = 1;
		break;
	case HINT_DOCHASCHANGED:		// file has changed?
	case HINT_DOCMOVERECORD:
		SelectSpkList(GetDocument()->GetcurrentSpkListIndex(), TRUE);
		OnDisplay();
		break;

	default:
		break;
	}
}

BOOL CViewSpikeHist::OnMove(UINT nIDMoveCommand) 
{
	BOOL flag = CDaoRecordView::OnMove(nIDMoveCommand);
	CdbWaveDoc* pDoc = GetDocument();
	if (pDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
	{
		((CChildFrame*)GetParent())->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		return false;
	}

	pDoc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	if (!m_pvdS->ballfiles)
		OnDisplay();
	SelectSpkList(GetDocument()->GetcurrentSpkListIndex(), TRUE);
	return flag;
}

/////////////////////////////////////////////////////////////////////////////
// remove objects
void CViewSpikeHist::OnDestroy() 
{
	CDaoRecordView::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CViewSpikeHist diagnostics

#ifdef _DEBUG
void CViewSpikeHist::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewSpikeHist::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewSpikeHist::GetDocument()
{ 
	return (CdbWaveDoc*)m_pDocument; 
}


#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewSpikeHist database support

CDaoRecordset* CViewSpikeHist::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}

/////////////////////////////////////////////////////////////////////////////


void CViewSpikeHist::OnEnChangeTimefirst() 
{
	if (!mm_timefirst.m_bEntryDone)
		return;

	float timefirst = m_timefirst;
	switch (mm_timefirst.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_timefirst++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_timefirst--;
		break;
	}	
	mm_timefirst.m_bEntryDone=FALSE;
	mm_timefirst.m_nChar=0;
	mm_timefirst.SetSel(0, -1); 	//select all text
	if (m_timefirst > m_timelast)
	{
		m_timefirst = timefirst;
		::MessageBeep(-1);
	}
	m_pvdS->timestart=m_timefirst;
	UpdateData(FALSE);
	if (timefirst != m_timefirst)
		OnDisplay();
}

// -------------------------------------------------------------

void CViewSpikeHist::OnEnChangeTimelast() 
{
	if (!mm_timelast.m_bEntryDone)
		return;

	float timelast = m_timelast;
	switch (mm_timelast.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_timelast++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_timelast--;
		break;
	}
	mm_timelast.m_bEntryDone=FALSE;
	mm_timelast.m_nChar=0;
	mm_timelast.SetSel(0, -1);
	if (m_timelast < m_timefirst)
	{
		m_timelast = timelast;
		::MessageBeep(-1);
	}
	m_pvdS->timeend = m_timelast;
	UpdateData(FALSE);
	if (timelast != m_timelast)
		OnDisplay();
}

// -------------------------------------------------------------
void CViewSpikeHist::OnEnChangeTimebin()
{
	if (!mm_timebinms.m_bEntryDone)
		return;
	float binms = m_timebinms;
	switch (mm_timebinms.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_timebinms++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_timebinms--;
		break;
	}
	mm_timebinms.m_bEntryDone=FALSE;
	mm_timebinms.m_nChar=0;
	mm_timebinms.SetSel(0, -1);
	m_pvdS->timebin = m_timebinms/t1000;
	UpdateData(FALSE);
	if (binms != m_timebinms)
		OnDisplay();
}


void CViewSpikeHist::OnEnChangebinISI()
{
	if (!mm_binISIms.m_bEntryDone)
		return;
	float binms = m_binISIms;
	switch (mm_binISIms.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_binISIms++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_binISIms--;
		break;
	}
	mm_binISIms.m_bEntryDone=FALSE;
	mm_binISIms.m_nChar=0;
	mm_binISIms.SetSel(0, -1);
	m_pvdS->binISI = m_binISIms /t1000;
	UpdateData(FALSE);
	if (binms != m_binISIms)
		OnDisplay();
}

// -------------------------------------------------------------

void CViewSpikeHist::OnEnChangeSpikeclass() 
{
	if (!mm_spikeclass.m_bEntryDone)
		return;

	int spikeclassoption = m_spikeclass;
	switch (mm_spikeclass.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_spikeclass++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_spikeclass--;
		break;
	}	
	mm_spikeclass.m_bEntryDone=FALSE;
	mm_spikeclass.m_nChar=0;
	mm_spikeclass.SetSel(0, -1); 	//select all text
	m_pvdS->classnb = m_spikeclass;
	UpdateData(FALSE);
	if (spikeclassoption != m_spikeclass)
		OnDisplay();
}


// 
// display data
//

void CViewSpikeHist::OnDisplay() 
{
	if (!m_binit)
		return;
	BuildData();						// build histogram(s)
	m_bmodified=TRUE;					// force display
	InvalidateRect(&m_displayRect);		// display
}

// -------------------------------------------------------------

void CViewSpikeHist::OnDraw(CDC* pDC) 
{
	CRect rect;								// get coordinates of display area
	CWnd* pWnd= GetDlgItem(IDC_STATIC12);	// get pointer to display static control
	pWnd->GetClientRect(&rect);				// get the final rect

	CDC dcMem;								// prepare device context
	dcMem.CreateCompatibleDC(pDC);

	// adjust size of the bitmap (eventually)
	if (m_displayRect.Width() != rect.right || m_displayRect.Height() != rect.bottom)
	{
		if (m_pbitmap != nullptr)	// erase old bitmap (size has changed)
			delete m_pbitmap;
		m_pbitmap= new CBitmap;
		ASSERT(m_pbitmap != NULL);
		m_displayRect=rect;
		m_displayRect.OffsetRect(m_topleft);
		m_pbitmap->CreateBitmap(rect.Width(), rect.Height(), 
			pDC->GetDeviceCaps(PLANES), 
			pDC->GetDeviceCaps(BITSPIXEL),
			nullptr);
		m_bmodified=TRUE;
	}

	// select bitmap into device context
	CBitmap* poldbitmap = dcMem.SelectObject(m_pbitmap);

	if (m_bmodified)					// replot only if flag is set
	{
		int isavedDC = dcMem.SaveDC();	// save DC
		dcMem.Rectangle(rect);			// erase window background
		CFont* poldFont = nullptr;
		if (!m_bPrint)
			poldFont = (CFont*) dcMem.SelectObject(&m_fontDisp);

		// call display routine according to selection
		switch(m_bhistType)
		{
		case 0: 
		case 1:
		case 2:
			DisplayHistogram(&dcMem, &rect);
			break;
		case 3:
			DisplayDot(&dcMem, &rect); 
			break;
		case 4:
			DisplayPSTHAutoc(&dcMem, &rect);
			break;
		default:
			break;
		}

		// restore parameters
		if (poldFont != nullptr) 
			dcMem.SelectObject(poldFont);

		dcMem.RestoreDC(isavedDC);		// restore DC
		pDC->SetMapMode(MM_TEXT);		// reset mapping mode to text
		pDC->SetWindowOrg(0,0);
		pDC->SetViewportOrg(0,0);
		m_bmodified=FALSE;				// set flag to FALSE (job done)
	}

	// transfer to the screen
	pDC->BitBlt(m_topleft.x,
		m_topleft.y,
		rect.right,
		rect.bottom,
		&dcMem,
		0,
		0,
		SRCCOPY);
	dcMem.SelectObject(poldbitmap);		// release bitmap
}

// -------------------------------------------------------------

void CViewSpikeHist::GetFileInfos(CString &strComment)
{
	if (m_nfiles==1)
	{
		CString Tab("    ");			// use 4 spaces as tabulation character
		CString RC("\n");				// next line
		strComment += "Title:";
		m_pSpkDoc = GetDocument()->m_pSpk;
		//strComment += m_pSpkDoc->GetAcqComment();
		strComment += RC;
		if (m_bPrint)				// print document's infos
		{
			if (mdPM->bDocName || mdPM->bAcqDateTime)// print doc infos?
			{
				if (mdPM->bDocName)					// print file name
				{
					CString filename = GetDocument()->DBGetCurrentSpkFileName(FALSE);
					strComment += filename + Tab;
				}
				if (mdPM->bAcqDateTime)				// print data acquisition date & time
				{
					CString date = (m_pSpkDoc->GetAcqTime()).Format("%#d %m %Y %X"); //("%c");
					strComment +=  date;
				}
				strComment += RC;
			}
		}		
	}
}


// -------------------------------------------------------------

void CViewSpikeHist::OnClickAllfiles() 
{
	if (((CButton*) GetDlgItem(IDC_CHECK1))->GetCheck())
	{
		m_pvdS->ballfiles = TRUE;
		m_nfiles = GetDocument()->DBGetNRecords();
	}
	else
	{
		m_pvdS->ballfiles = FALSE;
		m_nfiles=1;
	}
	OnDisplay();
}

// -------------------------------------------------------------

void CViewSpikeHist::OnClickCycleHist() 
{
	if (((CButton*) GetDlgItem(IDC_CHECK2))->GetCheck())
	{
		m_pvdS->bCycleHist = TRUE;
		m_pvdS->nstipercycle = GetDlgItemInt(IDC_EDITNSTIPERCYCLE);
		m_pvdS->istimulusindex = GetDlgItemInt(IDC_EDITLOCKONSTIM);
	}
	else
		m_pvdS->bCycleHist = FALSE;
	OnDisplay();
}


// -------------------------------------------------------------

void CViewSpikeHist::OnClickOneclass() 
{
	if(!m_pvdS->spikeclassoption)
	{
		m_pvdS->spikeclassoption = TRUE;
		(CWnd*)(GetDlgItem(IDC_SPIKECLASS))->EnableWindow(TRUE);
		OnDisplay();
	}
}

// -------------------------------------------------------------

void CViewSpikeHist::OnClickAllclasses() 
{
	if(m_pvdS->spikeclassoption)
	{
		m_pvdS->spikeclassoption = FALSE;
		(CWnd*)(GetDlgItem(IDC_SPIKECLASS))->EnableWindow(FALSE);
		OnDisplay();
	}
}

// -------------------------------------------------------------

void CViewSpikeHist::OnabsoluteTime() 
{
	if (!m_pvdS->babsolutetime)
	{
		m_pvdS->babsolutetime = TRUE;
		OnDisplay();
	}
}

// -------------------------------------------------------------

void CViewSpikeHist::OnrelativeTime() 
{
	if (m_pvdS->babsolutetime)
	{
		m_pvdS->babsolutetime = FALSE;
		OnDisplay();
	}
}


// -------------------------------------------------------------

void CViewSpikeHist::ShowControls(int iselect)
{
	m_bhistType=iselect;
	switch (iselect)
	{
	case 0:
		m_timebinms= m_pvdS->timebin*t1000;
		m_pvdS->exportdatatype = EXPORT_PSTH;
		break;
	case 1:
		m_nbinsISI= m_pvdS->nbinsISI;
		m_binISIms= m_pvdS->binISI*t1000;
		m_pvdS->exportdatatype = EXPORT_ISI;
		break;
	case 2:
		m_nbinsISI = m_pvdS->nbinsISI;
		m_binISIms = m_pvdS->binISI*t1000;
		m_pvdS->exportdatatype = EXPORT_AUTOCORR;
		break;
	case 3:
		break;
	case 4:
		m_nbinsISI = m_pvdS->nbinsISI;
		m_binISIms = m_pvdS->binISI*t1000;
		m_timebinms= m_pvdS->timebin*t1000;
		break;
	default:
		m_bhistType=0;
		m_timebinms= m_pvdS->timebin*t1000;
		break;
	}
	m_pvdS->bhistType = m_bhistType;

	((CListBox*) GetDlgItem(IDC_LIST1))->SetCurSel(m_bhistType);
	UpdateData(FALSE);

	int bsettings[5][11] = //1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	{
	SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_HIDE,
	SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW,SW_HIDE,
	SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_SHOW,
	SW_HIDE,SW_HIDE, SW_HIDE,SW_HIDE,SW_HIDE, SW_SHOW,SW_SHOW,SW_SHOW,SW_SHOW, SW_HIDE, SW_HIDE,
	SW_SHOW,SW_SHOW, SW_SHOW,SW_SHOW,SW_SHOW,SW_HIDE,SW_HIDE,SW_HIDE,SW_HIDE, SW_HIDE, SW_SHOW
	};

	int i = 0;	
	GetDlgItem(IDC_STATIC7)->ShowWindow(bsettings[iselect][i]); i++;	// 1  PSTH bin size (ms)
	GetDlgItem(IDC_EDIT4)->ShowWindow(bsettings[iselect][i]); i++;		// 2  edit:  PSTH bin size (ms)

	GetDlgItem(IDC_BINSIZE)->ShowWindow(bsettings[iselect][i]); i++;	// 3  Edit: ISI &bin size (ms) or 
	GetDlgItem(IDC_STATIC3)->ShowWindow(bsettings[iselect][i]); i++;	// 4  n bins
	GetDlgItem(IDC_EDIT1)->ShowWindow(bsettings[iselect][i]); i++;		// 5  Edit: nbins
		
	GetDlgItem(IDC_STATIC5)->ShowWindow(bsettings[iselect][i]); i++;	// 6  dot height
	GetDlgItem(IDC_EDIT2)->ShowWindow(bsettings[iselect][i]); i++;		// 7  Edit: dot height
	GetDlgItem(IDC_STATIC6)->ShowWindow(bsettings[iselect][i]); i++;	// 8  separator height
	GetDlgItem(IDC_EDIT3)->ShowWindow(bsettings[iselect][i]); i++;		// 9  Edit: separator height
	
	GetDlgItem(IDC_STATIC2)->ShowWindow(bsettings[iselect][i]); i++;	// 10 ISI &bin size (ms)
	GetDlgItem(IDC_STATIC13)->ShowWindow(bsettings[iselect][i]); i++;	// 11 autoc. &bin size (ms)
}

// -------------------------------------------------------------

void CViewSpikeHist::OnEnChangenbins() 
{
	if (!mm_nbinsISI.m_bEntryDone)
		return;

	int nbins = m_nbinsISI;
	switch (mm_nbinsISI.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_nbinsISI++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_nbinsISI--;
		break;
	}	
	mm_nbinsISI.m_bEntryDone=FALSE;
	mm_nbinsISI.m_nChar=0;
	mm_nbinsISI.SetSel(0, -1); 	//select all text
	m_pvdS->nbinsISI = m_nbinsISI;
	UpdateData(FALSE);
	if (nbins != m_nbinsISI)
		OnDisplay();
}

// -------------------------------------------------------------

void CViewSpikeHist::OnEnChangerowheight() 
{
	if (!mm_rowheight.m_bEntryDone)
		return;

	int rowheight = m_rowheight;
	switch (mm_rowheight.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_rowheight++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_rowheight--;
		break;
	}	
	mm_rowheight.m_bEntryDone=FALSE;
	mm_rowheight.m_nChar=0;
	mm_rowheight.SetSel(0, -1); 	//select all text
	m_pvdS->dotlineheight= m_rowheight + m_pvdS->dotheight;
	UpdateData(FALSE);
	if (rowheight != m_rowheight)
		OnDisplay();
}

// -------------------------------------------------------------

void CViewSpikeHist::OnEnChangeDotheight() 
{
	if (!mm_dotheight.m_bEntryDone)
		return;

	int dotheight = m_dotheight;
	switch (mm_dotheight.m_nChar)
	{		
	case VK_RETURN:			
		UpdateData(TRUE);		// load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_dotheight++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_dotheight--;
		break;
	}	
	mm_dotheight.m_bEntryDone=FALSE;
	mm_dotheight.m_nChar=0;
	mm_dotheight.SetSel(0, -1); 	//select all text
	m_pvdS->dotheight= m_dotheight;
	m_pvdS->dotlineheight= m_rowheight + m_dotheight;		
	UpdateData(FALSE);
	if (dotheight != m_dotheight)
		OnDisplay();
}

// -------------------------------------------------------------

void CViewSpikeHist::OnEditCopy() 
{
	// create metafile
	CMetaFileDC mDC;
	
	// size of the window
	CRect rectBound, rect;
	CWnd* pWnd= GetDlgItem(IDC_STATIC12);	// get pointer to display static control
	pWnd->GetClientRect(&rect);	// get the final rect
	rectBound = rect;
	rectBound.right *= 32;		// HIMETRIC UNIT (0.01 mm increments)
	rectBound.bottom *= 30;		// HIMETRIC UNIT (0.01 mm increments)

	// DC for output and objects
	CDC* pDCRef= pWnd->GetDC();
	CString csTitle = _T("dbWave\0") + (GetDocument())->GetTitle();
	csTitle += _T("\0\0");
	BOOL hmDC = mDC.CreateEnhanced(pDCRef, nullptr, &rectBound, csTitle);
	ASSERT (hmDC != NULL);

	// Draw document in metafile.
	CClientDC attribDC(this) ;	// Create and attach attribute DC
	mDC.SetAttribDC(attribDC.GetSafeHdc()) ; // from current screen

	// display curves
	// call display routine according to selection
	switch(m_bhistType)
	{
	case 0: 
	case 1:
	case 2:
		DisplayHistogram(&mDC, &rect);
		break;
	case 3:
		DisplayDot(&mDC, &rect); 
		break;
	case 4:
		DisplayPSTHAutoc(&mDC, &rect);
		break;
	default:
		break;
	}
	ReleaseDC(pDCRef);

	// Close metafile
	HENHMETAFILE hEmfTmp = mDC.CloseEnhanced();
	ASSERT (hEmfTmp != NULL);
	if (OpenClipboard())
	{				
		EmptyClipboard();							// prepare clipboard
		SetClipboardData(CF_ENHMETAFILE, hEmfTmp);	// put data
		CloseClipboard();							// close clipboard
	}
	else
	{
		// Someone else has the Clipboard open...
		DeleteEnhMetaFile(hEmfTmp);					// delete data
		MessageBeep(0) ;							// tell user something is wrong!
		AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION );
	}
}

// -------------------------------------------------------------

BOOL CViewSpikeHist::OnPreparePrinting(CPrintInfo* pInfo) 
{
	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;
	
	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	// printing margins	
	if (mdPM->vertRes <= 0 ||mdPM->horzRes <= 0
		||mdPM->horzRes !=  pInfo->m_rectDraw.Width()
		||mdPM->vertRes != pInfo->m_rectDraw.Height())
	{
		// compute printer's page dot resolution
		CPrintDialog dlg(FALSE); // borrowed from VC++ sample\drawcli\drawdoc.cpp
		VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));
		CDC dc;					// GetPrinterDC returns a HDC so attach it
		HDC hDC= dlg.CreatePrinterDC();
		ASSERT(hDC != NULL);
		dc.Attach(hDC);
		// Get the size of the page in pixels
		mdPM->horzRes=dc.GetDeviceCaps(HORZRES);
		mdPM->vertRes=dc.GetDeviceCaps(VERTRES);
	}

	// how many rows per page?
	int sizeRow = mdPM->HeightDoc + mdPM->heightSeparator;
	int nbrowsperpage = (mdPM->vertRes - 2*mdPM->topPageMargin) / sizeRow;
	int nfiles = 1;
	if (m_nfiles == 1)
		nfiles = GetDocument()->DBGetNRecords();

	if (nbrowsperpage == 0)			// prevent zero pages
		nbrowsperpage = 1;
	int npages = nfiles/nbrowsperpage;
	if (nfiles > nbrowsperpage*npages)
		npages++;

	pInfo->SetMaxPage(npages);		// one page printing/preview  
	pInfo->m_nNumPreviewPages = 1;  // preview 1 pages at a time
	// allow print only selection	
	if(mdPM->bPrintSelection) 
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION;
	else
		pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;

	// call dialog box
	BOOL flag = DoPreparePrinting(pInfo);
	// set max nb of pages according to selection
	mdPM->bPrintSelection = pInfo->m_pPD->PrintSelection();
	if (mdPM->bPrintSelection)
		pInfo->SetMaxPage(1);		
	return flag;
}

// -------------------------------------------------------------

void CViewSpikeHist::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	// select font, set print flag, save current file index
	CFont* pOldFont = pDC->SelectObject(&m_fontPrint);

	m_bPrint=TRUE;
	int file0 = GetDocument()->DBGetCurrentRecordPosition();

	// print page footer: file path, page number/total pages, date
	CTime t= CTime::GetCurrentTime();					// current date & time
	CString csFooter;									// first string to receive
	csFooter.Format(_T("  page %d:%d %d-%d-%d"),		// page and time infos
			pInfo->m_nCurPage, pInfo->GetMaxPage(),
			t.GetDay(),	t.GetMonth(),t.GetYear());
	CString ch_date;									// second string with data path
	ch_date = GetDocument()->DBGetCurrentSpkFileName(FALSE);
	ch_date = ch_date.Left(ch_date.GetLength() -1)+ csFooter;
	pDC->SetTextAlign(TA_CENTER);						// and print the footer
	pDC->TextOut(mdPM->horzRes/2, mdPM->vertRes-57,	ch_date);
	
	// define page rectangle (where data and comments are plotted)
	CRect rectPage = pInfo->m_rectDraw;
	rectPage.right = mdPM->horzRes-mdPM->rightPageMargin;
	rectPage.bottom = mdPM->vertRes-mdPM->bottomPageMargin;
	rectPage.left = mdPM->leftPageMargin;
	rectPage.top = mdPM->topPageMargin;

	// define data file rectangle - position of the first file
	int rWidth = mdPM->WidthDoc;				// margins
	int rHeight = mdPM->HeightDoc;				// margins
	CRect RWhere (	rectPage.left, rectPage.top, 
					rectPage.left+rWidth, rectPage.top+rHeight);	

	// prepare file loop
	CdbWaveDoc* pdbDoc = GetDocument();
	int nfiles = pdbDoc->DBGetNRecords();
	int sizeRow=mdPM->HeightDoc + mdPM->heightSeparator;		// size of one row
	int nbrowsperpage = pInfo->m_rectDraw.Height()/sizeRow;		// nb of rows per page
	if (nbrowsperpage == 0)
		nbrowsperpage = 1;
	int file1 = (pInfo->m_nCurPage-1)*nbrowsperpage;		// index first file
	int file2 = file1 + nbrowsperpage;						// index last file
	if (m_nfiles != 1)										// special case: all together
		file2 = file1+1;
	if (file2 > pdbDoc->DBGetNRecords())
		file2 = pdbDoc->DBGetNRecords();

	// loop through all files
	for (int ifile = file1; ifile<file2; ifile++)
	{		
		if (mdPM->bFrameRect)	// print data rect if necessary
		{
			pDC->MoveTo(RWhere.left,  RWhere.top);
			pDC->LineTo(RWhere.right, RWhere.top);
			pDC->LineTo(RWhere.right, RWhere.bottom);
			pDC->LineTo(RWhere.left,  RWhere.bottom);
			pDC->LineTo(RWhere.left,  RWhere.top);
		}
		m_commentRect = RWhere;		// calculate where the comments will be printed
		m_commentRect.OffsetRect(mdPM->textseparator + m_commentRect.Width(), 0);
		m_commentRect.right = pInfo->m_rectDraw.right;
		// refresh data if necessary
		if (m_nfiles == 1) //??? (m_nfiles > 1)
		{
			pdbDoc->DBSetCurrentRecordPosition(ifile);
			BuildData();
		}
		// print the histogram
		switch(m_bhistType)
		{
		case 0: 
		case 1:
		case 2:
			DisplayHistogram(pDC, &RWhere);
			break;
		case 3:
			DisplayDot(pDC, &RWhere); 
			break;
		case 4:
			DisplayPSTHAutoc(pDC, &RWhere);
			break;
		default:
			break;
		}
		// update display rectangle for next row
		RWhere.OffsetRect(0, rHeight+mdPM->heightSeparator);
	}

	// restore parameters
	if (pOldFont != nullptr) 
		pDC->SelectObject(pOldFont);

	pdbDoc->DBSetCurrentRecordPosition(file0);
	pdbDoc->OpenCurrentSpikeFile();
	m_pSpkDoc = pdbDoc->m_pSpk;			
}

// -------------------------------------------------------------

void CViewSpikeHist::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	m_fontPrint.DeleteObject();
	m_bPrint=FALSE;
	CDaoRecordView::OnEndPrinting(pDC, pInfo);
}

// -------------------------------------------------------------

void CViewSpikeHist::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	memset(&m_logFont, 0, sizeof(LOGFONT));		// prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));		// Arial font
	m_logFont.lfHeight = mdPM->fontsize;		// font height
	BOOL flag = m_fontPrint.CreateFontIndirect(&m_logFont);	
	pDC->SetBkMode (TRANSPARENT);
}

// -------------------------------------------------------------

void CViewSpikeHist::OnFormatHistogram() 
{
	CFormatHistogramDlg dlg;
	dlg.m_bYmaxAuto = m_pvdS->bYmaxAuto;
	dlg.m_Ymax= m_pvdS->Ymax;
	dlg.m_xfirst=m_pvdS->timestart;
	dlg.m_xlast=m_pvdS->timeend;
	dlg.m_crHistFill=m_pvdS->crHistFill;
	dlg.m_crHistBorder=m_pvdS->crHistBorder;
	dlg.m_crStimFill=m_pvdS->crStimFill;
	dlg.m_crStimBorder=m_pvdS->crStimBorder;
	dlg.m_crChartArea=m_pvdS->crChartArea;

	if (IDOK == dlg.DoModal())
	{
		m_pvdS->bYmaxAuto= dlg.m_bYmaxAuto;
		m_pvdS->Ymax = dlg.m_Ymax;
		m_pvdS->timestart= dlg.m_xfirst;
		m_pvdS->timeend=dlg.m_xlast;
		m_timefirst = m_pvdS->timestart;
		m_timelast = m_pvdS->timeend;
		m_pvdS->crHistFill=dlg.m_crHistFill;
		m_pvdS->crHistBorder=dlg.m_crHistBorder;
		m_pvdS->crStimFill=dlg.m_crStimFill;
		m_pvdS->crStimBorder=dlg.m_crStimBorder;
		m_pvdS->crChartArea=dlg.m_crChartArea;
		OnDisplay();
	}
}

// -------------------------------------------------------------

void CViewSpikeHist::BuildData()
{
	// adjust size of the data array
	int nbins=1;
	nbins = (int) ((m_timelast-m_timefirst)*t1000/m_timebinms);
	if (nbins <= 0)
		nbins = 1;

	switch (m_bhistType)
	{
	case 0:
		m_nPSTH = 0;
		if (m_sizepPSTH != nbins)	// make space for side PSTH
		{
			SAFE_DELETE_ARRAY(m_pPSTH);
			m_pPSTH = new long[nbins+2];
			ASSERT(m_pPSTH != NULL);
			m_sizepPSTH = nbins;
			m_pvdS->nbins = nbins;
		}
		break;

	case 1:
	case 2:
		m_nISI = 0;
		if (m_sizepISI != m_nbinsISI)	// make space for side autocorr
		{
			SAFE_DELETE_ARRAY(m_pISI);
			m_pISI = new long[m_nbinsISI+2];
			ASSERT(m_pISI != NULL);
			m_sizepISI  = m_nbinsISI;
			m_pvdS->nbinsISI = m_nbinsISI;
		}		
		break;

	case 4:
		m_nPSTH = 0;
		if (m_sizepPSTH != nbins)	// make space for side PSTH
		{
			SAFE_DELETE_ARRAY(m_pPSTH);
			m_pPSTH = new long[nbins+2];
			ASSERT(m_pPSTH != NULL);
			m_sizepPSTH = nbins;
			m_pvdS->nbins = nbins;
		}

		if (m_sizepISI != m_nbinsISI)	// make space for side autocorr
		{
			SAFE_DELETE_ARRAY(m_pISI);
			m_pISI = new long[m_nbinsISI+2];
			ASSERT(m_pISI != NULL);
			m_sizepISI  = m_nbinsISI;
			m_pvdS->nbinsISI = m_nbinsISI;
		}

		nbins = m_sizepPSTH * m_sizepISI;	// total bins for main histogram array
		if (m_sizeparrayISI != nbins)		// make space for side autocorr
		{
			SAFE_DELETE_ARRAY(m_parrayISI);
			m_parrayISI = new long[nbins+2];
			ASSERT(m_parrayISI != NULL);
			m_sizeparrayISI  = nbins;
		}
		break;

	default:
		break;
	}

	// erase the content of the arrays
	long* pLong;
	if (m_sizepPSTH != NULL)
	{
		pLong = m_pPSTH;
		for (int ui=0; ui<= m_sizepPSTH; ui++, pLong++)
			*pLong = 0;
	}
	if (m_sizepISI != 0)
	{
		pLong = m_pISI;
		for (int ui=0; ui<= m_sizepISI; ui++, pLong++)
			*pLong = 0;
	}
	if (m_sizeparrayISI != 0)
	{
		pLong = m_parrayISI;
		for (int ui=0; ui<= m_sizeparrayISI; ui++, pLong++)
			*pLong = 0;
	}

	CdbWaveDoc* pdbDoc = GetDocument();
	int currentfile = pdbDoc->DBGetCurrentRecordPosition(); // index current file
	int firstfile = currentfile;		// index first file in the series
	int lastfile = currentfile;			// index last file in the series

	CProgressDlg* pdlg = nullptr;
	int istep = 0;
	CString cscomment;
	CString csfilecomment = _T("Analyze file: ");

	if (m_nfiles > 1)
	{
		pdlg = new CProgressDlg;
		pdlg->Create();
		pdlg->SetStep (1);
		firstfile=0;
		lastfile = m_nfiles-1;
	}
	
	for (int ifile = firstfile; ifile <= lastfile; ifile++)
	{
		if (m_nfiles > 1)
		{
			// check if user wants to stop
			if(pdlg->CheckCancelButton())
				if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
					break;
			cscomment.Format(_T("Processing file [%i / %i]"), ifile+1, m_nfiles);
			pdlg->SetStatus(cscomment);
			if (MulDiv(ifile, 100, m_nfiles) > istep)
			{
				pdlg->StepIt();
				istep = MulDiv(ifile, 100, m_nfiles);
			}
		}

		// select spike file
		pdbDoc->DBSetCurrentRecordPosition(ifile);
		pdbDoc->OpenCurrentSpikeFile();
		m_pSpkDoc = pdbDoc->m_pSpk;
		if (nullptr == m_pSpkDoc )
			continue;

		m_pSpkDoc->SetSpkListCurrent(pdbDoc->GetcurrentSpkListIndex());

		switch(m_bhistType)
		{
			// -------------------------------------------------------------
			// PERIS-STIMULUS TIME HISTOGRAM
			// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
			// m_pvdS->timestart, m_pvdS->timeend
			// m_pvdS->timebin, m_pvdS->nbins
			case 0: 
				m_nPSTH += m_pSpkDoc->BuildPSTH(m_pvdS, m_pPSTH, m_spikeclass); 
				break;
			// -------------------------------------------------------------
			// INTER-SPIKES INTERVALS HISTOGRAM
			// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
			// m_pvdS->timestart, m_pvdS->timeend
			// m_pvdS->nbinsISI, m_pvdS->binISI
			case 1: 
				m_nISI += m_pSpkDoc->BuildISI(m_pvdS, m_pISI, m_spikeclass); 
				break;
			// -------------------------------------------------------------
			// AUTOCORRELATION
			// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
			// m_pvdS->timestart, m_pvdS->timeend
			// m_pvdS->nbinsISI, m_pvdS->binISI
			case 2: 
				m_nISI += m_pSpkDoc->BuildAUTOCORR(m_pvdS, m_pISI, m_spikeclass);
				break;
			// -------------------------------------------------------------
			// PSTH-AUTOCORRELATION DENSITY 
			// m_pvdS->babsolutetime, m_pvdS->spikeclassoption
			// m_pvdS->nbinsISI, m_pvdS->binISI
			// m_pvdS->timestart, m_pvdS->timeend
			// m_pvdS->timebin, m_pvdS->nbins
			case 4: 
				{
					m_nPSTH += m_pSpkDoc->BuildPSTH(m_pvdS, m_pPSTH, m_spikeclass);
					m_pSpkDoc->BuildAUTOCORR(m_pvdS, m_pISI, m_spikeclass);
					m_pSpkDoc->BuildPSTHAUTOCORR(m_pvdS, m_parrayISI, m_spikeclass);					
				}
				break;
			default:
				break;
		}
	}

	if (currentfile != pdbDoc->DBGetCurrentRecordPosition())
	{
		pdbDoc->DBSetCurrentRecordPosition(currentfile);
		pdbDoc->OpenCurrentSpikeFile();
		m_pSpkDoc = pdbDoc->m_pSpk;
		m_pSpkDoc->SetSpkListCurrent(pdbDoc->GetcurrentSpkListIndex());
	}
	SAFE_DELETE(pdlg);
	return;
}


// -------------------------------------------------------------

// plot histogram (generic)
//
// input (passed parameters):
//		pDC			pointer to DC context
//		dispRect	display rectangle (plotting area)
//		nbinshistog	number of bins within the histogram
//		phistog		pointer to an array of long with the values of the histogram
//		orientation	0=0 deg, +1=+90 deg; -1=-90 deg; +2,-2=-180 degrees
//		btype	type of display histogram (0=PSTH, 1=ISI, 2=autoc)


// input (internal parameters):
//		pvdS		array with spike display parameters OPTIONS_VIEWSPIKES
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
//		m_pSpkDoc->m_stimIntervals.nitems
//		m_pvdS->crStimFill
//		m_pvdS->crStimBorder
//		m_pSpkDoc->m_spklist
//		p_spk_list->GetAcqSampRate()
//
// output (passed parameter):
//		return max val of histogram
// output (modified internal parameters)
//		m_rectratio	 % of histogram height versus plotting area

long CViewSpikeHist::PlotHistog(CDC* pDC, CRect* pdispRect, int nbinshistog, long* phistog0, int orientation, int btype)
{
	CPen penbars;
	penbars.CreatePen(PS_SOLID, 0, m_pvdS->crHistBorder);
	CBrush brushbars;
	brushbars.CreateSolidBrush(m_pvdS->crHistFill);

	// update chart area
	pdispRect->InflateRect(1, 1);
	pDC->FillSolidRect(pdispRect, m_pvdS->crChartArea);

	CPen* poldPen = (CPen*) pDC->SelectObject(&penbars);	// rect fill
	pDC->MoveTo(pdispRect->left,  pdispRect->top);
	pDC->LineTo(pdispRect->right, pdispRect->top);
	pDC->LineTo(pdispRect->right, pdispRect->bottom);
	pDC->LineTo(pdispRect->left,  pdispRect->bottom);
	pDC->LineTo(pdispRect->left,  pdispRect->top);
	pdispRect->DeflateRect(1, 1);

	// display XY axis
	poldPen = (CPen*) pDC->SelectStockObject(BLACK_PEN);
	int xzero = pdispRect->left;
	CRect rectHz; 
	CRect rectVert;

	switch (orientation)
	{
	case -1:	// -90
		xzero = pdispRect->bottom;
		if (btype == 2) 
			xzero = (pdispRect->bottom + pdispRect->top)/2;		
		rectHz = CRect(pdispRect->left, xzero, pdispRect->right, xzero);
		rectVert = CRect(pdispRect->right, pdispRect->top, pdispRect->right, pdispRect->bottom);
		break;
	case 1:		// +90
		xzero = pdispRect->top;
		if (btype == 2) 
			xzero = (pdispRect->bottom + pdispRect->top)/2;
		rectHz = CRect(pdispRect->left, xzero, pdispRect->right, xzero);
		rectVert = CRect(pdispRect->left, pdispRect->top, pdispRect->left, pdispRect->bottom);
		
		break;
	case 2:		// -180 deg
		xzero = pdispRect->left;
		if (btype == 2) 
			xzero = (pdispRect->left + pdispRect->right)/2;		
		rectHz = CRect(pdispRect->left, pdispRect->top, pdispRect->right, pdispRect->top);
		rectVert = CRect(xzero, pdispRect->top, xzero, pdispRect->bottom);
		break;
	default:	//	normal
		xzero = pdispRect->left;
		if (btype == 2) 
			xzero = (pdispRect->left + pdispRect->right)/2;
		rectHz = CRect(pdispRect->left, pdispRect->bottom, pdispRect->right, pdispRect->bottom);
		rectVert = CRect(xzero, pdispRect->top, xzero, pdispRect->bottom);
		break;
	}
	pDC->MoveTo(rectHz.left, rectHz.top);		// hz line
	pDC->LineTo(rectHz.right, rectHz.bottom);
	pDC->MoveTo(rectVert.left, rectVert.top);	// vert line
	pDC->LineTo(rectVert.right, rectVert.bottom);

	// set position of max (assuming hardwired 80% of max: rectratio=80)
	long max = 0;
	if (m_pvdS->bYmaxAuto)
	{
		long* phistog = phistog0;
		for (int i=0; i < nbinshistog; i++, phistog++)
		{
			int val = *phistog;			// load value in a temp variable
			if (val > max)				// search max
				max = val;
		}
		m_rectratio = 80;					// span only 80%
	}
	else
	{
		float divisor = m_timebinms/t1000;
		if (btype > 0)
			divisor = m_timelast - m_timefirst;
		max = (long) (m_pvdS->Ymax * divisor * m_nfiles);
		m_rectratio = 100;				// 100% span for data
	}
	int rectmax = MulDiv(pdispRect->Height(), m_rectratio, 100);

	// prepare pen and brush
	poldPen = (CPen*) pDC->SelectObject(&penbars);	// rect outer line
	CBrush* poldBrush = (CBrush*) pDC->SelectObject(&brushbars);	// rect fill
	CRect rectBar = *pdispRect;		// rectangle to plot the bars

	int displen;
	int ui;
	switch (orientation)
	{	
	case -1:		// rotation -90 deg
		displen = pdispRect->Height();
		rectmax = MulDiv(pdispRect->Width(), m_rectratio, 100);
		for (ui=0; ui < m_sizepISI; ui++)
		{
			rectBar.bottom = MulDiv(displen, ui+1, nbinshistog) + pdispRect->top;
			rectBar.left = rectBar.right - MulDiv(*(phistog0+ui), rectmax, max);
			pDC->MoveTo(rectBar.left, rectBar.bottom);
			if (rectBar.top != rectBar.bottom)	
				pDC->Rectangle(rectBar);
			else
				pDC->LineTo(rectBar.left, rectBar.top);
			rectBar.top = rectBar.bottom;
		}
		break;

	case 1:			// rotation +90 deg
		displen = pdispRect->Height();
		rectmax = MulDiv(pdispRect->Width(), m_rectratio, 100);
		for (ui=0; ui < m_sizepISI; ui++)
		{
			rectBar.bottom = MulDiv(displen, ui+1, nbinshistog) + pdispRect->top;
			rectBar.right = rectBar.left + MulDiv(*(phistog0+ui), rectmax, max);
			pDC->MoveTo(rectBar.left, rectBar.bottom);
			if (rectBar.top != rectBar.bottom)	
				pDC->Rectangle(rectBar);
			else
				pDC->LineTo(rectBar.left, rectBar.top);
			rectBar.top = rectBar.bottom;
		}
		break;

	case 2:
		rectmax = MulDiv(pdispRect->Height(), m_rectratio, 100);
		displen = pdispRect->Width();
		for (ui=0; ui<nbinshistog; ui++)
		{
			rectBar.right= MulDiv(displen, ui+1, nbinshistog) + pdispRect->left;
			rectBar.bottom = rectBar.top + MulDiv(*(phistog0+ui), rectmax, max);
			pDC->MoveTo(rectBar.left, rectBar.bottom);
			if (rectBar.left != rectBar.right)
				pDC->Rectangle(rectBar);
			else
				pDC->LineTo(rectBar.left, rectBar.top);
			rectBar.left= rectBar.right;
		}
		break;

	default:
		displen = pdispRect->Width();
		rectmax = MulDiv(pdispRect->Height(), m_rectratio, 100);

		// display stimulus
		if (btype== 0 && m_pSpkDoc->m_stimIntervals.nitems > 0)
		{	
			CSpikeList* p_spk_list = m_pSpkDoc->GetSpkListCurrent();
			float samprate = p_spk_list->GetAcqSampRate();
			int iioffset0 = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(m_pvdS->istimulusindex);
			if (m_pvdS->babsolutetime )
				iioffset0 = 0;
	
			// search first stimulus transition within interval
			long iistart = (long) (m_pvdS->timestart*samprate) + iioffset0;
			long iiend = (long) (m_pvdS->timeend * samprate) + iioffset0;

			TEXTMETRIC tm;					// load characteristics of the font
			pDC->GetTextMetrics(&tm);
			CRect rect = *pdispRect;
			rect.bottom = rect.bottom + tm.tmHeight + tm.tmDescent -2;
			rect.top = pdispRect->bottom + 2 * tm.tmDescent +2;
			DisplayStim(pDC, &rect, &iistart, &iiend);
		}

		// display histogram
		for (ui=0; ui<nbinshistog; ui++)
		{
			rectBar.right= MulDiv(displen, ui+1, nbinshistog) + pdispRect->left;
			rectBar.top = rectBar.bottom - MulDiv(*(phistog0+ui), rectmax, max);
			pDC->MoveTo(rectBar.left, rectBar.bottom);
			if (rectBar.left != rectBar.right)	
				pDC->Rectangle(rectBar);
			else
				pDC->LineTo(rectBar.left, rectBar.top);
			rectBar.left= rectBar.right;
		}
		break;
	}

	// restore objects
	pDC->SelectObject(poldPen);
	pDC->SelectObject(poldBrush);
	return max;
}

// -------------------------------------------------------------

void CViewSpikeHist::DisplayDot(CDC* pDC, CRect* pRect)
{
	CWaitCursor wait;

	// save old pen and brush / restore on exit
	CPen penbars;
	penbars.CreatePen(PS_SOLID, 0, RGB(0x80, 0x80, 0x80));	
	CPen* poldPen = (CPen*) pDC->SelectObject(&penbars);
	CBrush brushbars;
	brushbars.CreateSolidBrush(RGB(0x80, 0x80, 0x80));

	CBrush* poldBrush=(CBrush*) pDC->SelectStockObject(BLACK_BRUSH);
	CRect dispRect = *pRect;			// this will be the display rect for histogram
	dispRect.left ++;
	dispRect.right--;

	// print comments
	TEXTMETRIC tm;						// load characteristics of the font
	pDC->GetTextMetrics(&tm);
	CRect commentRect = dispRect;
	commentRect.left += 4;
	commentRect.top ++;
	if (m_bPrint)
		commentRect = m_commentRect;

	// output legends
	int line=0;							// line nb (depend on title)
	CString strComment;
	GetFileInfos(strComment);			// title

	// histogram type and bin value
	m_xfirst = m_timefirst;				// abcissa first
	m_xlast = m_timelast;				// abcissa last
	strComment += _T("Dot Display");		// Dot display
	UINT uiFlag = pDC->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	line = pDC->DrawText(strComment, strComment.GetLength(), commentRect, 
			DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	pDC->SetTextAlign(uiFlag);
	line = line / tm.tmHeight;
	if (m_bPrint)
		line = 0;

	// define display rectangle and plot axis
	dispRect.top += tm.tmHeight*(line+1) + tm.tmDescent;
	dispRect.bottom -= tm.tmHeight + tm.tmDescent;
	dispRect.left += tm.tmDescent;
	dispRect.right -= tm.tmDescent;

	// print left abcissa value
	CString csXleft;
	csXleft.Format(_T("%1.3f"), m_xfirst);
	pDC->TextOut(dispRect.left, dispRect.bottom, csXleft);

	// print right abcissa value + unit	
	CString csXright;
	csXright.Format(_T("%1.3f s"), m_xlast);
	CSize left = pDC->GetTextExtent(csXright);
	pDC->TextOut(dispRect.right - left.cx - tm.tmDescent, dispRect.bottom, csXright);

	// display XY axis
	poldPen = (CPen*) pDC->SelectStockObject(BLACK_PEN);
	// abcissa
	pDC->MoveTo(dispRect.left, dispRect.bottom);
	pDC->LineTo(dispRect.right, dispRect.bottom);

	// set temp variables
	int rcleft = dispRect.left;	
	float span = m_xlast - m_xfirst;
	int rectlen = dispRect.Width();
	int row = dispRect.top;
	int dotheight = MulDiv (pRect->Height(), m_pvdS->dotheight, 1000);
	int dotlineheight = MulDiv (pRect->Height(), m_pvdS->dotlineheight, 1000);
	int vtTop		= dotheight +1;
	int vtBottom	= dotlineheight - 2;

	// prepare loop / files (stop when no more space is available)
	CdbWaveDoc* pdbDoc = GetDocument();
	int currentfile = pdbDoc->DBGetCurrentRecordPosition(); // index current file
	int firstfile = currentfile;		// index first file in the series
	int lastfile = firstfile;			// index last file in the series
	if (m_nfiles > 1)
	{
		firstfile=0;
		lastfile = m_nfiles-1;
	}

	// external loop: browse from file to file
	for (int ifile=firstfile; 
			ifile <= lastfile && row < dispRect.bottom; 
			ifile++)
	{
		pdbDoc->DBSetCurrentRecordPosition(ifile);
		pdbDoc->OpenCurrentSpikeFile();
		m_pSpkDoc = pdbDoc->m_pSpk;
		m_pSpkDoc->SetSpkListCurrent(pdbDoc->GetcurrentSpkListIndex());

		// load pointers to spike file and spike list
		CSpikeList* p_spk_list = m_pSpkDoc->GetSpkListCurrent();
		float samprate = p_spk_list->GetAcqSampRate();
		const long iiFrameFirst = (long) (m_timefirst*samprate);
		const long iiFrameLast = (long) (m_timelast*samprate);
		const long iiFrameLength = iiFrameLast - iiFrameFirst;
		int nspikes = p_spk_list->GetTotalSpikes();

		// display spikes and stimuli either on one line or on multiple lines
		if (m_pvdS->babsolutetime)
		{
			int nrows = 1;
			if (mdPM->bMultirowDisplay)
			{
				nrows = m_pSpkDoc->GetAcqSize() / iiFrameLength;
				if (nrows*iiFrameLength < m_pSpkDoc->GetAcqSize())
					nrows++;
			}
			long iiFirst = iiFrameFirst;
			long iiLast = iiFrameLast;
			int ispikefirst = 0;
			for (int irow = 0; irow < nrows; irow++)
			{
				// display stimuli
				if (m_pSpkDoc->m_stimIntervals.nitems > 0) 
				{
					CRect rect(rcleft, row + vtBottom, 
							rectlen + rcleft, row + vtTop);
					DisplayStim(pDC, &rect, &iiFirst, &iiLast);
				}
				// display spikes
				int iitime0 = -1;
				for (int i = ispikefirst; i<nspikes; i++)
				{		
					long iitime = p_spk_list->GetSpikeTime(i) -iiFirst;
					// check if this spike should be processed
					// assume that spikes occurence times are ordered
					if (iitime < 0)
						continue;
					if (iitime> iiFrameLength)
					{
						ispikefirst = i;
						break;
					}
					// check spike class
					if (m_pvdS->spikeclassoption 
						&& p_spk_list->GetSpikeClass(i) != m_spikeclass)
						continue;
					// convert interval into a pixel bin
					float spktime = iitime/samprate;
					iitime = (int) (spktime*rectlen / span) + rcleft;
					if (iitime != iitime0)	// avoid multiple drawing of the same dots
					{
						pDC->MoveTo(iitime, row);
						pDC->LineTo(iitime, row+ dotheight);
						iitime0 = iitime;
					}
				}

				// end loop - update parameters for next row
				iiFirst += iiFrameLength;
				iiLast  += iiFrameLength;
				row += dotlineheight;
			}
		}

		// display spikes in time locked to the stimuli
		else
		{
			// if !bCycleHist - only one pass is called
			int firstStim = m_pvdS->istimulusindex ;
			int lastStim = firstStim+1;
			int increment = 2;
			// if bCycleHist - one line per stimulus (or group of stimuli)
			if (m_pvdS->bCycleHist)
			{
				lastStim = m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize();
				if (lastStim == 0)
					lastStim = 1;
				increment = m_pvdS->nstipercycle;
				if (m_pSpkDoc->m_stimIntervals.npercycle > 1 
					&& increment > m_pSpkDoc->m_stimIntervals.npercycle)
					increment = m_pSpkDoc->m_stimIntervals.npercycle;
				increment *= 2;
			}

			// loop over stimuli 
			for (int istim = firstStim; istim <lastStim; istim+= increment)
			{
				// compute temp parameters
				long istart;
				if (m_pSpkDoc->m_stimIntervals.nitems > 0)
					istart = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(istim);
				else
					istart = (long) -(m_pvdS->timestart * samprate);
				long iend = iiFrameLast + istart;
				istart += iiFrameFirst;
				
				// draw dots -- look at all spikes...
				int iitime0 = -1;
				for (int i = 0; i<nspikes; i++)
				{		
					long iitime = p_spk_list->GetSpikeTime(i) -istart;
					// check if this spike should be processed
					// assume that spikes occurence times are ordered
					if (iitime < 0)
						continue;
					if (iitime> iiFrameLength)
						break;
					// check spike class
					if (m_pvdS->spikeclassoption 
						&& p_spk_list->GetSpikeClass(i) != m_spikeclass)
						continue;
					// convert interval into a pixel bin
					float spktime = iitime/samprate;
					iitime = (int) (spktime*rectlen / span) + rcleft;
					if (iitime != iitime0)	// avoid multiple drawing of the same dots
					{
						pDC->MoveTo(iitime, row);
						pDC->LineTo(iitime, row+ dotheight);
						iitime0 = iitime;
					}
				}				

				// next stim set -- jump to next line
				row += dotlineheight;
			}

			if (m_pSpkDoc->m_stimIntervals.nitems > 1) 
			{
				// stimulus
				long istart = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(m_pvdS->istimulusindex);				
				long iend = iiFrameLast+istart;
				istart = iiFrameFirst + istart;

				CRect rect(rcleft, row + dotlineheight - dotheight, 
						rectlen + rcleft, row + vtBottom);
				DisplayStim(pDC, &rect, &istart, &iend);
			}
		}

		// next file -- jump to next line..
		row += dotlineheight;
	}

	pdbDoc->DBSetCurrentRecordPosition(currentfile);
	pdbDoc->OpenCurrentSpikeFile();
	m_pSpkDoc = pdbDoc->m_pSpk;
	m_pSpkDoc->SetSpkListCurrent(pdbDoc->GetcurrentSpkListIndex());

	pDC->SelectObject(poldPen);
	pDC->SelectObject(poldBrush);
}

// -------------------------------------------------------------

void CViewSpikeHist::DisplayHistogram(CDC* pDC, CRect* pRect)
{
	CRect dispRect = *pRect;		// this will be the display rect for histogram

	// print text on horizontal lines
	TEXTMETRIC tm;					// load characteristics of the font
	pDC->GetTextMetrics(&tm);
	pDC->SetBkMode(TRANSPARENT);

	// define display rectangle and plot data within display area
	dispRect.top += tm.tmHeight* 2 + tm.tmDescent;
	dispRect.bottom -= tm.tmHeight + 2 * tm.tmDescent;
	dispRect.left += 2*tm.tmHeight;
	dispRect.right -= 2*tm.tmHeight;

	// get data pointer and size
	long* phistog0= nullptr;			// pointer to first element
	int   nbinshistog=0;			// nelements
	switch(m_bhistType)
	{
	case 0:	// PSTH
		phistog0 = m_pPSTH;			// pointer to first element
		nbinshistog = m_sizepPSTH;
		break;
	case 1:	// ISI		
	case 2:	// AUTOCORR
		phistog0 = m_pISI;			// pointer to first element
		nbinshistog = m_sizepISI;
		break;
	default:
		break;
	}
	// plot data
	int histogmax = PlotHistog(pDC, &dispRect, nbinshistog, phistog0, 0, m_bhistType);

	// output legends: title of the graph and data description
	CRect commentRect = dispRect;	// comment rectangle
	if (m_bPrint)					// adjust comment position if printing
		commentRect = m_commentRect;
	else
		commentRect.top -= tm.tmHeight* 2; // + tm.tmDescent;
	
	CSize left;						// store extent of a given char
	CString strComment;   			// scratch pad
	GetFileInfos(strComment);		// file comments

	// histogram type and bin value
	float divisor=1.f;					// factor to normalize histograms
	m_xfirst = m_timefirst;			// abcissa first
	m_xlast = m_timelast;			// abcissa last
	float binms= 1.f;	
	switch(m_bhistType)
	{
	case 0:	// PSTH
		strComment += _T("PSTH (");		 // PSTH
		divisor = m_nPSTH * m_timebinms/t1000; // divide by the size of the bin
		binms = m_timebinms;
		break;
	case 1:	// ISI
		strComment += _T("ISI (");		// ISI
		divisor = m_timelast - m_timefirst;
		m_xfirst = 0;
		m_xlast = m_binISIms/t1000 * m_nbinsISI;
		binms = m_binISIms;
		break;
	case 2:	// AUTOCORR
		strComment += _T("AUTOCORR (");	
		divisor = m_timelast - m_timefirst;
		m_xfirst = - m_binISIms/t1000 * m_nbinsISI/2;
		m_xlast = -m_xfirst;
		binms = m_binISIms;
		break;
	default:
		break;
	}	
	CString csT2;
	csT2.Format(_T("bin:%1.1f ms)"), binms);		// bin size (ms)
	strComment += csT2;

	// display title (on 2 lines)
	UINT uiFlag = pDC->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	pDC->DrawText(strComment, 
			strComment.GetLength(), 
			commentRect, 
			DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	pDC->SetTextAlign(uiFlag);

	// print abcissa first and last values
	CString csXleft;
	CString csXright;
	switch(m_bhistType)
	{
	case 1:									// ISI
		csXleft=_T("0"); 
		csXright.Format(_T("%1.0f ms"), double(m_xlast)*t1000);
		break;
	case 2:									// AUTOCORR
		csXleft.Format(_T("%1.0f"), double(m_xfirst)*t1000); 
		csXright.Format(_T("%1.0f ms"), double(m_xlast)*t1000);
		break;
	default: 								// PSTH (default)
		csXleft.Format(_T("%1.3f"), m_xfirst); 
		csXright.Format(_T("%1.3f s"), m_xlast);
		break;
	}

	// origin
	pDC->TextOut(dispRect.left, dispRect.bottom + tm.tmDescent, csXleft);
	// end (align on the right)
	left = pDC->GetTextExtent(csXright);
	pDC->TextOut(dispRect.right - left.cx - tm.tmDescent, dispRect.bottom + tm.tmDescent, csXright);

	// print value of ordinate max vertically
	CFont vFont;
	LOGFONT	logvFont;

	memset(&logvFont, 0, sizeof(LOGFONT));			// prepare font
	((CFont*)pDC->GetCurrentFont( ))->GetLogFont(&logvFont);	// fill it from CDC
	logvFont.lfEscapement = +900;
	BOOL flag = vFont.CreateFontIndirect(&logvFont);
	CFont* pOldFont = pDC->SelectObject(&vFont);

	// compute histog max
	float ymax;
	if (m_pvdS->bYmaxAuto)
		ymax = (MulDiv (histogmax, m_rectratio, 100) / divisor) /m_nfiles;
	else
		ymax = m_pvdS->Ymax;

	CString csYmax;
	csYmax.Format(_T("%1.3f spk/s"), ymax);
	pDC->GetTextMetrics(&tm);
	left = pDC->GetTextExtent(csYmax);
	left.cy += tm.tmDescent;
	pDC->TextOut(dispRect.left-left.cy, dispRect.top+left.cx, csYmax);
	pDC->TextOut(dispRect.left-left.cy, dispRect.bottom, _T("0"));

	// restore normal font
	pDC->SetBkMode(OPAQUE);
	pDC->SelectObject(pOldFont);
	vFont.DeleteObject();

	return;
}

// -------------------------------------------------------------

void CViewSpikeHist::DisplayPSTHAutoc(CDC* pDC, CRect* pRect)
{
	// print text on horizontal lines
	TEXTMETRIC tm;					// load characteristics of the font
	pDC->GetTextMetrics(&tm);
	pDC->SetBkMode(TRANSPARENT);
	
	// define display rectangle
	CRect dispRect = *pRect;		// this will be the display rect for histogram
	dispRect.top += tm.tmHeight* 2 + tm.tmDescent;
	dispRect.bottom -= tm.tmHeight + 2 * tm.tmDescent;
	dispRect.left += 2*tm.tmHeight;
	dispRect.right -= 2*tm.tmHeight;

	// save old pen and brush / restore on exit
	CPen penbars;
	penbars.CreatePen(PS_SOLID, 0, m_pvdS->crHistBorder);
	CBrush brushbars;
	brushbars.CreateSolidBrush(m_pvdS->crHistFill);

	CPen* poldPen = (CPen*) pDC->SelectObject(&penbars);
	CBrush* poldBrush=(CBrush*) pDC->SelectStockObject(BLACK_BRUSH);

	// histogram type and bin value
	m_xfirst = m_timefirst;		// abcissa first
	m_xlast = m_timelast;		// abcissa last
	float binms= 1.f;

	// update Peristimulus-Autocorrelation histogram rectangle
	int ipsheight = MulDiv (dispRect.Width(), 25, 100);
	int iautocheight = MulDiv (dispRect.Height(), 25, 100);
	if (ipsheight < iautocheight)
		iautocheight = ipsheight;
	else
		ipsheight = iautocheight;
	int separator = MulDiv (ipsheight, 1, 15);

	int histog1_width = dispRect.Width() - iautocheight - separator;
	int histog1_height = dispRect.Height() - ipsheight - separator;
	CRect RectHistog = dispRect;
	RectHistog.right = RectHistog.left + histog1_width;
	RectHistog.bottom = RectHistog.top + histog1_height;

	// search for max to adapt the scale
	int maxval=0;
	
	for (int i = 0; i < m_sizeparrayISI; i++)
	{
		if (*(m_parrayISI+i) > maxval)
			maxval = *(m_parrayISI+i);
	}

	// display rectangle around the area with the intensity
	pDC->MoveTo(RectHistog.left-1,  RectHistog.top-1);
	pDC->LineTo(RectHistog.right+1, RectHistog.top-1);
	pDC->LineTo(RectHistog.right+1, RectHistog.bottom+1);
	pDC->LineTo(RectHistog.left-1,  RectHistog.bottom+1);
	pDC->LineTo(RectHistog.left-1,  RectHistog.top-1);
	int ymiddle = (RectHistog.top + RectHistog.bottom)/2;
	pDC->MoveTo(RectHistog.left-1,  ymiddle);
	pDC->LineTo(RectHistog.right+1,  ymiddle);

	// display color as small rectangles
	CRect drect = RectHistog;
	int drectheight = RectHistog.Height();
	int drectwidth = RectHistog.Width();	

	// loop over columns to pass over the different autoc
	for (int iPSTH = 0; iPSTH < m_sizepPSTH; iPSTH++)
	{
		int iautoc = iPSTH * m_nbinsISI;
		long* parray = m_parrayISI + (iPSTH * m_nbinsISI);
		drect.right = RectHistog.left + MulDiv((iPSTH+1), drectwidth, m_sizepPSTH);
		drect.bottom = RectHistog.bottom;
		// loop over all time increments of the local autocorrelation histogram
		for (iautoc = 0; iautoc < m_sizepISI; iautoc++)
		{
			drect.top = RectHistog.bottom - MulDiv((iautoc+1), drectheight, m_sizepISI);
			int val = *parray;
			parray++;
			int icolor = MulDiv (val, NB_COLORS, maxval);
			if (icolor > NB_COLORS) icolor = NB_COLORS;
			if (icolor > 0)
				pDC->FillSolidRect(&drect, m_pvdS->crScale[icolor]);
			drect.bottom = drect.top;
		}
		drect.left = drect.right;
	}

	// display peristimulus histogram -----------------------------------
	CRect periRectHistog = RectHistog;
	periRectHistog.right = periRectHistog.left + RectHistog.Width();
	periRectHistog.top   = RectHistog.bottom + separator;
	periRectHistog.bottom = dispRect.bottom;

	long maxPSTH = PlotHistog(pDC, &periRectHistog, m_sizepPSTH, m_pPSTH, 0, 0);

	// display autocorrelation -----------------------------------
	CRect autoRectHistog = RectHistog;
	autoRectHistog.left  = RectHistog.right + separator;
	autoRectHistog.right = autoRectHistog.left + periRectHistog.Height();
	
	long maxAUTOC = PlotHistog(pDC, &autoRectHistog, m_sizepISI, m_pISI, -1, 2);

	// display colour scale
	int deltaXpix = separator;
	int deltaYpix =	MulDiv(periRectHistog.Height(), 1, 18);
	drect.left = periRectHistog.right + deltaXpix;
	drect.right = drect.left + 2 * deltaXpix ;
	drect.top    = periRectHistog.top;
	CPoint scaletop (drect.right, drect.top);

	for (int j=17; j>= 0; j--)
	{
		drect.bottom = drect.top + deltaYpix;			// update rectangle coordinates
		pDC->FillSolidRect(&drect, m_pvdS->crScale[j]);	// fill rectangle with color
		pDC->MoveTo(drect.left-1,  drect.top-1);		// draw a horizontal bar
		pDC->LineTo(drect.right, drect.top-1);			// at the top of the rectangle
		drect.top = drect.bottom;						// update rectangle coordinates
	}

	pDC->MoveTo(drect.left-1,  drect.bottom);			// draw last bar on the bottom
	pDC->LineTo(drect.right+1, drect.bottom);
	
	pDC->MoveTo(drect.left-1,  scaletop.y-1);			// draw left line
	pDC->LineTo(drect.left-1,  drect.bottom);

	pDC->MoveTo(drect.right,  scaletop.y-1);			// draw right line
	pDC->LineTo(drect.right,  drect.bottom);
														// draw ticks
	pDC->MoveTo(drect.right,			scaletop.y+ deltaYpix -1);
	pDC->LineTo(drect.right+deltaXpix,	scaletop.y+ deltaYpix-1);
	pDC->MoveTo(drect.right,			drect.bottom);
	pDC->LineTo(drect.right+deltaXpix,	drect.bottom);


	// display comments
	CRect commentRect = dispRect;	// comment rectangle
	if (m_bPrint)					// adjust comment position if printing
		commentRect = m_commentRect;
	else
		commentRect.top -= tm.tmHeight* 2;

	CString strComment;   			// scratch pad
	GetFileInfos(strComment);		// file comments
	strComment += _T("Peristimulus-Autocorrelation");
	UINT uiFlag = pDC->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
	pDC->DrawText(strComment, 
			strComment.GetLength(), 
			commentRect, 
			DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK);
	pDC->SetTextAlign(uiFlag);

	// display abcissa of PSH
	CString csXleft;
	CString csXright;
	csXleft.Format(_T("%1.3f"), m_timefirst);
	csXright.Format(_T("%1.3f s"), m_timelast);
	pDC->TextOut(periRectHistog.left, periRectHistog.bottom + tm.tmDescent, csXleft);
	CSize left = pDC->GetTextExtent(csXright);
	pDC->TextOut(periRectHistog.right - left.cx - tm.tmDescent, 
		periRectHistog.bottom + tm.tmDescent, csXright);

	// display scale max value
	CString csSpkpers;
	csSpkpers.Format(_T("%i spk / bin"), maxval);

	uiFlag = pDC->SetTextAlign(TA_TOP | TA_LEFT | TA_NOUPDATECP);
	pDC->TextOut(scaletop.x + deltaXpix, scaletop.y + deltaYpix -tm.tmDescent, csSpkpers);

	uiFlag = pDC->SetTextAlign(TA_BOTTOM | TA_LEFT | TA_NOUPDATECP);
	pDC->TextOut(scaletop.x + deltaXpix, drect.top + tm.tmDescent, _T("0"));
	pDC->SetTextAlign(uiFlag);

	// display abcissa of autocorrelation
	// print value of ordinate max vertically
	CFont vFont;		// vertical font
	LOGFONT	logvFont;	// array describing font parameters 
	memset(&logvFont, 0, sizeof(LOGFONT));				// prepare font
	((CFont*)pDC->GetCurrentFont( ))->GetLogFont(&logvFont);	// fill it from CDC
	logvFont.lfEscapement = +900;						// angle
	BOOL flag = vFont.CreateFontIndirect(&logvFont);	// create font
	CFont* pOldFont = pDC->SelectObject(&vFont);		// select font (now we can display txt)

	float bin_s = m_pvdS->binISI * 1000.f * m_nbinsISI /2.f;
	int leftx = autoRectHistog.right +tm.tmDescent;
	CString csXautoc;
	csXautoc.Format(_T("%1.0f ms"), bin_s);

	pDC->SetTextAlign(TA_TOP | TA_RIGHT | TA_NOUPDATECP);	// max autoc abcissa
	pDC->TextOut(leftx, autoRectHistog.top, csXautoc);

	csXautoc.Format(_T("-%1.0f"), bin_s);
	pDC->SetTextAlign(TA_TOP | TA_LEFT | TA_NOUPDATECP);	// min autoc abcissa
	pDC->TextOut(leftx, autoRectHistog.bottom, csXautoc);

	pDC->SetTextAlign(TA_TOP | TA_CENTER | TA_NOUPDATECP);	// center autoc abcissa
	pDC->TextOut(leftx, (autoRectHistog.bottom + autoRectHistog.top) / 2, _T("0"));

	// end of vertical font...
	pDC->SelectObject(pOldFont);		// reselect old font
	vFont.DeleteObject();				// delete vertical font
	
	pDC->SelectObject(poldPen);
	pDC->SelectObject(poldBrush);
	pDC->SetBkMode(OPAQUE);
}


// assume:
//	m_pvdS

void CViewSpikeHist::DisplayStim(CDC* pDC, CRect* pRect, long* l_first, long* l_last)
{
	// draw rectangle for stimulus
	if (m_pSpkDoc->m_stimIntervals.nitems <= 0)
		return;

	CPen bluepen;
	bluepen.CreatePen(PS_SOLID, 0, m_pvdS->crStimBorder);
	CPen* poldP = (CPen*) pDC->SelectObject(&bluepen);

	// search first stimulus transition within interval
	long iistart = *l_first; 
	long iiend = *l_last; 
	long iilen = iiend - iistart;
	int i0 = 0;
	while (i0 < m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize() 
			&& m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(i0) < iistart)
		i0++;				// loop until found

	int displen = pRect->Width();
	int top = pRect->top;
	int bottom = pRect->bottom;
	if (top > bottom)
	{
		top = bottom;
		bottom = pRect->top;
	}
	if ((bottom - top) < 2)
		bottom = top+2;
	bottom++;

	// start looping from the first interval that meet the criteria
	// set baseline according to the interval (broken pulse?)
	int istate =bottom;		// use this variable to keep track of pulse broken by display limits
	int ii = (i0/2)*2;		// keep index of the ON transition
	if (ii != i0)
		istate = top;
	pDC->MoveTo(pRect->left, istate);

	for (ii; ii< m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize(); ii++, ii++)
	{
		// stim starts here
		int iix0 = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(ii) - iistart;
		if (iix0 >= iilen)				// first transition ON after last graph pt?
			break;						// yes = exit loop

		if (iix0 < 0)					// first transition off graph?
			iix0 = 0;					// yes = clip

		iix0 = MulDiv(displen, iix0, iilen) + pRect->left;
		pDC->LineTo(iix0, istate);		// draw line up to the first point of the pulse
		pDC->LineTo(iix0, top);			// draw vertical line to top of pulse

		// stim ends here
		istate = bottom;				// after pulse, descend to bottom level
		int iix1 = iilen;
		if (ii < m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize()-1)
			iix1 = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(ii+1) - iistart;
		if (iix1 > iilen)				// last transition off graph?
		{
			iix1 = iilen;				// yes = clip
			istate = top;				// do not descend..
		}

		iix1 = MulDiv(displen, iix1, iilen) + pRect->left+1;

		pDC->LineTo(iix1, top);			// draw top of pulse
		pDC->LineTo(iix1, istate);		// draw descent to bottom line
	}

	// end of loop - draw the rest
	pDC->LineTo(pRect->right, istate);
	pDC->SelectObject(poldP);
}

void CViewSpikeHist::OnSelchangeHistogramtype() 
{
	int i = ((CListBox*) GetDlgItem(IDC_LIST1))->GetCurSel();	// CListBox
	if (m_bhistType ==i)
		return;		
	ShowControls(i);
	OnDisplay();
}


void CViewSpikeHist::OnEnChangeEditnstipercycle() 
{
	m_pvdS->nstipercycle = GetDlgItemInt(IDC_EDITNSTIPERCYCLE);	
	OnDisplay();
}

void CViewSpikeHist::OnEnChangeEditlockonstim() 
{
	if (m_pSpkDoc == nullptr)
		return;
	int ilock = GetDlgItemInt(IDC_EDITLOCKONSTIM);
	if (ilock != m_pvdS->istimulusindex)
	{
		if (ilock >= m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize())
			ilock = m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize()-1;
		if (ilock <0)
			ilock = 0;
		m_pvdS->istimulusindex = ilock;
		SetDlgItemInt(IDC_EDITLOCKONSTIM, m_pvdS->istimulusindex);
	}
	OnDisplay();
}


void CViewSpikeHist::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1) != pScrollBar)
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);

	// Get the current position of scroll box.
	int curpos = pScrollBar->GetScrollPos();
	float delta = 0.f;

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:			// Scroll to far left - next frame forwards
		delta = m_timelast - m_timefirst;
		break;
	case SB_RIGHT:			// Scroll to far right.
		delta = m_timefirst- m_timelast;
		break;
	case SB_ENDSCROLL:		// End scroll.
		delta = (m_timelast - m_timefirst) * (curpos - 50) / 100;
		break;
	case SB_LINELEFT:		// Scroll left.
	   delta = -2*m_timebinms / t1000;
	  break;
	case SB_LINERIGHT:		// Scroll right.
		delta = 2*m_timebinms / t1000;
		break;
	case SB_PAGELEFT:		// Scroll one page left.
		delta = m_timefirst- m_timelast;
		break;
	case SB_PAGERIGHT:		// Scroll one page right.
		delta = m_timelast - m_timefirst;
		break;
	case SB_THUMBPOSITION:	// Scroll to absolute position. nPos is the position
		curpos = nPos;		// of the scroll box at the end of the drag operation.
		return;				// no action
		break;
	case SB_THUMBTRACK:		// Drag scroll box to specified position. nPos is the
		curpos = nPos;		// position that the scroll box has been dragged to.
		return;				// no action
		break;
	default:
		return;
		break;
   }

	// Set the new position of the thumb (scroll box).
	pScrollBar->SetScrollPos(50);
	int nbins = (int) (delta*t1000/m_timebinms);
	delta = m_timebinms * nbins / t1000;
	m_timefirst += delta;
	m_timelast += delta;
	m_pvdS->timestart=m_timefirst;
	m_pvdS->timeend = m_timelast;
	OnDisplay();
	UpdateData(FALSE);
}



void CViewSpikeHist::SelectSpkList(int icur, BOOL bRefreshInterface)
{
	if (bRefreshInterface)
	{
		// reset tab control
		m_tabCtrl.DeleteAllItems();
		// load list of detection parameters 
		int j = 0;
		for (int i = 0; i< m_pSpkDoc->GetSpkListSize(); i++)
		{
			CSpikeList* pSL = m_pSpkDoc->SetSpkListCurrent(i);
			CString cs;
			if (pSL->GetdetectWhat() != 0)
				continue;
			cs.Format(_T("#%i %s"), i, (LPCTSTR) pSL->GetComment());
			m_tabCtrl.InsertItem(j, cs);
			j++;
		}
	}

	// select spike list 
	GetDocument()->SetcurrentSpkListIndex(icur);
	m_tabCtrl.SetCurSel(icur);
}

void CViewSpikeHist::OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	SelectSpkList(icursel);
	OnDisplay();
	*pResult = 0;
}


void CViewSpikeHist::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	SelectSpkList(icursel);
	OnDisplay();					// display data
	*pResult = 0;
}
