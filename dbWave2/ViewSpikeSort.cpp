// spksort1.cpp : implementation file
//

#include "StdAfx.h"
#include <math.h>
#include "dbWave.h"
#include "resource.h"

#include "Cscale.h"
#include "scopescr.h"
#include "Lineview.h"
#include "Editctrl.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"

#include "Spikedoc.h"
#include "spikeshape.h"
#include "spikehistp.h"
#include "spikexyp.h"
#include "Spikebar.h"

#include "Editspik.h"
#include "MainFrm.h"
#include "Copyasdl.h"
#include "ChildFrm.h"
#include "ProgDlg.h"

#include "ViewSpikeSort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpikeSort1View

IMPLEMENT_DYNCREATE(CSpikeSort1View, CDaoRecordView)

// -------------------------------------------------------------------------

CSpikeSort1View::CSpikeSort1View()
	: CDaoRecordView(CSpikeSort1View::IDD)
{
	m_t1 = 0.0f;
	m_t2 = 0.0f;
	m_lower = 0.0f;
	m_upper = 0.0f;
	m_sourceclass = 0;
	m_destinationclass = 0;
	m_timeFirst = 0.0f;
	m_timeLast = 2.6f;
	m_mVMax = 1.0f;
	m_mVMin = 0.0f;
	m_bAllfiles = FALSE;
	m_spikeno = 0;
	m_spikenoclass = 0;
	m_txyright = 1.;
	m_txyleft = 0.;
	m_tunit = 1000.0f;		// time unit (ms=1000, us=1E6)
	m_vunit = m_tunit;		// volt unit (mv)
	m_bMeasureDone=FALSE;	// no measure done yet
	m_bvalidextrema=FALSE;	// extrema are not valid
	m_spikeno = -1;			// spike currently selected
	m_pSpkDoc= nullptr;
	m_pSpkList= nullptr;
	m_divAmplitudeBy = 1;
	m_binit = FALSE;
	m_bEnableActiveAccessibility=FALSE; // workaround to crash / accessibility
}

// -------------------------------------------------------------------------

CSpikeSort1View::~CSpikeSort1View()
{
	// save spkD list i	 changed
	if (m_pSpkDoc != nullptr)
		SaveCurrentFileParms();	// save file if modified
	// save current spike detection parameters
	m_psC->bChanged = TRUE;
	m_psC->sourceclass=m_sourceclass;
	m_psC->destclass=m_destinationclass;
	m_psC->mvmax = m_mVMax;
	m_psC->mvmin = m_mVMin;
}

BOOL CSpikeSort1View::PreCreateWindow(CREATESTRUCT &cs)
{
// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

// -------------------------------------------------------------------------

void CSpikeSort1View::DoDataExchange(CDataExchange* pDX)
{
	CDaoRecordView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_PARAMETER, m_CBparameter);
	DDX_Text(pDX, IDC_T1, m_t1);
	DDX_Text(pDX, IDC_T2, m_t2);
	DDX_Text(pDX, IDC_LIMITLOWER, m_lower);
	DDX_Text(pDX, IDC_LIMITUPPER, m_upper);
	DDX_Text(pDX, IDC_SOURCECLASS, m_sourceclass);
	DDX_Text(pDX, IDC_DESTINATIONCLASS, m_destinationclass);
	DDX_Text(pDX, IDC_EDIT2, m_timeFirst);
	DDX_Text(pDX, IDC_EDIT3, m_timeLast);
	DDX_Text(pDX, IDC_EDIT6, m_mVMax);
	DDX_Text(pDX, IDC_EDIT7, m_mVMin);
	DDX_Check(pDX, IDC_CHECK1, m_bAllfiles);
	DDX_Text(pDX, IDC_NSPIKES, m_spikeno);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikenoclass);
	DDX_Text(pDX, IDC_EDITRIGHT2, m_txyright);
	DDX_Text(pDX, IDC_EDITLEFT2, m_txyleft);
	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}
	

// -------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CSpikeSort1View, CDaoRecordView)

	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)	 
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()	
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_SOURCECLASS,			OnEnChangeSourceclass)
	ON_EN_CHANGE(IDC_DESTINATIONCLASS,		OnEnChangeDestinationclass)
	ON_CBN_SELCHANGE(IDC_PARAMETER,			OnSelchangeParameter)
	ON_EN_CHANGE(IDC_LIMITLOWER,			OnEnChangelower)
	ON_EN_CHANGE(IDC_LIMITUPPER,			OnEnChangeupper)
	ON_EN_CHANGE(IDC_T1,					OnEnChangeT1)
	ON_EN_CHANGE(IDC_T2,					OnEnChangeT2)
	ON_BN_CLICKED(IDC_EXECUTE,				OnSort)
	ON_BN_CLICKED(IDC_MEASURE,				OnMeasure)
	ON_BN_CLICKED(IDC_CHECK1,				OnSelectAllFiles)
	ON_COMMAND(ID_FORMAT_ALLDATA,			OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE,		OnFormatCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST,		OnFormatGainadjust)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_ALIGNSPIKES,		OnToolsAlignspikes)
	ON_EN_CHANGE(IDC_EDIT2,					OnEnChangetimeFirst)
	ON_EN_CHANGE(IDC_EDIT3,					OnEnChangetimeLast)
	ON_EN_CHANGE(IDC_EDIT7,					OnEnChangemVMin)
	ON_EN_CHANGE(IDC_EDIT6,					OnEnChangemVMax)
	ON_EN_CHANGE(IDC_EDITLEFT2,				&CSpikeSort1View::OnEnChangeEditleft2)
	ON_EN_CHANGE(IDC_EDITRIGHT2,			&CSpikeSort1View::OnEnChangeEditright2)
	ON_EN_CHANGE(IDC_NSPIKES,				OnEnChangeNOspike)
	ON_BN_DOUBLECLICKED(IDC_DISPLAYPARM,	OnToolsEdittransformspikes)
	ON_EN_CHANGE(IDC_SPIKECLASS,			OnEnChangeSpikenoclass)
	
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CSpikeSort1View::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CSpikeSort1View::OnTcnSelchangeTab1)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpikeSort1View overriden functions


// -------------------------------------------------------------------------
// OnInitialUpdate - start here before displaying the view

void CSpikeSort1View::OnInitialUpdate()
{
	// attach set
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();    
	CdbWaveDoc* pdbDoc = GetDocument();
	CDaoRecordView::OnInitialUpdate();

	// load user parameters
	m_psC= &(pApp->spkC);		// spike classif parms	
	mdPM = &(pApp->vdP);		// viewdata options

	// assign controls to stretch
	m_stretch.AttachParent(this);		// attach formview pointer

	m_stretch.newProp(IDC_HISTOGRAM,	SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT6,		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT7,		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1,			XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_LIMITLOWER,	SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_LIMITUPPER,	SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_STATICLOWER,	SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_STATICUPPER2, SZEQ_XLEQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_DISPLAYPARM,	XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_DISPLAYBARS,	XLEQ_XREQ, SZEQ_YTEQ);	

	m_stretch.newProp(IDC_SCROLLBAR1,	XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_EDIT3,		SZEQ_XREQ, SZEQ_YTEQ);

	m_stretch.newProp(IDC_STATICRIGHT,	SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_STATICLEFT,	SZEQ_XLEQ, SZEQ_YBEQ);

	// subclass some controls
	VERIFY(m_spkHist.SubclassDlgItem(IDC_HISTOGRAM, this));
	VERIFY(m_spkXYp.SubclassDlgItem(IDC_DISPLAYPARM, this));
	VERIFY(m_spkForm.SubclassDlgItem(IDC_DISPLAYSPIKE, this));		
	VERIFY(mm_t1.SubclassDlgItem(IDC_T1, this));
	VERIFY(mm_t2.SubclassDlgItem(IDC_T2, this));
	VERIFY(mm_lower.SubclassDlgItem(IDC_LIMITLOWER, this));
	VERIFY(mm_upper.SubclassDlgItem(IDC_LIMITUPPER, this));
	VERIFY(mm_sourceclass.SubclassDlgItem(IDC_SOURCECLASS, this));
	VERIFY(mm_destinationclass.SubclassDlgItem(IDC_DESTINATIONCLASS, this));
	VERIFY(mm_timeFirst.SubclassDlgItem(IDC_EDIT2, this));
	VERIFY(mm_timeLast.SubclassDlgItem(IDC_EDIT3, this));
	VERIFY(mm_mVMax.SubclassDlgItem(IDC_EDIT6, this));
	VERIFY(mm_mVMin.SubclassDlgItem(IDC_EDIT7, this));
	VERIFY(m_spkBarView.SubclassDlgItem(IDC_DISPLAYBARS, this));
	VERIFY(mm_txyright.SubclassDlgItem(IDC_EDITRIGHT2, this));
	VERIFY(mm_txyleft.SubclassDlgItem(IDC_EDITLEFT2, this));

	VERIFY(mm_spikeno.SubclassDlgItem(IDC_NSPIKES, this));
	mm_spikeno.ShowScrollBar(SB_VERT);
	VERIFY(mm_spikenoclass.SubclassDlgItem(IDC_SPIKECLASS, this));
	mm_spikenoclass.ShowScrollBar(SB_VERT);
	
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100,FALSE);

	// assign values to controls
	m_CBparameter.SetCurSel(m_psC->iparameter);
	m_mVMax = m_psC->mvmax;
	m_mVMin = m_psC->mvmin;
	
	m_sourceclass=m_psC->sourceclass;
	m_destinationclass = m_psC->destclass;
	m_spkForm.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);	
	m_spkXYp.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	m_spkBarView.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	m_spkHist.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);

	m_spkForm.m_parms = mdPM->spksort1spk;
	m_spkXYp.m_parms = mdPM->spksort1parms;
	m_spkHist.m_parms = mdPM->spksort1hist;
	m_spkBarView.m_parms = mdPM->spksort1bars;

	// set bincrflagonsave
	((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->SetCheck(pApp->vdS.bincrflagonsave);

	// display tag lines at proper places
	m_spkformtagleft	= m_spkForm.AddVTtag(m_psC->ileft);		// first VTtag
	m_spkformtagright	= m_spkForm.AddVTtag(m_psC->iright);	// second VTtag
	m_itagup			= m_spkXYp.AddHZtag(m_psC->iupper, 0);	// first HZ tag
	m_itaglow			= m_spkXYp.AddHZtag(m_psC->ilower, 0);	// second HZ tag

	m_spkhistupper	= m_spkHist.AddVTtag(m_psC->iupper);
	m_spkhistlower	= m_spkHist.AddVTtag(m_psC->ilower);

	UpdateFileParameters();
	ActivateMode4();
	m_binit = TRUE;
}

void CSpikeSort1View::ActivateMode4()
{	
	int nCmdShow = SW_SHOW;
	if (m_psC->iparameter == 4)
	{
		nCmdShow = SW_SHOW;
		if (m_spkXYp.GetNVTtags() <1)
		{
			m_ixyright = m_spkXYp.AddVTtag(m_psC->ixyright);
			m_ixyleft = m_spkXYp.AddVTtag(m_psC->ixyleft);
			float delta = m_pSpkList->GetAcqSampRate() / m_tunit;
			m_txyright =((float) m_psC->ixyright) /delta ;
			m_txyleft = ((float) m_psC->ixyleft) /delta ;
		}
		m_spkXYp.SetNxScaleCells(2, 0, 0);
		m_spkXYp.m_parms.crScopeGrid= RGB(128,   128, 128);
		
		if (m_pSpkList != nullptr)
		{
			float spikelen_ms = (m_pSpkList->GetSpikeLength()* m_tunit) / m_pSpkList->GetAcqSampRate() ;
			CString csDummy;
			csDummy.Format(_T("%0.1f ms"), spikelen_ms);
			GetDlgItem(IDC_STATICRIGHT)->SetWindowText(csDummy);
			csDummy.Format(_T("%0.1f ms"), -spikelen_ms);
			GetDlgItem(IDC_STATICLEFT)->SetWindowText(csDummy);
		}
	}
	else
	{
		nCmdShow = SW_HIDE;
		
		m_spkXYp.DelAllVTtags();
		m_spkXYp.SetNxScaleCells(0, 0, 0);
	
	}
	GetDlgItem(IDC_STATICRIGHT)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATICLEFT)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATIC12)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATICLEFT2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATICRIGHT2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDITRIGHT2)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_EDITLEFT2)->ShowWindow(nCmdShow);
	m_spkXYp.Invalidate();
}

// -------------------------------------------------------------------------
// OnActivateView - activate this view or exit from the view

void CSpikeSort1View::OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		CMainFrame* pmF = (CMainFrame*) AfxGetMainWnd();
		pmF->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	else
	{
		SaveCurrentFileParms();	

		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		if (pApp->m_psort1spikesMemFile == nullptr)
		{
			pApp->m_psort1spikesMemFile = new CMemFile;
			ASSERT(pApp->m_psort1spikesMemFile != NULL);
		}

		CArchive ar(pApp->m_psort1spikesMemFile, CArchive::store);
		pApp->m_psort1spikesMemFile->SeekToBegin();
		ar.Close();	
		// set bincrflagonsave
		pApp->vdS.bincrflagonsave = ((CButton*) GetDlgItem(IDC_INCREMENTFLAG))->GetCheck();
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

// -------------------------------------------------------------------------
// OnUpdate - update view

void CSpikeSort1View::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_DOCHASCHANGED:		// file has changed?
	case HINT_DOCMOVERECORD:
	case HINT_REQUERY:
		UpdateFileParameters();
		break;
	case HINT_CLOSEFILEMODIFIED:	// close modified file: save
		SaveCurrentFileParms();
		break;
	case HINT_REPLACEVIEW:
	default:
		break;
	}
}

BOOL CSpikeSort1View::OnMove(UINT nIDMoveCommand) 
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


/////////////////////////////////////////////////////////////////////////////
// remove objects
void CSpikeSort1View::OnDestroy() 
{
	CDaoRecordView::OnDestroy();
}


/////////////////////////////////////////////////////////////////////////////
// CSpikeSort1View diagnostics

#ifdef _DEBUG
void CSpikeSort1View::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CSpikeSort1View::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CSpikeSort1View::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpikeSort1View database support

CDaoRecordset* CSpikeSort1View::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}

/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------
// UpdateFileParameters()

void CSpikeSort1View::UpdateFileParameters()
{
	// reset parms ? flag = single file or file list has changed
	if (  !m_bAllfiles
		 /*||(GetDocument()->GetNRecords() != m_nspkperfile.GetSize())*/)		
	{
		m_parm.SetSize(0);
		m_class.SetSize(0);
		m_iitime.SetSize(0);
		m_spkHist.RemoveHistData();
	}

	// change pointer to select new spike list & test if one spike is selected
	BOOL bfirstupdate = (m_pSpkDoc == nullptr);
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) m_pDocument;
	pdbDoc->OpenCurrentSpikeFile();
	m_pSpkDoc = pdbDoc->m_pSpk;
	if (m_pSpkDoc == nullptr)
		return;

	// reset tab control
	m_tabCtrl.DeleteAllItems();
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
	m_pSpkList = m_pSpkDoc->SetSpkListCurrent(pdbDoc->GetcurrentSpkListIndex());
	m_tabCtrl.SetCurSel(pdbDoc->GetcurrentSpkListIndex());

	// spike and classes
	int spikeno = m_pSpkList->m_selspike;
	if (spikeno > m_pSpkList->GetTotalSpikes()-1 || spikeno < 0)
		spikeno = -1;
	else	
	{	// set source class to the class of the selected spike
		m_sourceclass = m_pSpkList->GetSpikeClass(spikeno);		
		m_psC->sourceclass = m_sourceclass;
	}
	if (m_sourceclass > 32768)
		m_sourceclass = 0;

	// display source spikes
	m_spkForm.SetSourceData(m_pSpkList);
	m_spkBarView.SetSourceData(m_pSpkList, m_pSpkDoc);

	if (m_psC->ileft==0 && m_psC->iright==0)
	{
		m_psC->ileft = m_pSpkList->GetSpikePretrig();
		m_psC->iright = m_psC->ileft +  m_pSpkList->GetSpikeRefractory();
	}

	// refresh sorting parameters and data file properties
	m_t1 = (m_psC->ileft*m_tunit)/m_pSpkList->GetAcqSampRate();
	m_t2 = (m_psC->iright*m_tunit)/m_pSpkList->GetAcqSampRate();
	m_spkForm.SetVTtagVal(m_spkformtagleft, m_psC->ileft);
	m_spkForm.SetVTtagVal(m_spkformtagright, m_psC->iright);
	
	m_spkForm.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);
	m_spkXYp.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	m_spkBarView.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	m_spkHist.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);

	// this is like that if option "complete file selected"
	if (bfirstupdate || mdPM->bEntireRecord )
	{
		m_lFirst = 0;
		m_lLast = m_pSpkDoc->GetAcqSize()-1;
		m_timeFirst = 0.f;
		m_timeLast  = m_lLast  / m_pSpkList->GetAcqSampRate();
	}
	else
	{
		m_lFirst = long(m_timeFirst * m_pSpkList->GetAcqSampRate());
		m_lLast = long(m_timeLast * m_pSpkList->GetAcqSampRate());
	}

	// display & compute parameters
	if (!m_bAllfiles || !m_bMeasureDone)
	{
		m_spkXYp.SetSourceData(&m_parm, &m_iitime, &m_class, m_pSpkList);
		if (m_psC->iparameter != 4)
		{
			m_spkXYp.SetTimeIntervals(m_lFirst, m_lLast);
			if(m_spkXYp.GetNVTtags() >0)
			{
				m_spkXYp.DelAllVTtags();
				m_spkXYp.Invalidate();
			}
		}
		else
		{
			m_spkXYp.SetTimeIntervals(- m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
			if (m_spkXYp.GetNVTtags() <1)
			{
				m_ixyright = m_spkXYp.AddVTtag(m_psC->ixyright);	
				m_ixyleft = m_spkXYp.AddVTtag(m_psC->ixyleft);	
				float delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_txyright =((float) m_psC->ixyright) /delta ;
				m_txyleft = ((float) m_psC->ixyleft) /delta ;
				m_spkXYp.Invalidate();
			}
		}

		// update text , display and compute histogram
		m_bMeasureDone=FALSE;		// no parameters yet
		OnMeasure();
	}

	m_spkForm.SetTimeIntervals(m_lFirst, m_lLast);
	m_spkForm.Invalidate();
	m_spkBarView.SetTimeIntervals(m_lFirst, m_lLast);
	m_spkBarView.Invalidate();
	UpdateScrollBar();
	SelectSpike(spikeno, FALSE);
}


//----------------------------------------------------------------------------
// Update content of controls & replot the 3 windows with data

void CSpikeSort1View::UpdateLegends()
{
	// update text abcissa and horizontal scroll position
	m_pSpkList->m_lFirstSL = m_lFirst;
	m_pSpkList->m_lLastSL = m_lLast;	
	m_timeFirst = m_lFirst / m_pSpkList->GetAcqSampRate();
	m_timeLast  = m_lLast  / m_pSpkList->GetAcqSampRate();
	UpdateScrollBar();

	if (m_psC->iparameter != 4)
		m_spkXYp.SetTimeIntervals(m_lFirst, m_lLast);
	else
		m_spkXYp.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
	m_spkXYp.Invalidate();

	m_spkForm.SetTimeIntervals(m_lFirst, m_lLast);
	m_spkForm.Invalidate();

	m_spkBarView.SetTimeIntervals(m_lFirst, m_lLast);
	m_spkBarView.Invalidate();

	m_spkHist.Invalidate();
	
	UpdateData(FALSE);	// copy view object to controls	
}

/////////////////////////////////////////////////////////////////////////////
// CSpikeSort1View message handlers

// -------------------------------------------------------------------------
// OnSize - modif change of view

void CSpikeSort1View::OnSize(UINT nType, int cx, int cy)
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

// -------------------------------------------------------------------------
// OnEnChangeSourceclass - change source spike class : 
// display all spikes in grey, and the new source class in black

void CSpikeSort1View::OnEnChangeSourceclass()
{
	if (!mm_sourceclass.m_bEntryDone)
		return;

	short sourceclass = m_sourceclass;
	switch (mm_sourceclass.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); sourceclass = m_sourceclass; break;
	case VK_UP:
	case VK_PRIOR:	sourceclass++; break;
	case VK_DOWN:
	case VK_NEXT: 	sourceclass--; break;
	}

	// change display if necessary	
	mm_sourceclass.m_bEntryDone=FALSE;	// clear flag
	mm_sourceclass.m_nChar=0;			// empty buffer
	mm_sourceclass.SetSel(0, -1);		// select all text	
	if (sourceclass != m_sourceclass)
	{
		m_sourceclass = sourceclass;
		m_spkForm.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);
		m_spkXYp.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
		m_spkHist.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
		m_spkBarView.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	}

	// change histogram accordingly
	m_spkForm.Invalidate();
	m_spkBarView.Invalidate();
	m_spkXYp.Invalidate();
	m_spkHist.Invalidate();
	SelectSpike(-1);
	UpdateData(FALSE);
}

// -------------------------------------------------------------------------
// OnEnChangeDestinationclass - change no of spike class in which spikes
// matching criteria will be transformed

void CSpikeSort1View::OnEnChangeDestinationclass()
{
	if (!mm_destinationclass.m_bEntryDone)
		return;

	short destinationclass = m_destinationclass;
	switch (mm_destinationclass.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); destinationclass = m_destinationclass; break;
	case VK_UP:
	case VK_PRIOR:	destinationclass++; break;
	case VK_DOWN:
	case VK_NEXT: 	destinationclass--; break;
	}

	// change display if necessary	
	mm_destinationclass.m_bEntryDone=FALSE;	// clear flag
	mm_destinationclass.m_nChar=0;			// empty buffer
	mm_destinationclass.SetSel(0, -1);		// select all text	
	m_destinationclass = destinationclass;
	SelectSpike(-1);
	UpdateData(FALSE);
}


// -------------------------------------------------------------------------
// OnSelchangeParameter - change selection parameter

void CSpikeSort1View::OnSelchangeParameter()
{
	int iparameter = m_CBparameter.GetCurSel();
	if (iparameter != m_psC->iparameter)
	{
		m_psC->iparameter=iparameter;
		ActivateMode4();
		OnMeasure();
		OnFormatCentercurve();
	}

	//    STATIC3 lower STATIC4 upper STATIC5 T1 STATIC6 T2
	// 0  mV      vis    mV      vis    vis     vis  vis   vis 
	// 1  mV      vis    mV      vis    vis     vis  NOT   NOT 
	// 2  mS      vis    mS      vis    vis     vis  vis   vis 	
}

// -------------------------------------------------------------------------
// OnEnChangelower - change lowver value

void CSpikeSort1View::OnEnChangelower()
{
	if (!mm_lower.m_bEntryDone)
		return;

	float lower = m_lower;
	if (!m_bAllfiles)
		m_delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;

	switch (mm_lower.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); 	lower = m_lower;	break;
	case VK_UP:
	case VK_PRIOR:	lower+= m_delta; break;
	case VK_DOWN:
	case VK_NEXT: 	lower-= m_delta; break;
	}

	// check boundaries
	if (lower >= m_upper)
		lower = m_upper - m_delta*10.f;
	// change display if necessary	
	mm_lower.m_bEntryDone=FALSE;	// clear flag
	mm_lower.m_nChar=0;				// empty buffer
	mm_lower.SetSel(0, -1);			// select all text
	m_lower = lower;
	m_psC->ilower = (int) (m_lower/ m_delta);
	if (m_psC->ilower != m_spkXYp.GetHZtagVal(m_itaglow))
		m_spkXYp.MoveHZtagtoVal(m_itaglow, m_psC->ilower);
	if (m_psC->ilower != m_spkHist.GetVTtagVal(m_spkhistlower))
		m_spkHist.MoveVTtagtoVal(m_spkhistlower, m_psC->ilower);
	UpdateData(FALSE);
}

// -------------------------------------------------------------------------
// OnEnChangeupper - change upper value

void CSpikeSort1View::OnEnChangeupper()
{
	if (!mm_upper.m_bEntryDone)
		return;

	float upper = m_upper;
	if (!m_bAllfiles)
		m_delta =m_pSpkList->GetAcqVoltsperBin()*m_vunit;

	switch (mm_upper.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); upper = m_upper; break;
	case VK_UP:
	case VK_PRIOR:	upper+= m_delta; break;
	case VK_DOWN:
	case VK_NEXT: 	upper-= m_delta; break;
	}

	// check boundaries
	//if (upper <0)
	//	upper = 0;
	if (upper <= m_lower)
		upper = m_lower + m_delta *10.f;
	// change display if necessary	
	mm_upper.m_bEntryDone=FALSE;	// clear flag
	mm_upper.m_nChar=0;			// empty buffer
	mm_upper.SetSel(0, -1);		// select all text
	m_upper = upper;	
	m_psC->iupper = (int) (m_upper/m_delta);
	if (m_psC->iupper != m_spkXYp.GetHZtagVal(m_itagup))
		m_spkXYp.MoveHZtagtoVal(m_itagup, m_psC->iupper);		
	if (m_psC->ilower != m_spkHist.GetVTtagVal(m_spkhistupper))	
		m_spkHist.MoveVTtagtoVal(m_spkhistupper, m_psC->iupper);		
	UpdateData(FALSE);
}

// -------------------------------------------------------------------------
// change t1 value

void CSpikeSort1View::OnEnChangeT1()
{
	if (!mm_t1.m_bEntryDone)
		return;

	float t1 = m_t1;
	float delta = m_tunit/m_pSpkList->GetAcqSampRate();
	
	switch (mm_t1.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	
			UpdateData(TRUE); 	t1 = m_t1;	break;
		case VK_UP:
		case VK_PRIOR: t1+=delta; break;
		case VK_DOWN:
		case VK_NEXT: t1-=delta;	break;
		}
	// check boundaries
	if (t1 <0)
		t1 = 0.0f;
	if (t1 >= m_t2)
		t1 = m_t2 - delta;
	// change display if necessary	
	mm_t1.m_bEntryDone=FALSE;	// clear flag
	mm_t1.m_nChar=0;			// empty buffer
	mm_t1.SetSel(0, -1);		// select all text
	m_t1 = t1;
	int it1 = (int) (m_t1/delta);
	if (it1 != m_spkForm.GetVTtagVal(m_spkformtagleft))
	{
		m_psC->ileft = it1;		
		m_spkForm.MoveVTtrack(m_spkformtagleft, m_psC->ileft);
		m_pSpkList->m_imaxmin1SL = m_psC->ileft;
	}
	UpdateData(FALSE);
}

// -------------------------------------------------------------------------
// change t2 value

void CSpikeSort1View::OnEnChangeT2()
{
	if (!mm_t2.m_bEntryDone)
		return;

	float t2 = m_t2;
	float delta = m_tunit/m_pSpkList->GetAcqSampRate();
	switch (mm_t2.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE);
		t2 = m_t2;
		break;
	case VK_UP:
	case VK_PRIOR: 
		t2+=delta;
		break;
	case VK_DOWN:
	case VK_NEXT: 
		t2-=delta; 
		break;
	}

	// check boundaries
	if (t2 <m_t1)
		t2 = m_t1+delta;
	float tmax = (m_pSpkList->GetSpikeLength()-1) *delta;
	if (t2 >= tmax)
		t2 = tmax;
	// change display if necessary	
	mm_t2.m_bEntryDone=FALSE;	// clear flag
	mm_t2.m_nChar=0;			// empty buffer
	mm_t2.SetSel(0, -1);		// select all text
	m_t2 = t2;
	int it2 = (int) (m_t2/delta);
	if (it2 != m_spkForm.GetVTtagVal(m_spkformtagright))
	{
		m_psC->iright = it2;		
		m_spkForm.MoveVTtrack(m_spkformtagright, m_psC->iright);
		m_pSpkList->m_imaxmin2SL = m_psC->iright;
	}
	UpdateData(FALSE);
}

// -------------------------------------------------------------------------
// sort spikes according to the parameters

void CSpikeSort1View::OnSort()
{
	if (!m_bMeasureDone)
	{
		AfxMessageBox(_T("Evaluate parameters first!\nHit ""Measure"" "));
		return;
	}
	
	// set file indexes - assume only one file selected
	CdbWaveDoc* pdbDoc = GetDocument();
	int currentfile = pdbDoc->DBGetCurrentRecordPosition(); // index current file
	int firstfile = currentfile;				// index first file in the series
	int lastfile = firstfile;					// index last file in the series
	int nfiles = pdbDoc->DBGetNRecords();
	
	//m_nspkperfile.SetSize(nfiles);	// nb spk per file
	int currentlist = m_pSpkDoc->GetSpkListCurrentIndex();
	
	// change indexes if ALL files selected
	CProgressDlg* pdlg = nullptr;
	int istep = 0;
	CString cscomment;

	if (m_bAllfiles)
	{
		firstfile = 0;						// index first file
		lastfile = pdbDoc->DBGetNRecords() -1;// index last file
		pdlg = new CProgressDlg;
		pdlg->Create();
		pdlg->SetStep (1);
	}	
	BOOL flag = FALSE;	// flag to tell param routines to erase all data the first time

	// loop over all selected files (or only one file currently selected)
	int ilast = 0;
	pdbDoc->DBSetCurrentRecordPosition(firstfile);

	for (int ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// store nb of spikes within array
		BOOL flagchanged = FALSE;
		if (m_bAllfiles)
		{
			pdbDoc->DBSetCurrentRecordPosition(ifile);
			pdbDoc->OpenCurrentSpikeFile();
			m_pSpkDoc = pdbDoc->m_pSpk;
			if (m_pSpkDoc == nullptr)
				continue;
			
			// check if user wants to stop
			if (pdlg->CheckCancelButton())
				if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
					break;
			cscomment.Format(_T("Processing file [%i / %i]"), ifile+1, nfiles);
			pdlg->SetStatus(cscomment);
			if (MulDiv(ifile, 100, nfiles) > istep)
			{
				pdlg->StepIt();
				istep = MulDiv(ifile, 100, nfiles);
			}
		}
		if (m_pSpkDoc == nullptr)
			continue;

		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist);
		if (m_pSpkList == nullptr)
			continue;

		int nspikes = m_pSpkList->GetTotalSpikes();
		//m_nspkperfile[ifile] = nspikes;
		if (m_pSpkList->GetSpikeLength() == 0)
			continue;

		int ifirst = ilast;		// global index of first spike, file "ifile"
		ilast += nspikes;		// global index +1 of last spike, file "ifile"

		// loop over all spikes of the list and compare to a single parameter
		if (m_psC->iparameter != 4)
		{
			for (int ispike = ifirst; ispike < ilast; ispike++)
			{
				WORD sclass = m_class[ispike];
				if (m_class[ispike] != (WORD) m_sourceclass)
					continue;
				if ((long) m_iitime[ispike] > m_lLast 
					|| (long) m_iitime[ispike] < m_lFirst)
					continue;
				short val = m_parm[ispike];
				if (val < m_psC->ilower || val > m_psC->iupper)
					continue;

				// change spike class
				int ilocal = ispike - ifirst;	// local spike index
				m_pSpkList->SetSpikeClass(ilocal, m_destinationclass);
				m_class[ispike] = m_destinationclass;
				flagchanged = TRUE;
			}
		}
		// sort option with 2 boudaries
		else 
		{
			for (int ispike = ifirst; ispike < ilast; ispike++)
			{
				WORD sclass = m_class[ispike];
				if (m_class[ispike] != (WORD) m_sourceclass)
					continue;
				if ((long) m_iitime[ispike] > m_lLast 
					|| (long) m_iitime[ispike] < m_lFirst)
					continue;

				short val = m_parm[ispike];
				if (val < m_psC->ilower || val > m_psC->iupper)
					continue;

				long lval = (long) m_parm2[ispike];
				if (lval < m_psC->ixyleft || lval > m_psC->ixyright)
					continue;

				// spike fits criteria - change class
				int ilocal = ispike - ifirst;	// local spike index
				m_pSpkList->SetSpikeClass(ilocal, m_destinationclass);
				m_class[ispike] = m_destinationclass;
				flagchanged = TRUE;
			}
		}

		if (flagchanged)
		{
			m_pSpkDoc->OnSaveDocument(pdbDoc->DBGetCurrentSpkFileName(FALSE));
			pdbDoc->Setnbspikes(nspikes);
		}
	}

	// end of loop, select current file again if necessary
	if (m_bAllfiles)
	{
		delete pdlg;
		pdbDoc->DBSetCurrentRecordPosition(currentfile);
		pdbDoc->OpenCurrentSpikeFile();
		m_pSpkDoc = pdbDoc->m_pSpk;
		m_pSpkList = (CSpikeList*) m_pSpkDoc->GetSpkListCurrent();
	}

	// refresh data windows
	m_spkXYp.Invalidate();
	m_spkForm.Invalidate();
	m_spkBarView.Invalidate();
	m_spkHist.BuildHistFromWordArray(&m_parm, &m_iitime, &m_class,
							m_lFirst, m_lLast,
							m_parmmax, m_parmmin, 
							m_spkHist.GetnBins(),
							TRUE);
	m_spkHist.Invalidate();
	SelectSpike(-1);
	m_pSpkDoc->SetModifiedFlag(TRUE); // set flag: document has changed
}

// --------------------------------------------------------------------------
// message handler for messages from CScopeScreenWnd windows and derived classes

LRESULT CSpikeSort1View::OnMyMessage(WPARAM code, LPARAM lParam)
{
	short threshold = LOWORD(lParam);	// value associated	
	short wParam = HIWORD(lParam);

	switch (code)
	{
	case HINT_SETMOUSECURSOR: // ------------- change mouse cursor (all 3 items)	
		if (threshold >CURSOR_ZOOM)	// clip cursor shape to max
			threshold = 0;
		SetViewMouseCursor(threshold);	// change cursor val in the other button
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(threshold, 0));
		break;	

	// ------------------- select bar/display bars or zoom		
	// if (HIWORD(lParam) == IDC_DISPLAYBARS || HIWORD(lParam) == NULL)

	case HINT_CHANGEHZLIMITS:// -------------  abcissa have changed
		if (m_psC->iparameter != 4)
		{
			m_lFirst = m_spkXYp.GetTimeFirst();
			m_lLast = m_spkXYp.GetTimeLast();
		}
		else
		{
			m_lFirst = m_spkBarView.GetTimeFirst();
			m_lLast = m_spkBarView.GetTimeLast();
		}
		UpdateLegends();
		break;

	case HINT_HITSPIKE:		// -------------  spike is selected or deselected
		{
			if (m_pSpkList->GetSpikeFlagArrayCount() > 0)
			{
				m_pSpkList->RemoveAllSpikeFlags();
				m_spkXYp.Invalidate();
				m_spkForm.Invalidate();
				m_spkBarView.Invalidate();
			}
			int spikeno=0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spikeno = m_spkForm.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spikeno = m_spkBarView.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spikeno = m_spkXYp.GetHitSpike();  // if m_bAllFiles, spikeno is global, otherwise it comes from a single file...
			SelectSpike (spikeno, (m_bAllfiles && HIWORD(lParam) == IDC_DISPLAYPARM));
		}
		break;

	case HINT_SELECTSPIKES:
		m_spkXYp.Invalidate();
		m_spkForm.Invalidate();
		m_spkBarView.Invalidate();
		break;

	case HINT_DBLCLKSEL:
		{
			int spikeno=0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				m_spikeno = m_spkForm.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				m_spikeno = m_spkBarView.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				m_spikeno = m_spkXYp.GetHitSpike();  // if m_bAllFiles, spikeno is global, otherwise it comes from a single file...
			SelectSpike(spikeno, (m_bAllfiles && HIWORD(lParam) == IDC_DISPLAYPARM));
			OnToolsEdittransformspikes();
		}		
		break;

	//case HINT_MOVEVERTTAG:	 // -------------  vertical tag has moved lowp = tag index		
	case HINT_CHANGEVERTTAG: // -------------  vertical tag val has changed
		if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
		{
			if (threshold == m_spkformtagleft)		// first tag
			{
				m_psC->ileft = m_spkForm.GetVTtagVal(m_spkformtagleft);
				m_t1 = m_psC->ileft*m_tunit/m_pSpkList->GetAcqSampRate();
				mm_t1.m_bEntryDone = TRUE;
				OnEnChangeT1();
			}
			else if (threshold == m_spkformtagright)	// second tag
			{
				m_psC->iright = m_spkForm.GetVTtagVal(m_spkformtagright);
				m_t2 = m_psC->iright*m_tunit/m_pSpkList->GetAcqSampRate();
				mm_t2.m_bEntryDone = TRUE;
				OnEnChangeT2();
			}
		}
		else if (HIWORD(lParam) == IDC_HISTOGRAM)
		{
			if (threshold == m_spkhistlower)		// first tag
			{
				m_psC->ilower = m_spkHist.GetVTtagVal(m_spkhistlower);	// load new value
				m_lower = m_psC->ilower*m_pSpkList->GetAcqVoltsperBin()*m_vunit;
				mm_lower.m_bEntryDone = TRUE;
				OnEnChangelower();
			}
			else if (threshold == m_spkhistupper)	// second tag
			{
				m_psC->iupper = m_spkHist.GetVTtagVal(m_spkhistupper);	// load new value
				m_upper = m_psC->iupper*m_pSpkList->GetAcqVoltsperBin()*m_vunit;
				mm_upper.m_bEntryDone = TRUE;
				OnEnChangeupper();
			}
		}
		else if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (threshold == m_ixyright)
			{
				float delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_psC->ixyright = m_spkXYp.GetVTtagVal(m_ixyright);
				m_txyright =((float) m_psC->ixyright) /delta ;
				mm_txyright.m_bEntryDone = TRUE;
				OnEnChangeEditright2();
			}
			else if (threshold == m_ixyleft)
			{
				float delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_psC->ixyleft = m_spkXYp.GetVTtagVal(m_ixyleft);
				m_txyleft = ((float)m_psC->ixyleft) / delta ;
				mm_txyleft.m_bEntryDone = TRUE;
				OnEnChangeEditleft2();
			}
		}
		break;

	case HINT_CHANGEHZTAG:	// ------------- change horizontal tag value
	//case HINT_MOVEHZTAG:	// ------------- move horizontal tag
		if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (threshold == m_itaglow)		// first tag
			{
				m_psC->ilower = m_spkXYp.GetHZtagVal(m_itaglow);	// load new value
				m_lower = m_psC->ilower*m_pSpkList->GetAcqVoltsperBin()*m_vunit;
				mm_lower.m_bEntryDone = TRUE;
				OnEnChangelower();
			}
			else if (threshold == m_itagup)	// second tag
			{
				m_psC->iupper = m_spkXYp.GetHZtagVal(m_itagup);	// load new value
				m_upper = m_psC->iupper*m_pSpkList->GetAcqVoltsperBin()*m_vunit;
				mm_upper.m_bEntryDone = TRUE;
				OnEnChangeupper();
			}
		}
		break;

	case HINT_VIEWSIZECHANGED:   // ------------- change zoom
		UpdateLegends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		mdPM->spksort1spk = m_spkForm.m_parms;
		mdPM->spksort1parms = m_spkXYp.m_parms;
		mdPM->spksort1hist = m_spkHist.m_parms;
		mdPM->spksort1bars = m_spkBarView.m_parms;
		break;

	default:
		break;		
	}
	return 0L;
}

// -------------------------------------------------------------------------
// OnMeasure - measure parameters for 1 file (current) or all

void CSpikeSort1View::OnMeasure() 
{
	// set file indexes - assume only one file selected
	CdbWaveDoc* pdbDoc = GetDocument();
	int currentfile = pdbDoc->DBGetCurrentRecordPosition(); // index current file	
	int nfiles		= pdbDoc->DBGetNRecords();
	int currentlist = m_pSpkDoc->GetSpkListCurrentIndex();
	int firstfile;				// index first file in the series
	int lastfile ;				// index last file in the series
	
	// change size of arrays and prepare temporary dialog
	SelectSpike(-1);
	if (m_bAllfiles)
	{
		firstfile = 0;						// index first file
		lastfile = nfiles-1;				// index last file
	}
	else
	{
		firstfile = currentfile;			// index first file in the series
		lastfile = firstfile;				// index last file in the series
	}
	m_nspkperfile.SetSize(lastfile-firstfile+2);		// nb spk per file
	BOOL flag = FALSE;						// flag to tell param routines to erase all data the first time they are called

	// loop over all selected files (or only one file currently selected)	
	int totalspikes = 0;
	for (int ifile0=firstfile; ifile0 <= lastfile; ifile0++)
	{
		// check if user wants to continue
		if (m_bAllfiles)
		{
			m_nspkperfile[ifile0-firstfile] = totalspikes; // store nb of spikes within array
			pdbDoc->DBSetCurrentRecordPosition(ifile0);
			pdbDoc->OpenCurrentSpikeFile();
			m_pSpkDoc = pdbDoc->m_pSpk;
		}
		// check if this file is ok
		if (m_pSpkDoc == nullptr)
			continue;
		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist);
		if (m_pSpkList == nullptr)
			continue;

		totalspikes += m_pSpkList->GetTotalSpikes();
		m_nspkperfile[ifile0-firstfile +1] = totalspikes;
	}
	int growby = 16384;
	m_parm.SetSize(totalspikes, growby);		// parameter value
	m_parm2.SetSize(totalspikes, growby);
	m_class.SetSize(totalspikes, growby);		// class value
	m_iitime.SetSize(totalspikes, growby);		// time index

	// loop over all selected files (or only one file currently selected)
	for (int ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// check if user wants to continue
		if (m_bAllfiles)
		{
			pdbDoc->DBSetCurrentRecordPosition(ifile);
			pdbDoc->OpenCurrentSpikeFile();
			m_pSpkDoc = pdbDoc->m_pSpk;
		}
		// check if this file is ok
		if (m_pSpkDoc == nullptr)
			continue;
		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist);
		if (m_pSpkList == nullptr)
			continue;
		int nspikes = m_pSpkList->GetTotalSpikes();
		if (nspikes <= 0 ||  m_pSpkList->GetSpikeLength() == 0)
			continue;

		BOOL bChanged = FALSE;
		switch(m_psC->iparameter)
		{
		case 0:		// max - min between t1 and t2
			bChanged = MeasureSpkParm1(flag, 0, ifile);
			break;
		case 1:		// value at t1 
			MeasureSpkParm2(flag, 0, ifile);
			break;
		case 2:		// value at t2
			MeasureSpkParm2(flag, 1, ifile);
			break;
		case 3:		// value t2-t1
			MeasureSpkParm2(flag, 2, ifile);
			break;
		case 4:		// max-min vs tmax-tmin
			bChanged = MeasureSpkParm1(flag, 1, ifile);
			break;
		case 5:
			MeasureSpkParm4(flag, 1, ifile);
			break;
		case 6:
			MeasureSpkParm4(flag, 2, ifile);
			break;
		case 7:
			MeasureSpkParm4(flag, 3, ifile);
			break;
		default:
			bChanged = MeasureSpkParm1(flag, 0, ifile);
			break;
		}

		//save only if changed?
		if (bChanged)
			m_pSpkDoc->OnSaveDocument(pdbDoc->DBGetCurrentSpkFileName(FALSE));
		flag = TRUE;		
	}

	// end of loop, select current file again if necessary
	m_parm.FreeExtra();
	m_parm2.FreeExtra();
	m_class.FreeExtra();
	m_iitime.FreeExtra();

	if (m_bAllfiles)
	{
		currentfile = pdbDoc->DBGetCurrentRecordPosition(); // index current file	
		pdbDoc->DBSetCurrentRecordPosition(currentfile);
		pdbDoc->OpenCurrentSpikeFile();
		m_pSpkDoc = pdbDoc->m_pSpk;
		m_pSpkList = (CSpikeList*) m_pSpkDoc->GetSpkListCurrent();
		m_spkForm.SetSourceData(m_pSpkList);
	}
			
	// adjust display (search max & min), modify tags
 	if (m_parmmax < m_parmmin)		// make sure that max > min
	{
		int i = m_parmmax;
		m_parmmax = m_parmmin;
		m_parmmin = i;
	}

	if (m_psC->ilower ==0 && m_psC->iupper ==0)	// make sure that HZ tags are defined
	{
		m_psC->ilower = m_parmmin;
		m_psC->iupper = m_parmmax;
	}
	int max = m_parmmax;			// set limits to include tags
	if (max < m_psC->iupper)
		max = m_psC->iupper;
	int min = m_parmmin;
	if (min > m_psC->ilower)
		min = m_psC->ilower;

	// tell display routine where data are (pass pointer)
	if (m_psC->iparameter != 4)
	{
		m_spkXYp.SetSourceData(&m_parm, &m_iitime, &m_class, m_pSpkList);		
	}
	else
	{
		m_spkXYp.SetSourceData(&m_parm, &m_parm2, &m_class, m_pSpkList);
		int xWE = m_pSpkList->GetSpikeLength()*2;
		m_spkXYp.SetXWExtOrg(xWE, 0);
		m_spkXYp.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
	}

	// adapt the gain
	float delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
	
	if (mdPM->bMaximizeGain)
	{
		int yWE = MulDiv(max-min, 10, 8);	// get extension and origin
		int yWO = (max+min)/2;
		m_mVMin = (yWO - (yWE / 2))*delta;
		m_mVMax = (yWO + (yWE / 2))*delta;
	}

	m_lower = m_psC->ilower*delta;
	m_upper = m_psC->iupper*delta;
	m_spkXYp.SetHZtagVal(m_itaglow, m_psC->ilower);		// set HZ tags
	m_spkXYp.SetHZtagVal(m_itagup, m_psC->iupper);

	// compute histogram
	int nbins = m_parmmax-m_parmmin+1;
	if (nbins < 0)		// no values...
	{
		nbins = m_spkHist.Width()/2;
		m_parmmax = 0;
		m_parmmin = 0;
	}
	if (nbins > m_spkHist.Width()/2)
		nbins = m_spkHist.Width()/2;
	m_spkHist.BuildHistFromWordArray(&m_parm, &m_iitime, &m_class,
							m_lFirst, m_lLast,
							m_parmmax, m_parmmin, nbins,
							TRUE);

	m_spkHist.SetVTtagVal(m_itaglow, m_psC->ilower);
	m_spkHist.SetVTtagVal(m_itagup, m_psC->iupper);	

	UpdateGain();
	UpdateData(FALSE);
}

void CSpikeSort1View::UpdateGain()
{
	float delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
	
	int max = int (m_mVMax / delta);
	int min = int (m_mVMin / delta);
	int yWE = max-min;
	int yWO = (max+min)/2;
	m_spkXYp.SetYWExtOrg(yWE, yWO);
	m_spkXYp.Invalidate();
	m_spkHist.SetXWExtOrg(yWE, yWO-yWE/2);
	m_spkHist.Invalidate();
}

//---------------------------------------------------------------------------  
//	parm1 = Amplitude max-min (t1:t2)
//	parm2 = Amplitude at offset t1 or t2
//	parm3 = Time max-min (t1:t2)
// ioption: 0= display max-min vs time; 1= display max-min vs tmax-tmin
// assume m_pSpklist is valid


BOOL CSpikeSort1View::MeasureSpkParm1(BOOL bkeepOldData, int ioption, int currentfile)
{
	BOOL bChanged = FALSE;
	int nspikes = m_pSpkList->GetTotalSpikes(); //m_nspkperfile[currentfile];
	// index first spike within array
	int pindex = 0;
	if (m_bAllfiles)
		pindex = LocalIndextoGlobal(currentfile, 0);

	//	parm1 = Amplitude max-min (t1:t2)
	if (m_psC->ileft <0)
		m_psC->ileft = 0;
	if (m_psC->iright > m_pSpkList->GetSpikeLength()-1)
		m_psC->iright = m_pSpkList->GetSpikeLength()-1;
	if (m_psC->ileft > m_psC->iright)
		m_psC->ileft = m_psC->iright;
	
	if (!bkeepOldData)
	{
		m_parmmax = 0;
		m_parmmin = 8192;
	}

	// loop over all spikes of the list
	
	// look if parameters have changed - if not, then go on to the next file ...
	BOOL bValidExtrema = FALSE;
	if (m_pSpkList->m_imaxmin1SL == m_psC->ileft && m_pSpkList->m_imaxmin2SL == m_psC->iright)
	{
		if (m_psC->iparameter != 4) // then, we need imax imin ...
			bValidExtrema = TRUE;
	}
	else
	{
		m_pSpkList->m_imaxmin1SL = m_psC->ileft;
		m_pSpkList->m_imaxmin2SL = m_psC->iright;
		bChanged = TRUE;
	}

	int max, min, dmaxmin;
	int imax= m_psC->ileft;
	int imin= m_psC->iright;
		
	for (int ispike = 0; ispike < nspikes; ispike++, pindex++)
	{
		m_iitime[pindex] = m_pSpkList->GetSpikeTime(ispike);
		m_class[pindex] = m_pSpkList->GetSpikeClass(ispike);
		CSpikeElemt* se = (CSpikeElemt *) m_pSpkList->GetSpikeElemt(ispike);

		if (!bValidExtrema)
		{
			m_pSpkList->MeasureSpikeMaxMinEx(ispike, &max, &imax, &min, &imin, m_psC->ileft, m_psC->iright);
			dmaxmin = imin - imax;
			se->SetSpikeMaxMin(max, min, dmaxmin);
			bChanged = TRUE;
		}
		else
		{
			se->GetSpikeMaxMin  (&max, &min, &dmaxmin) ;	
		}

		// store measure into parm and update max min
		int diff = max-min;		// get parameter
		m_parm[pindex] = diff;	// store parm value
		if (m_psC->iparameter == 4)
			m_parm2[pindex] = dmaxmin; 
		if (diff > m_parmmax)	// store max & min / array
			m_parmmax = diff;
		if (diff < m_parmmin)
			m_parmmin = diff;
	}

	// exit
	m_bvalidextrema = TRUE;
	m_bMeasureDone=TRUE;
	return bChanged;
}

// -------------------------------------------------------------------------
// measurespkparm2 
// ioption = 0: measure at t1 (default)
// ioption = 1: measure at t2
// assume m_pSpkList is valid

void CSpikeSort1View::MeasureSpkParm2(BOOL bkeepOldData, int ioption, int currentfile)
{	
	int nspikes = m_pSpkList->GetTotalSpikes(); //m_nspkperfile[currentfile];
	// index first spike within array
	int pindex = 0;
	if (m_bAllfiles)
		pindex = LocalIndextoGlobal(currentfile, 0);

	//	parm2 = Amplitude at t1
	short* lpB;
	if (m_psC->ileft <0)
		m_psC->ileft = 0;
	if (m_psC->iright > m_pSpkList->GetSpikeLength()-1)
		m_psC->iright = m_pSpkList->GetSpikeLength()-1;
	if (m_psC->ileft > m_psC->iright)
		m_psC->ileft = m_psC->iright;
	if (!bkeepOldData)
	{
		m_parmmax = 0;
		m_parmmin = 8192;
	}

	// loop over all spikes of the list
	int indexoffset = m_psC->ileft;
	if (ioption == 1)
		indexoffset = m_psC->iright;

	for (int ispike = 0; ispike < nspikes; ispike++, pindex++)
	{
		m_iitime[pindex] = m_pSpkList->GetSpikeTime(ispike);
		m_class[pindex] = m_pSpkList->GetSpikeClass(ispike);
		lpB = m_pSpkList->GetpSpikeData(ispike)+indexoffset;
		short val= *lpB;		//  value
		if (ioption == 2)
			val = *(m_pSpkList->GetpSpikeData(ispike)+m_psC->iright);

		m_parm[pindex] = val;	// store parm value
		if (val > m_parmmax)	// store max & min / array
			m_parmmax = val;
		if (val < m_parmmin)
			m_parmmin = val;
	}

	// exit
	m_bvalidextrema = TRUE;
	m_bMeasureDone=TRUE;
}

// -------------------------------------------------------------------------
// OnFormatAlldata - view all data (length of the recordings)

void CSpikeSort1View::OnFormatAlldata() 
{
	// build new histogram only if necessary
	BOOL buildHistogram = FALSE;

	// dots: spk file length
	if (m_lFirst != 0 || m_lLast != m_pSpkDoc->GetAcqSize()-1)
	{	
		m_lFirst = 0;
		m_lLast = m_pSpkDoc->GetAcqSize()-1;

		if (m_psC->iparameter != 4) // then, we need imax imin ...
			m_spkXYp.SetTimeIntervals(m_lFirst, m_lLast);
		else
			m_spkXYp.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
		m_spkXYp.Invalidate();

		m_spkForm.SetTimeIntervals(m_lFirst, m_lLast);
		m_spkForm.Invalidate();

		m_spkBarView.SetTimeIntervals(m_lFirst, m_lLast);
		m_spkBarView.Invalidate();
		buildHistogram = TRUE;
	}

	// spikes: center spikes horizontally and adjust hz size of display	
	
	int xWE = m_pSpkList->GetSpikeLength();
	if (xWE != m_spkForm.GetXWExtent() || 0 != m_spkForm.GetXWOrg())
		m_spkForm.SetXWExtOrg(xWE, 0);

	// change m_spkHist
	if (buildHistogram)
	{
		short nbins = m_parmmax-m_parmmin+1;
		if (nbins > m_spkHist.Width()/2)
			nbins = m_spkHist.Width()/2;
			m_spkHist.BuildHistFromWordArray(&m_parm, &m_iitime, &m_class,
					m_lFirst, m_lLast,
					m_parmmax, m_parmmin, nbins,
					TRUE);
	}
	UpdateLegends();
}

// -------------------------------------------------------------------------
// OnFormatCentercurve - center spikes & parameters

void CSpikeSort1View::OnFormatCentercurve() 
{
	// loop over all spikes of the list  
	int nspikes = m_pSpkList->GetTotalSpikes();
	for (int ispike = 0; ispike < nspikes; ispike++)
		m_pSpkList->CenterSpikeAmplitude(ispike, m_psC->ileft, m_psC->iright, 1);

	int max, min;
	m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
	int middle = (max + min)/2;
	m_spkForm.SetYWExtOrg(m_spkForm.GetYWExtent(), middle);
	m_spkBarView.SetYWExtOrg(m_spkForm.GetYWExtent(), middle);

	UpdateLegends();
}

// -------------------------------------------------------------------------
// OnFormatGainadjust - adjust gain to fit data within window

void CSpikeSort1View::OnFormatGainadjust() 
{		
	// adjust gain of spkform and spkbar: data = raw signal
	int max, min; 
	GetDocument()->GetAllSpkMaxMin(m_bAllfiles, TRUE, &max, &min);
	//m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);

	int yWE = MulDiv(max-min+1, 10, 9);
	int yWO = (max + min)/2;
	m_spkForm.SetYWExtOrg(yWE, yWO);
	m_spkBarView.SetYWExtOrg(yWE, yWO);
	
	// adjust gain for m_spkHist and XYp: data = computed values
	// search max min of parameter values
	if (m_parm.GetSize() > 0)
		max = m_parm[0];
	min = max;
	for (int i=0; i<m_parm.GetSize(); i++)
	{
		int ival = (int) m_parm[i];
		if (ival > max)
			max = ival;
		if (ival < min)
			min = ival;
	}

	float delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
	
	int max2 = int (m_upper / delta);
	int min2 = int (m_lower / delta);
	if (max2 > max)
		max = max2;
	if (min2 < min)
		min = min2;
	yWE = MulDiv(max-min+1, 10, 8);
	yWO = (max + min)/2;

	// update display
	m_spkXYp.SetYWExtOrg(yWE, yWO);
	DWORD yww = m_spkHist.GetHistMax();
	m_spkHist.SetXWExtOrg(yWE, yWO-yWE/2);
	m_spkHist.SetYWExtOrg(MulDiv((int)yww, 10, 8), 0);	
	
	// update edit controls
	m_mVMax = max* delta;
	m_mVMin = min* delta;
	UpdateLegends();
}


// -------------------------------------------------------------------------
// SelectSpike - display selected spike in red both in the form window and
// in the parameter window.
// if mode is parameters displayed from all, enventually display file containing
// this spike

void CSpikeSort1View::SelectSpike(int spikeno, BOOL bglobal)
{
	// convert global index to local
	int ispike_local = spikeno;		// always local index [/ current file]
	int ispike_global = spikeno;	// global or local according to bglobal

	if (spikeno >= 0 && bglobal)	// then spikeno is global and we need to find ispike_local for spkForm and spkBarView
	{
		int i_currentfile = GetDocument()->DBGetCurrentRecordPosition();
		int i_newfile = i_currentfile;
		ispike_local = GlobalIndextoLocal(spikeno, &i_newfile);

		if (i_newfile != i_currentfile)
		{
			GetDocument()->DBSetCurrentRecordPosition(i_newfile);
			UpdateFileParameters();
		}
	}

	// indexes are computed, display corresponding spike
	m_spkForm.SelectSpikeShape(ispike_local);
	m_spkBarView.SelectSpike(ispike_local);
	m_pSpkList->m_selspike = ispike_local;
	if (m_bMeasureDone)
		m_spkXYp.SelectSpike(ispike_global);

	long spkFirst = 0;
	long spkLast = 0;
	m_spikenoclass = -1;
	if (ispike_local >= 0) //&& ispike_local < m_pSpkList->GetTotalSpikes())
	{
		// get address of spike parms
		CSpikeElemt* pS = m_pSpkList->GetSpikeElemt(ispike_local);
		m_spikenoclass = pS->GetSpikeClass();
		spkFirst = pS->GetSpikeTime() - m_pSpkList->GetSpikePretrig();
		spkLast = spkFirst + m_pSpkList->GetSpikeLength();
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SPIKECLASS)->ShowWindow(SW_SHOW);		
	}
	else
	{
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SPIKECLASS)->ShowWindow(SW_HIDE);
	}
	m_spikeno = ispike_global;
	UpdateData(FALSE);
}

// ------------------------------------------------
// edit and transform spikes

void CSpikeSort1View::OnToolsEdittransformspikes() 
{
	CSpikeEditDlg dlg;							// dialog box
	dlg.m_yextent = m_spkForm.GetYWExtent();	// load display parameters
	dlg.m_yzero = m_spkForm.GetYWOrg();			// ordinates
	dlg.m_xextent = m_spkForm.GetXWExtent();	// and
	dlg.m_xzero = m_spkForm.GetXWOrg();			// abcissa
	dlg.m_spikeno = m_spikeno;					// load index of selected spike	
	dlg.m_parent = this;
	dlg.m_pSpkList = m_pSpkList;				// pass spike list

	// refresh pointer to data file because it not used elsewhere in the view
	CString docname = GetDocument()->DBGetCurrentDatFileName();
	BOOL bDocExists = FALSE;					// assume it is not found
	if (!docname.IsEmpty())						// no name ? test if any file exist..
	{
		CFileStatus rStatus;					// file status: time creation, ...
		bDocExists = CFile::GetStatus(docname, rStatus);
	}
	if (bDocExists) {
		BOOL flag = GetDocument()->OpenCurrentDataFile();
		ASSERT(flag);
	}
		
	dlg.m_dbDoc = GetDocument()->m_pDat;			// pass pointer to parent	

	// run dialog box
	dlg.DoModal();

	if (dlg.m_bchanged)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
		int currentlist = m_tabCtrl.GetCurSel();
		CSpikeList* pS = m_pSpkDoc->SetSpkListCurrent(currentlist);
		int nspikes = pS->GetTotalSpikes();
		for (int ispike=0; ispike< nspikes; ispike++)
			m_class[ispike] = pS->GetSpikeClass(ispike);
	}

	if (!dlg.m_bartefact && m_spikeno != dlg.m_spikeno)
		SelectSpike(dlg.m_spikeno, TRUE);

	UpdateLegends();			    			// update spkform & legends
}


///////////////////////////////////////////////////////////////////////
// select & display parameters from all files

// -------------------------------------------------------------------------
// OnSelectAllFiles - called when IDC_CHECK1 is changed
// call OnMeasure

void CSpikeSort1View::OnSelectAllFiles() 
{
	m_bAllfiles = ((CButton*)GetDlgItem(IDC_CHECK1))->GetCheck();
	m_bMeasureDone=FALSE;
	m_spkBarView.DisplayAllFiles(m_bAllfiles, GetDocument());
	m_spkForm.DisplayAllFiles(m_bAllfiles, GetDocument());
	OnMeasure();
}


// -------------------------------------------------------------------------
// convert global index into local index (filenb & index within this file)

int CSpikeSort1View::GlobalIndextoLocal(int index_global, int* filenb)
{
	int nbfiles	= m_nspkperfile.GetSize();
	int index_local = 0;
	int i;
	for (i=0; i< nbfiles; i++)
	{
		if (index_global < m_nspkperfile[i])
		{
			break;
		}
	}
	i--;
	ASSERT(i>= 0);
	index_local = index_global - m_nspkperfile[i];
	*filenb = i;
	return index_local;
}

// -------------------------------------------------------------------------
// convert local index (spike "index" within "filenb") into global index
// filenb = 0 to (nbfiles -1)

int	 CSpikeSort1View::LocalIndextoGlobal(int filenb, int index_local)
{
	int index_global = index_local + m_nspkperfile[filenb];	
	return index_global;
}

//---------------------------------------------------------------------------
// save file if modified, update parameters if they have changed

void CSpikeSort1View::SaveCurrentFileParms()
{
	// save previous file if anything has changed
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		int currentlist = m_tabCtrl.GetCurSel();
		CSpikeList* pS = m_pSpkDoc->SetSpkListCurrent(currentlist);
		int nspikes1 = pS->GetTotalSpikes();
		m_pSpkDoc->OnSaveDocument(GetDocument()->DBGetCurrentSpkFileName(FALSE));
		int nspikes2 = pS->GetTotalSpikes();
//		ASSERT(nspikes2 >= nspikes1);

		//// check if artefacts were removed
		//if (nspikes1 != nspikes2)
		//{
		//	int currentfile = GetDocument()->DBGetCurrentRecordPosition();
		//	int nspikes = nspikes1;
		//	if (nspikes2 > nspikes)
		//		nspikes = nspikes2;
		//	//m_nspkperfile[currentfile] = nspikes2;
		//	int globalindex = LocalIndextoGlobal(currentfile, 0) + nspikes1-1;
		//	int nremoved=0;
		//	for (int i=0; i<nspikes1; i++, globalindex--)
		//	{
		//		if (m_class[globalindex] >= 0)
		//			continue;
		//		nremoved++;
		//		m_class.RemoveAt(globalindex);
		//		m_iitime.RemoveAt(globalindex);				
		//		m_parm.RemoveAt(globalindex);
		//	}
		//	ASSERT(nspikes1 == (nspikes2 + nremoved));
		//}

		// save modifications into the database
		GetDocument()->Setnbspikes(m_pSpkList->GetTotalSpikes());
		GetDocument()->Setnbspikeclasses(m_pSpkList->GetNbclasses());

		// change flag is button is checked 
		if (((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck())
		{
			int flag = GetDocument()->DBGetCurrentRecordFlag();
			flag++;
			GetDocument()->DBSetCurrentRecordFlag(flag);
		}
	}
}

//---------------------------------------------------------------------------

void CSpikeSort1View::OnToolsAlignspikes() 
{
	// get source data
	BOOL bDocExist = FALSE;
	CString docname = m_pSpkDoc->GetSourceFilename();
	if (!docname.IsEmpty())
	{
		CFileStatus rStatus;	// file status: time creation, ...
		bDocExist = CFile::GetStatus(docname, rStatus);
	}
	if (!bDocExist)
	{
		AfxMessageBox(_T("Source data not found - operation aborted"));
		return;
	}
	
	// first prepare array with SUM

	int spikelen = m_pSpkList->GetSpikeLength();	// length of one spike
	int totalspikes = m_pSpkList->GetTotalSpikes();	// total nb of spikes /record
	double* const pSUM0 = new(double [spikelen]);	// array with results / SUMy
	double* const pCxy0 = new(double [spikelen]);	// temp array to store correlat 
	short* const pMean0 = new(short [spikelen]);	// mean (template) / at scale
	short* const pDummy0 = new(short [spikelen]);	// results of correlation / at scale

	// init pSUM with zeros
	double* pSUM = pSUM0;
	for (int i=0; i < spikelen; i++, pSUM++)
		*pSUM = 0;

	// compute mean
	int nbspk_selclass=0;
	short* pSpk;
	for (int ispk=0; ispk< totalspikes; ispk++)
	{
		if (m_pSpkList->GetSpikeClass(ispk) != (WORD) m_sourceclass)
			continue;
		nbspk_selclass++;
		pSpk = m_pSpkList->GetpSpikeData(ispk);
		pSUM = pSUM0;
		for (int i = 0; i< spikelen; i++, pSpk++, pSUM++)
			*pSUM += *pSpk;
	}

	// build avg and avg autocorrelation, tehn display
	short* pMean = pMean0;
	pSUM = pSUM0;

	for (int i=0; i< spikelen; i++, pMean++, pSUM++)
		*pMean = (short) (*pSUM / nbspk_selclass);

	m_spkForm.DisplayExData(pMean0);

	// for each spike, compute correlation and take max value correlation
	int kstart = m_psC->ileft;		// start of template match 
	int kend   = m_psC->iright;		// end of template match
	if (kend <= kstart)
		return;
	int j0 = kstart - (kend-kstart)/2;	// start time lag
	int j1 = kend - (kend-kstart)/2 +1;	// last lag

	// compute autocorrelation for mean;
	double Cxx_mean=0;
	pMean = pMean0 + kstart;
	for (int i=kstart; i< kend; i++, pMean++)
	{
		int val = *pMean;		
		Cxx_mean += val*val;
	}

	// get parameters from document
	CAcqDataDoc* pDatDoc = GetDocument()->m_pDat;
	pDatDoc->OnOpenDocument(docname);
	int const docChan = m_pSpkList->GetextractChan();		// source channel	
	short const nchans = pDatDoc->GetpWaveFormat()->scan_count;	// number of data chans / source buffer
	int const method = m_pSpkList->GetextractTransform();
	int const pretrig = m_pSpkList->GetSpikePretrig();
	short const offset = (method>0) ? 1 : nchans;			// offset between points / detection
	short const nspan = pDatDoc->GetTransfDataSpan(method);	// nb pts to read before transf

	// pre-load data
	long iitime0 = m_pSpkList->GetSpikeTime(0); //-pretrig;
	long lRWFirst0 = iitime0 - spikelen;
	long lRWLast0 = iitime0 + spikelen;
	if (!pDatDoc->LoadRawData(&lRWFirst0, &lRWLast0, nspan))
		return;										// exit if error reported
	short* pData = pDatDoc->LoadTransfData(lRWFirst0, lRWLast0, method,	docChan);

	// loop over all spikes now
	int spkpretrig = m_pSpkList->GetSpikePretrig();
	for (int ispk = 0; ispk < totalspikes; ispk++)
	{
		// exclude spikes that do not fall within time limits
		if (m_pSpkList->GetSpikeClass(ispk) != (WORD) m_sourceclass)
			continue;

		//iitime0 -= pretrig;						// offset beginning of spike

		// make sure that source data are loaded and get pointer to it (pData)
		long lRWFirst = iitime0 - spikelen;		// first point (eventually) needed
		long lRWLast = iitime0 + spikelen;		// last pt needed
		long iitime0 = m_pSpkList->GetSpikeTime(ispk);
		if (iitime0 > m_lLast || iitime0 < m_lFirst)
			continue;
		if (!pDatDoc->LoadRawData(&lRWFirst, &lRWLast, nspan))
			break;				// exit if error reported

		// load data only if necessary
		if (lRWFirst != lRWFirst0 || lRWLast != lRWLast0)
		{
			pData = pDatDoc->LoadTransfData(lRWFirst, lRWLast, method, docChan);
			lRWLast0 = lRWLast;				// index las pt within pData
			lRWFirst0 = lRWFirst;			// index first pt within pData
		}
		
		// pointer to first point of spike
		short* pDataSpike0 = pData +(iitime0 -spkpretrig -lRWFirst)*offset;

		// for spike ispk: loop over spikelen time lags centered over interval center

		// compute autocorrelation & cross correlation at first time lag
		double* pCxy_lag = pCxy0;			// pointer to array with correl coeffs
		*pCxy_lag =0;						// init cross corr
		double Cxx_spike = 0;				// autocorrelation
		short* pMean_k = pMean0+kstart;		// pointer to template

		short* pdat_k0 = pDataSpike0 + j0*offset;// source data start

		// loop over all time lag requested

		for (int j= j0; j < j1; j++, pCxy_lag++, pdat_k0 += offset)
		{
			*pCxy_lag =0;

			// add cross product for each point: data * meanlong iitime
			pMean_k = pMean0+kstart;		// first point / template
			short* pdat_k = pdat_k0;		// first data point
			Cxx_spike = 0;					// autocorrelation

			// loop over all points of source data and mean

			for (int k=kstart; k<kend; k++, pMean_k++, pdat_k += offset)
			{
				int val = *pdat_k;
				*pCxy_lag += (*pMean_k * val);
				Cxx_spike += val*val;
			}

			*pCxy_lag /= (kend - kstart +1);
			*pCxy_lag = *pCxy_lag / sqrt(Cxx_mean * Cxx_spike);
		}

		// get max and min of this correlation
		double* pCxy = pCxy0;
		double Cxy_max = *pCxy;		// correlation max value
		int i_Cxy_max = 0;			// correlation max index

		for (int i=0; i < kend-kstart; pCxy++, i++)
		{
			if (Cxy_max < *pCxy)	// get max and max position
			{
				Cxy_max = *pCxy;
				i_Cxy_max = i;
			}
		}

		// offset spike so that max is at spikelen/2
		int jdecal = i_Cxy_max - (kend-kstart)/2;
		if (jdecal != 0)
		{
			pDataSpike0 = pData + (WORD) (iitime0 +jdecal -lRWFirst)*offset + docChan;
			m_pSpkList->SetSpikeData(ispk, pDataSpike0, nchans);
			m_pSpkDoc->SetModifiedFlag(TRUE);
			m_pSpkList->SetSpikeTime(ispk, iitime0 +spkpretrig);
		}

		// now offset spike vertically to align it with the mean
		pMean = pMean0 + kstart;
		pSpk = m_pSpkList->GetpSpikeData(ispk) + kstart;
		long lDiff = 0;		
		for (int i = kstart; i< kend; i++, pSpk++, pMean++)
			lDiff += (*pSpk - *pMean);
		lDiff /= (kend-kstart+1);
		pSpk = m_pSpkList->GetpSpikeData(ispk);
		short val = (short) lDiff;
		for (int i = 0; i< spikelen; i++, pSpk++)
			*pSpk -= val;
	}

	// exit : delete resources used locally
	if (m_pSpkDoc->IsModified())
	{
		m_spkForm.Invalidate();
		m_spkForm.DisplayExData(pMean0);
	}

	delete [] pSUM0;
	delete [] pMean0;
	delete [] pCxy0;
	delete [] pDummy0;

	OnMeasure();
}


//---------------------------------------------------------------------------  
//	parm1 = Amplitude max-min (t1:t2)
//	parm2 = Amplitude at offset t1 or t2
//	parm3 = Time max-min (t1:t2)

void CSpikeSort1View::MeasureSpkParm4(BOOL bkeepOldData, int ioption, int currentfile)
{	
	int nspikes = m_pSpkList->GetTotalSpikes(); //m_nspkperfile[currentfile];
	
	// index first spike within array
	int pindex = 0;
	if (m_bAllfiles)
		pindex = LocalIndextoGlobal(currentfile, 0);

	//	parm1 = sum (t1:t2)
	short* lpB;
	if (m_psC->ileft <0)
		m_psC->ileft = 0;
	if (m_psC->iright > m_pSpkList->GetSpikeLength()-1)
		m_psC->iright = m_pSpkList->GetSpikeLength()-1;
	if (m_psC->ileft > m_psC->iright)
		m_psC->ileft = m_psC->iright;

	int binzero = m_pSpkList->GetAcqBinzero();
	int npoints = m_psC->iright - m_psC->ileft;
	if (!bkeepOldData)
	{
		// compute initial max and min
		lpB = m_pSpkList->GetpSpikeData(0)+m_psC->ileft;
		int sum = 0;
		switch (ioption)
		{
		case 1:
		default:
			{
			for (int i = m_psC->ileft; i <= m_psC->iright; i++, lpB++)
				sum += abs(*lpB - binzero);
			}
			sum /= npoints;
			break;
		case 2:
			{
			int oldval = *lpB;
			lpB++;
			for (int i = m_psC->ileft+1; i <= m_psC->iright; i++, lpB++)
			{
				sum += abs(*lpB - oldval);
				oldval = *lpB;
			}
			//sum /= npoints;
			}
			break;
		case 3:
			{
			int oldval;
			sum = abs(*(lpB+1) - *(lpB)) 
							+ abs(*(lpB+2) - *(lpB+1)) 
							+ abs(*(lpB+3) - *(lpB+2)) 
							//+ abs(*(lpB+4) - *(lpB+3)) 
							;
			lpB++;
			for (int i = m_psC->ileft+1; i <= m_psC->iright; i++, lpB++)
			{
				oldval = abs(*(lpB+1) - *(lpB)) 
							+ abs(*(lpB+2) - *(lpB+1)) 
							+ abs(*(lpB+3) - *(lpB+2)) 
							//+ abs(*(lpB+4) - *(lpB+3))
							;
				if (oldval > sum)
					sum = oldval;
			}
			}
			break;
		}
		m_parmmax = sum;
		m_parmmin = sum;
	}

	// loop over all spikes of the list
	switch (ioption)
	{
	case 1:
	default:
		{
		for (int ispike = 0; ispike < nspikes; ispike++, pindex++)
		{
			m_iitime[pindex] = m_pSpkList->GetSpikeTime(ispike);
			m_class[pindex] = m_pSpkList->GetSpikeClass(ispike);
			lpB = m_pSpkList->GetpSpikeData(ispike)+m_psC->ileft;

			// loop over individual spike data to find total
			int sum=0;
			for (int i = m_psC->ileft; i <= m_psC->iright; i++, lpB++)
			{
				sum += abs(*lpB - binzero);
			}
			sum /= npoints;

			// store measure into parm and update max min
			m_parm[pindex] = sum;	// store parm value
			if (m_parmmax < sum) m_parmmax = sum;
			if (m_parmmin > sum) m_parmmin = sum;
		}
		}
		break;
	case 2:
		{
			for (int ispike = 0; ispike < nspikes; ispike++, pindex++)
			{
				m_iitime[pindex] = m_pSpkList->GetSpikeTime(ispike);
				m_class[pindex] = m_pSpkList->GetSpikeClass(ispike);
				lpB = m_pSpkList->GetpSpikeData(ispike)+m_psC->ileft;

				// loop over individual spike data to find total
				int sum=0;
				int oldval = *lpB;
				lpB++;
				for (int i = m_psC->ileft+1; i <= m_psC->iright; i++, lpB++)
				{
					sum += abs(*lpB - oldval);
					oldval = *lpB;
				}
				//sum /= npoints;

				// store measure into parm and update max min
				m_parm[pindex] = sum;	// store parm value
				if (m_parmmax < sum) m_parmmax = sum;
				if (m_parmmin > sum) m_parmmin = sum;
			}
		}
		break;

	case 3:
		{
			for (int ispike = 0; ispike < nspikes; ispike++, pindex++)
			{
				m_iitime[pindex] = m_pSpkList->GetSpikeTime(ispike);
				m_class[pindex] = m_pSpkList->GetSpikeClass(ispike);
				lpB = m_pSpkList->GetpSpikeData(ispike)+m_psC->ileft;

				// loop over individual spike data to find total			
				int oldval;
				int sum = abs(*(lpB+1) - *(lpB)) 
								+ abs(*(lpB+2) - *(lpB+1)) 
								+ abs(*(lpB+3) - *(lpB+2)) 
								//+ abs(*(lpB+4) - *(lpB+3)) 
								;
				lpB++;
				for (int i = m_psC->ileft+1; i <= m_psC->iright; i++, lpB++)
				{
					oldval = abs(*(lpB+1) - *(lpB)) 
								+ abs(*(lpB+2) - *(lpB+1)) 
								+ abs(*(lpB+3) - *(lpB+2)) 
								//+ abs(*(lpB+4) - *(lpB+3))
								;
					if (oldval > sum)
						sum = oldval;
				}
				// store measure into parm and update max min
				m_parm[pindex] = sum;	// store parm value
				if (m_parmmax < sum) m_parmmax = sum;
				if (m_parmmin > sum) m_parmmin = sum;
			}
		}
		break;
	}

	// exit
	m_bvalidextrema = TRUE;
	m_bMeasureDone=TRUE;
}


void CSpikeSort1View::OnEnChangetimeFirst() 
{
	if (!mm_timeFirst.m_bEntryDone)
		return;
	float timeFirst = m_timeFirst;

	switch (mm_timeFirst.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE); timeFirst = m_timeFirst; break;
	case VK_UP:
	case VK_PRIOR:	timeFirst++; break;
	case VK_DOWN:
	case VK_NEXT: 	timeFirst--;	break;
	}
	
	// check boundaries
	if (timeFirst <0.f)
		timeFirst = 0.f;
	if (timeFirst >= m_timeLast)
		timeFirst = 0.f;
	
	// change display if necessary
	mm_timeFirst.m_bEntryDone=FALSE;	// clear flag
	mm_timeFirst.m_nChar=0;			// empty buffer
	mm_timeFirst.SetSel(0, -1);		// select all text
	m_timeFirst = timeFirst;
	m_lFirst = long(m_timeFirst * m_pSpkList->GetAcqSampRate());
	UpdateLegends();
}

void CSpikeSort1View::OnEnChangetimeLast() 
{
	if (!mm_timeLast.m_bEntryDone)
		return;
	float timeLast = m_timeLast;

	switch (mm_timeLast.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE); timeLast = m_timeLast; break;
	case VK_UP:
	case VK_PRIOR:	timeLast++; break;
	case VK_DOWN:
	case VK_NEXT: 	timeLast--;	break;
	}
	
	// check boundaries
	if (timeLast <= m_timeFirst)
		m_lLast = long ((m_pSpkDoc->GetAcqSize()-1) / m_pSpkList->GetAcqSampRate());
	//if (timeLast >= m_timeLast)
	//	timeLast = m_timeLast;
	
	// change display if necessary
	mm_timeLast.m_bEntryDone=FALSE;	// clear flag
	mm_timeLast.m_nChar=0;			// empty buffer
	mm_timeLast.SetSel(0, -1);		// select all text
	m_timeLast = timeLast;
	m_lLast = long(m_timeLast * m_pSpkList->GetAcqSampRate());
	UpdateLegends();
}


void CSpikeSort1View::OnEnChangemVMin() 
{
	if (!mm_mVMin.m_bEntryDone)
		return;
	
	float mVMin = m_mVMin;
	switch (mm_mVMin.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); 
		mVMin = m_mVMin; break;
	case VK_UP:
	case VK_PRIOR:	mVMin++; break;
	case VK_DOWN:
	case VK_NEXT: 	mVMin--;	break;
	}
	
	// check boundaries
	if (mVMin >= m_mVMax)
		mVMin = m_mVMax -1.f;
	
	// change display if necessary
	mm_mVMin.m_bEntryDone=FALSE;	// clear flag
	mm_mVMin.m_nChar=0;			// empty buffer
	mm_mVMin.SetSel(0, -1);		// select all text
	m_mVMin = mVMin;

	UpdateGain();
	UpdateLegends();
}

void CSpikeSort1View::OnEnChangemVMax() 
{
	if (!mm_mVMax.m_bEntryDone)
		return;

	float mVMax = m_mVMax;
	switch (mm_mVMax.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); 
		mVMax = m_mVMax; break;
	case VK_UP:
	case VK_PRIOR:	mVMax++; break;
	case VK_DOWN:
	case VK_NEXT: 	mVMax--;	break;
	}
	
	// check boundaries
	if (mVMax <= m_mVMin)
		mVMax = m_mVMin +1.f;
	
	// change display if necessary
	mm_mVMax.m_bEntryDone=FALSE;	// clear flag
	mm_mVMax.m_nChar=0;			// empty buffer
	mm_mVMax.SetSel(0, -1);		// select all text
	m_mVMax = mVMax;

	UpdateGain();
	UpdateLegends();
}

void CSpikeSort1View::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
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
		UpdateLegends();
	}
	else
		UpdateScrollBar();
}

void CSpikeSort1View::UpdateScrollBar()
{
	GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_SHOW);		
	m_scrollFilePos_infos.fMask = SIF_ALL;
	m_scrollFilePos_infos.nMin = 0;
	m_scrollFilePos_infos.nMax = m_pSpkDoc->GetAcqSize()-1;
	m_scrollFilePos_infos.nPos = m_lFirst;
	m_scrollFilePos_infos.nPage = m_lLast-m_lFirst;
	((CScrollBar*) GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&m_scrollFilePos_infos);
}

void CSpikeSort1View::OnEnChangeNOspike() 
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
		if (m_spikeno >= 0)
		{
			// test if spike visible in the current time interval
			CSpikeElemt* pS = m_pSpkList->GetSpikeElemt(m_spikeno);
			long spkFirst = pS->GetSpikeTime() - m_pSpkList->GetSpikePretrig();
			long spkLast = spkFirst + m_pSpkList->GetSpikeLength();

			if (spkFirst < m_lFirst || spkLast > m_lLast)
			{
				long lspan = (m_lLast - m_lFirst)/2;
				long lcenter = (spkLast + spkFirst)/2;
				m_lFirst = lcenter - lspan;
				m_lLast  = lcenter + lspan;
				UpdateLegends();
			}
		}
	}
	SelectSpike(m_spikeno, m_bAllfiles);
}


void CSpikeSort1View::OnEnChangeSpikenoclass() 
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
		m_pSpkDoc->SetModifiedFlag(TRUE);
		int currentlist = m_tabCtrl.GetCurSel();
		CSpikeList* pS = m_pSpkDoc->SetSpkListCurrent(currentlist);
		pS->SetSpikeClass(m_spikeno, m_spikenoclass);
		m_class[m_spikeno] = m_spikenoclass;
		UpdateLegends();
	}
}

void CSpikeSort1View::SelectSpkList(int icursel)
{
	m_pSpkList = m_pSpkDoc->SetSpkListCurrent(icursel);
	GetDocument()->SetcurrentSpkListIndex(icursel);
	ASSERT(m_pSpkList != NULL);

	OnMeasure();

	// update source data: change data channel and update display
	m_spkHist.SetSpkList(m_pSpkList);		// source data
	m_spkForm.SetSpkList(m_pSpkList);		// all spikes in displayspikes
	m_spkBarView.SetSpkList(m_pSpkList);	// bars with spike height

	m_spkHist.Invalidate();
	m_spkXYp.Invalidate();
	m_spkForm.Invalidate();
	m_spkBarView.Invalidate();
}


void CSpikeSort1View::OnEnChangeEditleft2()
{
	if (!mm_txyleft.m_bEntryDone)
		return;

	float left = m_txyleft;
	float delta = m_tunit / m_pSpkList->GetAcqSampRate();
	switch (mm_txyleft.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); 	left = m_txyleft;	break;
	case VK_UP:
	case VK_PRIOR:	left+= delta; break;
	case VK_DOWN:
	case VK_NEXT: 	left-= delta; break;
	}

	// check boundaries
	if (left >= m_txyright)
		left = m_txyright-delta;

	// change display if necessary	
	mm_txyleft.m_bEntryDone=FALSE;		// clear flag
	mm_txyleft.m_nChar=0;				// empty buffer
	mm_txyleft.SetSel(0, -1);			// select all text
	m_txyleft = left;
	left =  m_txyleft /delta;
	int itleft = (int) left ;
	if (itleft != m_spkXYp.GetVTtagVal(m_ixyleft))
	{
		m_psC->ixyleft = itleft ;
		m_spkXYp.MoveVTtagtoVal(m_ixyleft, itleft);
	}
	UpdateData(FALSE);
}


void CSpikeSort1View::OnEnChangeEditright2()
{
	if (!mm_txyright.m_bEntryDone)
		return;

	float right = m_txyright;
	float delta = m_tunit / m_pSpkList->GetAcqSampRate();
	
	switch (mm_txyright.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	
		UpdateData(TRUE); 	right = m_txyright;	break;
	case VK_UP:
	case VK_PRIOR:	right+= delta; break;
	case VK_DOWN:
	case VK_NEXT: 	right-=delta; break;
	}

	// check boundaries
	if (right <= m_txyleft)
		right = m_txyleft+delta;

	// change display if necessary	
	mm_txyright.m_bEntryDone=FALSE;		// clear flag
	mm_txyright.m_nChar=0;				// empty buffer
	mm_txyright.SetSel(0, -1);			// select all text
	m_txyright = right;
	right = m_txyright/delta;
	int itright = (int) right;
	if (itright != m_spkXYp.GetVTtagVal(m_ixyright))
	{
		m_psC->ixyright = itright;	
		m_spkXYp.MoveVTtagtoVal(m_ixyright, itright);
	}
	UpdateData(FALSE);
}


void CSpikeSort1View::OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	m_tabCtrl.SetCurSel(icursel);
	SelectSpkList(icursel);
	*pResult = 0;
}


void CSpikeSort1View::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	int icursel = m_tabCtrl.GetCurSel();
	SelectSpkList(icursel);
	*pResult = 0;
}
