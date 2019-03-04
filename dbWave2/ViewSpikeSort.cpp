#include "StdAfx.h"
#include <math.h>
#include "dbWave.h"
#include "resource.h"
//#include "dbMainTable.h"
//#include "Cscale.h"
//#include "Spikebar.h"
//#include "Lineview.h"
//#include "Copyasdl.h"
//#include "ChildFrm.h"
#include "Editctrl.h"
#include "scopescr.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "spikeshape.h"
#include "spikehistp.h"
#include "spikexyp.h"
#include "Editspik.h"
#include "MainFrm.h"
#include "ProgDlg.h"
#include "ViewSpikeSort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CViewSpikeSort, CDaoRecordView)

// TODO limit size of measure array to nbspikes within currently selected spikelist

CViewSpikeSort::CViewSpikeSort()
	: CDaoRecordView(CViewSpikeSort::IDD)
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

CViewSpikeSort::~CViewSpikeSort()
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

BOOL CViewSpikeSort::PreCreateWindow(CREATESTRUCT &cs)
{
// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

void CViewSpikeSort::DoDataExchange(CDataExchange* pDX)
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
	DDX_Text(pDX, IDC_BINMV, m_mVbin);
	DDX_Check(pDX, IDC_CHECK1, m_bAllfiles);
	DDX_Text(pDX, IDC_NSPIKES, m_spikeno);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikenoclass);
	DDX_Text(pDX, IDC_EDITRIGHT2, m_txyright);
	DDX_Text(pDX, IDC_EDITLEFT2, m_txyleft);

	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

BEGIN_MESSAGE_MAP(CViewSpikeSort, CDaoRecordView)

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
	ON_COMMAND(ID_TOOLS_EDITSPIKES,			OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_ALIGNSPIKES,		OnToolsAlignspikes)
	ON_EN_CHANGE(IDC_EDIT2,					OnEnChangetimeFirst)
	ON_EN_CHANGE(IDC_EDIT3,					OnEnChangetimeLast)
	ON_EN_CHANGE(IDC_EDIT7,					OnEnChangemVMin)
	ON_EN_CHANGE(IDC_EDIT6,					OnEnChangemVMax)
	ON_EN_CHANGE(IDC_EDITLEFT2,				&CViewSpikeSort::OnEnChangeEditleft2)
	ON_EN_CHANGE(IDC_EDITRIGHT2,			&CViewSpikeSort::OnEnChangeEditright2)
	ON_EN_CHANGE(IDC_NSPIKES,				OnEnChangeNOspike)
	ON_BN_DOUBLECLICKED(IDC_DISPLAYPARM,	OnToolsEdittransformspikes)
	ON_EN_CHANGE(IDC_SPIKECLASS,			OnEnChangeSpikenoclass)
	ON_EN_CHANGE(IDC_BINMV, OnEnChangeNBins)
	
	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CViewSpikeSort::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CViewSpikeSort::OnTcnSelchangeTab1)
END_MESSAGE_MAP()

void CViewSpikeSort::OnInitialUpdate()
{
	// attach set
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	CDaoRecordView::OnInitialUpdate();
	m_psC= &(p_app->spkC);
	mdPM = &(p_app->options_viewdata);

	// assign controls to stretch
	m_stretch.AttachParent(this);

	m_stretch.newProp(IDC_HISTOGRAM,	SZEQ_XLEQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_EDIT7,		SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_BINMV,		SZEQ_XLEQ, SZEQ_YBEQ);

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
	m_stretch.newProp(IDC_STATIC12, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDITLEFT2, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDITRIGHT2, SZEQ_XLEQ, SZEQ_YBEQ);

	// subclass some controls
	VERIFY(yhistogram_wnd_.SubclassDlgItem(IDC_HISTOGRAM, this));
	VERIFY(xygraph_wnd_.SubclassDlgItem(IDC_DISPLAYPARM, this));
	VERIFY(spikeshape_wnd_.SubclassDlgItem(IDC_DISPLAYSPIKE, this));		
	VERIFY(spikebars_wnd_.SubclassDlgItem(IDC_DISPLAYBARS, this));

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
	VERIFY(mm_txyright.SubclassDlgItem(IDC_EDITRIGHT2, this));
	VERIFY(mm_txyleft.SubclassDlgItem(IDC_EDITLEFT2, this));
	VERIFY(mm_mVbin.SubclassDlgItem(IDC_BINMV, this));

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
	spikeshape_wnd_.DisplayAllFiles(false, GetDocument());
	xygraph_wnd_.DisplayAllFiles(false, GetDocument());
	spikebars_wnd_.DisplayAllFiles(false, GetDocument());

	spikeshape_wnd_.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);	
	xygraph_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	spikebars_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	yhistogram_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);

	*spikeshape_wnd_.GetScopeParameters() = mdPM->spksort1spk;
	*xygraph_wnd_.GetScopeParameters() = mdPM->spksort1parms;
	*yhistogram_wnd_.GetScopeParameters() = mdPM->spksort1hist;
	*spikebars_wnd_.GetScopeParameters() = mdPM->spksort1bars;

	// set bincrflagonsave
	((CButton*)(GetDlgItem(IDC_INCREMENTFLAG)))->SetCheck(p_app->options_viewspikes.bincrflagonsave);

	// display tag lines at proper places
	m_spkformtagleft	= spikeshape_wnd_.AddVTtag(m_psC->ileft);	// first VTtag
	m_spkformtagright	= spikeshape_wnd_.AddVTtag(m_psC->iright);	// second VTtag
	m_itagup			= xygraph_wnd_.AddHZtag(m_psC->iupper, 0);	// first HZ tag
	m_itaglow			= xygraph_wnd_.AddHZtag(m_psC->ilower, 0);	// second HZ tag
	m_spkhistupper		= yhistogram_wnd_.AddVTtag(m_psC->iupper);
	m_spkhistlower		= yhistogram_wnd_.AddVTtag(m_psC->ilower);

	UpdateFileParameters();
	if (m_pSpkList != nullptr) {
		m_delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
		m_lower = m_psC->ilower*m_delta;
		m_upper = m_psC->iupper*m_delta;
		UpdateData(false);
	}

	ActivateMode4();
	m_binit = TRUE;
}

void CViewSpikeSort::ActivateMode4()
{
	auto n_cmd_show = SW_HIDE;
	if (m_psC->iparameter == 4)
	{
		n_cmd_show = SW_SHOW;
		if (xygraph_wnd_.GetNVTtags() <1)
		{
			m_ixyright = xygraph_wnd_.AddVTtag(m_psC->ixyright);
			m_ixyleft = xygraph_wnd_.AddVTtag(m_psC->ixyleft);
			const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
			m_txyright = static_cast<float>(m_psC->ixyright) /delta ;
			m_txyleft = static_cast<float>(m_psC->ixyleft) /delta ;
		}
		xygraph_wnd_.SetNxScaleCells(2, 0, 0);
		xygraph_wnd_.GetScopeParameters()->crScopeGrid= RGB(128,   128, 128);
		
		if (m_pSpkList != nullptr)
		{
			const auto spikelen_ms = (m_pSpkList->GetSpikeLength()* m_tunit) / m_pSpkList->GetAcqSampRate() ;
			CString cs_dummy;
			cs_dummy.Format(_T("%0.1f ms"), spikelen_ms);
			GetDlgItem(IDC_STATICRIGHT)->SetWindowText(cs_dummy);
			cs_dummy.Format(_T("%0.1f ms"), -spikelen_ms);
			GetDlgItem(IDC_STATICLEFT)->SetWindowText(cs_dummy);
		}
	}
	else
	{
		xygraph_wnd_.DelAllVTtags();
		xygraph_wnd_.SetNxScaleCells(0, 0, 0);
	}
	GetDlgItem(IDC_STATICRIGHT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATICLEFT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC12)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITRIGHT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITLEFT2)->ShowWindow(n_cmd_show);
	xygraph_wnd_.Invalidate();
}

void CViewSpikeSort::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		auto* p_mainframe = (CMainFrame*) AfxGetMainWnd();
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		SaveCurrentFileParms();
		auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		if (p_app->m_psort1spikesMemFile == nullptr)
		{
			p_app->m_psort1spikesMemFile = new CMemFile;
			ASSERT(p_app->m_psort1spikesMemFile != NULL);
		}
		CArchive ar(p_app->m_psort1spikesMemFile, CArchive::store);
		p_app->m_psort1spikesMemFile->SeekToBegin();
		ar.Close();	
		p_app->options_viewspikes.bincrflagonsave = ((CButton*)(GetDlgItem(IDC_INCREMENTFLAG)))->GetCheck();
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CViewSpikeSort::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_binit)
	{
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
}

BOOL CViewSpikeSort::OnMove(UINT nIDMoveCommand) 
{
	SaveCurrentFileParms();
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	auto p_doc = GetDocument();
	if (p_doc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		return false;
	}
	p_doc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CViewSpikeSort::OnDestroy() 
{
	CDaoRecordView::OnDestroy();
}

#ifdef _DEBUG
void CViewSpikeSort::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewSpikeSort::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewSpikeSort::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)));
	return (CdbWaveDoc*) m_pDocument;
}
#endif //_DEBUG

CDaoRecordset* CViewSpikeSort::OnGetRecordset()
{
	return GetDocument()->DBGetRecordset();
}

void CViewSpikeSort::UpdateFileParameters()
{
	// reset parms ? flag = single file or file list has changed
	if (  !m_bAllfiles)		
	{
		yhistogram_wnd_.RemoveHistData();
	}

	// change pointer to select new spike list & test if one spike is selected
	const BOOL bfirstupdate = (m_pSpkDoc == nullptr);
	auto pdb_doc = GetDocument();
	m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
	if (m_pSpkDoc == nullptr)
		return;

	// reset tab control
	m_tabCtrl.DeleteAllItems();
	auto j = 0;
	for (auto i = 0; i < m_pSpkDoc->GetSpkListSize(); i++)
	{
		const auto spike_list = m_pSpkDoc->SetSpkListCurrent(i);
		if (spike_list->GetdetectWhat() == 0) 
		{
			CString cs;
			cs.Format(_T("#%i %s"), i, static_cast<LPCTSTR>(spike_list->GetComment()));
			m_tabCtrl.InsertItem(j, cs);
			j++;
		}
	}
	m_pSpkList = m_pSpkDoc->SetSpkListCurrent(pdb_doc->GetcurrentSpkListIndex());
	m_tabCtrl.SetCurSel(pdb_doc->GetcurrentSpkListIndex());

	// spike and classes
	auto spikeno = m_pSpkList->m_selspike;
	if (spikeno > m_pSpkList->GetTotalSpikes()-1 || spikeno < 0)
		spikeno = -1;
	else	
	{	// set source class to the class of the selected spike
		m_sourceclass = m_pSpkList->GetSpikeClass(spikeno);		
		m_psC->sourceclass = m_sourceclass;
	}
	ASSERT(m_sourceclass < 32768);

	// display source spikes
	spikeshape_wnd_.SetSourceData(m_pSpkList, GetDocument());
	spikebars_wnd_.SetSourceData(m_pSpkList, GetDocument());

	if (m_psC->ileft==0 && m_psC->iright==0)
	{
		m_psC->ileft = m_pSpkList->GetSpikePretrig();
		m_psC->iright = m_psC->ileft +  m_pSpkList->GetSpikeRefractory();
	}

	// refresh sorting parameters and data file properties
	m_t1 = (m_psC->ileft*m_tunit)/m_pSpkList->GetAcqSampRate();
	m_t2 = (m_psC->iright*m_tunit)/m_pSpkList->GetAcqSampRate();
	spikeshape_wnd_.SetVTtagVal(m_spkformtagleft, m_psC->ileft);
	spikeshape_wnd_.SetVTtagVal(m_spkformtagright, m_psC->iright);
	
	spikeshape_wnd_.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);
	xygraph_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	spikebars_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	yhistogram_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);

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
		m_lFirst = static_cast<long >(m_timeFirst * m_pSpkList->GetAcqSampRate());
		m_lLast = static_cast<long>(m_timeLast * m_pSpkList->GetAcqSampRate());
	}

	// display & compute parameters
	if (!m_bAllfiles || !m_bMeasureDone)
	{
		xygraph_wnd_.SetSourceData(m_pSpkList, GetDocument());
		if (m_psC->iparameter != 4)
		{
			xygraph_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
			if(xygraph_wnd_.GetNVTtags() >0)
			{
				xygraph_wnd_.DelAllVTtags();
				xygraph_wnd_.Invalidate();
			}
		}
		else
		{
			xygraph_wnd_.SetTimeIntervals(- m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
			if (xygraph_wnd_.GetNVTtags() <1)
			{
				m_ixyright = xygraph_wnd_.AddVTtag(m_psC->ixyright);	
				m_ixyleft = xygraph_wnd_.AddVTtag(m_psC->ixyleft);
				const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_txyright =static_cast<float>(m_psC->ixyright) /delta ;
				m_txyleft = static_cast<float>(m_psC->ixyleft) /delta ;
				xygraph_wnd_.Invalidate();
			}
		}
		// update text , display and compute histogram
		m_bMeasureDone=FALSE;		// no parameters yet
		OnMeasure();
	}
	spikeshape_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
	spikebars_wnd_.SetTimeIntervals(m_lFirst, m_lLast);

	spikeshape_wnd_.Invalidate();
	spikebars_wnd_.Invalidate();
	UpdateScrollBar();
	SelectSpikeFromCurrentList(spikeno);
}

void CViewSpikeSort::UpdateLegends()
{
	// update text abcissa and horizontal scroll position
	m_pSpkList->m_lFirstSL = m_lFirst;
	m_pSpkList->m_lLastSL = m_lLast;	
	m_timeFirst = m_lFirst / m_pSpkList->GetAcqSampRate();
	m_timeLast  = m_lLast  / m_pSpkList->GetAcqSampRate();
	UpdateScrollBar();

	if (m_psC->iparameter != 4)
		xygraph_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
	else
		xygraph_wnd_.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
	spikeshape_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
	spikebars_wnd_.SetTimeIntervals(m_lFirst, m_lLast);

	xygraph_wnd_.Invalidate();
	spikeshape_wnd_.Invalidate();
	spikebars_wnd_.Invalidate();
	yhistogram_wnd_.Invalidate();

	UpdateData(FALSE);	// copy view object to controls	
}

void CViewSpikeSort::OnSize(UINT nType, int cx, int cy)
{
	if (m_binit)
	{
		switch (nType)
		{
		case SIZE_MAXIMIZED:
		case SIZE_RESTORED:
			if (cx > 0 && cy > 0) 
				m_stretch.ResizeControls(nType, cx, cy);
			break;
		default:
			break;
		}
	}
	CDaoRecordView::OnSize(nType, cx, cy);
}

void CViewSpikeSort::OnSort()
{
	// set file indexes - assume only one file selected
	auto pdb_doc = GetDocument();
	const int currentfile = pdb_doc->DBGetCurrentRecordPosition(); 
	auto firstfile = currentfile;
	auto lastfile = firstfile;
	const auto nfiles = pdb_doc->DBGetNRecords();
	const auto currentlist = m_pSpkDoc->GetSpkListCurrentIndex();
	
	// change indexes if ALL files selected
	CProgressDlg* pdlg = nullptr;
	auto istep = 0;
	CString cscomment;
	if (m_bAllfiles)
	{
		firstfile = 0;						// index first file
		lastfile = pdb_doc->DBGetNRecords() -1;// index last file
		pdlg = new CProgressDlg;
		pdlg->Create();
		pdlg->SetStep (1);
	}

	// loop over all selected files (or only one file currently selected)
	pdb_doc->DBSetCurrentRecordPosition(firstfile);

	for (auto ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// load spike file
		BOOL flagchanged;
		pdb_doc->DBSetCurrentRecordPosition(ifile);
		m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		if (m_pSpkDoc == nullptr)
			continue;
		
		// update screen if multi-file requested 
		if (m_bAllfiles)
		{
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

		// load spike list
		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist);
		if ((m_pSpkList == nullptr) || (m_pSpkList->GetSpikeLength() == 0))
			continue;

		// loop over all spikes of the list and compare to a single parameter
		const CSize limits1(m_psC->ilower, m_psC->iupper);
		const CSize fromclass_toclass(m_sourceclass, m_destinationclass);
		const CSize timewindow(m_lFirst, m_lLast);
		// sort on 1 parameter
		if (m_psC->iparameter != 4)
		{
			flagchanged = m_pSpkList->SortSpikeWithY1(fromclass_toclass, timewindow, limits1);
		}
		// sort on 2 parameters
		else 
		{
			const CSize limits2(m_psC->ixyleft, m_psC->ixyright);
			flagchanged = m_pSpkList->SortSpikeWithY1AndY2(fromclass_toclass, timewindow, limits1, limits2);
		}

		if (flagchanged)
		{
			m_pSpkDoc->OnSaveDocument(pdb_doc->DBGetCurrentSpkFileName(FALSE));
			pdb_doc->Setnbspikes(m_pSpkList->GetTotalSpikes());
		}
	}

	// end of loop, select current file again if necessary
	if (m_bAllfiles)
	{
		delete pdlg;
		pdb_doc->DBSetCurrentRecordPosition(currentfile);
		m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		m_pSpkList = m_pSpkDoc->GetSpkListCurrent();
	}

	// refresh data windows
	xygraph_wnd_.Invalidate();
	spikeshape_wnd_.Invalidate();
	spikebars_wnd_.Invalidate();
	BuildHistogram();
	yhistogram_wnd_.Invalidate();
	SelectSpikeFromCurrentList(-1);
	m_pSpkDoc->SetModifiedFlag(TRUE); // set flag: document has changed
}

LRESULT CViewSpikeSort::OnMyMessage(WPARAM code, LPARAM lParam)
{
	short threshold = LOWORD(lParam);
	//short wParam = HIWORD(lParam);

	switch (code)
	{
	case HINT_SETMOUSECURSOR:	// ------------- change mouse cursor (all 3 items)	
		if (threshold >CURSOR_ZOOM)		// clip cursor shape to max
			threshold = 0;
		SetViewMouseCursor(threshold);	// change cursor val in the other button
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(threshold, 0));
		break;	

	case HINT_CHANGEHZLIMITS:	// -------------  abcissa have changed
		if (m_psC->iparameter != 4)
		{
			m_lFirst = xygraph_wnd_.GetTimeFirst();
			m_lLast = xygraph_wnd_.GetTimeLast();
		}
		else
		{
			m_lFirst = spikebars_wnd_.GetTimeFirst();
			m_lLast = spikebars_wnd_.GetTimeLast();
		}
		UpdateLegends();
		break;

	case HINT_HITSPIKE:			// -------------  spike is selected or deselected
		{
			if (m_pSpkList->GetSpikeFlagArrayCount() > 0)
				UnflagAllSpikes();
			auto spikeno=0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spikeno = spikeshape_wnd_.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spikeno = spikebars_wnd_.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM) 
				spikeno = xygraph_wnd_.GetHitSpike();
	
			SelectSpikeFromCurrentList (spikeno);
		}
		break;

	case HINT_SELECTSPIKES:
		xygraph_wnd_.Invalidate();
		spikeshape_wnd_.Invalidate();
		spikebars_wnd_.Invalidate();
		break;

	case HINT_DBLCLKSEL:
		{
			auto spikeno = 0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spikeno = spikeshape_wnd_.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spikeno = spikebars_wnd_.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spikeno = xygraph_wnd_.GetHitSpike();  // if m_bAllFiles, spikeno is global, otherwise it comes from a single file...
			SelectSpikeFromCurrentList(spikeno);
			OnToolsEdittransformspikes();
		}		
		break;

	//case HINT_MOVEVERTTAG: // -------------  vertical tag has moved lowp = tag index		
	case HINT_CHANGEVERTTAG: // -------------  vertical tag val has changed
		if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
		{
			if (threshold == m_spkformtagleft)		// first tag
			{
				m_psC->ileft = spikeshape_wnd_.GetVTtagVal(m_spkformtagleft);
				m_t1 = m_psC->ileft*m_tunit/m_pSpkList->GetAcqSampRate();
				mm_t1.m_bEntryDone = TRUE;
				OnEnChangeT1();
			}
			else if (threshold == m_spkformtagright)	// second tag
			{
				m_psC->iright = spikeshape_wnd_.GetVTtagVal(m_spkformtagright);
				m_t2 = m_psC->iright*m_tunit/m_pSpkList->GetAcqSampRate();
				mm_t2.m_bEntryDone = TRUE;
				OnEnChangeT2();
			}
		}
		else if (HIWORD(lParam) == IDC_HISTOGRAM)
		{
			if (threshold == m_spkhistlower)		// first tag
			{
				m_psC->ilower = yhistogram_wnd_.GetVTtagVal(m_spkhistlower);
				m_lower = m_psC->ilower*m_pSpkList->GetAcqVoltsperBin()*m_vunit;
				UpdateData(false);
			}
			else if (threshold == m_spkhistupper)	// second tag
			{
				m_psC->iupper = yhistogram_wnd_.GetVTtagVal(m_spkhistupper);	// load new value
				m_upper = m_psC->iupper*m_pSpkList->GetAcqVoltsperBin()*m_vunit;
				UpdateData(false);
			}
		}
		else if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (threshold == m_ixyright)
			{
				const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_psC->ixyright = xygraph_wnd_.GetVTtagVal(m_ixyright);
				m_txyright =static_cast<float>(m_psC->ixyright) /delta ;
				mm_txyright.m_bEntryDone = TRUE;
				OnEnChangeEditright2();
			}
			else if (threshold == m_ixyleft)
			{
				const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_psC->ixyleft = xygraph_wnd_.GetVTtagVal(m_ixyleft);
				m_txyleft = static_cast<float>(m_psC->ixyleft) / delta ;
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
				m_psC->ilower = xygraph_wnd_.GetHZtagVal(m_itaglow);	// load new value
				m_lower = m_psC->ilower*m_pSpkList->GetAcqVoltsperBin()*m_vunit;
				mm_lower.m_bEntryDone = TRUE;
				OnEnChangelower();
			}
			else if (threshold == m_itagup)	// second tag
			{
				m_psC->iupper = xygraph_wnd_.GetHZtagVal(m_itagup);	// load new value
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
		mdPM->spksort1spk = *spikeshape_wnd_.GetScopeParameters();
		mdPM->spksort1parms = *xygraph_wnd_.GetScopeParameters();
		mdPM->spksort1hist = *yhistogram_wnd_.GetScopeParameters();
		mdPM->spksort1bars = *spikebars_wnd_.GetScopeParameters();
		break;

	default:
		break;		
	}
	return 0L;
}

void CViewSpikeSort::UnflagAllSpikes()
{
	if (m_bAllfiles) {
		auto pdb_doc = GetDocument();
		for (auto ifile = 0; ifile < pdb_doc->DBGetNRecords(); ifile++)
		{
			pdb_doc->DBSetCurrentRecordPosition(ifile);
			m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();

			for (auto j = 0; j < m_pSpkDoc->GetSpkListSize(); j++) {
				m_pSpkList = m_pSpkDoc->SetSpkListCurrent(j);
				m_pSpkList->RemoveAllSpikeFlags();
			}
		}
	}
	else
		m_pSpkList->RemoveAllSpikeFlags();
	xygraph_wnd_.Invalidate();
	spikeshape_wnd_.Invalidate();
	spikebars_wnd_.Invalidate();
}

void CViewSpikeSort::OnMeasure() 
{
	// set file indexes - assume only one file selected
	auto pdb_doc = GetDocument();
	int currentfile = pdb_doc->DBGetCurrentRecordPosition(); // index current file	
	const int nfiles		= pdb_doc->DBGetNRecords();
	const auto currentlist = m_pSpkDoc->GetSpkListCurrentIndex();
	int firstfile = currentfile;
	int lastfile = currentfile;
	// change size of arrays and prepare temporary dialog
	SelectSpikeFromCurrentList(-1);
	if (m_bAllfiles)
	{
		firstfile = 0;						// index first file
		lastfile = nfiles-1;				// index last file
	}

	// loop over all selected files (or only one file currently selected)
	for (auto ifile=firstfile; ifile <= lastfile; ifile++)
	{
		// check if user wants to continue
		if (m_bAllfiles)
		{
			pdb_doc->DBSetCurrentRecordPosition(ifile);
			m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		}
		// check if this file is ok
		if (m_pSpkDoc == nullptr)
			continue;
		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist);
		if (m_pSpkList == nullptr)
			continue;

		const auto nspikes = m_pSpkList->GetTotalSpikes();
		if (nspikes <= 0 ||  m_pSpkList->GetSpikeLength() == 0)
			continue;

		switch(m_psC->iparameter)
		{
		case 1:		// value at t1 
			m_pSpkList->Measure_case1_AmplitudeAtT(m_psC->ileft);
			m_bMeasureDone = TRUE;
			break;
		case 2:		// value at t2
			m_pSpkList->Measure_case1_AmplitudeAtT(m_psC->iright);
			m_bMeasureDone = TRUE;
			break;

		case 3:		// value at t2- value at t1
			m_pSpkList->Measure_case2_AmplitudeAtT2MinusAtT1(m_psC->ileft, m_psC->iright);
			m_bMeasureDone = TRUE;
			break;

		case 0:		// max - min between t1 and t2
		case 4:		// max-min vs tmax-tmin
		default:
			m_pSpkList->Measure_case0_AmplitudeMinToMax(m_psC->ileft, m_psC->iright);
			break;
		}

		//save only if changed?
		m_pSpkDoc->OnSaveDocument(pdb_doc->DBGetCurrentSpkFileName(FALSE));		
	}

	if (m_bAllfiles)
	{
		currentfile = pdb_doc->DBGetCurrentRecordPosition(); 
		pdb_doc->DBSetCurrentRecordPosition(currentfile);
		m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		m_pSpkList = m_pSpkDoc->GetSpkListCurrent();
		spikeshape_wnd_.SetSourceData(m_pSpkList, GetDocument());
	}
			
	xygraph_wnd_.SetHZtagVal(m_itaglow, m_psC->ilower);
	xygraph_wnd_.SetHZtagVal(m_itagup, m_psC->iupper);

	BuildHistogram();
	yhistogram_wnd_.SetVTtagVal(m_itaglow, m_psC->ilower);
	yhistogram_wnd_.SetVTtagVal(m_itagup, m_psC->iupper);	

	UpdateGain();
	UpdateData(FALSE);
}

void CViewSpikeSort::UpdateGain()
{
	const auto delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
	const auto max = int (m_mVMax / delta);
	const auto min = int (m_mVMin / delta);
	if (max == min)
		return;

	const auto y_we = max-min;
	const auto y_wo = (max+min)/2;

	xygraph_wnd_.SetYWExtOrg(y_we, y_wo);
	xygraph_wnd_.Invalidate();

	yhistogram_wnd_.SetXWExtOrg(y_we, y_wo - y_we / 2);
	yhistogram_wnd_.Invalidate();

}
 
void CViewSpikeSort::OnFormatAlldata() 
{
	// build new histogram only if necessary
	auto build_histogram = FALSE;

	// dots: spk file length
	if (m_lFirst != 0 || m_lLast != m_pSpkDoc->GetAcqSize()-1)
	{	
		m_lFirst = 0;
		m_lLast = m_pSpkDoc->GetAcqSize()-1;

		if (m_psC->iparameter != 4) // then, we need imax imin ...
			xygraph_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
		else
			xygraph_wnd_.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
		xygraph_wnd_.Invalidate();

		spikeshape_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
		spikeshape_wnd_.Invalidate();

		spikebars_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
		spikebars_wnd_.Invalidate();
		build_histogram = TRUE;
	}

	// spikes: center spikes horizontally and adjust hz size of display	

	const auto x_we = m_pSpkList->GetSpikeLength();
	if (x_we != spikeshape_wnd_.GetXWExtent() || 0 != spikeshape_wnd_.GetXWOrg())
		spikeshape_wnd_.SetXWExtOrg(x_we, 0);

	// change spk_hist_wnd_
	if (build_histogram)
	{
		BuildHistogram();
	}
	UpdateLegends();
}

void CViewSpikeSort::BuildHistogram()
{
	auto pdb_doc = GetDocument();
	if (pdb_doc == nullptr)
		return;

	const auto delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
	m_parmmax = static_cast<int> (m_mVMax / delta);
	m_parmmin = static_cast<int> (m_mVMin / delta);
	int nbins = static_cast<int> ((m_mVMax - m_mVMin) / m_mVbin);
	if (nbins <= 0)
		return;

	yhistogram_wnd_.BuildHistFromDocument(pdb_doc, m_bAllfiles, m_lFirst, m_lLast, m_parmmax, m_parmmin, nbins, TRUE);
}

void CViewSpikeSort::OnFormatCentercurve() 
{
	// loop over all spikes of the list  
	const auto nspikes = m_pSpkList->GetTotalSpikes();
	for (auto ispike = 0; ispike < nspikes; ispike++)
		m_pSpkList->CenterSpikeAmplitude(ispike, m_psC->ileft, m_psC->iright, 1);

	int max, min;
	m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
	const auto middle = (max + min)/2;
	spikeshape_wnd_.SetYWExtOrg(spikeshape_wnd_.GetYWExtent(), middle);
	spikebars_wnd_.SetYWExtOrg(spikeshape_wnd_.GetYWExtent(), middle);

	UpdateLegends();
}

void CViewSpikeSort::OnFormatGainadjust() 
{		
	// adjust gain of spkform and spkbar: data = raw signal
	int max, min; 
	GetDocument()->GetAllSpkMaxMin(m_bAllfiles, TRUE, &max, &min);
	//m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);

	auto y_we = MulDiv(max-min+1, 10, 9);
	auto y_wo = (max + min)/2;
	spikeshape_wnd_.SetYWExtOrg(y_we, y_wo);
	spikebars_wnd_.SetYWExtOrg(y_we, y_wo);
	
	// adjust gain for spk_hist_wnd_ and XYp: data = computed values
	// search max min of parameter values
	CSize measure = GetDocument()->GetSpkMaxMin_y1(m_bAllfiles);
	max = measure.cx;
	min = measure.cy;

	const auto delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
	const auto max2 = static_cast<int>(m_upper / delta);
	const auto min2 = static_cast<int> (m_lower / delta);
	if (max2 > max)
		max = max2;
	if (min2 < min)
		min = min2;
	y_we = MulDiv(max-min+1, 10, 8);
	y_wo = (max + min)/2;

	// update display
	xygraph_wnd_.SetYWExtOrg(y_we, y_wo);
	const auto ymax = static_cast<int> (yhistogram_wnd_.GetHistMax());
	yhistogram_wnd_.SetXWExtOrg(y_we, y_wo-y_we/2);
	yhistogram_wnd_.SetYWExtOrg(MulDiv(ymax, 10, 8), 0);	
	
	// update edit controls
	m_mVMax = static_cast<float>(max)* delta;
	m_mVMin = static_cast<float>(min)* delta;
	BuildHistogram();
	UpdateLegends();
}

void CViewSpikeSort::SelectSpikeFromCurrentList(int spikeno)
{
	const auto ispike_local = spikeno;

	spikeshape_wnd_.SelectSpikeShape(ispike_local);
	spikebars_wnd_.SelectSpike(ispike_local);
	xygraph_wnd_.SelectSpike(ispike_local);
	m_pSpkList->m_selspike = ispike_local;

	m_spikenoclass = -1;
	auto n_cmd_show = SW_HIDE;
	if (ispike_local >= 0) 
	{
		const auto spike_elemt = m_pSpkList->GetSpikeElemt(ispike_local);
		m_spikenoclass = spike_elemt->get_class();
		n_cmd_show= SW_SHOW;		
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_SPIKECLASS)->ShowWindow(n_cmd_show);
	m_spikeno = ispike_local;
	UpdateData(FALSE);
}

void CViewSpikeSort::OnToolsEdittransformspikes() 
{
	CSpikeEditDlg dlg;
	dlg.m_yextent = spikeshape_wnd_.GetYWExtent();
	dlg.m_yzero = spikeshape_wnd_.GetYWOrg();	
	dlg.m_xextent = spikeshape_wnd_.GetXWExtent();
	dlg.m_xzero = spikeshape_wnd_.GetXWOrg();
	dlg.m_spikeno = m_spikeno;
	dlg.m_parent = this;
	dlg.m_pdbWaveDoc = GetDocument();

	// refresh pointer to data file because it not used elsewhere in the view
	auto docname = GetDocument()->DBGetCurrentDatFileName();
	auto b_doc_exists = FALSE;	
	if (!docname.IsEmpty())	
	{
		CFileStatus r_status;
		b_doc_exists = CFile::GetStatus(docname, r_status);
	}
	if (b_doc_exists) {
		const auto flag = (GetDocument()->OpenCurrentDataFile() != nullptr);
		ASSERT(flag);
	}	

	// run dialog box
	dlg.DoModal();
	if (dlg.m_bchanged)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
		const auto currentlist = m_tabCtrl.GetCurSel();
		const auto spike_list = m_pSpkDoc->SetSpkListCurrent(currentlist);
		const auto nspikes = spike_list->GetTotalSpikes();
	}

	if (!dlg.m_bartefact && m_spikeno != dlg.m_spikeno)
		SelectSpikeFromCurrentList(dlg.m_spikeno);

	UpdateLegends();
}

void CViewSpikeSort::OnSelectAllFiles() 
{
	m_bAllfiles = ((CButton*) GetDlgItem(IDC_CHECK1))->GetCheck();
	spikebars_wnd_.DisplayAllFiles(m_bAllfiles, GetDocument());
	spikeshape_wnd_.DisplayAllFiles(m_bAllfiles, GetDocument());
	xygraph_wnd_.DisplayAllFiles(m_bAllfiles, GetDocument());
	
	m_bMeasureDone = FALSE;
	OnMeasure();
}

void CViewSpikeSort::SaveCurrentFileParms()
{
	// save previous file if anything has changed
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		const auto currentlist = m_tabCtrl.GetCurSel();
		m_pSpkList = m_pSpkDoc->SetSpkListCurrent(currentlist);
		m_pSpkDoc->OnSaveDocument(GetDocument()->DBGetCurrentSpkFileName(FALSE));

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

void CViewSpikeSort::OnToolsAlignspikes() 
{
	// get source data
	auto b_doc_exist = FALSE;
	auto docname = m_pSpkDoc->GetSourceFilename();
	if (!docname.IsEmpty())
	{
		CFileStatus r_status;	// file status: time creation, ...
		b_doc_exist = CFile::GetStatus(docname, r_status);
	}
	if (!b_doc_exist)
	{
		AfxMessageBox(_T("Source data not found - operation aborted"));
		return;
	}
	
	// first prepare array with SUM

	const auto spikelen = m_pSpkList->GetSpikeLength();	// length of one spike
	const auto totalspikes = m_pSpkList->GetTotalSpikes();	// total nb of spikes /record
	const auto p_sum0 = new double [spikelen];		// array with results / SUMy
	const auto p_cxy0 = new double [spikelen];		// temp array to store correlat 
	auto* const p_mean0 = new short [spikelen];		// mean (template) / at scale
	const auto p_dummy0 = new short [spikelen];		// results of correlation / at scale

	// init pSUM with zeros
	auto p_sum = p_sum0;
	for (auto i=0; i < spikelen; i++, p_sum++)
		*p_sum = 0;

	// compute mean
	auto nbspk_selclass=0;
	short* p_spk;
	for (auto ispk=0; ispk< totalspikes; ispk++)
	{
		if (m_pSpkList->GetSpikeClass(ispk) != m_sourceclass)
			continue;
		nbspk_selclass++;
		p_spk = m_pSpkList->GetpSpikeData(ispk);
		p_sum = p_sum0;
		for (auto i = 0; i< spikelen; i++, p_spk++, p_sum++)
			*p_sum += *p_spk;
	}

	// build avg and avg autocorrelation, then display
	auto p_mean = p_mean0;
	p_sum = p_sum0;

	for (auto i=0; i< spikelen; i++, p_mean++, p_sum++)
		*p_mean = static_cast<short>(*p_sum / nbspk_selclass);

	spikeshape_wnd_.DisplayExData(p_mean0);

	// for each spike, compute correlation and take max value correlation
	const auto kstart = m_psC->ileft;		// start of template match 
	const auto kend   = m_psC->iright;		// end of template match
	if (kend <= kstart)
		return;
	const auto j0 = kstart - (kend-kstart)/2;	// start time lag
	const auto j1 = kend - (kend-kstart)/2 +1;	// last lag

	// compute autocorrelation for mean;
	double cxx_mean=0;
	p_mean = p_mean0 + kstart;
	for (auto i=kstart; i< kend; i++, p_mean++)
	{
		const auto val = static_cast<double>(*p_mean);		
		cxx_mean += val*val;
	}

	// get parameters from document
	auto p_dat_doc = GetDocument()->m_pDat;
	p_dat_doc->OnOpenDocument(docname);
	auto const doc_chan = m_pSpkList->GetextractChan();		// source channel	
	auto const nchans = p_dat_doc->GetpWaveFormat()->scan_count;	// number of data chans / source buffer
	auto const method = m_pSpkList->GetextractTransform();
	auto const pretrig = m_pSpkList->GetSpikePretrig();
	short const offset = (method>0) ? 1 : nchans;			// offset between points / detection
	short const nspan = p_dat_doc->GetTransfDataSpan(method);	// nb pts to read before transf

	// pre-load data
	auto iitime0 = m_pSpkList->GetSpikeTime(0); //-pretrig;
	auto l_rw_first0 = iitime0 - spikelen;
	auto l_rw_last0 = iitime0 + spikelen;
	if (!p_dat_doc->LoadRawData(&l_rw_first0, &l_rw_last0, nspan))
		return;										// exit if error reported
	auto p_data = p_dat_doc->LoadTransfData(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto spkpretrig = m_pSpkList->GetSpikePretrig();
	for (auto ispk = 0; ispk < totalspikes; ispk++)
	{
		// exclude spikes that do not fall within time limits
		if (m_pSpkList->GetSpikeClass(ispk) != m_sourceclass)
			continue;

		iitime0 = m_pSpkList->GetSpikeTime(ispk);
		iitime0 -= pretrig;						// offset beginning of spike

		// make sure that source data are loaded and get pointer to it (p_data)
		auto l_rw_first = iitime0 - spikelen;	// first point (eventually) needed
		auto l_rw_last = iitime0 + spikelen;	// last pt needed
		if (iitime0 > m_lLast || iitime0 < m_lFirst)
			continue;
		if (!p_dat_doc->LoadRawData(&l_rw_first, &l_rw_last, nspan))
			break;								// exit if error reported

		// load data only if necessary
		if (l_rw_first != l_rw_first0 || l_rw_last != l_rw_last0)
		{
			p_data = p_dat_doc->LoadTransfData(l_rw_first, l_rw_last, method, doc_chan);
			l_rw_last0 = l_rw_last;					// index las pt within p_data
			l_rw_first0 = l_rw_first;				// index first pt within p_data
		}
		
		// pointer to first point of spike
		auto p_data_spike0 = p_data +(iitime0 -l_rw_first)*offset;

		// for spike ispk: loop over spikelen time lags centered over interval center

		// compute autocorrelation & cross correlation at first time lag
		auto p_cxy_lag = p_cxy0;					// pointer to array with correl coeffs
		*p_cxy_lag =0;								// init cross corr
		auto pdat_k0 = p_data_spike0 + j0*offset;	// source data start

		// loop over all time lag requested
		for (auto j= j0; j < j1; j++, p_cxy_lag++, pdat_k0 += offset)
		{
			*p_cxy_lag =0;

			// add cross product for each point: data * meanlong iitime
			auto p_mean_k = p_mean0 + kstart;		// first point / template
			short* pdat_k = pdat_k0;				// first data point
			double cxx_spike = 0;					// autocorrelation

			// loop over all points of source data and mean
			for (auto k=kstart; k<kend; k++, p_mean_k++, pdat_k += offset)
			{
				const auto val = static_cast<double>(*pdat_k);
				*p_cxy_lag += double(*p_mean_k) * val;
				cxx_spike += val*val;
			}

			*p_cxy_lag /= (double(kend) - kstart +1);
			*p_cxy_lag = double(*p_cxy_lag) / sqrt(cxx_mean * cxx_spike);
		}

		// get max and min of this correlation
		auto p_cxy = p_cxy0;
		auto cxy_max = *p_cxy;				// correlation max value
		auto i_cxy_max = 0;					// correlation max index
		for (auto i=0; i < kend-kstart; p_cxy++, i++)
		{
			if (cxy_max < *p_cxy)			// get max and max position
			{
				cxy_max = *p_cxy;
				i_cxy_max = i;
			}
		}

		// offset spike so that max is at spikelen/2
		const auto jdecal = i_cxy_max - (kend-kstart)/2;
		if (jdecal != 0)
		{
			p_data_spike0 = p_data + static_cast<WORD>(iitime0 + jdecal - l_rw_first)*offset + doc_chan;
			m_pSpkList->TransferDataToSpikeBuffer(ispk, p_data_spike0, nchans);
			m_pSpkDoc->SetModifiedFlag(TRUE);
			m_pSpkList->SetSpikeTime(ispk, iitime0 +spkpretrig);
		}

		// now offset spike vertically to align it with the mean
		p_mean = p_mean0 + kstart;
		p_spk = m_pSpkList->GetpSpikeData(ispk) + kstart;
		long l_diff = 0;		
		for (auto i = kstart; i< kend; i++, p_spk++, p_mean++)
			l_diff += (*p_spk - *p_mean);
		l_diff /= (kend-kstart+1);
		p_spk = m_pSpkList->GetpSpikeData(ispk);
		const auto val = static_cast<short>(l_diff);
		for (auto i = 0; i< spikelen; i++, p_spk++)
			*p_spk -= val;
	}

	// exit : delete resources used locally
	if (m_pSpkDoc->IsModified())
	{
		spikeshape_wnd_.Invalidate();
		spikeshape_wnd_.DisplayExData(p_mean0);
	}

	delete [] p_sum0;
	delete [] p_mean0;
	delete [] p_cxy0;
	delete [] p_dummy0;

	OnMeasure();
}

void CViewSpikeSort::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// get corresponding data
	const auto total_scroll= m_pSpkDoc->GetAcqSize();
	const auto page_scroll = (m_lLast - m_lFirst);
	auto sb_scroll = MulDiv(page_scroll, 10, 100);
	if (sb_scroll == 0)
		sb_scroll = 1;
	auto l_first = m_lFirst;
	switch (nSBCode)
	{		
	case SB_LEFT:		
		l_first = 0;	break;			// Scroll to far left.
	case SB_LINELEFT:	
		l_first -= sb_scroll;	break;	// Scroll left.
	case SB_LINERIGHT:	
		l_first += sb_scroll; break;	// Scroll right
	case SB_PAGELEFT:	
		l_first -= page_scroll; break;	// Scroll one page left
	case SB_PAGERIGHT:	
		l_first += page_scroll; break;	// Scroll one page right.
	case SB_RIGHT:		
		l_first = total_scroll - page_scroll+1; 
		break;
	case SB_THUMBPOSITION:				// scroll to pos = nPos			
	case SB_THUMBTRACK:					// drag scroll box -- pos = nPos
		l_first = static_cast<int>(nPos);
		break;
	default:
		return;
	}

	if (l_first < 0)
		l_first = 0;
	auto l_last = l_first + page_scroll;

	if (l_last >= total_scroll)
	{
		l_last = total_scroll - 1;
		l_first = l_last - page_scroll;
	}
	
	// adjust display
	if (l_first != m_lFirst)
	{
		m_lFirst = l_first;
		m_lLast = l_last;
		UpdateLegends();
	}
	else
		UpdateScrollBar();
}

void CViewSpikeSort::UpdateScrollBar()
{
	GetDlgItem(IDC_SCROLLBAR1)->ShowWindow(SW_SHOW);		
	m_scroll_file_pos_infos_.fMask = SIF_ALL;
	m_scroll_file_pos_infos_.nMin = 0;
	m_scroll_file_pos_infos_.nMax = m_pSpkDoc->GetAcqSize()-1;
	m_scroll_file_pos_infos_.nPos = m_lFirst;
	m_scroll_file_pos_infos_.nPage = m_lLast-m_lFirst;
	((CScrollBar*) GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&m_scroll_file_pos_infos_);
}

void CViewSpikeSort::SelectSpkList(int icursel)
{
	m_pSpkList = m_pSpkDoc->SetSpkListCurrent(icursel);
	GetDocument()->SetcurrentSpkListIndex(icursel);
	ASSERT(m_pSpkList != NULL);
	OnMeasure();

	// update source data: change data channel and update display
	yhistogram_wnd_.SetSpkList(m_pSpkList);
	spikeshape_wnd_.SetSpkList(m_pSpkList);
	spikebars_wnd_.SetSpkList(m_pSpkList);
	xygraph_wnd_.SetSpkList(m_pSpkList);

	yhistogram_wnd_.Invalidate();
	xygraph_wnd_.Invalidate();
	spikeshape_wnd_.Invalidate();
	spikebars_wnd_.Invalidate();
}

void CViewSpikeSort::OnNMClickTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	m_tabCtrl.SetCurSel(icursel);
	SelectSpkList(icursel);
	*pResult = 0;
}

void CViewSpikeSort::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	const auto icursel = m_tabCtrl.GetCurSel();
	SelectSpkList(icursel);
	*pResult = 0;
}

// ---------- OnEnChange

void CViewSpikeSort::OnEnChangeEditleft2()
{
	if (mm_txyleft.m_bEntryDone)
	{
		auto left = m_txyleft;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();
		switch (mm_txyleft.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE); 	left = m_txyleft;	break;
		case VK_UP:
		case VK_PRIOR:	left += delta; break;
		case VK_DOWN:
		case VK_NEXT: 	left -= delta; break;
		default: ;
		}
		// check boundaries
		if (left >= m_txyright)
			left = m_txyright - delta;

		// change display if necessary	
		mm_txyleft.m_bEntryDone = FALSE;
		mm_txyleft.m_nChar = 0;	
		mm_txyleft.SetSel(0, -1);
		m_txyleft = left;
		left = m_txyleft / delta;
		const auto itleft = static_cast<int>(left);
		if (itleft != xygraph_wnd_.GetVTtagVal(m_ixyleft))
		{
			m_psC->ixyleft = itleft;
			xygraph_wnd_.MoveVTtagtoVal(m_ixyleft, itleft);
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnEnChangeEditright2()
{
	if (mm_txyright.m_bEntryDone)
	{
		auto right = m_txyright;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();
		switch (mm_txyright.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE); 	right = m_txyright;	break;
		case VK_UP:
		case VK_PRIOR:	right += delta; break;
		case VK_DOWN:
		case VK_NEXT: 	right -= delta; break;
		default: ;
		}

		// check boundaries
		if (right <= m_txyleft)
			right = m_txyleft + delta;

		// change display if necessary	
		mm_txyright.m_bEntryDone = FALSE;
		mm_txyright.m_nChar = 0;
		mm_txyright.SetSel(0, -1);
		m_txyright = right;
		right = m_txyright / delta;
		const auto itright = static_cast<int>(right);
		if (itright != xygraph_wnd_.GetVTtagVal(m_ixyright))
		{
			m_psC->ixyright = itright;
			xygraph_wnd_.MoveVTtagtoVal(m_ixyright, itright);
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnEnChangeSourceclass()
{
	if (mm_sourceclass.m_bEntryDone)
	{
		auto sourceclass = m_sourceclass;
		switch (mm_sourceclass.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE); sourceclass = m_sourceclass; break;
		case VK_UP:
		case VK_PRIOR:	sourceclass++; break;
		case VK_DOWN:
		case VK_NEXT: 	sourceclass--; break;
		default:;
		}
		// change display if necessary	
		mm_sourceclass.m_bEntryDone = FALSE;// clear flag
		mm_sourceclass.m_nChar = 0;			// empty buffer
		mm_sourceclass.SetSel(0, -1);		// select all text	
		if (sourceclass != m_sourceclass)
		{
			m_sourceclass = sourceclass;
			spikeshape_wnd_.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);
			xygraph_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
			yhistogram_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
			spikebars_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
		}
		// change histogram accordingly
		spikeshape_wnd_.Invalidate();
		spikebars_wnd_.Invalidate();
		xygraph_wnd_.Invalidate();
		yhistogram_wnd_.Invalidate();

		SelectSpikeFromCurrentList(-1);
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnEnChangeDestinationclass()
{
	if (mm_destinationclass.m_bEntryDone)
	{
		short destinationclass = m_destinationclass;
		switch (mm_destinationclass.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE); destinationclass = m_destinationclass; break;
		case VK_UP:
		case VK_PRIOR:	destinationclass++; break;
		case VK_DOWN:
		case VK_NEXT: 	destinationclass--; break;
		default:;
		}
		// change display if necessary	
		mm_destinationclass.m_bEntryDone = FALSE;	// clear flag
		mm_destinationclass.m_nChar = 0;			// empty buffer
		mm_destinationclass.SetSel(0, -1);		// select all text	
		m_destinationclass = destinationclass;
		SelectSpikeFromCurrentList(-1);
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnSelchangeParameter()
{
	const auto iparameter = m_CBparameter.GetCurSel();
	if (iparameter != m_psC->iparameter)
	{
		m_psC->iparameter = iparameter;
		ActivateMode4();
		OnMeasure();
		OnFormatCentercurve();
	}

	//    STATIC3 lower STATIC4 upper STATIC5 T1 STATIC6 T2
	// 0  mV      vis    mV      vis    vis     vis  vis   vis 
	// 1  mV      vis    mV      vis    vis     vis  NOT   NOT 
	// 2  mS      vis    mS      vis    vis     vis  vis   vis 	
}

void CViewSpikeSort::OnEnChangelower()
{
	if (mm_lower.m_bEntryDone)
	{
		auto lower = m_lower;
		//if (!m_bAllfiles)
			m_delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;
		switch (mm_lower.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE); 	lower = m_lower;	break;
		case VK_UP:
		case VK_PRIOR:	lower += m_delta; break;
		case VK_DOWN:
		case VK_NEXT: 	lower -= m_delta; break;
		default:;
		}
		// check boundaries
		if (lower < 0) lower = 0;
		if (lower >= m_upper)
			lower = m_upper - m_delta * 10.f;
		// change display if necessary	
		mm_lower.m_bEntryDone = FALSE;	// clear flag
		mm_lower.m_nChar = 0;			// empty buffer
		mm_lower.SetSel(0, -1);			// select all text
		m_lower = lower;
		m_psC->ilower = static_cast<int>(m_lower / m_delta);
		if (m_psC->ilower != xygraph_wnd_.GetHZtagVal(m_itaglow))
			xygraph_wnd_.MoveHZtagtoVal(m_itaglow, m_psC->ilower);
		if (m_psC->ilower != yhistogram_wnd_.GetVTtagVal(m_spkhistlower))
			yhistogram_wnd_.MoveVTtagtoVal(m_spkhistlower, m_psC->ilower);
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnEnChangeupper()
{
	if (mm_upper.m_bEntryDone)
	{
		auto upper = m_upper;
		m_delta = m_pSpkList->GetAcqVoltsperBin()*m_vunit;

		switch (mm_upper.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE); upper = m_upper; break;
		case VK_UP:
		case VK_PRIOR:	upper += m_delta; break;
		case VK_DOWN:
		case VK_NEXT: 	upper -= m_delta; break;
		default:;
		}

		// check boundaries
		if (upper < 0)
			upper = 0;
		if (upper <= m_lower)
			upper = m_lower + m_delta * 10.f;
		
		// change display if necessary	
		mm_upper.m_bEntryDone = FALSE;	// clear flag
		mm_upper.m_nChar = 0;			// empty buffer
		mm_upper.SetSel(0, -1);			// select all text
		m_upper = upper;
		m_psC->iupper = static_cast<int>(m_upper / m_delta);
		if (m_psC->iupper != xygraph_wnd_.GetHZtagVal(m_itagup))
			xygraph_wnd_.MoveHZtagtoVal(m_itagup, m_psC->iupper);
		if (m_psC->ilower != yhistogram_wnd_.GetVTtagVal(m_spkhistupper))
			yhistogram_wnd_.MoveVTtagtoVal(m_spkhistupper, m_psC->iupper);
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnEnChangeT1()
{
	if (mm_t1.m_bEntryDone)
	{
		auto t1 = m_t1;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();

		switch (mm_t1.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			t1 = m_t1;
			break;
		case VK_UP:
		case VK_PRIOR:
			t1 += delta;
			break;
		case VK_DOWN:
		case VK_NEXT:
			t1 -= delta;
			break;
		default:;
		}
		// check boundaries
		if (t1 < 0)
			t1 = 0.0f;
		if (t1 >= m_t2)
			t1 = m_t2 - delta;
		// change display if necessary	
		mm_t1.m_bEntryDone = FALSE;	// clear flag
		mm_t1.m_nChar = 0;			// empty buffer
		mm_t1.SetSel(0, -1);		// select all text
		m_t1 = t1;
		const auto it1 = static_cast<int>(m_t1 / delta);
		if (it1 != spikeshape_wnd_.GetVTtagVal(m_spkformtagleft))
		{
			m_psC->ileft = it1;
			spikeshape_wnd_.MoveVTtrack(m_spkformtagleft, m_psC->ileft);
			m_pSpkList->m_imaxmin1SL = m_psC->ileft;
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnEnChangeT2()
{
	if (mm_t2.m_bEntryDone)
	{
		auto t2 = m_t2;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();
		switch (mm_t2.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			t2 = m_t2;
			break;
		case VK_UP:
		case VK_PRIOR:
			t2 += delta;
			break;
		case VK_DOWN:
		case VK_NEXT:
			t2 -= delta;
			break;
		default:;
		}

		// check boundaries
		if (t2 < m_t1)
			t2 = m_t1 + delta;
		const auto tmax = (m_pSpkList->GetSpikeLength() - 1) *delta;
		if (t2 >= tmax)
			t2 = tmax;
		// change display if necessary	
		mm_t2.m_bEntryDone = FALSE;	// clear flag
		mm_t2.m_nChar = 0;			// empty buffer
		mm_t2.SetSel(0, -1);		// select all text
		m_t2 = t2;
		const auto it2 = static_cast<int>(m_t2 / delta);
		if (it2 != spikeshape_wnd_.GetVTtagVal(m_spkformtagright))
		{
			m_psC->iright = it2;
			spikeshape_wnd_.MoveVTtrack(m_spkformtagright, m_psC->iright);
			m_pSpkList->m_imaxmin2SL = m_psC->iright;
		}
		UpdateData(FALSE);
	}
}

void CViewSpikeSort::OnEnChangetimeFirst()
{
	if (mm_timeFirst.m_bEntryDone)
	{
		auto time_first = m_timeFirst;
		switch (mm_timeFirst.m_nChar)
		{
		case VK_RETURN:	UpdateData(TRUE); time_first = m_timeFirst; break;
		case VK_UP:
		case VK_PRIOR:	time_first++; break;
		case VK_DOWN:
		case VK_NEXT: 	time_first--;	break;
		default:;
		}

		// check boundaries
		if (time_first < 0.f)
			time_first = 0.f;
		if (time_first >= m_timeLast)
			time_first = 0.f;

		// change display if necessary
		mm_timeFirst.m_bEntryDone = FALSE;
		mm_timeFirst.m_nChar = 0;
		mm_timeFirst.SetSel(0, -1);
		m_timeFirst = time_first;
		m_lFirst = static_cast<long>(m_timeFirst * m_pSpkList->GetAcqSampRate());
		UpdateLegends();
	}
}

void CViewSpikeSort::OnEnChangetimeLast()
{
	if (mm_timeLast.m_bEntryDone)
	{
		auto time_last = m_timeLast;
		switch (mm_timeLast.m_nChar)
		{
		case VK_RETURN:	UpdateData(TRUE); time_last = m_timeLast; break;
		case VK_UP:
		case VK_PRIOR:	time_last++; break;
		case VK_DOWN:
		case VK_NEXT: 	time_last--;	break;
		default:;
		}

		// check boundaries
		if (time_last <= m_timeFirst)
			m_lLast = long((m_pSpkDoc->GetAcqSize() - 1) / m_pSpkList->GetAcqSampRate());

		// change display if necessary
		mm_timeLast.m_bEntryDone = FALSE;
		mm_timeLast.m_nChar = 0;
		mm_timeLast.SetSel(0, -1);
		m_timeLast = time_last;
		m_lLast = static_cast <long> (m_timeLast * m_pSpkList->GetAcqSampRate());
		UpdateLegends();
	}
}

void CViewSpikeSort::OnEnChangemVMin()
{
	if (mm_mVMin.m_bEntryDone)
	{
		auto mv_min = m_mVMin;
		switch (mm_mVMin.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			mv_min = m_mVMin; break;
		case VK_UP:
		case VK_PRIOR:	mv_min++; break;
		case VK_DOWN:
		case VK_NEXT: 	mv_min--;	break;
		default:;
		}
		// check boundaries
		if (mv_min >= m_mVMax)
			mv_min = m_mVMax - 1.f;

		// change display if necessary
		mm_mVMin.m_bEntryDone = FALSE;
		mm_mVMin.m_nChar = 0;
		mm_mVMin.SetSel(0, -1);
		m_mVMin = mv_min;

		UpdateGain();
		UpdateLegends();
	}
}

void CViewSpikeSort::OnEnChangemVMax()
{
	if (mm_mVMax.m_bEntryDone)
	{
		auto mv_max = m_mVMax;
		switch (mm_mVMax.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			mv_max = m_mVMax; break;
		case VK_UP:
		case VK_PRIOR:	mv_max++; break;
		case VK_DOWN:
		case VK_NEXT: 	mv_max--;	break;
		default:;
		}

		// check boundaries
		if (mv_max <= m_mVMin)
			mv_max = m_mVMin + 1.f;

		// change display if necessary
		mm_mVMax.m_bEntryDone = FALSE;
		mm_mVMax.m_nChar = 0;
		mm_mVMax.SetSel(0, -1);
		m_mVMax = mv_max;

		UpdateGain();
		UpdateLegends();
	}
}

void CViewSpikeSort::OnEnChangeNOspike()
{
	if (mm_spikeno.m_bEntryDone)
	{
		const auto spikeno = m_spikeno;
		switch (mm_spikeno.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikeno++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikeno--;	break;
		default:;
		}

		// check boundaries
		if (m_spikeno < 0)
			m_spikeno = -1;
		if (m_spikeno >= m_pSpkList->GetTotalSpikes())
			m_spikeno = m_pSpkList->GetTotalSpikes() - 1;

		mm_spikeno.m_bEntryDone = FALSE;
		mm_spikeno.m_nChar = 0;
		mm_spikeno.SetSel(0, -1);
		if (m_spikeno != spikeno)
		{
			if (m_spikeno >= 0)
			{
				// test if spike visible in the current time interval
				const auto spike_element = m_pSpkList->GetSpikeElemt(m_spikeno);
				const auto spk_first = spike_element->get_time() - m_pSpkList->GetSpikePretrig();
				const auto spk_last = spk_first + m_pSpkList->GetSpikeLength();

				if (spk_first < m_lFirst || spk_last > m_lLast)
				{
					const auto lspan = (m_lLast - m_lFirst) / 2;
					const auto lcenter = (spk_last + spk_first) / 2;
					m_lFirst = lcenter - lspan;
					m_lLast = lcenter + lspan;
					UpdateLegends();
				}
			}
		}
		SelectSpikeFromCurrentList(m_spikeno);
	}
}

void CViewSpikeSort::OnEnChangeSpikenoclass()
{
	if (mm_spikenoclass.m_bEntryDone)
	{
		const auto spikenoclass = m_spikenoclass;
		switch (mm_spikenoclass.m_nChar)
		{
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikenoclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikenoclass--;	break;
		default:;
		}

		mm_spikenoclass.m_bEntryDone = FALSE;
		mm_spikenoclass.m_nChar = 0;
		mm_spikenoclass.SetSel(0, -1);

		if (m_spikenoclass != spikenoclass)
		{
			m_pSpkDoc->SetModifiedFlag(TRUE);
			const auto currentlist = m_tabCtrl.GetCurSel();
			auto* spike_list = m_pSpkDoc->SetSpkListCurrent(currentlist);
			spike_list->SetSpikeClass(m_spikeno, m_spikenoclass);
			UpdateLegends();
		}
	}
}

void CViewSpikeSort::OnEnChangeNBins()
{
	if (mm_mVbin.m_bEntryDone)
	{
		auto mVbin = m_mVbin;
		const auto delta = (m_mVMax - m_mVMin) / 10.f;
		switch (mm_mVbin.m_nChar)
		{
		case VK_RETURN:	UpdateData(TRUE); mVbin = m_mVbin; break;
		case VK_UP:
		case VK_PRIOR:	mVbin += delta;	break;
		case VK_DOWN:
		case VK_NEXT:   mVbin -= delta;	break;
		default:;
		}

		mm_mVbin.m_bEntryDone = FALSE;
		mm_mVbin.m_nChar = 0;
		mm_mVbin.SetSel(0, -1);

		if (m_mVbin != mVbin)
		{
			m_mVbin = mVbin;
			BuildHistogram();
			UpdateLegends();
		}
	}
}
