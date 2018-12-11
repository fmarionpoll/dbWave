// Viewspikes.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave_constants.h"
#include "dbWave.h"
#include "resource.h"
#include <math.h>

#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"

#include "Spikedoc.h"
#include "Spikebar.h"
#include "spikeshape.h"
#include "SpikeClassListBox.h"

#include "Editspik.h"
#include "MainFrm.h"
#include "Copyasdl.h"
#include "ChildFrm.h"

#include ".\ViewSpikes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpikeView

IMPLEMENT_DYNCREATE(CSpikeView, CDaoRecordView)

CSpikeView::CSpikeView()
	: CDaoRecordView(CSpikeView::IDD), m_hBias(nullptr), m_hZoom(nullptr), m_yscaleFactor(0), m_VBarMode(0),
	  m_dbDoc(nullptr), m_pSpkList(nullptr), m_scrollFilePos_infos(), m_jitter(0)
{
	m_timefirst = 0.0f;
	m_timelast = 0.0f;
	m_spikeno = -1;
	m_spikenoclass = 0;
	m_zoom = 0.2f;
	m_sourceclass = 0;
	m_destclass = 1;
	m_bresetzoom = TRUE;
	m_bartefact = FALSE;
	m_jitter_ms = 1.0f;
	m_baddspikemode = FALSE;
	m_bInitSourceView = TRUE;
	m_bDatDocExists = FALSE;
	m_bSpkDocExists = FALSE;
	m_lFirst = 0;
	m_lLast = -1;
	m_maxclasses = 1;
	m_binit = FALSE;
	m_yWE = 1;
	m_yWO = 0;
	m_zoominteger = 1;
	m_ptVT = -1;
	m_rectVTtrack = CRect(0, 0, 0, 0);
	m_pSpkDoc = nullptr;
	m_pDataDoc = nullptr;
	m_bdummy = TRUE;
	mdPM = nullptr; // view data options
	mdMO = nullptr; // measure options
	m_psC = nullptr;
	m_pspkDP = nullptr; // spike detection parameters

	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

CSpikeView::~CSpikeView()
{	
	m_psC->vdestclass = m_destclass;
	m_psC->vsourceclass = m_sourceclass;
	m_psC->bresetzoom = m_bresetzoom;
	m_psC->fjitter_ms = m_jitter_ms;
}

BOOL CSpikeView::PreCreateWindow(CREATESTRUCT &cs)
{
// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

BEGIN_MESSAGE_MAP(CSpikeView, CDaoRecordView)

	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)	 
	ON_WM_SIZE()
	ON_WM_DESTROY()	
	ON_WM_SETFOCUS()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()

	ON_COMMAND(ID_FORMAT_ALLDATA, OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, OnFormatCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, OnFormatGainadjust)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, OnToolsEdittransformspikes)	
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_FORMAT_PREVIOUSFRAME, OnFormatPreviousframe)
	ON_COMMAND(ID_FORMAT_NEXTFRAME, OnFormatNextframe)
	ON_COMMAND(ID_RECORD_SHIFTLEFT, OnRecordShiftleft)
	ON_COMMAND(ID_RECORD_SHIFTRIGHT, OnRecordShiftright)
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)

	ON_EN_CHANGE(IDC_NSPIKES, OnEnChangeNOspike)
	ON_EN_CHANGE(IDC_EDIT2, OnEnChangeSpikenoclass)
	ON_EN_CHANGE(IDC_TIMEFIRST, OnEnChangeTimefirst)
	ON_EN_CHANGE(IDC_TIMELAST, OnEnChangeTimelast)
	ON_EN_CHANGE(IDC_EDIT3, OnEnChangeZoom)
	ON_EN_CHANGE(IDC_EDIT4, OnEnChangeSourceclass)
	ON_EN_CHANGE(IDC_EDIT5, OnEnChangeDestclass)
	ON_EN_CHANGE(IDC_JITTER, OnEnChangeJitter)
	
	ON_BN_CLICKED(IDC_BUTTON2, OnZoom)
	ON_BN_CLICKED(IDC_GAIN_button, OnGAINbutton)
	ON_BN_CLICKED(IDC_BIAS_button, OnBIASbutton)
	ON_BN_CLICKED(IDC_ARTEFACT, OnArtefact)
	
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CSpikeView::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CSpikeView::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


void CSpikeView::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_NSPIKES, m_spikeno);
	DDX_Text(pDX, IDC_EDIT2, m_spikenoclass);
	DDX_Text(pDX, IDC_EDIT3, m_zoom);
	DDX_Text(pDX, IDC_EDIT4, m_sourceclass);
	DDX_Text(pDX, IDC_EDIT5, m_destclass);
	DDX_Check(pDX, IDC_CHECK1, m_bresetzoom);
	DDX_Check(pDX, IDC_ARTEFACT, m_bartefact);
	DDX_Text(pDX, IDC_JITTER, m_jitter_ms);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}


/////////////////////////////////////////////////////////////////////////////
// CSpikeView message handlers
void CSpikeView::OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		CMainFrame* pmF = (CMainFrame*)AfxGetMainWnd();
		pmF->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	else
	{
		if (this != pActivateView && this == pDeactiveView)
		{
			SaveCurrentFileParms();
			// save column parameters
			m_psC->colseparator = m_spkClass.GetColsSeparatorWidth();
			m_psC->rowheight = m_spkClass.GetRowHeight();
			m_psC->colspikes  = m_spkClass.GetColsSpikesWidth();
			m_psC->coltext = m_spkClass.GetColsTextWidth();

			CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
			if (pApp->m_pviewspikesMemFile == nullptr)
			{
				pApp->m_pviewspikesMemFile = new CMemFile;
				ASSERT(pApp->m_pviewspikesMemFile != NULL);
			}

			CArchive ar(pApp->m_pviewspikesMemFile, CArchive::store);
			pApp->m_pviewspikesMemFile->SeekToBegin();
			m_sourceView.Serialize(ar);
			ar.Close();					// close archive
		}
		// set bincrflagonsave
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		pApp->vdS.bincrflagonsave = ((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->GetCheck();
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}


// --------------------------------------------------------------------------
void CSpikeView::OnSize(UINT nType, int cx, int cy) 
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

		// set track rectangle
		CRect rect0, rect1, rect2;
		GetWindowRect(&rect0);
		m_sourceView.GetWindowRect(&rect1);
		m_spkClass.GetWindowRect(&rect2);
		m_rectVTtrack.top = rect1.top - rect0.top;
		m_rectVTtrack.bottom = rect2.bottom - rect0.top;
		m_rectVTtrack.left = rect1.left - rect0.left;
		m_rectVTtrack.right = rect1.right - rect0.left;
	}
	CDaoRecordView::OnSize(nType, cx, cy);
}

// --------------------------------------------------------------------------
BOOL CSpikeView::OnMove(UINT nIDMoveCommand) 
{
	SaveCurrentFileParms();
	BOOL flag = CDaoRecordView::OnMove(nIDMoveCommand);
	CdbWaveDoc* pDoc = GetDocument();
	if (pDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
	{
		((CChildFrame*)GetParent())->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		return false;
	}

	pDoc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

// --------------------------------------------------------------------------
void CSpikeView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_DOCHASCHANGED:		// file has changed?
	case HINT_DOCMOVERECORD:
		UpdateFileParameters();
		UpdateLegends(TRUE);
		break;
	case HINT_CLOSEFILEMODIFIED:	// close modified file: save
		SaveCurrentFileParms();
		break;
	case HINT_REPLACEVIEW:
	default:
		break;
	}
}


// --------------------------------------------------------------------------
LRESULT CSpikeView::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	int threshold = LOWORD(lParam);	// value associated	

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		if (threshold > CURSOR_MEASURE)	 // clip cursor shape to max 2
			threshold = 0;
		if (m_sourceView.GetMouseCursorType() != threshold)
			SetAddspikesMode(threshold);
		m_sourceView.SetMouseCursorType(threshold);
		m_spkClass.SetMouseCursorType(threshold);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(threshold, 0));
		break;
	
	case HINT_DEFINEDRECT:
		if (m_sourceView.GetMouseCursorType() == CURSOR_MEASURE)
		{
			CRect rect = m_sourceView.GetDefinedRect();
			long left = m_sourceView.GetDataOffsetfromPixel(rect.left);
			long right = m_sourceView.GetDataOffsetfromPixel(rect.right);
			// set a tag in the center of the rectangle
			m_sourceView.AddVTLtag((left+right)/2);
			m_sourceView.Invalidate();
		}
		break;

	case HINT_SELECTSPIKES:
		m_sourceView.Invalidate();
		m_spkClass.Invalidate();
		break;

// ----------------------------- select bar/display bars or zoom		
// if (HIWORD(lParam) == IDC_DISPLAYBARS || HIWORD(lParam) == NULL)
	case HINT_CHANGEHZLIMITS:		// abcissa have changed
	case HINT_CHANGEZOOM:
	case HINT_VIEWSIZECHANGED:		// change zoom
		if (HIWORD(lParam) == IDC_BUTTON1)
		{
			m_lFirst = m_sourceView.GetDataFirst();
			m_lLast = m_sourceView.GetDataLast();
		}		
		else if (HIWORD(lParam) == IDC_LISTCLASSES) //[ne marche pas! HIWORD(lParam)==1]
		{
			m_lFirst = m_spkClass.GetTimeFirst();
			m_lLast = m_spkClass.GetTimeLast();
		}
		UpdateLegends(FALSE);
		break;

	case HINT_HITSPIKE:
		SelectSpike(threshold);
		break;

	case HINT_DBLCLKSEL:
		if (threshold < 0) 
			threshold = 0;
		m_spikeno=threshold;			
		OnToolsEdittransformspikes();
		break;

	case HINT_DROPPED:
		m_pSpkDoc->SetModifiedFlag();
		m_spikenoclass = m_pSpkList->GetSpikeClass(m_spikeno);
		UpdateData(FALSE);
		break;

	case HINT_WINDOWPROPSCHANGED:
		mdPM->spkviewdata = m_sourceView.m_parms;
		break;

	case HINT_HITSPIKE_SHIFT:
		SelectSpike(threshold);
		break;

	default:
		break;
	}
	return 0L;
}

// 
BOOL CSpikeView::AddSpiketoList(long iitime, BOOL bcheck_if_otheraround)
{
	long lFirst0 = m_lFirst;
	long lLast0 = m_lLast;

	int method = m_pSpkList->GetdetectTransform();
	int docChan	= m_pSpkList->GetextractChan(); 
	int prethreshold = m_pSpkList->GetSpikePretrig();
	int refractory	= m_pSpkList->GetSpikeRefractory();
	int postthreshold = m_pSpkList->GetextractNpoints() - prethreshold;
	int spikelen = m_pSpkList->GetSpikeLength();

	// get parameters from document
	int nchans;									// number of data chans / source buffer
	short* pBuf = m_pDataDoc->LoadRawDataParams(&nchans);
	int nspan = m_pDataDoc->GetTransfDataSpan(method);	// nb pts to read before transf

	long iitime0 = iitime - prethreshold;
	long lRWFirst = iitime0 ;			// first point (eventually) needed
	long lRWLast = iitime0 + spikelen;	// last pt needed
	if (!m_pDataDoc->LoadRawData(&lRWFirst, &lRWLast, nspan))
		return FALSE;					// exit if error
	//short* pData = 
	m_pDataDoc->LoadTransfData(lRWFirst, lRWLast, method, docChan);
	short* pDataSpike0 = m_pDataDoc->GetpTransfDataElmt(iitime0 - lRWFirst);

	// add a new spike if no spike is found around
	int spikeindex;
	int jitter = 0;
	BOOL bfound = FALSE;
	if (bcheck_if_otheraround)
	{
		jitter = int((m_pSpkDoc->GetAcqRate() * m_jitter) / 1000);
		bfound = m_pSpkList->IsAnySpikeAround(iitime0+prethreshold, jitter, spikeindex, docChan);
	}

	if (!bfound)
	{
		spikeindex = m_pSpkList->AddSpike(pDataSpike0,		//lpSource	= buff pointer to the buffer to copy
						1, //offset, /*nchans,*/			//nchans	= nb of interleaved channels
						iitime0+prethreshold,				//time = file index of first pt of the spk
						docChan,							//detectChan	= data source chan index
						m_destclass, bcheck_if_otheraround);

		m_pSpkDoc->SetModifiedFlag();
	}

	// else change the spike class (if necessary)
	else if (m_pSpkList->GetSpikeClass(spikeindex) != m_destclass)
	{
		m_pSpkList->SetSpikeClass(spikeindex, m_destclass);
		m_pSpkDoc->SetModifiedFlag();
	}

	// save the data into the file
	if (m_pSpkDoc->IsModified())
	{
		m_pSpkList->UpdateClassList();
		m_pSpkDoc->OnSaveDocument(GetDocument()->DBGetCurrentSpkFileName(FALSE));
		m_pSpkDoc->SetModifiedFlag(FALSE);
		GetDocument()->Setnbspikes(m_pSpkList->GetTotalSpikes());
		GetDocument()->Setnbspikeclasses(m_pSpkList->GetNbclasses());

		BOOL boldparm = m_bresetzoom;
		m_bresetzoom = FALSE;
		UpdateFileParameters();
		m_bresetzoom = boldparm;
	}

	SelectSpike(spikeindex);
	UpdateLegends(TRUE);	
	m_spkClass.Invalidate();
	return TRUE;
}

// --------------------------------------------------------------------------

void CSpikeView::SelectSpike(int spikeno)
{
	if (!m_bSpkDocExists)
		return;
	if (spikeno >= m_pSpkList->GetTotalSpikes())
		spikeno = -1;
	m_spikeno = spikeno;
	m_pSpkList->m_selspike = spikeno;
	m_spkClass.SelectSpike(spikeno);

	long spkFirst = 0;
	long spkLast = 0;
	m_spikenoclass = -1;
	int nCmdShow;
	if (spikeno >= 0 && spikeno < m_pSpkList->GetTotalSpikes())
	{
		// get address of spike parms
		CSpikeElemt* pS = m_pSpkList->GetSpikeElemt(m_spikeno);
		m_spikenoclass = pS->GetSpikeClass();
		m_bartefact = (m_spikenoclass < 0);
		spkFirst = pS->GetSpikeTime() - m_pSpkList->GetSpikePretrig();
		spkLast = spkFirst + m_pSpkList->GetSpikeLength();
		nCmdShow = SW_SHOW;
		if (m_bDatDocExists)
		{
			m_DWintervals.SetAt(3, spkFirst);
			m_DWintervals.SetAt(4, spkLast);
			m_sourceView.SetHighlightData(&m_DWintervals);
			m_sourceView.Invalidate();
		}		
	}
	else
	{
		nCmdShow = SW_HIDE;		
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_ARTEFACT)->ShowWindow(nCmdShow);
		
	UpdateData(FALSE);
}

// --------------------------------------------------------------------------
// OnEnChangeSpikeno
// update edit control and update all infos displayed (check boundaries)
// spikeno = spike index displayed
// --------------------------------------------------------------------------

void CSpikeView::OnEnChangeNOspike()
{	
	if (!mm_spikeno.m_bEntryDone)
		return;
	int spikeno = m_spikeno;
	switch (mm_spikeno.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikeno++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikeno--;	break;
	}

	// check boundaries
	if (m_spikeno<0)
		m_spikeno = -1;
	if (m_spikeno >= m_pSpkList->GetTotalSpikes())
		m_spikeno = m_pSpkList->GetTotalSpikes()-1;

	mm_spikeno.m_bEntryDone=FALSE;	// clear flag
	mm_spikeno.m_nChar=0;			// empty buffer
	mm_spikeno.SetSel(0, -1);		// select all text    
	if (m_spikeno != spikeno)		// change display if necessary
	{
		SelectSpike(m_spikeno);
		if (m_spikeno >= 0)
		{
			// test if spike visible in the current time interval
			CSpikeElemt* pS = m_pSpkList->GetSpikeElemt(m_spikeno);
			long spkFirst = pS->GetSpikeTime() - m_pSpkList->GetSpikePretrig();
			long spkLast = spkFirst + m_pSpkList->GetSpikeLength();
			long lcenter = (spkLast + spkFirst)/2;

			if (spkFirst < m_lFirst || spkLast > m_lLast)
			{
				long lspan = (m_lLast - m_lFirst)/2;
				m_lFirst = lcenter - lspan;
				m_lLast  = lcenter + lspan;
				UpdateLegends(FALSE);
			}
			// center curve vertically
			int ixpixel = MulDiv(lcenter - m_lFirst, m_sourceView.GetNxPixels(), m_lLast - m_lFirst);
			int ival = m_sourceView.GetChanlistBinAt(0, ixpixel);
			m_sourceView.SetChanlistYzero(0, ival);
			// display data
			m_spkClass.Invalidate();
			m_sourceView.Invalidate();
		}
	}
	else
		UpdateData(FALSE);
}

// --------------------------------------------------------------------------

void CSpikeView::OnInitialUpdate()
{
	CdbWaveDoc* pdbDoc = GetDocument();
	CDaoRecordView::OnInitialUpdate();
	
	// attach controls
	VERIFY(mm_spikeno.SubclassDlgItem(IDC_NSPIKES, this));
	mm_spikeno.ShowScrollBar(SB_VERT);
	VERIFY(mm_spikenoclass.SubclassDlgItem(IDC_EDIT2, this));
	mm_spikenoclass.ShowScrollBar(SB_VERT);
	VERIFY(m_spkClass.SubclassDlgItem(IDC_LISTCLASSES, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100,FALSE);
	VERIFY(m_sourceView.SubclassDlgItem(IDC_BUTTON1, this));
	VERIFY(mm_zoom.SubclassDlgItem(IDC_EDIT3, this));
	mm_zoom.ShowScrollBar(SB_VERT);
	VERIFY(mm_sourceclass.SubclassDlgItem(IDC_EDIT4, this));
	mm_sourceclass.ShowScrollBar(SB_VERT);
	VERIFY(mm_destclass.SubclassDlgItem(IDC_EDIT5, this));
	mm_destclass.ShowScrollBar(SB_VERT);
	VERIFY(mm_jitter_ms.SubclassDlgItem(IDC_JITTER, this));	

	// left scrollbar and button
	VERIFY(m_scrolly.SubclassDlgItem(IDC_SCROLLY_scrollbar, this));
	m_scrolly.SetScrollRange(0, 100);
	m_hBias=AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom=AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hBias);
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)(HANDLE)m_hZoom);

	// load global parameters
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	mdPM = &(pApp->vdP);					// viewdata options
	mdMO = &(pApp->vdM);					// measure options
	m_psC= &(pApp->spkC);					// get address of spike classif parms	
	m_destclass = m_psC->vdestclass;
	m_sourceclass = m_psC->vsourceclass;
	m_bresetzoom = m_psC->bresetzoom;
	m_jitter_ms = m_psC->fjitter_ms;

	// adjust size of the row and cols with text, spikes, and bars
	CRect rect;
	GetDlgItem(IDC_LISTCLASSES)->GetWindowRect(&rect);
	m_spkClass.SetRowHeight(m_psC->rowheight);
	CRect rect2;				
	GetDlgItem(IDC_BUTTON1)->GetWindowRect(&rect2);
	int coldata = rect2.Width()+2;
	int leftcolwidth = rect2.left - rect.left -2;
	m_spkClass.SetLeftColWidth(leftcolwidth);
	if (m_psC->coltext < 0)
	{
		m_psC->colspikes = m_psC->rowheight;
		m_psC->coltext = leftcolwidth - 2*m_psC->colseparator - m_psC->colspikes;
		if (m_psC->coltext < 20)
		{
			int half = leftcolwidth - m_psC->colseparator;
			m_psC->colspikes = half;
			m_psC->coltext = half;
		}
	}
	m_psC->coltext = leftcolwidth - m_psC->colspikes - 2*m_psC->colseparator;
	m_spkClass.SetColsWidth(m_psC->coltext, m_psC->colspikes, m_psC->colseparator);	

	m_stretch.AttachParent(this);		// attach formview pointer
	m_stretch.newProp(IDC_LISTCLASSES, 		XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1,				XLEQ_XREQ, SZEQ_YBEQ);
	
	m_stretch.newProp(IDC_BUTTON1, 			XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_TIMEINTERVALS,	SZEQ_XLEQ, SZEQ_YBEQ); 
	m_stretch.newProp(IDC_TIMEFIRST,  		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMELAST,  		SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SCROLLBAR1, 		XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_GAIN_button, 		SZEQ_XREQ, SZEQ_YTEQ);	
	m_stretch.newProp(IDC_BIAS_button,		SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar,SZEQ_XREQ, SZEQ_YTEQ);

	m_binit = TRUE;

	// init relation with document, display data, adjust parameters
	m_sourceView.m_parms = mdPM->spkviewdata;
	// set bincrflagonsave
	((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->SetCheck(pApp->vdS.bincrflagonsave);

	UpdateFileParameters();
	UpdateLegends(TRUE);
	if (m_baddspikemode)
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_CURSORMODE_MEASURE, NULL);
		m_sourceView.SetTrackSpike(m_baddspikemode, m_pspkDP->extractNpoints, m_pspkDP->prethreshold, m_pspkDP->extractChan);		
	}
}

/////////////////////////////////////////////////////////////////////////////
// remove objects
void CSpikeView::OnDestroy() 
{
	CDaoRecordView::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CSpikeView diagnostics

#ifdef _DEBUG
void CSpikeView::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CSpikeView::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CSpikeView::GetDocument()
{ 
	return (CdbWaveDoc*)m_pDocument; 
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CdbWaveView database support

CDaoRecordset* CSpikeView::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}

/////////////////////////////////////////////////////////////////////////////


//----------------------------------------------------------------------------

void CSpikeView::UpdateLegends(BOOL bFirst)
{
	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast <= m_lFirst)
		m_lLast = m_lFirst + 120;
	if (m_lLast >= m_pSpkDoc->GetAcqSize())
		m_lLast = m_pSpkDoc->GetAcqSize()-1;
	if (m_lFirst > m_lLast)
		m_lFirst = m_lLast -120;

	// update comments and time
	if (bFirst)
	{
		// if data found, pass data to the display button
		if (m_bDatDocExists)
		{
			GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_SHOW);
			m_sourceView.SetbUseDIB(FALSE);
			m_sourceView.AttachDataFile(m_pDataDoc, m_pDataDoc->GetDOCchanLength());

			int isourceview = m_pSpkList->GetextractChan();
			if (isourceview >= m_pDataDoc->GetpWaveFormat()->scan_count)
			{
				m_pSpkList->SetextractChan(0);
				isourceview = 0;
			}
			if(m_pSpkList->GetdetectWhat() == 1) 
			{
				isourceview = m_pSpkList->GetdetectChan();
				if (isourceview >= m_pDataDoc->GetpWaveFormat()->scan_count)
				{
					m_pSpkList->SetdetectChan(0);
					isourceview = 0;
				}
			}

			// set detection channel
			if (m_sourceView.SetChanlistSourceChan(0, isourceview) < 0)
			{
				m_sourceView.RemoveAllChanlistItems();
			}
			else
			{
				m_sourceView.ResizeChannels(m_sourceView.Width(), m_lLast - m_lFirst);
				m_sourceView.GetDataFromDoc(m_lFirst, m_lLast);
				if (m_bInitSourceView)
				{
					m_bInitSourceView=FALSE;
					int max, min;
					m_sourceView.GetChanlistMaxMin(0, &max, &min);
					int iextent = MulDiv(max-min+1, 11, 10);
					int izero = (max+min)/2;
					m_sourceView.SetChanlistYextent(0, iextent);
					m_sourceView.SetChanlistYzero(0, izero);
				}
			}
			m_DWintervals.SetSize(3+2);					// total size
			m_DWintervals.SetAt(0, 0);					// source channel
			m_DWintervals.SetAt(1, (DWORD) RGB(255,   0,   0));	// red color
			m_DWintervals.SetAt(2, 1);					// pen size
			m_DWintervals.SetAt(3, 0);					// pen size
			m_DWintervals.SetAt(4, 0);					// pen size		
		}
		else
			GetDlgItem(IDC_BUTTON1)->ShowWindow(SW_HIDE);
	}

	// set cursor
	HWND hwnd = GetSafeHwnd();	
	if (!m_baddspikemode)
		hwnd = nullptr;
	m_sourceView.ReflectMouseMoveMessg(hwnd);
	m_spkClass.ReflectBarsMouseMoveMessg(hwnd);
	m_sourceView.SetTrackSpike(m_baddspikemode, m_pspkDP->extractNpoints, m_pspkDP->prethreshold, m_pspkDP->extractChan);

	// update spike bars & forms CListBox
	if (m_lFirst != m_spkClass.GetTimeFirst() 
		|| m_lLast != m_spkClass.GetTimeLast())
		m_spkClass.SetTimeIntervals(m_lFirst, m_lLast);

	// update text abcissa and horizontal scroll position
	m_timefirst = m_lFirst/m_pSpkDoc->GetAcqRate();
	m_timelast = (m_lLast+1)/m_pSpkDoc->GetAcqRate();	
	m_sourceView.GetDataFromDoc(m_lFirst, m_lLast);

	// store current file settings
	m_pSpkList->m_lFirstSL = m_lFirst;
	m_pSpkList->m_lLastSL = m_lLast;

	// update scrollbar and select spikes
	SelectSpike(m_spikeno);
	UpdateScrollBar();	
}


//---------------------------------------------------------------------------
// UpdateFileParameters()
//---------------------------------------------------------------------------

void CSpikeView::UpdateFileParameters()
{
	// init data view
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) m_pDocument;
	pdbDoc->DBGetCurrentSpkFileName(FALSE);
	BOOL flag = GetDocument()->OpenCurrentSpikeFile();
	m_pSpkDoc = pdbDoc->m_pSpk;

	// init spike views
	if (!flag)
	{
		m_bSpkDocExists = FALSE;
		m_spkClass.SetSourceData(nullptr, nullptr);
	}
	else
	{
		m_bSpkDocExists = TRUE;
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->DBGetCurrentSpkFileName(), FALSE);
		m_pSpkDoc->SetSpkListCurrent(GetDocument()->GetcurrentSpkListIndex());
		int icur = m_pSpkDoc->GetSpkListCurrentIndex();
		m_pspkDP =  m_pSpkDoc->GetSpkListCurrent()->GetDetectParms();

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
		
		// select spike list 
		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(icur);
		m_tabCtrl.SetCurSel(icur);
		if (!m_pSpkList->IsClassListValid())		// if class list not valid:
		{
			m_pSpkList->UpdateClassList();			// rebuild list of classes
			m_pSpkDoc->SetModifiedFlag();			// and set modified flag
		}
		m_spkClass.SetSourceData(m_pSpkList, m_pSpkDoc);	// tell CListBox where spikes are
		m_spkClass.SetCurSel(0);					// select first line from listbox

		// display classes
		ASSERT(m_lFirst >= 0);
		if (m_bresetzoom)
		{
			m_spkClass.SetRedraw(FALSE);
			OnZoom();
			m_spkClass.SetRedraw(TRUE);
		}
		else if (m_lLast > m_pSpkDoc->GetAcqSize()-1 || m_lLast <= m_lFirst)
			m_lLast = m_pSpkDoc->GetAcqSize()-1;	// clip to the end of the data		
		m_spkClass.SetTimeIntervals(m_lFirst, m_lLast);
		if (m_yWE == 1)
		{
			// adjust gain of spkform
			int max, min;		
			m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
			m_yWE = MulDiv(max-min+1, 10, 8);
			m_yWO = max/2 + min/2;		
		}
		m_spkClass.SetYzoom(m_yWE, m_yWO);
	}

	// get source data
	CString docname = GetDocument()->DBGetCurrentDatFileName();
	m_bDatDocExists = GetDocument()->OpenCurrentDataFile();
	m_pDataDoc = nullptr;
	if (m_bDatDocExists)
		m_pDataDoc = GetDocument()->m_pDat;
	m_spikeno = m_pSpkList->m_selspike;
	if (m_spikeno > m_pSpkList->GetTotalSpikes()-1)
		m_spikeno = -1;	

	// update interface elements
	UpdateScrollBar();	
}

// select one of the spikelist item in the spike list array
// icursel = actual index in the array (indirection / ctrlTab)

void CSpikeView::SelectSpkList(int icursel)
{
	m_pSpkList = m_pSpkDoc->SetSpkListCurrent(icursel);
	GetDocument()->SetcurrentSpkListIndex(icursel);
	ASSERT(m_pSpkList != NULL);
	m_spkClass.SetSpkList(m_pSpkList);
	m_spkClass.Invalidate();
	m_pspkDP = m_pSpkList->GetDetectParms();	

	// update source data: change data channel and update display
	int isourceview = m_pSpkList->GetextractChan();
	if(m_pSpkList->GetdetectWhat() == 1)
		isourceview = m_pSpkList->GetdetectChan();

	// no data available
	if (m_sourceView.SetChanlistSourceChan(0, isourceview) <0)
	{
		m_sourceView.RemoveAllChanlistItems();
	}
	// data are ok
	else
	{
		m_sourceView.ResizeChannels(m_sourceView.Width(), m_lLast - m_lFirst);
		m_sourceView.GetDataFromDoc(m_lFirst, m_lLast);
		int max, min;
		m_sourceView.GetChanlistMaxMin(0, &max, &min);
		int iextent = MulDiv(max-min+1, 11, 10);
		int izero = (max+min)/2;
		m_sourceView.SetChanlistYextent(0, iextent);
		m_sourceView.SetChanlistYzero(0, izero);
	}
	m_sourceView.Invalidate();
}

// --------------------------------------------------------------------------

void CSpikeView::SaveCurrentFileParms()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		if (!m_pSpkList->IsClassListValid())	// if class list not valid:
			m_pSpkList->UpdateClassList();		// rebuild list of classes
		CString spkdoc = GetDocument()->DBGetCurrentSpkFileName(FALSE);
		m_pSpkDoc->OnSaveDocument(spkdoc);

		int nclasses=1;
		int ntotalspikes = m_pSpkList->GetTotalSpikes();
		if (ntotalspikes > 0)
		{
			if (!m_pSpkList->IsClassListValid())		// if class list not valid:
				nclasses = m_pSpkList->UpdateClassList();
			else
				nclasses = m_pSpkList->GetNbclasses();
		}
		GetDocument()->Setnbspikes(ntotalspikes);
		GetDocument()->Setnbspikeclasses(nclasses);

		// change flag is button is checked
		if (((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck())
		{
			int flag = GetDocument()->DBGetCurrentRecordFlag();
			flag++;
			GetDocument()->DBSetCurrentRecordFlag(flag);
		}
	}	
}

// --------------------------------------------------------------------------

void CSpikeView::OnFormatAlldata() 
{
	// dots: spk file length
	m_lFirst = 0;
	m_lLast = m_pSpkDoc->GetAcqSize()-1;
	// spikes: center spikes horizontally and adjust hz size of display	
	short xWO = 0;
	short xWE = m_pSpkList->GetSpikeLength();
	m_spkClass.SetXzoom(xWE, xWO);

	UpdateLegends(FALSE);
	// display data
	m_spkClass.Invalidate();
	m_sourceView.Invalidate();
}

// --------------------------------------------------------------------------

void CSpikeView::OnFormatCentercurve() 
{
	/*
	if (m_bSpkDocExists)
	{
		// loop over all spikes of the list 
		int nspikes = m_pSpkList->GetTotalSpikes();
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();		// get pointer to application
		SPKCLASSIF* m_psC= &(pApp->spkC);					// get address of spike classif parms
		short it1 = m_psC->ileft;
		short it2 = m_psC->iright;		
		for (int ispike = 0; ispike < nspikes; ispike++)
			m_pSpkList->CenterSpikeAmplitude(ispike, it1, it2, 1);

		int max, min;
		m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
		WORD middle = max/2 + min/2;
		m_spkClass.SetYzoom(m_spkClass.GetYWExtent(), middle);
	}
	*/
	if (m_bDatDocExists)
		m_sourceView.CenterChan(0);

	UpdateLegends(FALSE);
	// display data
	m_spkClass.Invalidate();
	m_sourceView.Invalidate();
}

// --------------------------------------------------------------------------

void CSpikeView::OnFormatGainadjust() 
{
	if (m_bSpkDocExists)
	{
		// adjust gain of spkform
		int max, min;		
		m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
		m_yWE = MulDiv(max-min+1, 10, 8);
		m_yWO = max/2 + min/2;
		m_spkClass.SetYzoom(m_yWE, m_yWO);
	}
	if (m_bDatDocExists)
		m_sourceView.MaxgainChan(0);
	UpdateLegends(FALSE);
	// display data
	m_spkClass.Invalidate();
	m_sourceView.Invalidate();
}

// --------------------------------------------------------------------------

void CSpikeView::OnToolsEdittransformspikes()
{
	// return if no spike shape
	if (m_spkClass.GetXWExtent() == 0)	// if no spike shape, return
		return;
	// save time frame to restore it on return
	long lFirst = m_spkClass.GetTimeFirst();
	long lLast = m_spkClass.GetTimeLast();
	
	CSpikeEditDlg dlg;						// dialog box
	dlg.m_yextent = m_spkClass.GetYWExtent();// load display parameters
	dlg.m_yzero = m_spkClass.GetYWOrg();	// ordinates
	dlg.m_xextent = m_spkClass.GetXWExtent();// and	
	dlg.m_xzero = m_spkClass.GetXWOrg();	// abcissa
	dlg.m_spikeno = m_spikeno;				// load index of selected spike
	dlg.m_parent = this;
	dlg.m_pSpkList = m_pSpkList;			// pass spike list
	dlg.m_dbDoc = GetDocument()->m_pDat;		// data source pointer

	// open dialog box and wait for response
	dlg.DoModal();
	if (!dlg.m_bartefact)
		m_spikeno = dlg.m_spikeno;			// set no spike selected

	if (dlg.m_bchanged)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
		SaveCurrentFileParms();
		UpdateFileParameters();
	}
	m_lFirst = lFirst;
	m_lLast = lLast;
	UpdateLegends(TRUE);
	// display data
	m_spkClass.Invalidate();
	m_sourceView.Invalidate();
}

//-----------------------------------------------------------------------
// compute printer's page dot resolution
// borrowed from sample\drawcli\drawdoc.cpp
//-----------------------------------------------------------------------

void CSpikeView::PrintComputePageSize()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	HDC hDC= dlg.CreatePrinterDC();		// to delete at the end -- see doc!
	ASSERT(hDC != NULL);
	dc.Attach(hDC);

	// Get the size of the page in pixels
	mdPM->horzRes=dc.GetDeviceCaps(HORZRES);
	mdPM->vertRes=dc.GetDeviceCaps(VERTRES);	

	// margins (pixels)
	m_printRect.right = mdPM->horzRes-mdPM->rightPageMargin;
	m_printRect.bottom = mdPM->vertRes-mdPM->bottomPageMargin;
	m_printRect.left = mdPM->leftPageMargin;
	m_printRect.top = mdPM->topPageMargin;
}

/////////////////////////////////////////////////////////////////////////////
// print paging information, date on the bottom of the page
/////////////////////////////////////////////////////////////////////////////

void CSpikeView::PrintFileBottomPage(CDC* pDC, CPrintInfo* pInfo)
{
	CTime t= CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"), // %d:%d",
			pInfo->m_nCurPage, pInfo->GetMaxPage(),
			t.GetDay(),	t.GetMonth(),t.GetYear());
	CString ch_date = GetDocument()->DBGetCurrentSpkFileName();
	pDC->SetTextAlign(TA_CENTER);
	pDC->TextOut(mdPM->horzRes/2, mdPM->vertRes-57,	ch_date);
}

// ---------------------------------------------------------------------------------------
// PrintConvertFileIndex
// return ascii string
// ---------------------------------------------------------------------------------------

CString CSpikeView::PrintConvertFileIndex(long lFirst, long lLast)
{
	CString csUnit= _T(" s");			// get time,  prepare time unit
	TCHAR szValue[64];				// buffer to receive ascii represent of values
	LPTSTR pszValue = szValue;
	float xScaleFactor;				// scale factor returned by changeunit
	float x = PrintChangeUnit( 
		(float) lFirst / m_pSpkDoc->GetAcqRate(), &csUnit, &xScaleFactor);
	int fraction = (int) ((x - ((int) x)) * (float) 1000.);	// separate fractional part
	wsprintf(pszValue, _T("time = %i.%03.3i - "), (int) x, fraction); // print value
	CString csComment = pszValue;	// save ascii to string

	x = lLast / (m_pSpkDoc->GetAcqRate() * xScaleFactor);	// same operations for last interval
	fraction = (int) ((x - ((int) x)) *  (float) 1000.);
	wsprintf(pszValue, _T("%i.%03.3i %s"), (int) x, fraction, (LPCTSTR) csUnit);
	csComment += pszValue;
	return csComment;
}


// -----------------------------------------------------------------------------------
// PrintGetFileSeriesIndexFromPage
//
// parameters in:
//	page : current printer page
//	file : filelist index
// returns lFirst = index first pt to display
// assume correct parameters:
// 	m_lprintFirst
// 	m_lprintLen
//
// count how many rows by reading the length of each file of the list starting 
// from the first. Stop looping through list whenever count of rows is reached 
// or when file number reaches zero
// -----------------------------------------------------------------------------------

long CSpikeView::PrintGetFileSeriesIndexFromPage(int page, int* filenumber)
{
	long lFirst = m_lprintFirst;	

	int maxrow=m_nbrowsperpage*page;	// row to get
	int ifile = 0;						// file list index
	if (mdPM->bPrintSelection)			// current file if selection only
		ifile = m_file0;
	long current = GetDocument()->DBGetCurrentRecordPosition();
	GetDocument()->DBSetCurrentRecordPosition(ifile);	
	long veryLast= GetDocument()->DBGetDataLen()-1;
	for (int row = 0; row <maxrow; row++)
	{		
		lFirst += m_lprintLen;			// end of row
		if (lFirst >= veryLast)			// next file ?
		{			
			ifile++;					// next file index
			if  (ifile > m_nfiles)		// last file ??
			{
				ifile--;
				break;
			}
			// update end-of-file
			GetDocument()->DBMoveNext();
			veryLast=GetDocument()->DBGetDataLen()-1;
			lFirst = m_lprintFirst;
		}
	}
	*filenumber = ifile;	// return index / file list
	GetDocument()->DBSetCurrentRecordPosition(current);
	return lFirst;			// return index first point / data file
}

/////////////////////////////////////////////////////////////////////////////
// print text for a file items
// take care of user's options saved in mdPM structure
/////////////////////////////////////////////////////////////////////////////

CString CSpikeView::PrintGetFileInfos()
{
	CString strComment;   					// scratch pad
	CString Tab("    ");					// use 4 spaces as tabulation character
	CString RC("\n");						// next line
	CWaveFormat* pWF = &m_pSpkDoc->m_wformat;	// get data description

	// document's name, date and time
	if (mdPM->bDocName || mdPM->bAcqDateTime)// print doc infos?
	{
		if (mdPM->bDocName)					// print file name
			strComment += GetDocument()->DBGetCurrentSpkFileName(FALSE) + Tab;
		if (mdPM->bAcqDateTime)				// print data acquisition date & time
		{
			CTime acqtime = m_pSpkDoc->GetAcqTime();
			CString date = acqtime.Format(_T("%#d %m %Y %X")); //("%x %X");
			// or more explicitely %d-%b-%Y %H:%M:%S");
			strComment +=  date;
		}
		strComment += RC;
	}

	// document's main comment (print on multiple lines if necessary)
	if (mdPM->bAcqComment)
	{
		strComment += pWF->GetComments(_T(" "));// csComment 
		strComment += RC;
	}

	return strComment;
}

// ---------------------------------------------------------------------------------------
// PrintBars
// print bars and afferent comments ie value for each channel
// return comment associated with bars and individual channels
// ---------------------------------------------------------------------------------------
CString CSpikeView::PrintBars(CDC* pDC, CRect* rect)
{
	CString strComment;
	CString RC(_T("\n"));
	CString Tab(_T("     "));
	CBrush* pOldBrush= (CBrush*) pDC->SelectStockObject(BLACK_BRUSH);
	char szValue[64];
	LPSTR lpszVal = szValue;	
	CString csUnit;
	CString csComment;
	CRect rectHorzBar;
	CRect rectVertBar;
	float xScaleFactor=0.0f;
	CPoint barOrigin(-10,-10);	// origine barre à 10,10 pts de coin inf gauche rectangle
	CSize  barSize(5, 5);		// épaisseur barre

	// same len ratio as displayed on viewdata
	int horzBar = 100;
	int vertBar = 100;

	///// time abcissa ///////////////////////////	
	int iifirst = m_spkClass.GetTimeFirst();
	int iilast = m_spkClass.GetTimeLast();
	csComment = PrintConvertFileIndex(iifirst, iilast); 

	///// horizontal time bar ///////////////////////////	
	if (mdPM->bTimeScaleBar)
	{
		// print horizontal bar
		rectHorzBar.left = rect->left + barOrigin.x;
		rectHorzBar.right = rectHorzBar.left + horzBar;
		rectHorzBar.top = rect->bottom -barOrigin.y;
		rectHorzBar.bottom = rectHorzBar.top - barSize.cy;
		BOOL flag = pDC->Rectangle(&rectHorzBar);
		//get time equivalent of bar length
		int iibar = MulDiv(iilast - iifirst, rectHorzBar.Width(), rect->Width());
		float xbar = float(iibar) / m_pSpkDoc->GetAcqRate();
		CString cs;
		cs.Format(_T("\nbar = %f s"), xbar);
		csComment += cs;
		strComment += csComment + RC;
	}

	///// vertical voltage bars ///////////////////////////	
	if (mdPM->bVoltageScaleBar)
	{
		rectVertBar.left = rect->left + barOrigin.x;
		rectVertBar.right = rectVertBar.left - barSize.cx;
		rectVertBar.bottom = rect->bottom -barOrigin.y;
		rectVertBar.top = rectVertBar.bottom - vertBar;
		pDC->Rectangle(&rectVertBar);
	}

	// comments, bar value and chan settings for each channel	
	if (mdPM->bChansComment || mdPM->bVoltageScaleBar || mdPM->bChanSettings)
	{
/*
		int imax = m_sourceView.GetChanlistSize();	// number of data channels
		for (int ichan=0; ichan< imax; ichan++)		// loop
		{
			// boucler sur les commentaires de chan n a chan 0...		
			wsprintf(lpszVal, "chan#%i ", ichan);	// channel number
			csComment = lpszVal;
			if (mdPM->bVoltageScaleBar)				// bar scale value
			{
				csUnit = " V";						// provisional unit				
				float z= 	(float) m_yscalebar.Height()* m_sourceView.GetChanlistVoltsperPixel(ichan);
				float x = PrintChangeUnit(z, &csUnit, &xScaleFactor); // convert

				// approximate
				int j = (int) x;					// get int val				
				if (x-j > 0.5)						// increment integer if diff > 0.5
					j++;
				int k = (int) NiceUnit(x);			// compare with nice unit abs
				if (j > 750)                        // there is a gap between 500 and 1000
					k=1000;
				if (MulDiv(100, abs(k-j),j) <= 1)	// keep nice unit if difference is less= than 1 %
					j = k;
				if (k >= 1000)
				{
					z = (float) k * xScaleFactor;
					j = (int) PrintChangeUnit(z, &csUnit, &xScaleFactor); // convert
				}
				wsprintf(szValue, "bar = %i %s ", j, csUnit);	// store val into comment
				csComment += szValue;
			}
			strComment += csComment;

			// print chan comment 
			if (mdPM->bChansComment)
			{
				strComment += Tab;
				strComment += m_sourceView.GetChanlistComment(ichan);
			}
			strComment += RC;

			// print amplifiers settings (gain & filter), next line
			if (mdPM->bChanSettings)
			{
				CString cs;
				WORD channb = m_sourceView.GetChanlistSourceChan(ichan);
				CWaveChan* pChan = m_pDataDoc->m_pDataFile->GetpWavechanArray()->get_p_channel(channb);
				wsprintf(lpszVal, "headstage=%s", pChan->headstage);
				cs += lpszVal;
				wsprintf(lpszVal, " g=%li", (long) (pChan->xgain));
				cs += lpszVal;
				wsprintf(lpszVal, " LP=%i", pChan->am_lowpass);
				cs += lpszVal;
				cs += " IN+=";
				cs += pChan->am_csInputpos;
				cs += " IN-=";
				cs += pChan->am_csInputneg;
				strComment += cs;
				strComment += RC;
			}
		}
*/
	}	
	pDC->SelectObject(pOldBrush);
	return strComment;
}


/////////////////////////////////////////////////////////////////////////////
// (1) OnPreparePrinting()
// override standard setting before calling print dialog
/////////////////////////////////////////////////////////////////////////////

BOOL CSpikeView::OnPreparePrinting(CPrintInfo* pInfo)
{  
	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;
	
	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	// save current state of the windows
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	if (pApp->m_pviewspikesMemFile == nullptr)
	{
		pApp->m_pviewspikesMemFile = new CMemFile;
		ASSERT(pApp->m_pviewspikesMemFile != NULL);
	}

	CArchive ar(pApp->m_pviewspikesMemFile, CArchive::store);
	pApp->m_pviewspikesMemFile->SeekToBegin();
	m_sourceView.Serialize(ar);
	//m_spkBarView.Serialize(ar);
	//m_spkShapeView.Serialize(ar);
	ar.Close();					// close archive

	// printing margins	
	if (mdPM->vertRes <= 0 ||mdPM->horzRes <= 0
		||mdPM->horzRes !=  pInfo->m_rectDraw.Width()
		||mdPM->vertRes != pInfo->m_rectDraw.Height())
		PrintComputePageSize();

	// how many rows per page?
	int sizeRow=mdPM->HeightDoc + mdPM->heightSeparator;
	m_nbrowsperpage = m_printRect.Height()/sizeRow;
	if (m_nbrowsperpage == 0)			// prevent zero pages
		m_nbrowsperpage = 1;

	//------------------------------------------------------
	int nbrect;			// number of rectangles -- or nb of rows
	int npages;			// number of pages

	// compute number of rows according to multirow flag 
	m_lprintFirst = m_spkClass.GetTimeFirst();
	m_lprintLen = m_spkClass.GetTimeLast() -m_lprintFirst + 1;

	// make sure the number of classes per file is known
	int nnclasses = 0;			// store sum (nclasses from file (i=ifile0, ifile1))
	CdbWaveDoc* pdbDoc = GetDocument();
	m_file0 = pdbDoc->DBGetCurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_printFirst = m_file0;
	m_printLast = m_file0;
	m_nfiles = 1;
	
	if (!mdPM->bPrintSelection)
	{
		m_printFirst = 0;
		m_nfiles = pdbDoc->DBGetNRecords();
		m_printLast = m_nfiles-1;
	}

	// update the nb of classes per file selected and add this number
	m_maxclasses = 1;
	pdbDoc->DBSetCurrentRecordPosition(m_printFirst);
	nbrect=0;										// total nb of rows
	for (int i = m_printFirst; i<= m_printLast; i++, pdbDoc->DBMoveNext())
	{		
		// get number of classes
		if (pdbDoc->Getnbspikeclasses() <= 0)
		{
			pdbDoc->OpenCurrentSpikeFile();
			m_pSpkDoc = pdbDoc->m_pSpk;
			m_pSpkList = m_pSpkDoc->GetSpkListCurrent();	
			if (!m_pSpkList->IsClassListValid())	// if class list not valid:
			{
				m_pSpkList->UpdateClassList();		// rebuild list of classes
				m_pSpkDoc->SetModifiedFlag();		// and set modified flag
			}

			int nclasses=1;
			if (m_pSpkList->GetTotalSpikes() > 0)
				nclasses = m_pSpkList->GetNbclasses();
			ASSERT(nclasses > 0);
			pdbDoc->Setnbspikeclasses(nclasses);
			nnclasses += nclasses;
		}

		if (pdbDoc->Getnbspikeclasses() > m_maxclasses)
			m_maxclasses = pdbDoc->Getnbspikeclasses();

		if (mdPM->bMultirowDisplay)
		{
			long len = pdbDoc->DBGetDataLen()-m_lprintFirst;// file length
			long nrows = len/m_lprintLen;	// how many rows for this file?
			if (len > nrows*m_lprintLen)	// remainder?
				nrows++;
			nbrect += (int) nrows;			// update nb of rows
		}
	}

	// multiple rows?
	if (!mdPM->bMultirowDisplay)
		nbrect = m_nfiles;

	// npages
	npages = nbrect/m_nbrowsperpage;
	if (nbrect > m_nbrowsperpage*npages)
			npages++;
		
	//------------------------------------------------------
	pInfo->SetMaxPage(npages);			//one page printing/preview  
	pInfo->m_nNumPreviewPages = 1;  	// preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if(mdPM->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION;	// set button to selection
	
	// call dialog box
	BOOL flag = DoPreparePrinting(pInfo);
	// set max nb of pages according to selection
	mdPM->bPrintSelection = pInfo->m_pPD->PrintSelection();
	if (mdPM->bPrintSelection)
	{		
		nbrect = nnclasses;
		npages = 1;
		m_nfiles = 1;
		if (mdPM->bMultirowDisplay)
		{
			long lFirst0 = m_spkClass.GetTimeFirst();
			long lLast0 = m_spkClass.GetTimeLast();	
			long len = m_pSpkDoc->GetAcqSize() - lFirst0;
			nbrect = len / (lLast0-lFirst0);
			if (nbrect * (lLast0-lFirst0) < len)
				nbrect++;

			npages = nbrect/m_nbrowsperpage;
			if (npages*m_nbrowsperpage < nbrect)
				npages++;
		}
		pInfo->SetMaxPage(npages);
	}

	pdbDoc->DBSetCurrentRecordPosition(m_file0);	
	return flag; 
}

/////////////////////////////////////////////////////////////////////////////
// (2) OnBeginPrinting
/////////////////////////////////////////////////////////////////////////////

void CSpikeView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_spkClass.GetTimeFirst();
	m_lLast0 = m_spkClass.GetTimeLast();

	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT));		// prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));		// Arial font
	m_logFont.lfHeight = mdPM->fontsize;		// font height
	m_pOldFont = nullptr;
	BOOL flag = m_fontPrint.CreateFontIndirect(&m_logFont);
	pDC->SetBkMode (TRANSPARENT);
}

/////////////////////////////////////////////////////////////////////////////
//	(3) OnPrint() -- for each page
/////////////////////////////////////////////////////////////////////////////

void CSpikeView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	m_pOldFont = pDC->SelectObject(&m_fontPrint);
	pDC->SetMapMode(MM_TEXT);								// (1 pixel = 1 logical point)
	PrintFileBottomPage(pDC,pInfo);							// print bottom - text, date, etc
	int curpage = pInfo->m_nCurPage;						// get current page number

	// --------------------- load data corresponding to the first row of current page	

	// print only current selection - transform current page into file index
	int filenumber;			   								// file number and file index
	long lFirst = PrintGetFileSeriesIndexFromPage(curpage-1, &filenumber);
	GetDocument()->DBSetCurrentRecordPosition(filenumber);
	UpdateFileParameters();									// update file parameters
	long veryLast = m_pSpkDoc->GetAcqSize()-1;				// index last data point / current file

	CRect RWhere (	m_printRect.left, 						// left
					m_printRect.top,						// top
					m_printRect.left + mdPM->WidthDoc,		// right ( and bottom next line)
					m_printRect.top + mdPM->HeightDoc);

	// loop through all files	--------------------------------------------------------
	for (int i = 0; i<m_nbrowsperpage; i++)
	{
		// save conditions (Save/RestoreDC is mandatory!) --------------------------------
		
		int oldDC = pDC->SaveDC();						// save DC

		// set first rectangle where data will be printed
		
		CRect CommentRect = RWhere;						// save RWhere for comments        
		pDC->SetMapMode(MM_TEXT);						// 1 pixel = 1 logical unit
		pDC->SetTextAlign(TA_LEFT); 					// set text align mode
		if (mdPM->bFrameRect)							// print rectangle if necessary
		{
			pDC->MoveTo(RWhere.left,  RWhere.top);
			pDC->LineTo(RWhere.right, RWhere.top);		// top hz
			pDC->LineTo(RWhere.right, RWhere.bottom);	// right vert
			pDC->LineTo(RWhere.left,  RWhere.bottom);	// bottom hz
			pDC->LineTo(RWhere.left,  RWhere.top);		// left vert
		}
		pDC->SetViewportOrg(RWhere.left, RWhere.top);

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data

		CRect RW2 = RWhere;								// printing rectangle - constant
		RW2.OffsetRect(-RWhere.left, -RWhere.top);		// set RW2 origin = 0,0
		int rheight = RW2.Height() / m_maxclasses;		// ncount;
		if (m_bDatDocExists)
			rheight = RW2.Height() / (m_maxclasses+1);
		int rseparator = rheight / 8;
		int rcol = rcol = RW2.Width()/8;

		// rectangles for the 3 categories of data
		CRect RWtext = RW2;
		CRect RWspikes = RW2;
		CRect RWbars = RW2;

		// horizontal size and position of the 3 rectangles

		RWtext.right = RWtext.left + rcol;
		RWspikes.left = RWtext.right + rseparator;
		int n = m_pSpkDoc->GetSpkListCurrent()->GetSpikeLength();
		if (m_pSpkDoc->GetSpkListCurrent()->GetSpikeLength() > 1)
			RWspikes.right = RWspikes.left + rcol;
		else
			RWspikes.right = RWspikes.left;
		RWbars.left = RWspikes.right + rseparator;
		
		// bottom of the first rectangle
		RWbars.bottom = RW2.top + rheight;
		long lLast = lFirst + m_lprintLen;				// compute last pt to load
		if (lLast > veryLast)							// check end across file length
			lLast = veryLast;
		if ((lLast-lFirst+1) <m_lprintLen)				// adjust rect to length of data
		{
			RWbars.right = MulDiv(RWbars.Width(), lLast-lFirst, m_lprintLen)
						+ RWbars.left;
			ASSERT(RWbars.right > RWbars.left);
		}

		// ------------------------ print data

		int iextent = m_spkClass.GetYWExtent();			// get current extents
		int izero = m_spkClass.GetYWOrg();

		if (m_bDatDocExists)
		{
			if (mdPM->bClipRect)						// clip curve display ?
				pDC->IntersectClipRect(&RWbars);		// yes
			m_sourceView.GetDataFromDoc(lFirst, lLast);	// load data from file
			m_sourceView.CenterChan(0);
			m_sourceView.Print(pDC, &RWbars);			// print data
			pDC->SelectClipRgn(nullptr);

			iextent = m_sourceView.GetChanlistYextent(0);
			izero = m_sourceView.GetChanlistYzero(0);

			RWbars.top = RWbars.bottom;
		}

		// ------------------------ print spikes

		RWbars.bottom = RWbars.top + rheight;		// adjust the rect position
		RWspikes.top = RWbars.top;
		RWspikes.bottom = RWbars.bottom;
		RWtext.top = RWbars.top;
		RWtext.bottom = RWbars.bottom;

		int max, min;
		((CSpikeList*) m_pSpkDoc->GetSpkListCurrent())->GetTotalMaxMin(TRUE, &max, &min);
		short middle = max/2 + min/2;
		m_spkClass.SetYzoom(iextent, middle);
		int ncount = m_spkClass.GetCount();				// get nb of items in this file

		for (int icount = 0; icount < ncount; icount++)
		{
			m_spkClass.SetTimeIntervals(lFirst, lLast);
			m_spkClass.PrintItem(pDC, &RWtext, &RWspikes, &RWbars, icount);
			RWtext.OffsetRect(0, rheight);
			RWspikes.OffsetRect(0, rheight);
			RWbars.OffsetRect(0, rheight);
		}

		// ------------------------ print stimulus

		if (m_pSpkDoc->m_stimIntervals.nitems > 0)
		{
			CBrush bluebrush;	// create and select a solid blue brush
			bluebrush.CreateSolidBrush(RGB(0, 0, 255));
			CBrush* oldb = (CBrush*) pDC->SelectObject(&bluebrush);

			CPen bluepen;		// create and select a solid blue pen
			bluepen.CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
			CPen* ppen = (CPen*) pDC->SelectObject(&bluepen);

			CSpikeList* pSpkList = m_pSpkDoc->GetSpkListCurrent();
			RWspikes.bottom = RW2.bottom;	// define a rectangle for stimulus
			RWspikes.top = RW2.bottom - rheight/10;

			if (RWspikes.top == RWspikes.bottom) 
				RWspikes.bottom++;

			for (int ii=0; ii < m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize(); ii++, ii++)
			{
				int iifirst = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(ii);
				if ((ii+1)>=  m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize())
					continue;
				int iilast =  m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(ii+1);
				if (iifirst > lLast || iilast < lFirst)
					continue;
				if (iifirst < lFirst)
					iifirst = lFirst;
				if (iilast > lLast)
					iilast = lLast;

				RWspikes.left = MulDiv(iifirst-lFirst, RWbars.Width(), lLast-lFirst) +RWbars.left;
				RWspikes.right = MulDiv(iilast-lFirst, RWbars.Width(), lLast-lFirst) +RWbars.left;
				if (RWspikes.right <= RWspikes.left)
					RWspikes.right = RWspikes.left + 1;
				pDC->Rectangle(RWspikes);
			}

			pDC->SelectObject(oldb);
			pDC->SelectObject(ppen);
		}


		// ------------------------ print stimulus

		// update display rectangle for next row
		RWhere.OffsetRect(0, mdPM->HeightDoc + mdPM->heightSeparator);

		// restore DC ------------------------------------------------------

		pDC->RestoreDC(oldDC);					// restore Display context	

		// print comments --------------------------------------------------

		pDC->SetMapMode(MM_TEXT);				// 1 LP = 1 pixel
		pDC->SelectClipRgn(nullptr);				// no more clipping
		pDC->SetViewportOrg(0, 0);				// org = 0,0        

		// print comments according to row within file
		CString csComment;
		if (lFirst == m_lprintFirst)			// first row = full comment
		{
			csComment += PrintGetFileInfos();
			csComment += PrintBars(pDC, &CommentRect);// bars and bar legends
		}

		// print comments stored into csComment		
		CommentRect.OffsetRect(mdPM->textseparator + CommentRect.Width(), 0);
		CommentRect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		UINT uiFlag = pDC->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		pDC->DrawText(csComment, csComment.GetLength(), CommentRect, 
				DT_NOPREFIX | /*DT_NOCLIP |*/ DT_LEFT | DT_WORDBREAK);
		pDC->SetTextAlign(uiFlag);

		// update file parameters for next row --------------------------------------------
		lFirst += m_lprintLen;		
		// next file? 
		// if index next point is past the end of the file
		// OR not entire record and not multirow display
		if ((lFirst >= veryLast)
			|| (!mdPM->bEntireRecord && 
				!mdPM->bMultirowDisplay))
		{
			filenumber++;						// next index
			if  (filenumber < m_nfiles)			// last file ??
			{									// NO: select new file
				GetDocument()->DBMoveNext();				
				UpdateFileParameters(); 		// update file parameters
				veryLast=m_pSpkDoc->GetAcqSize()-1;
			}
			else
				i = m_nbrowsperpage;			// YES: break
			lFirst = m_lprintFirst;
		}
	}	// this is the end of a very long for loop.....................

	if (m_pOldFont != nullptr)
		pDC->SelectObject(m_pOldFont);
}

/////////////////////////////////////////////////////////////////////////////
//	(4) OnEndPrinting() - lastly
/////////////////////////////////////////////////////////////////////////////

void CSpikeView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bIsPrinting = FALSE;

	GetDocument()->DBSetCurrentRecordPosition(m_file0);
	UpdateFileParameters();
	m_spkClass.SetTimeIntervals(m_lFirst0, m_lLast0);
	m_spkClass.Invalidate();
	
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	if (pApp->m_pviewspikesMemFile != nullptr)
	{
		CArchive ar(pApp->m_pviewspikesMemFile, CArchive::load);
		pApp->m_pviewspikesMemFile->SeekToBegin();
		m_sourceView.Serialize(ar);
		ar.Close();					// close archive
	}
}

// --------------------------------------------------------------------------
// (where could we store these values???)
// --------------------------------------------------------------------------
static char VsUnits[] = {"GM  mµpf  "};		// units & corresp powers
static int  VsUnitsPower[] = { 9,6, 0, 0, -3, -6, -9, -12, 0}; 
static int	VsmaxIndex		= 8;				// nb of elmts
//static int  VsniceIntervals[] = {1, 5, 
//						10,  20,  25,  30,  40, 50, 
//						100, 200, 250, 300, 400, 500,  /*600,*/ /*700,*//* 800, *//*900,*/
//						0};

// --------------------------------------------------------------------------
// PrintChangeUnit: adapt xUnit (string) & scalefactor to xval
// xval not modified
// return scaled value
// --------------------------------------------------------------------------

float CSpikeView::PrintChangeUnit(float xVal, CString* xUnit, float* xScalefactor)
{   
	if (xVal == 0)	// evite div par zero error
	{
		*xScalefactor = 1.0f;
		return 0.0f;
	}

	short	i;  
	short isign = 1;
	if ( xVal <0)
		{
		isign = -1;
		xVal = -xVal;
		}
	short iprec= (short) log10(xVal);		// log10 of value (upper limit)     floor ceil scPower
	if (( iprec <=0) && (xVal < 1.))	// perform extra checking if iprec <= 0
		i = 4-iprec/3;					// change equation if Units values change!
	else			      
		i = 3-iprec/3;					// change equation if Units values change!
	if (i >VsmaxIndex)					// clip to max index
		i = VsmaxIndex;
	else if (i<0)						// or clip to min index
		i = 0;
	*xScalefactor = (float) pow(10.0f, VsUnitsPower[i]);	// convert & store
	xUnit->SetAt(0, VsUnits[i]);          // replace character corresp to unit
	return xVal*isign / *xScalefactor;	// return value/scale_factor
}

////////////////////////////////////////////////////////////////////////

void CSpikeView::OnEnChangeSpikenoclass() 
{
	if (!mm_spikenoclass.m_bEntryDone)
		return;
	int spikenoclass = m_spikenoclass;
	switch (mm_spikenoclass.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikenoclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikenoclass--;	break;
	}


	mm_spikenoclass.m_bEntryDone=FALSE;	// clear flag
	mm_spikenoclass.m_nChar=0;			// empty buffer
	mm_spikenoclass.SetSel(0, -1);		// select all text
	
	if (m_spikenoclass != spikenoclass)	// change display if necessary
	{		
		m_spkClass.ChangeSpikeClass(m_spikeno, m_spikenoclass);
		m_pSpkDoc->SetModifiedFlag(TRUE);
		UpdateLegends(FALSE);
		// display data
		m_spkClass.Invalidate();
		m_sourceView.Invalidate();
	}
}

void CSpikeView::OnEnChangeTimefirst()
{
	if (!mm_timefirst.m_bEntryDone)
		return;

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

	long lFirst = (long) (m_timefirst*m_pSpkDoc->GetAcqRate());
	if (lFirst != m_lFirst)
	{
		m_lFirst = lFirst;
		UpdateLegends(FALSE);
		// display data
		m_spkClass.Invalidate();
		m_sourceView.Invalidate();
	}
}

// --------------------------------------------------------------------------

void CSpikeView::OnEnChangeTimelast()
{
	if (!mm_timelast.m_bEntryDone)
		return;

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
	mm_timelast.SetSel(0, -1); 	//select all text

	long lLast = (long) (m_timelast*m_pSpkDoc->GetAcqRate());
	if (lLast != m_lLast)
	{
		m_lLast = lLast;
		UpdateLegends(FALSE);
		// display data
		m_spkClass.Invalidate();
		m_sourceView.Invalidate();
	}
}

// --------------------------------------------------------------------------

void CSpikeView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// get corresponding data
	long totalScroll= m_pSpkDoc->GetAcqSize();
	long pageScroll = (m_lLast - m_lFirst);
	long sbScroll = MulDiv(pageScroll, 10, 100);
	if (sbScroll == 0)
		sbScroll = 1;
	long lFirst = m_lFirst;
	long lLast = m_lLast;	

	switch (nSBCode)
	{		
	case SB_LEFT:		
		lFirst = 0;	break;			// Scroll to far left.
	case SB_LINELEFT:	
		lFirst -= sbScroll;	break;	// Scroll left.
	case SB_LINERIGHT:	
		lFirst += sbScroll; break;	// Scroll right
	case SB_PAGELEFT:	
		lFirst -= pageScroll; break;// Scroll one page left
	case SB_PAGERIGHT:	
		lFirst += pageScroll; break;// Scroll one page right.
	case SB_RIGHT:		
		lFirst = totalScroll - pageScroll+1; 
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		lFirst = (int) nPos;
		break;
	default:
		return;
		break;
	}

	if (lFirst < 0)
		lFirst = 0;
	lLast = lFirst + pageScroll;

	if (lLast >= totalScroll)
	{
		lLast = totalScroll - 1;
		lFirst = lLast - pageScroll;
	}
	
	// adjust display
	if (lFirst != m_lFirst)
	{
		m_lFirst = lFirst;
		m_lLast = lLast;
		UpdateLegends(FALSE);
		m_spkClass.Invalidate();
		m_sourceView.Invalidate();
	}
	else
		UpdateScrollBar();
	if (m_bDatDocExists)
		m_sourceView.CenterChan(0);
}

// --------------------------------------------------------------------------

void CSpikeView::UpdateScrollBar()
{
	m_scrollFilePos_infos.fMask = SIF_ALL;
	m_scrollFilePos_infos.nMin = 0;
	m_scrollFilePos_infos.nMax = m_pSpkDoc->GetAcqSize()-1;
	m_scrollFilePos_infos.nPos = m_lFirst;
	m_scrollFilePos_infos.nPage = m_lLast-m_lFirst;
	((CScrollBar*) GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&m_scrollFilePos_infos);
}

// --------------------------------------------------------------------------


void CSpikeView::OnEditCopy() 
{
	CCopyAsDlg dlg;
	dlg.m_nabcissa=mdPM->hzResolution;
	dlg.m_nordinates=mdPM->vtResolution;
	dlg.m_bgraphics=mdPM->bgraphics;
	dlg.m_ioption=mdPM->bcontours;
	dlg.m_iunit=mdPM->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		mdPM->bgraphics= dlg.m_bgraphics;
		mdPM->bcontours= dlg.m_ioption;
		mdPM->bunits= dlg.m_iunit;
		mdPM->hzResolution=dlg.m_nabcissa;
		mdPM->vtResolution=dlg.m_nordinates;

		// output rectangle requested by user
		CRect rect(0,0, mdPM->hzResolution, mdPM->vtResolution);

		// create metafile
		CMetaFileDC mDC;
		CRect rectBound = rect;
		rectBound.right *= 32;
		rectBound.bottom *= 30;
		CDC* pDCRef = GetDC();
		int oldDC = pDCRef->SaveDC();		// save DC

		CString csTitle = _T("dbWave\0") + GetDocument()->GetTitle();
		csTitle += _T("\0\0");
		BOOL hmDC = mDC.CreateEnhanced(pDCRef, nullptr, &rectBound, csTitle);
		ASSERT (hmDC != NULL);

		// Draw document in metafile.
		CPen blackPen(PS_SOLID, 0, RGB(  0,   0,   0));
		CPen* poldpen=mDC.SelectObject(&blackPen);
		CBrush* pOldBrush= (CBrush*) mDC.SelectStockObject(BLACK_BRUSH);
		CClientDC attribDC(this) ;					// Create and attach attribute DC
		mDC.SetAttribDC(attribDC.GetSafeHdc()) ;	// from current screen

		// print comments : set font
		m_pOldFont = nullptr;
		int oldsize = mdPM->fontsize;
		mdPM->fontsize = 10;
		memset(&m_logFont, 0, sizeof(LOGFONT));		// prepare font
		lstrcpy(m_logFont.lfFaceName, _T("Arial"));		// Arial font
		m_logFont.lfHeight = mdPM->fontsize;		// font height
		m_pOldFont = nullptr;
		BOOL flag = m_fontPrint.CreateFontIndirect(&m_logFont);
		mDC.SetBkMode (TRANSPARENT);

		mdPM->fontsize = oldsize;
		m_pOldFont = mDC.SelectObject(&m_fontPrint);

		int lineheight = m_logFont.lfHeight + 5;
		int ypxrow=0;
		int xcol = 10;
		CString comments;

		// display data: source data and spikes
		int iextent = m_spkClass.GetYWExtent();			// get current extents
		int izero   = m_spkClass.GetYWOrg();
		int rheight = MulDiv(m_spkClass.GetRowHeight(), rect.Width(), 
								 m_spkClass.GetColsTimeWidth());
		CRect RWspikes = rect;							// adjust rect size
		RWspikes.bottom = rheight;						// height ratio like screen
		CRect RWtext = RWspikes;
		CRect RWbars = RWspikes;
		// horizontal size and position of the 3 rectangles
		int rseparator = rheight / 5;
		RWtext.right = RWtext.left + rheight;
		RWspikes.left = RWtext.right + rseparator;
		RWspikes.right = RWspikes.left + rheight;
		RWbars.left = RWspikes.right + rseparator;
		
		// display data	if data file was found
		if (m_bDatDocExists)
		{
			m_sourceView.CenterChan(0);
			m_sourceView.Print(&mDC, &RWbars);

			iextent = m_sourceView.GetChanlistYextent(0);
			izero = m_sourceView.GetChanlistYzero(0);
			RWspikes.OffsetRect(0, rheight);
			RWbars.OffsetRect(0, rheight);
			RWtext.OffsetRect(0, rheight);
		}

		// display spikes and bars

		int max, min;
		((CSpikeList*) m_pSpkDoc->GetSpkListCurrent())->GetTotalMaxMin(TRUE, &max, &min);
		short middle = max/2 + min/2;
		m_spkClass.SetYzoom(iextent, middle);
		int ncount = m_spkClass.GetCount();				// get nb of items in this file

		for (int icount = 0; icount < ncount; icount++)
		{
			m_spkClass.PrintItem(&mDC, &RWtext, &RWspikes, &RWbars, icount);
			RWspikes.OffsetRect(0, rheight);
			RWbars.OffsetRect(0, rheight);
			RWtext.OffsetRect(0, rheight);
		}

/*
			// display curves : data
			CRect rectdata = rect;
			int rspkwidth = MulDiv(m_spkShapeView.Width(), rect.Width(),
				m_spkShapeView.Width() + m_sourceView.Width());
			int rdataheight = MulDiv(m_sourceView.Height(), rect.Height(),
				m_sourceView.Height() + m_spkBarView.Height());
			int separator = rspkwidth / 10;
			rectdata.bottom = rect.top + rdataheight - separator/2;
			rectdata.left = rect.left + rspkwidth + separator;
			m_sourceView.Print(&mDC, &rectdata);

			// title & ordinates, abcissa
			GetDlgItem(IDC_DATACOMMENTS)->GetWindowText(comments);
			mDC.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;

			// ordinates & abcissa
			comments = "Abcissa: ";
			CString content;
			GetDlgItem(IDC_TIMEFIRST)->GetWindowText(content);
			comments += content;
			comments += " - ";
			GetDlgItem(IDC_TIMELAST)->GetWindowText(content);
			comments += content;
			mDC.TextOut(xcol, ypxrow, comments);
			ypxrow += lineheight;

			comments += PrintDataBars(&mDC, &rectdata);
			UINT uiFlag = mDC.SetTextAlign(TA_LEFT | TA_NOUPDATECP);
			UINT nFormat = DT_NOPREFIX | DT_NOCLIP | DT_LEFT | DT_WORDBREAK;
			CRect rectComment = rect;
			rectComment.top = ypxrow;
			rectComment.right = rectdata.left;
			ypxrow += mDC.DrawText(comments, comments.GetLength(), rectComment, nFormat);

			// display spike bars
			CRect rectbars = rectdata;
			rectbars.top = rectdata.bottom + separator;
			rectbars.bottom = rect.bottom -2*lineheight;
			m_spkBarView.Print(&mDC, &rectbars);

			// display spike shapes
			CRect rectspk = rect;					// compute output rectangle
			rectspk.left += separator;
			rectspk.right = rect.left + rspkwidth;
			rectspk.bottom = rect.bottom -2*lineheight;
			rectspk.top = rectspk.bottom - rectbars.Height();
			m_spkShapeView.Print(&mDC, &rectspk);
			comments = PrintSpkShapeBars(&mDC, &rectspk, TRUE);
			rectComment.top = rectspk.bottom;
			mDC.DrawText(comments, comments.GetLength(), rectComment, nFormat);
			mDC.SelectObject(pOldBrush);
*/
			if (m_pOldFont != nullptr)
				mDC.SelectObject(m_pOldFont);
			m_fontPrint.DeleteObject();

			// restore oldpen
			mDC.SelectObject(poldpen);
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

			// restore initial conditions
			pDCRef->RestoreDC(oldDC);		// restore Display context
		}

		// restore screen in previous state
		UpdateFileParameters();
		m_spkClass.Invalidate();
		if (m_bDatDocExists)
		{
			m_sourceView.GetDataFromDoc(m_lFirst, m_lLast);
			m_sourceView.ResizeChannels(m_sourceView.Width(), m_lLast - m_lFirst);
			m_sourceView.Invalidate();
		}
}

void CSpikeView::OnZoom() 
{
	ZoomOnPresetInterval(0);
}

void CSpikeView::ZoomOnPresetInterval(int iistart)
{
	if (iistart < 0)
		iistart = 0;
	m_lFirst = iistart;
	float acqrate = m_pSpkDoc->GetAcqRate();	
	m_lLast = (long) ((m_lFirst/acqrate + m_zoom) * acqrate);
	UpdateLegends(FALSE);
	// display data
	m_spkClass.Invalidate();
	m_sourceView.Invalidate();
}

void CSpikeView::OnEnChangeZoom()
{	
	if (!mm_zoom.m_bEntryDone)
		return;
	float zoom = m_zoom;
	switch (mm_zoom.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_zoom++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_zoom--;	break;
	}

	// check boundaries
	if (m_zoom<0.0f)
		m_zoom = 1.0f;

	mm_zoom.m_bEntryDone=FALSE;	// clear flag
	mm_zoom.m_nChar=0;			// empty buffer
	mm_zoom.SetSel(0, -1);		// select all text    
	if (m_zoom != zoom)		// change display if necessary
		OnZoom();
	else
		UpdateData(FALSE);
}

void CSpikeView::OnEnChangeSourceclass() 
{
	if (!mm_sourceclass.m_bEntryDone)
		return;
	int sourceclass = m_sourceclass;
	switch (mm_sourceclass.m_nChar)
	{
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_sourceclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_sourceclass--;	break;
	}

	// check boundaries
	mm_sourceclass.m_bEntryDone=FALSE;	// clear flag
	mm_sourceclass.m_nChar=0;			// empty buffer
	mm_sourceclass.SetSel(0, -1);		// select all text
	UpdateData(FALSE);
}

void CSpikeView::OnEnChangeDestclass() 
{
	if (!mm_destclass.m_bEntryDone)
		return;
	int destclass = m_destclass;
	switch (mm_destclass.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_destclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_destclass--;	break;
	}

	// check boundaries
	mm_destclass.m_bEntryDone=FALSE;	// clear flag
	mm_destclass.m_nChar=0;			// empty buffer
	mm_destclass.SetSel(0, -1);		// select all text    
	UpdateData(FALSE);
}

void CSpikeView::OnFormatPreviousframe() 
{
	ZoomOnPresetInterval(m_lFirst*2 - m_lLast);
}

void CSpikeView::OnFormatNextframe() 
{
	long len = m_lLast - m_lFirst;
	long last = m_lLast + len;
	if (last > m_pSpkDoc->GetAcqSize())
		last = m_lLast-len;
	ZoomOnPresetInterval(last);
}


void CSpikeView::OnGAINbutton() 
{
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void CSpikeView::OnBIASbutton() 
{
	((CButton*) GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*) GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

// --------------------------------------------------------------------------
// SetVBarMode
// --------------------------------------------------------------------------

void CSpikeView::SetVBarMode (short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}        

// --------------------------------------------------------------------------
// UpdateGainScroll()
// --------------------------------------------------------------------------

void CSpikeView::UpdateGainScroll()
{
	m_scrolly.SetScrollPos(
		MulDiv(m_sourceView.GetChanlistYextent(0), 
			100, 
			YEXTENT_MAX)
		+50, 
		TRUE);
}

// --------------------------------------------------------------------------
// OnGainScroll()
// --------------------------------------------------------------------------
void CSpikeView::OnGainScroll(UINT nSBCode, UINT nPos)
{
	int lSize = m_sourceView.GetChanlistYextent(0);
	// get corresponding data
	switch (nSBCode)
	{
	// .................scroll to the start
	case SB_LEFT:		lSize = YEXTENT_MIN; break;
	// .................scroll one line left
	case SB_LINELEFT:	lSize -= lSize/10 +1; break;
	// .................scroll one line right
	case SB_LINERIGHT:	lSize += lSize/10 +1; break;
	// .................scroll one page left
	case SB_PAGELEFT:	lSize -= lSize/2+1; break;
	// .................scroll one page right
	case SB_PAGERIGHT:	lSize += lSize+1; break;
	// .................scroll to end right
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;
	// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	lSize = MulDiv(nPos-50,YEXTENT_MAX,100); break;
	// .................NOP: set position only
	default:			break;			
	}

	// change y extent
	if (lSize>0 ) //&& lSize<=YEXTENT_MAX)
	{
		m_sourceView.SetChanlistYextent(0, lSize);
		UpdateLegends(FALSE);
		m_sourceView.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll();
}


// --------------------------------------------------------------------------
// UpdateBiasScroll()
// -- not very nice code; interface counter intuitive
// --------------------------------------------------------------------------

void CSpikeView::UpdateBiasScroll()
{
	int iPos = (int) ((m_sourceView.GetChanlistYzero(0)
						- m_sourceView.GetChanlistBinZero(0))
					*100/(int)YZERO_SPAN)+(int)50;
	m_scrolly.SetScrollPos(iPos, TRUE);
}

// --------------------------------------------------------------------------
// OnBiasScroll()
// --------------------------------------------------------------------------

void CSpikeView::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	int lSize =  m_sourceView.GetChanlistYzero(0) - m_sourceView.GetChanlistBinZero(0);
	int yextent = m_sourceView.GetChanlistYextent(0);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
		lSize = YZERO_MIN;
		break;
	case SB_LINELEFT:		// scroll one line left
		lSize -= yextent/100+1;
		break;
	case SB_LINERIGHT:		// scroll one line right
		lSize += yextent/100+1;
		break;
	case SB_PAGELEFT:		// scroll one page left
		lSize -= yextent/10+1;
		break;
	case SB_PAGERIGHT:		// scroll one page right
		lSize += yextent/10+1;
		break;
	case SB_RIGHT:			// scroll to end right
		lSize = YZERO_MAX;
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos			
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		lSize = (nPos-50)*(YZERO_SPAN/100);
		break;
	default:				// NOP: set position only
		break;			
	}

	// try to read data with this new size
	if (lSize>YZERO_MIN && lSize<YZERO_MAX)
	{		
		m_sourceView.SetChanlistYzero(0, lSize+ m_sourceView.GetChanlistBinZero(0));
		m_sourceView.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll();
}


void CSpikeView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	
	// CDataView scroll: vertical scroll bar
	switch (m_VBarMode)
	{
	case BAR_GAIN:
		OnGainScroll(nSBCode, nPos);
		break;
	case BAR_BIAS:
		OnBiasScroll(nSBCode, nPos);
	default:		
		break;
	}	
	CDaoRecordView::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CSpikeView::SetAddspikesMode(int mousecursorType)
{
	// display or hide corresponding controls within this view
	BOOL bSetAddspikemode = (mousecursorType == CURSOR_MEASURE);
	int nCmdShow = SW_SHOW;
	if (!bSetAddspikemode)
	{
		nCmdShow = SW_HIDE;
	}
	if (m_baddspikemode == bSetAddspikemode)
		return;
	m_baddspikemode = bSetAddspikemode;

	// tell parent window (childframe) that we want to change the mode
	// so that it updates the toolbar
	int commandID = ID_VIEW_CURSORMODE_NORMAL;
	if (m_baddspikemode)
		commandID = ID_VIEW_CURSORMODE_MEASURE;
	GetParent()->SendMessage(WM_COMMAND, commandID, NULL);
	HWND hwnd = GetSafeHwnd();	
	if (!m_baddspikemode)
		hwnd = nullptr;
	m_sourceView.ReflectMouseMoveMessg(hwnd);
	m_spkClass.ReflectBarsMouseMoveMessg(hwnd);
	m_sourceView.SetTrackSpike(m_baddspikemode, m_pspkDP->extractNpoints, m_pspkDP->prethreshold, m_pspkDP->extractChan);

	// display or not controls used when add_spike_mode is selected
	GetDlgItem(IDC_SOURCECLASS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_DESTCLASS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT4)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDIT5)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_GROUPBOX)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_JITTER)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_JITTERSTATIC)->ShowWindow(nCmdShow);	
}

void CSpikeView::OnArtefact() 
{
	UpdateData(TRUE);				// load value from control
	if (m_spikeno <0)
	{
		m_bartefact = FALSE;		// no action if spike index < 0
	}
	else
	{								// load old class nb
		int spkclass = m_pSpkList->GetSpikeClass(m_spikeno);
		// if artefact: set class to negative value		
		if (m_bartefact && spkclass >= 0)
			spkclass = -(spkclass+1);

		// if not artefact: if spike has negative class, set to positive value
		else if (spkclass < 0)
			spkclass = -(spkclass+1);
		m_pSpkList->SetSpikeClass(m_spikeno, spkclass);
	}	
	CheckDlgButton(IDC_ARTEFACT, m_bartefact);	
	m_pSpkDoc->SetModifiedFlag(TRUE);
	UpdateLegends(FALSE);
	m_spkClass.Invalidate();
}

void CSpikeView::OnEnChangeJitter() 
{
	if (!mm_jitter_ms.m_bEntryDone)
		return;
	float jitter = m_jitter_ms;
	switch (mm_jitter_ms.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_jitter_ms++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_jitter_ms--;	break;
	}

	// check boundaries
	mm_jitter_ms.m_bEntryDone=FALSE;// clear flag
	mm_jitter_ms.m_nChar=0;			// empty buffer
	mm_jitter_ms.SetSel(0, -1);		// select all text    
	UpdateData(FALSE);
}

void CSpikeView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_baddspikemode)
	{
		if (m_bdummy && m_rectVTtrack.PtInRect(point))
			m_ptVT = point.x - m_rectVTtrack.left;	
		else
			m_ptVT = -1;				
		m_bdummy = TRUE;
		m_sourceView.XorTempVTtag(m_ptVT);
		m_spkClass.XorTempVTtag(m_ptVT);
	}
	// pass message to parent
	CDaoRecordView::OnMouseMove(nFlags, point);
}

void CSpikeView::OnLButtonUp(UINT nFlags, CPoint point) 
{		
	ReleaseCapture();
	if (m_rectVTtrack.PtInRect(point))
	{
		int iitime = m_sourceView.GetDataOffsetfromPixel(point.x - m_rectVTtrack.left);
		m_jitter = m_jitter_ms;
		BOOL bCheck = TRUE;
		if (nFlags && MK_CONTROL)
			bCheck = FALSE;
		AddSpiketoList(iitime, bCheck);
		m_bdummy = FALSE;
	}
	CDaoRecordView::OnLButtonUp(nFlags, point);
}

void CSpikeView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (m_rectVTtrack.PtInRect(point))
		SetCapture();
	CDaoRecordView::OnLButtonDown(nFlags, point);
}

void CSpikeView::OnRecordShiftleft() 
{
	OnHScroll (SB_PAGELEFT, NULL, (CScrollBar*) GetDlgItem(IDC_SCROLLBAR1)); 
}

void CSpikeView::OnRecordShiftright() 
{
	OnHScroll (SB_PAGERIGHT, NULL, (CScrollBar*) GetDlgItem(IDC_SCROLLBAR1)); 
}

void CSpikeView::OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	SelectSpkList(icursel);
	*pResult = 0;
}


void CSpikeView::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	SelectSpkList(icursel);
	*pResult = 0;
}
