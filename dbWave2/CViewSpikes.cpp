// Viewspikes.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave_constants.h"
#include "dbWave.h"
#include "resource.h"
//#include <math.h>
//#include "Cscale.h"
#include "chart.h"
#include "ChartData.h"
#include "Editctrl.h"
//#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "SpikeClassListBox.h"
#include "Editspik.h"
#include "MainFrm.h"
#include "Copyasdl.h"
#include "ChildFrm.h"
#include "CViewSpikes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CViewSpikes, CDaoRecordView)

CViewSpikes::CViewSpikes() : CDaoRecordView(CViewSpikes::IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

CViewSpikes::~CViewSpikes()
{
	m_psC->vdestclass = m_destclass;
	m_psC->vsourceclass = m_sourceclass;
	m_psC->bresetzoom = m_bresetzoom;
	m_psC->fjitter_ms = m_jitter_ms;
}

BOOL CViewSpikes::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
		//  the CREATESTRUCT cs
	return CDaoRecordView::PreCreateWindow(cs);
}

void CViewSpikes::DoDataExchange(CDataExchange* pDX)
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
	DDX_Check(pDX, IDC_SAMECLASS, m_bKeepSameClass);
}

BEGIN_MESSAGE_MAP(CViewSpikes, CDaoRecordView)

	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()

	ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)

	ON_COMMAND(ID_FORMAT_ALLDATA, OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, OnFormatCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, OnFormatGainadjust)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, OnToolsEdittransformspikes)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_FORMAT_PREVIOUSFRAME, OnFormatPreviousframe)
	ON_COMMAND(ID_FORMAT_NEXTFRAME, OnFormatNextframe)
	ON_COMMAND(ID_RECORD_SHIFTLEFT, OnHScrollLeft)
	ON_COMMAND(ID_RECORD_SHIFTRIGHT, OnHScrollRight)
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

	ON_NOTIFY(NM_CLICK, IDC_TAB1, &CViewSpikes::OnNMClickTab1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CViewSpikes::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_SAMECLASS, &CViewSpikes::OnBnClickedSameclass)
END_MESSAGE_MAP()

void CViewSpikes::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		auto p_mainframe = (CMainFrame*)AfxGetMainWnd();
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW, (LPARAM)pActivateView->GetDocument());
	}
	else
	{
		if (this != pActivateView && this == pDeactiveView)
		{
			SaveCurrentFileParms();
			// save column parameters
			m_psC->colseparator = m_spkClassListBox.GetColsSeparatorWidth();
			m_psC->rowheight = m_spkClassListBox.GetRowHeight();
			m_psC->colspikes = m_spkClassListBox.GetColsSpikesWidth();
			m_psC->coltext = m_spkClassListBox.GetColsTextWidth();

			auto* p_app = (CdbWaveApp*)AfxGetApp();
			if (p_app->m_pviewspikesMemFile == nullptr)
			{
				p_app->m_pviewspikesMemFile = new CMemFile;
				ASSERT(p_app->m_pviewspikesMemFile != NULL);
			}

			CArchive ar(p_app->m_pviewspikesMemFile, CArchive::store);
			p_app->m_pviewspikesMemFile->SeekToBegin();
			m_ChartDataWnd.Serialize(ar);
			ar.Close();	
		}
		// set bincrflagonsave
		auto p_app = (CdbWaveApp*)AfxGetApp();
		p_app->options_viewspikes.bincrflagonsave = ((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck();
		// save zoom dataview
		((CdbWaveApp*)AfxGetApp())->options_viewdata.viewdata = *(m_ChartDataWnd.GetScopeParameters());
	}
	CDaoRecordView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CViewSpikes::OnSize(UINT nType, int cx, int cy)
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
		m_ChartDataWnd.GetWindowRect(&rect1);
		m_spkClassListBox.GetWindowRect(&rect2);
		m_rectVTtrack.top = rect1.top - rect0.top;
		m_rectVTtrack.bottom = rect2.bottom - rect0.top;
		m_rectVTtrack.left = rect1.left - rect0.left;
		m_rectVTtrack.right = rect1.right - rect0.left;
	}
	CDaoRecordView::OnSize(nType, cx, cy);
}

BOOL CViewSpikes::OnMove(UINT nIDMoveCommand)
{
	SaveCurrentFileParms();
	const auto flag = CDaoRecordView::OnMove(nIDMoveCommand);
	auto p_document = GetDocument();
	if (p_document->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
	{
		((CChildFrame*)GetParent())->PostMessage(WM_COMMAND, ID_VIEW_SPIKEDETECTION, NULL);
		return false;
	}
	p_document->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	return flag;
}

void CViewSpikes::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (!m_binit)
		return;

	switch (LOWORD(lHint))
	{
	case HINT_DOCHASCHANGED:		// file has changed?
	case HINT_DOCMOVERECORD:
		UpdateFileParameters(TRUE);
		break;
	case HINT_CLOSEFILEMODIFIED:	// close modified file: save
		SaveCurrentFileParms();
		break;
	case HINT_REPLACEVIEW:
	default:
		break;
	}
}

LRESULT CViewSpikes::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	int threshold = LOWORD(lParam);	// value associated

	// ----------------------------- change mouse cursor (all 3 items)
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		if (threshold > CURSOR_MEASURE)	 // clip cursor shape to max 2
			threshold = 0;
		if (m_ChartDataWnd.GetMouseCursorType() != threshold)
			SetAddspikesMode(threshold);
		m_ChartDataWnd.SetMouseCursorType(threshold);
		m_spkClassListBox.SetMouseCursorType(threshold);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(threshold, 0));
		break;

	case HINT_DEFINEDRECT:
		if (m_ChartDataWnd.GetMouseCursorType() == CURSOR_MEASURE)
		{
			const auto rect = m_ChartDataWnd.GetDefinedRect();
			const auto left = m_ChartDataWnd.GetDataOffsetfromPixel(rect.left);
			const auto right = m_ChartDataWnd.GetDataOffsetfromPixel(rect.right);
			// set a tag in the center of the rectangle
			m_ChartDataWnd.m_VTtags.AddLTag((left + right) / 2, 0);
			m_ChartDataWnd.Invalidate();
		}
		break;

	case HINT_SELECTSPIKES:
		m_ChartDataWnd.Invalidate();
		m_spkClassListBox.Invalidate();
		break;

		// ----------------------------- select bar/display bars or zoom
		// if (HIWORD(lParam) == IDC_DISPLAYBARS || HIWORD(lParam) == NULL)
	case HINT_CHANGEHZLIMITS:		// abcissa have changed
	case HINT_CHANGEZOOM:
	case HINT_VIEWSIZECHANGED:		// change zoom
		if (HIWORD(lParam) == IDC_DISPLAYDAT)
		{
			m_lFirst = m_ChartDataWnd.GetDataFirst();
			m_lLast = m_ChartDataWnd.GetDataLast();
		}
		else if (HIWORD(lParam) == IDC_LISTCLASSES) //[ne marche pas! HIWORD(lParam)==1]
		{
			m_lFirst = m_spkClassListBox.GetTimeFirst();
			m_lLast = m_spkClassListBox.GetTimeLast();
		}
		UpdateLegends(TRUE);
		break;

	case HINT_HITSPIKE:
		SelectSpike(threshold);
		break;

	case HINT_DBLCLKSEL:
		if (threshold < 0)
			threshold = 0;
		m_spikeno = threshold;
		OnToolsEdittransformspikes();
		break;

	case HINT_DROPPED:
		m_pSpkDoc->SetModifiedFlag();
		m_spikenoclass = m_pSpkList->GetSpikeClass(m_spikeno);
		UpdateData(FALSE);
		break;

	case HINT_WINDOWPROPSCHANGED:
		options_viewdata->spkviewdata = *m_ChartDataWnd.GetScopeParameters();
		break;

	case HINT_HITSPIKE_SHIFT:
		SelectSpike(threshold);
		break;

	default:
		break;
	}
	return 0L;
}

BOOL CViewSpikes::AddSpiketoList(long iitime, BOOL bcheck_if_otheraround)
{
	const auto method = m_pSpkList->GetdetectTransform();
	const auto doc_channel = m_pSpkList->GetextractChan();
	const auto prethreshold = m_pSpkList->GetSpikePretrig();
	//int refractory	= m_pSpkList->GetSpikeRefractory();
	//int postthreshold = m_pSpkList->GetextractNpoints() - prethreshold;
	const auto spikelen = m_pSpkList->GetSpikeLength();

	// get parameters from document
	//int nchans;										// number of data chans / source buffer
	//auto p_buffer = m_pDataDoc->LoadRawDataParams(&nchans);
	const auto nspan = m_pDataDoc->GetTransfDataSpan(method);	// nb pts to read before transf

	const auto iitime0 = iitime - prethreshold;
	auto l_read_write_first = iitime0;					// first point (eventually) needed
	auto l_read_write_last = iitime0 + spikelen;		// last pt needed
	if (!m_pDataDoc->LoadRawData(&l_read_write_first, &l_read_write_last, nspan))
		return FALSE;									// exit if error
	//short* p_data =
	m_pDataDoc->LoadTransfData(l_read_write_first, l_read_write_last, method, doc_channel);
	const auto p_data_spike_0 = m_pDataDoc->GetpTransfDataElmt(iitime0 - l_read_write_first);

	// add a new spike if no spike is found around
	int spikeindex = 0;
	auto bfound = FALSE;
	if (bcheck_if_otheraround)
	{
		const auto jitter = int((m_pSpkDoc->GetAcqRate() * m_jitter) / 1000);
		bfound = m_pSpkList->IsAnySpikeAround(iitime0 + prethreshold, jitter, spikeindex, doc_channel);
	}

	if (!bfound)
	{
		spikeindex = m_pSpkList->AddSpike(p_data_spike_0,		//lpSource	= buff pointer to the buffer to copy
			1, //offset, /*nchans,*/							//nchans	= nb of interleaved channels
			iitime0 + prethreshold,								//time = file index of first pt of the spk
			doc_channel,										//detectChan	= data source chan index
			m_destclass, bcheck_if_otheraround);

		m_pSpkDoc->SetModifiedFlag();
	}
	else if (m_pSpkList->GetSpikeClass(spikeindex) != m_destclass)
	{
		m_pSpkList->SetSpikeClass(spikeindex, m_destclass);
		m_pSpkDoc->SetModifiedFlag();
	}

	// save the modified data into the spike file
	if (m_pSpkDoc->IsModified())
	{
		m_pSpkList->UpdateClassList();
		m_pSpkDoc->OnSaveDocument(GetDocument()->GetDB_CurrentSpkFileName(FALSE));
		m_pSpkDoc->SetModifiedFlag(FALSE);
		GetDocument()->SetDB_nbspikes(m_pSpkList->GetTotalSpikes());
		GetDocument()->SetDB_nbspikeclasses(m_pSpkList->GetNbclasses());
		const auto boldparm = m_bresetzoom;
		m_bresetzoom = FALSE;
		UpdateSpikeFile(TRUE);
		m_bresetzoom = boldparm;
	}
	m_spikeno = spikeindex;

	UpdateDataFile(TRUE);
	UpdateLegends(TRUE);
	m_spkClassListBox.Invalidate();
	return TRUE;
}

void CViewSpikes::SelectSpike(int spikeno)
{
	if (m_pSpkDoc == nullptr)
		return;
	if (spikeno >= m_pSpkList->GetTotalSpikes())
		spikeno = -1;
	m_spikeno = spikeno;
	m_pSpkList->m_selspike = spikeno;
	m_spkClassListBox.SelectSpike(spikeno);

	m_spikenoclass = -1;
	int n_cmd_show;
	if (spikeno >= 0 && spikeno < m_pSpkList->GetTotalSpikes())
	{
		// get address of spike parms
		const auto p_spike_element = m_pSpkList->GetSpikeElemt(m_spikeno);
		m_spikenoclass = p_spike_element->get_class();
		m_bartefact = (m_spikenoclass < 0);
		const auto spk_first = p_spike_element->get_time() - m_pSpkList->GetSpikePretrig();
		const auto spk_last = spk_first + m_pSpkList->GetSpikeLength();
		n_cmd_show = SW_SHOW;
		if (m_pDataDoc != nullptr)
		{
			m_DWintervals.SetAt(3, spk_first);
			m_DWintervals.SetAt(4, spk_last);
			m_ChartDataWnd.SetHighlightData(&m_DWintervals);
			m_ChartDataWnd.Invalidate();
		}
	}
	else
	{
		n_cmd_show = SW_HIDE;
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_ARTEFACT)->ShowWindow(n_cmd_show);

	UpdateData(FALSE);
}

void CViewSpikes::DefineSubClassedItems()
{
	// attach controls
	VERIFY(mm_spikeno.SubclassDlgItem(IDC_NSPIKES, this));
	mm_spikeno.ShowScrollBar(SB_VERT);
	VERIFY(mm_spikenoclass.SubclassDlgItem(IDC_EDIT2, this));
	mm_spikenoclass.ShowScrollBar(SB_VERT);
	VERIFY(m_spkClassListBox.SubclassDlgItem(IDC_LISTCLASSES, this));
	VERIFY(mm_timefirst.SubclassDlgItem(IDC_TIMEFIRST, this));
	VERIFY(mm_timelast.SubclassDlgItem(IDC_TIMELAST, this));
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollRange(0, 100, FALSE);
	VERIFY(m_ChartDataWnd.SubclassDlgItem(IDC_DISPLAYDAT, this));
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
	m_hBias = AfxGetApp()->LoadIcon(IDI_BIAS);
	m_hZoom = AfxGetApp()->LoadIcon(IDI_ZOOM);
	GetDlgItem(IDC_BIAS_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), (LPARAM)(HANDLE)m_hBias);
	GetDlgItem(IDC_GAIN_button)->SendMessage(BM_SETIMAGE, static_cast<WPARAM>(IMAGE_ICON), (LPARAM)(HANDLE)m_hZoom);
}

void CViewSpikes::DefineStretchParameters()
{
	m_stretch.AttachParent(this);		// attach formview pointer
	m_stretch.newProp(IDC_LISTCLASSES, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_DISPLAYDAT, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_TIMEINTERVALS, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMEFIRST, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TIMELAST, SZEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_SCROLLBAR1, XLEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_GAIN_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_BIAS_button, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_SCROLLY_scrollbar, SZEQ_XREQ, SZEQ_YTEQ);

	m_binit = TRUE;
}

void CViewSpikes::OnInitialUpdate()
{
	CDaoRecordView::OnInitialUpdate();
	DefineSubClassedItems();
	DefineStretchParameters();

	// load global parameters
	auto* p_app = (CdbWaveApp*)AfxGetApp();
	options_viewdata = &(p_app->options_viewdata);		// viewdata options
	mdMO = &(p_app->options_viewdata_measure);			// measure options
	m_psC = &(p_app->spkC);								// get address of spike classif parms
	m_destclass = m_psC->vdestclass;
	m_sourceclass = m_psC->vsourceclass;
	m_bresetzoom = m_psC->bresetzoom;
	m_jitter_ms = m_psC->fjitter_ms;

	// adjust size of the row and cols with text, spikes, and bars
	CRect rect;
	GetDlgItem(IDC_LISTCLASSES)->GetWindowRect(&rect);
	m_spkClassListBox.SetRowHeight(m_psC->rowheight);
	CRect rect2;
	GetDlgItem(IDC_DISPLAYDAT)->GetWindowRect(&rect2);
	const int leftcolwidth = rect2.left - rect.left - 2;
	m_spkClassListBox.SetLeftColWidth(leftcolwidth);
	if (m_psC->coltext < 0)
	{
		m_psC->colspikes = m_psC->rowheight;
		m_psC->coltext = leftcolwidth - 2 * m_psC->colseparator - m_psC->colspikes;
		if (m_psC->coltext < 20)
		{
			const auto half = leftcolwidth - m_psC->colseparator;
			m_psC->colspikes = half;
			m_psC->coltext = half;
		}
	}
	m_psC->coltext = leftcolwidth - m_psC->colspikes - 2 * m_psC->colseparator;
	m_spkClassListBox.SetColsWidth(m_psC->coltext, m_psC->colspikes, m_psC->colseparator);

	// init relation with document, display data, adjust parameters
	m_ChartDataWnd.SetScopeParameters(&(options_viewdata->viewdata));
	// set bincrflagonsave
	((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->SetCheck(p_app->options_viewspikes.bincrflagonsave);

	UpdateFileParameters(TRUE);
	if (m_baddspikemode)
	{
		GetParent()->PostMessage(WM_COMMAND, ID_VIEW_CURSORMODE_MEASURE, NULL);
		m_ChartDataWnd.SetTrackSpike(m_baddspikemode, m_pspkDP->extractNpoints, m_pspkDP->prethreshold, m_pspkDP->extractChan);
	}
}

void CViewSpikes::OnDestroy()
{
	CDaoRecordView::OnDestroy();
}

#ifdef _DEBUG
void CViewSpikes::AssertValid() const
{
	CDaoRecordView::AssertValid();
}

void CViewSpikes::Dump(CDumpContext& dc) const
{
	CDaoRecordView::Dump(dc);
}

CdbWaveDoc* CViewSpikes::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}

#endif //_DEBUG

CDaoRecordset* CViewSpikes::OnGetRecordset()
{
	return GetDocument()->GetDB_Recordset();
}

void CViewSpikes::UpdateFileParameters(BOOL bUpdateInterface) {
	UpdateSpikeFile(bUpdateInterface);
	UpdateDataFile(bUpdateInterface);
	UpdateLegends(bUpdateInterface);
}

void CViewSpikes::UpdateDataFile(BOOL bUpdateInterface) {
	//CString filename = GetDocument()->GetDB_CurrentDatFileName();
	m_pDataDoc = GetDocument()->OpenCurrentDataFile();
	if (m_pDataDoc == nullptr)
		return;
	
	m_ChartDataWnd.SetbUseDIB(FALSE);
	m_ChartDataWnd.AttachDataFile(m_pDataDoc);

	
	int isourceview = m_pSpkList->GetextractChan();
	if (isourceview >= m_pDataDoc->GetpWaveFormat()->scan_count)
	{
		m_pSpkList->SetextractChan(0);
		isourceview = 0;
	}
	if (m_pSpkList->GetdetectWhat() == DETECT_STIMULUS)
	{
		isourceview = m_pSpkList->GetdetectChan();
		if (isourceview >= m_pDataDoc->GetpWaveFormat()->scan_count)
		{
			m_pSpkList->SetdetectChan(0);
			isourceview = 0;
		}
	}

	// set detection channel
	if (m_ChartDataWnd.SetChanlistSourceChan(0, isourceview) < 0)
	{
		m_ChartDataWnd.RemoveAllChanlistItems();
	}
	else
	{
		m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.GetRectWidth(), m_lLast - m_lFirst);
		m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);

		if (m_bInitSourceView)
		{
			m_bInitSourceView = FALSE;
			int max, min;
			m_ChartDataWnd.GetChanlistMaxMin(0, &max, &min);
			const auto iextent = MulDiv(max - min + 1, 11, 10);
			const auto izero = (max + min) / 2;
			m_ChartDataWnd.SetChanlistYextent(0, iextent);
			m_ChartDataWnd.SetChanlistYzero(0, izero);
		}
	}
	
	if (bUpdateInterface)
	{
		m_ChartDataWnd.Invalidate();
	}

	m_DWintervals.SetSize(3 + 2);				// total size
	m_DWintervals.SetAt(0, 0);					// source channel
	m_DWintervals.SetAt(1, static_cast<DWORD>(RGB(255, 0, 0)));	// red color
	m_DWintervals.SetAt(2, 1);					// pen size
	m_DWintervals.SetAt(3, 0);					// pen size
	m_DWintervals.SetAt(4, 0);					// pen size
}

void CViewSpikes::UpdateSpikeFile(BOOL bUpdateInterface)
{
	m_pSpkDoc = GetDocument()->OpenCurrentSpikeFile();

	if (nullptr == m_pSpkDoc)
	{
		m_spkClassListBox.SetSourceData(nullptr, nullptr);
	}
	else
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->GetDB_CurrentSpkFileName(), FALSE);
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);

		int icur = GetDocument()->GetcurrentSpkDocument()->GetSpkList_CurrentIndex();
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(icur);
		m_pspkDP = m_pSpkList->GetDetectParms();

		m_spkClassListBox.SetSourceData(m_pSpkList, GetDocument());
		if (bUpdateInterface) {
			m_tabCtrl.SetCurSel(icur);
			// adjust Y zoom
			ASSERT(m_lFirst >= 0);
			if (m_bresetzoom)
			{
				m_spkClassListBox.SetRedraw(FALSE);
				ZoomOnPresetInterval(0);
				m_spkClassListBox.SetRedraw(TRUE);
			}
			else if (m_lLast > m_pSpkDoc->GetAcqSize() - 1 || m_lLast <= m_lFirst)
				m_lLast = m_pSpkDoc->GetAcqSize() - 1;	// clip to the end of the data

			m_spkClassListBox.SetTimeIntervals(m_lFirst, m_lLast);
			AdjustYZoomToMaxMin(false);
		}
	}

	// select row
	if (bUpdateInterface)
		m_spkClassListBox.SetCurSel(0);
}

void CViewSpikes::UpdateLegends(BOOL bUpdateInterface)
{
	if (!bUpdateInterface)
		return;

	if (m_lFirst < 0)
		m_lFirst = 0;
	if (m_lLast <= m_lFirst)
		m_lLast = m_lFirst + 120;
	if (m_lLast >= m_pSpkDoc->GetAcqSize())
		m_lLast = m_pSpkDoc->GetAcqSize() - 1;
	if (m_lFirst > m_lLast)
		m_lFirst = m_lLast - 120;

	// set cursor
	auto hwnd = GetSafeHwnd();
	if (!m_baddspikemode)
		hwnd = nullptr;
	m_ChartDataWnd.ReflectMouseMoveMessg(hwnd);
	m_spkClassListBox.ReflectBarsMouseMoveMessg(hwnd);
	m_ChartDataWnd.SetTrackSpike(m_baddspikemode, m_pspkDP->extractNpoints, m_pspkDP->prethreshold, m_pspkDP->extractChan);

	// update spike bars & forms CListBox
	if (m_lFirst != m_spkClassListBox.GetTimeFirst()
		|| m_lLast != m_spkClassListBox.GetTimeLast())
		m_spkClassListBox.SetTimeIntervals(m_lFirst, m_lLast);

	// update text abcissa and horizontal scroll position
	m_timefirst = m_lFirst / m_pSpkDoc->GetAcqRate();
	m_timelast = (m_lLast + 1) / m_pSpkDoc->GetAcqRate();
	m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);

	// update scrollbar and select spikes
	SelectSpike(m_spikeno);
	UpdateScrollBar();
}

void CViewSpikes::AdjustYZoomToMaxMin(BOOL bForceSearchMaxMin) {
	if (m_yWE == 1 || bForceSearchMaxMin)
	{
		// adjust gain of spkform
		int max, min;
		m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
		m_yWE = MulDiv(max - min + 1, 10, 8);
		m_yWO = (max  + min) / 2;
	}
	m_spkClassListBox.SetYzoom(m_yWE, m_yWO);
}

void CViewSpikes::SelectSpkList(int icursel)
{
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(icursel);
	ASSERT(m_pSpkList != NULL);

	m_spkClassListBox.SetSpkList(m_pSpkList);

	m_spkClassListBox.Invalidate();
	m_pspkDP = m_pSpkList->GetDetectParms();

	// update source data: change data channel and update display
	int isourceview = m_pSpkList->GetextractChan();
	ASSERT(isourceview == m_pspkDP->extractChan);
	if (m_pSpkList->GetdetectWhat() == DETECT_STIMULUS)
		isourceview = m_pSpkList->GetdetectChan();

	// no data available
	if (m_ChartDataWnd.SetChanlistSourceChan(0, isourceview) < 0)
	{
		m_ChartDataWnd.RemoveAllChanlistItems();
	}
	// data are ok
	else
	{
		m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.GetRectWidth(), m_lLast - m_lFirst);
		m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);
		int max, min;
		m_ChartDataWnd.GetChanlistMaxMin(0, &max, &min);
		const auto iextent = MulDiv(max - min + 1, 11, 10);
		const auto izero = (max + min) / 2;
		m_ChartDataWnd.SetChanlistYextent(0, iextent);
		m_ChartDataWnd.SetChanlistYzero(0, izero);
	}
	m_ChartDataWnd.Invalidate();
}

void CViewSpikes::SaveCurrentFileParms()
{
	if (m_pSpkDoc != nullptr && m_pSpkDoc->IsModified())
	{
		if (!m_pSpkList->IsClassListValid())	// if class list not valid:
			m_pSpkList->UpdateClassList();		// rebuild list of classes
		const auto spkfile_name = GetDocument()->GetDB_CurrentSpkFileName(FALSE);
		m_pSpkDoc->OnSaveDocument(spkfile_name);

		auto nclasses = 1;
		const auto  ntotalspikes = m_pSpkList->GetTotalSpikes();
		if (ntotalspikes > 0)
		{
			if (!m_pSpkList->IsClassListValid())		// if class list not valid:
				nclasses = m_pSpkList->UpdateClassList();
			else
				nclasses = m_pSpkList->GetNbclasses();
		}
		GetDocument()->SetDB_nbspikes(ntotalspikes);
		GetDocument()->SetDB_nbspikeclasses(nclasses);

		// change flag is button is checked
		if (((CButton*)GetDlgItem(IDC_INCREMENTFLAG))->GetCheck())
		{
			int flag = GetDocument()->GetDB_CurrentRecordFlag();
			flag++;
			GetDocument()->SetDB_CurrentRecordFlag(flag);
		}
	}
}

void CViewSpikes::OnFormatAlldata()
{
	// dots: spk file length
	m_lFirst = 0;
	m_lLast = m_pSpkDoc->GetAcqSize() - 1;
	// spikes: center spikes horizontally and adjust hz size of display
	const short x_wo = 0;
	const short x_we = m_pSpkList->GetSpikeLength();
	m_spkClassListBox.SetXzoom(x_we, x_wo);

	UpdateLegends(TRUE);
	// display data
	m_spkClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void CViewSpikes::OnFormatCentercurve()
{
	/*
	if (m_bSpkDocExists)
	{
		// loop over all spikes of the list
		int nspikes = m_pSpkList->GetTotalSpikes();
		CdbWaveApp* p_app = (CdbWaveApp*) AfxGetApp();		// get pointer to application
		SPKCLASSIF* m_psC= &(p_app->spkC);					// get address of spike classif parms
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
	if (m_pDataDoc != nullptr)
		m_ChartDataWnd.CenterChan(0);

	UpdateLegends(TRUE);
	m_spkClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void CViewSpikes::OnFormatGainadjust()
{
	if (m_pSpkDoc != nullptr)
	{
		AdjustYZoomToMaxMin(true);
	}
	if (m_pDataDoc != nullptr)
		m_ChartDataWnd.MaxgainChan(0);

	UpdateLegends(TRUE);
	m_spkClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void CViewSpikes::OnToolsEdittransformspikes()
{
	// return if no spike shape
	if (m_spkClassListBox.GetXWExtent() == 0)	// if no spike shape, return
		return;
	// save time frame to restore it on return
	const auto l_first = m_spkClassListBox.GetTimeFirst();
	const auto l_last = m_spkClassListBox.GetTimeLast();

	CSpikeEditDlg dlg;						// dialog box
	dlg.m_yextent = m_spkClassListBox.GetYWExtent();// load display parameters
	dlg.m_yzero = m_spkClassListBox.GetYWOrg();	// ordinates
	dlg.m_xextent = m_spkClassListBox.GetXWExtent();// and
	dlg.m_xzero = m_spkClassListBox.GetXWOrg();	// abcissa
	dlg.m_spikeno = m_spikeno;				// load index of selected spike
	dlg.m_parent = this;
	dlg.m_pdbWaveDoc = GetDocument();

	// open dialog box and wait for response
	dlg.DoModal();
	if (!dlg.m_bartefact)
		m_spikeno = dlg.m_spikeno;			// set no spike selected

	if (dlg.m_bchanged)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
		SaveCurrentFileParms();
		UpdateSpikeFile(TRUE);
	}
	m_lFirst = l_first;
	m_lLast = l_last;
	UpdateDataFile(TRUE);
	UpdateLegends(TRUE);
	// display data
	m_spkClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

//-----------------------------------------------------------------------
// compute printer's page dot resolution
// borrowed from sample\drawcli\drawdoc.cpp
//-----------------------------------------------------------------------

void CViewSpikes::PrintComputePageSize()
{
	// magic to get printer dialog that would be used if we were printing!
	CPrintDialog dlg(FALSE);
	VERIFY(AfxGetApp()->GetPrinterDeviceDefaults(&dlg.m_pd));

	// GetPrinterDC returns a HDC so attach it
	CDC dc;
	const auto h_dc = dlg.CreatePrinterDC();		// to delete at the end -- see doc!
	ASSERT(h_dc != NULL);
	dc.Attach(h_dc);

	// Get the size of the page in pixels
	options_viewdata->horzRes = dc.GetDeviceCaps(HORZRES);
	options_viewdata->vertRes = dc.GetDeviceCaps(VERTRES);

	// margins (pixels)
	m_printRect.right = options_viewdata->horzRes - options_viewdata->rightPageMargin;
	m_printRect.bottom = options_viewdata->vertRes - options_viewdata->bottomPageMargin;
	m_printRect.left = options_viewdata->leftPageMargin;
	m_printRect.top = options_viewdata->topPageMargin;
}

void CViewSpikes::PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo)
{
	auto t = CTime::GetCurrentTime();
	CString ch;
	ch.Format(_T("  page %d:%d %d-%d-%d"), // %d:%d",
		pInfo->m_nCurPage, pInfo->GetMaxPage(),
		t.GetDay(), t.GetMonth(), t.GetYear());
	const auto ch_date = GetDocument()->GetDB_CurrentSpkFileName();
	p_dc->SetTextAlign(TA_CENTER);
	p_dc->TextOut(options_viewdata->horzRes / 2, options_viewdata->vertRes - 57, ch_date);
}

CString CViewSpikes::PrintConvertFileIndex(long l_first, long l_last)
{
	CString cs_unit = _T(" s");
	TCHAR sz_value[64];
	const auto psz_value = sz_value;
	float x_scale_factor;
	auto x = PrintChangeUnit(
		static_cast<float>(l_first) / m_pSpkDoc->GetAcqRate(), &cs_unit, &x_scale_factor);
	auto fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.));
	wsprintf(psz_value, _T("time = %i.%03.3i - "), static_cast<int>(x), fraction);
	CString cs_comment = psz_value;

	x = l_last / (m_pSpkDoc->GetAcqRate() * x_scale_factor);	// same operations for last interval
	fraction = static_cast<int>((x - static_cast<int>(x)) * static_cast<float>(1000.));
	wsprintf(psz_value, _T("%i.%03.3i %s"), static_cast<int>(x), fraction, static_cast<LPCTSTR>(cs_unit));
	cs_comment += psz_value;
	return cs_comment;
}

long CViewSpikes::PrintGetFileSeriesIndexFromPage(int page, int* filenumber)
{
	auto l_first = m_lprintFirst;

	const auto maxrow = m_nbrowsperpage * page;	// row to get
	auto ifile = 0;						// file list index
	if (options_viewdata->bPrintSelection)			// current file if selection only
		ifile = m_file0;
	const auto current = GetDocument()->GetDB_CurrentRecordPosition();
	GetDocument()->SetDB_CurrentRecordPosition(ifile);
	auto very_last = GetDocument()->GetDB_DataLen() - 1;
	for (auto row = 0; row < maxrow; row++)
	{
		l_first += m_lprintLen;			// end of row
		if (l_first >= very_last)		// next file ?
		{
			ifile++;					// next file index
			if (ifile > m_nfiles)		// last file ??
			{
				ifile--;
				break;
			}
			// update end-of-file
			GetDocument()->DBMoveNext();
			very_last = GetDocument()->GetDB_DataLen() - 1;
			l_first = m_lprintFirst;
		}
	}
	*filenumber = ifile;	// return index / file list
	GetDocument()->SetDB_CurrentRecordPosition(current);
	return l_first;			// return index first point / data file
}

CString CViewSpikes::PrintGetFileInfos()
{
	CString str_comment;   					// scratch pad
	const CString tab("    ");					// use 4 spaces as tabulation character
	const CString rc("\n");						// next line
	const auto p_wave_format = &m_pSpkDoc->m_wformat;	// get data description

	// document's name, date and time
	if (options_viewdata->bDocName || options_viewdata->bAcqDateTime)// print doc infos?
	{
		if (options_viewdata->bDocName)					// print file name
			str_comment += GetDocument()->GetDB_CurrentSpkFileName(FALSE) + tab;
		if (options_viewdata->bAcqDateTime)				// print data acquisition date & time
		{
			auto acqtime = m_pSpkDoc->GetAcqTime();
			const auto date = acqtime.Format(_T("%#d %m %Y %X")); //("%x %X");
			// or more explicitely %d-%b-%Y %H:%M:%S");
			str_comment += date;
		}
		str_comment += rc;
	}

	// document's main comment (print on multiple lines if necessary)
	if (options_viewdata->bAcqComment)
	{
		str_comment += p_wave_format->GetComments(_T(" "));// cs_comment
		str_comment += rc;
	}

	return str_comment;
}

CString CViewSpikes::PrintBars(CDC* p_dc, CRect* rect)
{
	CString str_comment;
	const CString rc(_T("\n"));
	CString tab(_T("     "));
	const auto p_old_brush = (CBrush*)p_dc->SelectStockObject(BLACK_BRUSH);

	CString cs_unit;
	CRect rect_horz_bar;
	CRect rect_vert_bar;
	const CPoint bar_origin(-10, -10);	// origine barre à 10,10 pts de coin inf gauche rectangle
	const CSize  bar_size(5, 5);		// épaisseur barre

	// same len ratio as displayed on viewdata
	const auto horz_bar = 100;
	const auto vert_bar = 100;

	///// time abcissa ///////////////////////////
	const int iifirst = m_spkClassListBox.GetTimeFirst();
	const int iilast = m_spkClassListBox.GetTimeLast();
	auto cs_comment = PrintConvertFileIndex(iifirst, iilast);

	///// horizontal time bar ///////////////////////////
	if (options_viewdata->bTimeScaleBar)
	{
		// print horizontal bar
		rect_horz_bar.left = rect->left + bar_origin.x;
		rect_horz_bar.right = rect_horz_bar.left + horz_bar;
		rect_horz_bar.top = rect->bottom - bar_origin.y;
		rect_horz_bar.bottom = rect_horz_bar.top - bar_size.cy;
		p_dc->Rectangle(&rect_horz_bar);
		//get time equivalent of bar length
		const auto iibar = MulDiv(iilast - iifirst, rect_horz_bar.Width(), rect->Width());
		const auto xbar = float(iibar) / m_pSpkDoc->GetAcqRate();
		CString cs;
		cs.Format(_T("\nbar = %f s"), xbar);
		cs_comment += cs;
		str_comment += cs_comment + rc;
	}

	///// vertical voltage bars ///////////////////////////
	if (options_viewdata->bVoltageScaleBar)
	{
		rect_vert_bar.left = rect->left + bar_origin.x;
		rect_vert_bar.right = rect_vert_bar.left - bar_size.cx;
		rect_vert_bar.bottom = rect->bottom - bar_origin.y;
		rect_vert_bar.top = rect_vert_bar.bottom - vert_bar;
		p_dc->Rectangle(&rect_vert_bar);
	}

	// comments, bar value and chan settings for each channel
	if (options_viewdata->bChansComment || options_viewdata->bVoltageScaleBar || options_viewdata->bChanSettings)
	{
		/*
				int imax = m_sourceView.GetChanlistSize();	// number of data channels
				for (int ichan=0; ichan< imax; ichan++)		// loop
				{
					// boucler sur les commentaires de chan n a chan 0...
					wsprintf(lpszVal, "chan#%i ", ichan);	// channel number
					cs_comment = lpszVal;
					if (mdPM->bVoltageScaleBar)				// bar scale value
					{
						csUnit = " V";						// provisional unit
						float z= 	(float) m_yscalebar.GetRectHeight()* m_sourceView.GetChanlistVoltsperPixel(ichan);
						float x = PrintChangeUnit(z, &csUnit, &x_scale_factor); // convert

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
							z = (float) k * x_scale_factor;
							j = (int) PrintChangeUnit(z, &csUnit, &x_scale_factor); // convert
						}
						wsprintf(sz_value, "bar = %i %s ", j, csUnit);	// store val into comment
						cs_comment += sz_value;
					}
					str_comment += cs_comment;

					// print chan comment
					if (mdPM->bChansComment)
					{
						str_comment += tab;
						str_comment += m_sourceView.GetChanlistComment(ichan);
					}
					str_comment += rc;

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
						str_comment += cs;
						str_comment += rc;
					}
				}
		*/
	}
	p_dc->SelectObject(p_old_brush);
	return str_comment;
}

BOOL CViewSpikes::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (!CView::DoPreparePrinting(pInfo))
		return FALSE;

	if (!COleDocObjectItem::OnPreparePrinting(this, pInfo))
		return FALSE;

	// save current state of the windows
	auto p_app = (CdbWaveApp*)AfxGetApp();
	if (p_app->m_pviewspikesMemFile == nullptr)
	{
		p_app->m_pviewspikesMemFile = new CMemFile;
		ASSERT(p_app->m_pviewspikesMemFile != NULL);
	}

	CArchive ar(p_app->m_pviewspikesMemFile, CArchive::store);
	p_app->m_pviewspikesMemFile->SeekToBegin();
	m_ChartDataWnd.Serialize(ar);
	//spk_bar_wnd_.Serialize(ar);
	//m_spkShapeView.Serialize(ar);
	ar.Close();					// close archive

	// printing margins
	if (options_viewdata->vertRes <= 0 || options_viewdata->horzRes <= 0
		|| options_viewdata->horzRes != pInfo->m_rectDraw.Width()
		|| options_viewdata->vertRes != pInfo->m_rectDraw.Height())
		PrintComputePageSize();

	// how many rows per page?
	const auto size_row = options_viewdata->HeightDoc + options_viewdata->heightSeparator;
	m_nbrowsperpage = m_printRect.Height() / size_row;
	if (m_nbrowsperpage == 0)			// prevent zero pages
		m_nbrowsperpage = 1;

	// compute number of rows according to multirow flag
	m_lprintFirst = m_spkClassListBox.GetTimeFirst();
	m_lprintLen = m_spkClassListBox.GetTimeLast() - m_lprintFirst + 1;

	// make sure the number of classes per file is known
	auto nnclasses = 0;			// store sum (nclasses from file (i=ifile0, ifile1))
	auto p_dbwave_doc = GetDocument();
	m_file0 = p_dbwave_doc->GetDB_CurrentRecordPosition();
	ASSERT(m_file0 >= 0);
	m_printFirst = m_file0;
	m_printLast = m_file0;
	m_nfiles = 1;

	if (!options_viewdata->bPrintSelection)
	{
		m_printFirst = 0;
		m_nfiles = p_dbwave_doc->GetDB_NRecords();
		m_printLast = m_nfiles - 1;
	}

	// update the nb of classes per file selected and add this number
	m_maxclasses = 1;
	p_dbwave_doc->SetDB_CurrentRecordPosition(m_printFirst);
	auto nbrect = 0;										// total nb of rows
	for (auto i = m_printFirst; i <= m_printLast; i++, p_dbwave_doc->DBMoveNext())
	{
		// get number of classes
		if (p_dbwave_doc->GetDB_nbspikeclasses() <= 0)
		{
			m_pSpkDoc = p_dbwave_doc->OpenCurrentSpikeFile();
			m_pSpkList = m_pSpkDoc->GetSpkList_Current();
			if (!m_pSpkList->IsClassListValid())	// if class list not valid:
			{
				m_pSpkList->UpdateClassList();		// rebuild list of classes
				m_pSpkDoc->SetModifiedFlag();		// and set modified flag
			}

			int nclasses = 1;
			if (m_pSpkList->GetTotalSpikes() > 0)
				nclasses = m_pSpkList->GetNbclasses();
			ASSERT(nclasses > 0);
			p_dbwave_doc->SetDB_nbspikeclasses(nclasses);
			nnclasses += nclasses;
		}

		if (p_dbwave_doc->GetDB_nbspikeclasses() > m_maxclasses)
			m_maxclasses = p_dbwave_doc->GetDB_nbspikeclasses();

		if (options_viewdata->bMultirowDisplay)
		{
			const auto len = p_dbwave_doc->GetDB_DataLen() - m_lprintFirst;// file length
			auto nrows = len / m_lprintLen;	// how many rows for this file?
			if (len > nrows * m_lprintLen)	// remainder?
				nrows++;
			nbrect += static_cast<int>(nrows);	// update nb of rows
		}
	}

	// multiple rows?
	if (!options_viewdata->bMultirowDisplay)
		nbrect = m_nfiles;

	// npages
	auto npages = nbrect / m_nbrowsperpage;
	if (nbrect > m_nbrowsperpage * npages)
		npages++;

	//------------------------------------------------------
	pInfo->SetMaxPage(npages);			//one page printing/preview
	pInfo->m_nNumPreviewPages = 1;  	// preview 1 pages at a time
	pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION; // allow print only selection

	if (options_viewdata->bPrintSelection)
		pInfo->m_pPD->m_pd.Flags |= PD_SELECTION;	// set button to selection

	// call dialog box
	const auto flag = DoPreparePrinting(pInfo);
	// set max nb of pages according to selection
	options_viewdata->bPrintSelection = pInfo->m_pPD->PrintSelection();
	if (options_viewdata->bPrintSelection)
	{
		npages = 1;
		m_nfiles = 1;
		if (options_viewdata->bMultirowDisplay)
		{
			const auto l_first0 = m_spkClassListBox.GetTimeFirst();
			const auto l_last0 = m_spkClassListBox.GetTimeLast();
			const auto len = m_pSpkDoc->GetAcqSize() - l_first0;
			nbrect = len / (l_last0 - l_first0);
			if (nbrect * (l_last0 - l_first0) < len)
				nbrect++;

			npages = nbrect / m_nbrowsperpage;
			if (npages * m_nbrowsperpage < nbrect)
				npages++;
		}
		pInfo->SetMaxPage(npages);
	}

	p_dbwave_doc->SetDB_CurrentRecordPosition(m_file0);
	return flag;
}

void CViewSpikes::OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_bIsPrinting = TRUE;
	m_lFirst0 = m_spkClassListBox.GetTimeFirst();
	m_lLast0 = m_spkClassListBox.GetTimeLast();

	//---------------------init objects-------------------------------------
	memset(&m_logFont, 0, sizeof(LOGFONT));					// prepare font
	lstrcpy(m_logFont.lfFaceName, _T("Arial"));				// Arial font
	m_logFont.lfHeight = options_viewdata->fontsize;		// font height
	m_pOldFont = nullptr;
	m_fontPrint.CreateFontIndirect(&m_logFont);
	p_dc->SetBkMode(TRANSPARENT);
}

void CViewSpikes::OnPrint(CDC* p_dc, CPrintInfo* pInfo)
{
	m_pOldFont = p_dc->SelectObject(&m_fontPrint);
	p_dc->SetMapMode(MM_TEXT);								// (1 pixel = 1 logical point)
	PrintFileBottomPage(p_dc, pInfo);						// print bottom - text, date, etc
	const int curpage = pInfo->m_nCurPage;					// get current page number

	// --------------------- load data corresponding to the first row of current page

	// print only current selection - transform current page into file index
	int filenumber;			   								// file number and file index
	auto l_first = PrintGetFileSeriesIndexFromPage(curpage - 1, &filenumber);
	GetDocument()->SetDB_CurrentRecordPosition(filenumber);
	UpdateFileParameters(FALSE);
	UpdateScrollBar();
	auto very_last = m_pSpkDoc->GetAcqSize() - 1;			// index last data point / current file

	CRect r_where(m_printRect.left, 						// left
		m_printRect.top,									// top
		m_printRect.left + options_viewdata->WidthDoc,		// right ( and bottom next line)
		m_printRect.top + options_viewdata->HeightDoc);

	// loop through all files	--------------------------------------------------------
	for (int i = 0; i < m_nbrowsperpage; i++)
	{
		// save conditions (Save/RestoreDC is mandatory!) --------------------------------

		const auto old_dc = p_dc->SaveDC();					// save DC

		// set first rectangle where data will be printed

		auto comment_rect = r_where;						// save RWhere for comments
		p_dc->SetMapMode(MM_TEXT);							// 1 pixel = 1 logical unit
		p_dc->SetTextAlign(TA_LEFT); 						// set text align mode
		if (options_viewdata->bFrameRect)							// print rectangle if necessary
		{
			p_dc->MoveTo(r_where.left, r_where.top);
			p_dc->LineTo(r_where.right, r_where.top);		// top hz
			p_dc->LineTo(r_where.right, r_where.bottom);	// right vert
			p_dc->LineTo(r_where.left, r_where.bottom);		// bottom hz
			p_dc->LineTo(r_where.left, r_where.top);		// left vert
		}
		p_dc->SetViewportOrg(r_where.left, r_where.top);

		// load data and adjust display rectangle ----------------------------------------
		// reduce width to the size of the data

		auto rw2 = r_where;									// printing rectangle - constant
		rw2.OffsetRect(-r_where.left, -r_where.top);		// set RW2 origin = 0,0
		auto rheight = rw2.Height() / m_maxclasses;			// ncount;
		if (m_pDataDoc != nullptr)
			rheight = rw2.Height() / (m_maxclasses + 1);
		const auto rseparator = rheight / 8;
		const auto rcol = rw2.Width() / 8;

		// rectangles for the 3 categories of data
		auto r_wtext = rw2;
		auto rw_spikes = rw2;
		auto r_wbars = rw2;

		// horizontal size and position of the 3 rectangles

		r_wtext.right = r_wtext.left + rcol;
		rw_spikes.left = r_wtext.right + rseparator;
		//auto n = m_pSpkDoc->GetSpkListCurrent()->GetSpikeLength();
		if (m_pSpkDoc->GetSpkList_Current()->GetSpikeLength() > 1)
			rw_spikes.right = rw_spikes.left + rcol;
		else
			rw_spikes.right = rw_spikes.left;
		r_wbars.left = rw_spikes.right + rseparator;

		// bottom of the first rectangle
		r_wbars.bottom = rw2.top + rheight;
		auto l_last = l_first + m_lprintLen;			// compute last pt to load
		if (l_last > very_last)							// check end across file length
			l_last = very_last;
		if ((l_last - l_first + 1) < m_lprintLen)				// adjust rect to length of data
		{
			r_wbars.right = MulDiv(r_wbars.Width(), l_last - l_first, m_lprintLen)
				+ r_wbars.left;
			ASSERT(r_wbars.right > r_wbars.left);
		}

		// ------------------------ print data

		auto iextent = m_spkClassListBox.GetYWExtent();			// get current extents
		//auto izero = m_spkClass.GetYWOrg();

		if (m_pDataDoc != nullptr)
		{
			if (options_viewdata->bClipRect)						// clip curve display ?
				p_dc->IntersectClipRect(&r_wbars);		// yes
			m_ChartDataWnd.GetDataFromDoc(l_first, l_last);	// load data from file
			m_ChartDataWnd.CenterChan(0);
			m_ChartDataWnd.Print(p_dc, &r_wbars);			// print data
			p_dc->SelectClipRgn(nullptr);

			iextent = m_ChartDataWnd.GetChanlistYextent(0);
			r_wbars.top = r_wbars.bottom;
		}

		// ------------------------ print spikes

		r_wbars.bottom = r_wbars.top + rheight;			// adjust the rect position
		rw_spikes.top = r_wbars.top;
		rw_spikes.bottom = r_wbars.bottom;
		r_wtext.top = r_wbars.top;
		r_wtext.bottom = r_wbars.bottom;

		//m_pSpkList = m_pSpkDoc->GetSpkList_Current();
		//AdjustYZoomToMaxMin(true); 
		int max, min;
		m_pSpkDoc->GetSpkList_Current()->GetTotalMaxMin(TRUE, &max, &min);
		const short middle = max / 2 + min / 2;
		m_spkClassListBox.SetYzoom(iextent, middle);
		const auto ncount = m_spkClassListBox.GetCount();				// get nb of items in this file

		for (auto icount = 0; icount < ncount; icount++)
		{
			m_spkClassListBox.SetTimeIntervals(l_first, l_last);
			m_spkClassListBox.PrintItem(p_dc, &r_wtext, &rw_spikes, &r_wbars, icount);
			r_wtext.OffsetRect(0, rheight);
			rw_spikes.OffsetRect(0, rheight);
			r_wbars.OffsetRect(0, rheight);
		}

		// ------------------------ print stimulus

		if (m_pSpkDoc->m_stimIntervals.nitems > 0)
		{
			CBrush bluebrush;	// create and select a solid blue brush
			bluebrush.CreateSolidBrush(RGB(0, 0, 255));
			const auto oldb = (CBrush*)p_dc->SelectObject(&bluebrush);

			CPen bluepen;		// create and select a solid blue pen
			bluepen.CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
			const auto ppen = (CPen*)p_dc->SelectObject(&bluepen);

			rw_spikes.bottom = rw2.bottom;	// define a rectangle for stimulus
			rw_spikes.top = rw2.bottom - rheight / 10;

			if (rw_spikes.top == rw_spikes.bottom)
				rw_spikes.bottom++;

			for (auto ii = 0; ii < m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize(); ii++, ii++)
			{
				int iifirst = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(ii);
				if ((ii + 1) >= m_pSpkDoc->m_stimIntervals.intervalsArray.GetSize())
					continue;
				int iilast = m_pSpkDoc->m_stimIntervals.intervalsArray.GetAt(ii + 1);
				if (iifirst > l_last || iilast < l_first)
					continue;
				if (iifirst < l_first)
					iifirst = l_first;
				if (iilast > l_last)
					iilast = l_last;

				rw_spikes.left = MulDiv(iifirst - l_first, r_wbars.Width(), l_last - l_first) + r_wbars.left;
				rw_spikes.right = MulDiv(iilast - l_first, r_wbars.Width(), l_last - l_first) + r_wbars.left;
				if (rw_spikes.right <= rw_spikes.left)
					rw_spikes.right = rw_spikes.left + 1;
				p_dc->Rectangle(rw_spikes);
			}

			p_dc->SelectObject(oldb);
			p_dc->SelectObject(ppen);
		}

		// ------------------------ print stimulus

		// update display rectangle for next row
		r_where.OffsetRect(0, options_viewdata->HeightDoc + options_viewdata->heightSeparator);

		// restore DC ------------------------------------------------------

		p_dc->RestoreDC(old_dc);					// restore Display context

		// print comments --------------------------------------------------

		p_dc->SetMapMode(MM_TEXT);				// 1 LP = 1 pixel
		p_dc->SelectClipRgn(nullptr);				// no more clipping
		p_dc->SetViewportOrg(0, 0);				// org = 0,0

		// print comments according to row within file
		CString cs_comment;
		if (l_first == m_lprintFirst)			// first row = full comment
		{
			cs_comment += PrintGetFileInfos();
			cs_comment += PrintBars(p_dc, &comment_rect);// bars and bar legends
		}

		// print comments stored into cs_comment
		comment_rect.OffsetRect(options_viewdata->textseparator + comment_rect.Width(), 0);
		comment_rect.right = m_printRect.right;

		// reset text align mode (otherwise pbs!) output text and restore text alignment
		const auto ui_flag = p_dc->SetTextAlign(TA_LEFT | TA_NOUPDATECP);
		p_dc->DrawText(cs_comment, cs_comment.GetLength(), comment_rect,
			DT_NOPREFIX | /*DT_NOCLIP |*/ DT_LEFT | DT_WORDBREAK);
		p_dc->SetTextAlign(ui_flag);

		// update file parameters for next row --------------------------------------------
		l_first += m_lprintLen;
		// next file?
		// if index next point is past the end of the file
		// OR not entire record and not multirow display
		if ((l_first >= very_last)
			|| (!options_viewdata->bEntireRecord &&
				!options_viewdata->bMultirowDisplay))
		{
			filenumber++;						// next index
			if (filenumber < m_nfiles)			// last file ??
			{									// NO: select new file
				GetDocument()->DBMoveNext();
				UpdateFileParameters(FALSE);
				UpdateScrollBar();
				very_last = m_pSpkDoc->GetAcqSize() - 1;
			}
			else
				i = m_nbrowsperpage;			// YES: break
			l_first = m_lprintFirst;
		}
	}	// this is the end of a very long for loop.....................

	if (m_pOldFont != nullptr)
		p_dc->SelectObject(m_pOldFont);
}

void CViewSpikes::OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo)
{
	m_fontPrint.DeleteObject();
	m_bIsPrinting = FALSE;

	GetDocument()->SetDB_CurrentRecordPosition(m_file0);
	UpdateFileParameters(TRUE);
	m_spkClassListBox.SetTimeIntervals(m_lFirst0, m_lLast0);
	m_spkClassListBox.Invalidate();

	auto p_app = (CdbWaveApp*)AfxGetApp();
	if (p_app->m_pviewspikesMemFile != nullptr)
	{
		CArchive ar(p_app->m_pviewspikesMemFile, CArchive::load);
		p_app->m_pviewspikesMemFile->SeekToBegin();
		m_ChartDataWnd.Serialize(ar);
		ar.Close();					// close archive
	}
}

// --------------------------------------------------------------------------
// (where could we store these values???)
// --------------------------------------------------------------------------
static char vs_units[] = { "GM  mµpf  " };		// units & corresp powers
static int  vs_units_power[] = { 9,6, 0, 0, -3, -6, -9, -12, 0 };
static int	vsmax_index = 8;				// nb of elmts
//static int  VsniceIntervals[] = {1, 5,
//						10,  20,  25,  30,  40, 50,
//						100, 200, 250, 300, 400, 500,  /*600,*/ /*700,*//* 800, *//*900,*/
//						0};

float CViewSpikes::PrintChangeUnit(float xVal, CString* xUnit, float* xScalefactor)
{
	if (xVal == 0)	// prevent diviion by zero
	{
		*xScalefactor = 1.0f;
		return 0.0f;
	}

	short	i;
	short isign = 1;
	if (xVal < 0)
	{
		isign = -1;
		xVal = -xVal;
	}
	const auto iprec = static_cast<short>(log10(xVal));		// log10 of value (upper limit)     floor ceil scPower
	if ((iprec <= 0) && (xVal < 1.))	// perform extra checking if iprec <= 0
		i = 4 - iprec / 3;					// change equation if Units values change!
	else
		i = 3 - iprec / 3;					// change equation if Units values change!
	if (i > vsmax_index)					// clip to max index
		i = vsmax_index;
	else if (i < 0)						// or clip to min index
		i = 0;
	*xScalefactor = static_cast<float>(pow(10.0f, vs_units_power[i]));	// convert & store
	xUnit->SetAt(0, vs_units[i]);       // replace character corresp to unit
	return xVal * isign / *xScalefactor;	// return value/scale_factor
}

////////////////////////////////////////////////////////////////////////

void CViewSpikes::OnEnChangeNOspike()
{
	if (mm_spikeno.m_bEntryDone)
	{
		const auto spikeno = m_spikeno;

		switch (mm_spikeno.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_spikeno = m_pSpkList->GetNextSpike(spikeno, 1, m_bKeepSameClass);	break;
		case VK_DOWN:
		case VK_NEXT:   m_spikeno = m_pSpkList->GetNextSpike(spikeno , -1, m_bKeepSameClass);	break;
		default:;
		}

		// check boundaries
		m_spikeno = m_pSpkList->GetValidSpikeNumber(m_spikeno);

		mm_spikeno.m_bEntryDone = FALSE;	// clear flag
		mm_spikeno.m_nChar = 0;			// empty buffer
		mm_spikeno.SetSel(0, -1);		// select all text

		if (m_spikeno != spikeno)		// change display if necessary
		{
			SelectSpike(m_spikeno);
			if (m_spikeno >= 0)
			{
				// test if spike visible in the current time interval
				const auto p_spike_element = m_pSpkList->GetSpikeElemt(m_spikeno);
				const auto spk_first = p_spike_element->get_time() - m_pSpkList->GetSpikePretrig();
				const auto spk_last = spk_first + m_pSpkList->GetSpikeLength();
				const auto lcenter = (spk_last + spk_first) / 2;

				if (spk_first < m_lFirst || spk_last > m_lLast)
				{
					const long lspan = (m_lLast - m_lFirst) / 2;
					m_lFirst = lcenter - lspan;
					m_lLast = lcenter + lspan;
					UpdateLegends(TRUE);
				}
				// center curve vertically
				const auto ixpixel = MulDiv(lcenter - m_lFirst, m_ChartDataWnd.GetNxPixels(), m_lLast - m_lFirst);
				const auto ival = m_ChartDataWnd.GetChanlistBinAt(0, ixpixel);
				m_ChartDataWnd.SetChanlistYzero(0, ival);
				// display data
				m_spkClassListBox.Invalidate();
				m_ChartDataWnd.Invalidate();
			}
		}
		else
			UpdateData(FALSE);
	}
}

void CViewSpikes::OnEnChangeSpikenoclass()
{
	if (!mm_spikenoclass.m_bEntryDone)
		return;
	const auto spikenoclass = m_spikenoclass;
	switch (mm_spikenoclass.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:	UpdateData(TRUE);	break;
	case VK_UP:
	case VK_PRIOR:	m_spikenoclass++;	break;
	case VK_DOWN:
	case VK_NEXT:   m_spikenoclass--;	break;
	default:;
	}

	mm_spikenoclass.m_bEntryDone = FALSE;	// clear flag
	mm_spikenoclass.m_nChar = 0;			// empty buffer
	mm_spikenoclass.SetSel(0, -1);		// select all text

	if (m_spikenoclass != spikenoclass)	// change display if necessary
	{
		m_spkClassListBox.ChangeSpikeClass(m_spikeno, m_spikenoclass);
		m_pSpkDoc->SetModifiedFlag(TRUE);
		UpdateLegends(TRUE);
		m_spkClassListBox.Invalidate();
		m_ChartDataWnd.Invalidate();
	}
}

void CViewSpikes::OnEnChangeTimefirst()
{
	if (mm_timefirst.m_bEntryDone)
	{
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
		default:;
		}

		mm_timefirst.m_bEntryDone = FALSE;
		mm_timefirst.m_nChar = 0;
		mm_timefirst.SetSel(0, -1); 	//select all text

		const auto l_first = static_cast<long>(m_timefirst * m_pSpkDoc->GetAcqRate());
		if (l_first != m_lFirst)
		{
			m_lFirst = l_first;
			UpdateLegends(TRUE);
			m_spkClassListBox.Invalidate();
			m_ChartDataWnd.Invalidate();
		}
	}
}

void CViewSpikes::OnEnChangeTimelast()
{
	if (mm_timelast.m_bEntryDone)
	{
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
		default:;
		}
		mm_timelast.m_bEntryDone = FALSE;
		mm_timelast.m_nChar = 0;
		mm_timelast.SetSel(0, -1);

		const auto l_last = static_cast<long>(m_timelast * m_pSpkDoc->GetAcqRate());
		if (l_last != m_lLast)
		{
			m_lLast = l_last;
			UpdateLegends(TRUE);
			m_spkClassListBox.Invalidate();
			m_ChartDataWnd.Invalidate();
		}
	}
}

void CViewSpikes::OnEnChangeZoom()
{
	if (mm_zoom.m_bEntryDone)
	{
		const auto zoom = m_zoom;
		switch (mm_zoom.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_zoom++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_zoom--;	break;
		default:;
		}

		// check boundaries
		if (m_zoom < 0.0f)
			m_zoom = 1.0f;

		mm_zoom.m_bEntryDone = FALSE;	// clear flag
		mm_zoom.m_nChar = 0;			// empty buffer
		mm_zoom.SetSel(0, -1);			// select all text
		if (m_zoom != zoom)				// change display if necessary
			ZoomOnPresetInterval(0);
		else
			UpdateData(FALSE);
	}
}

void CViewSpikes::OnEnChangeSourceclass()
{
	if (mm_sourceclass.m_bEntryDone)
	{
		switch (mm_sourceclass.m_nChar)
		{
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_sourceclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_sourceclass--;	break;
		default:;
		}

		// check boundaries
		mm_sourceclass.m_bEntryDone = FALSE;	// clear flag
		mm_sourceclass.m_nChar = 0;			// empty buffer
		mm_sourceclass.SetSel(0, -1);		// select all text
		UpdateData(FALSE);
	}
}

void CViewSpikes::OnEnChangeDestclass()
{
	if (mm_destclass.m_bEntryDone)
	{
		switch (mm_destclass.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE); break;
		case VK_UP:
		case VK_PRIOR:	m_destclass++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_destclass--;	break;
		default:;
		}

		// check boundaries
		mm_destclass.m_bEntryDone = FALSE;	// clear flag
		mm_destclass.m_nChar = 0;			// empty buffer
		mm_destclass.SetSel(0, -1);		// select all text
		UpdateData(FALSE);
	}
}

void CViewSpikes::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// get corresponding data
	const auto total_scroll = m_pSpkDoc->GetAcqSize();
	const long page_scroll = (m_lLast - m_lFirst);
	long sb_scroll = MulDiv(page_scroll, 10, 100);
	if (sb_scroll == 0)
		sb_scroll = 1;
	long l_first = m_lFirst;

	switch (nSBCode)
	{
	case SB_LEFT:
		l_first = 0;	break;			// Scroll to far left.
	case SB_LINELEFT:
		l_first -= sb_scroll;	break;	// Scroll left.
	case SB_LINERIGHT:
		l_first += sb_scroll; break;	// Scroll right
	case SB_PAGELEFT:
		l_first -= page_scroll; break;// Scroll one page left
	case SB_PAGERIGHT:
		l_first += page_scroll; break;// Scroll one page right.
	case SB_RIGHT:
		l_first = total_scroll - page_scroll + 1;
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
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
		UpdateLegends(TRUE);
		m_spkClassListBox.Invalidate();
		m_ChartDataWnd.Invalidate();
	}
	else
		UpdateScrollBar();
	if (m_pDataDoc != nullptr)
		m_ChartDataWnd.CenterChan(0);
}

void CViewSpikes::UpdateScrollBar()
{
	m_scrollFilePos_infos.fMask = SIF_ALL;
	m_scrollFilePos_infos.nMin = 0;
	m_scrollFilePos_infos.nMax = m_pSpkDoc->GetAcqSize() - 1;
	m_scrollFilePos_infos.nPos = m_lFirst;
	m_scrollFilePos_infos.nPage = m_lLast - m_lFirst;
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR1))->SetScrollInfo(&m_scrollFilePos_infos);
}

// --------------------------------------------------------------------------

void CViewSpikes::OnEditCopy()
{
	CCopyAsDlg dlg;
	dlg.m_nabcissa = options_viewdata->hzResolution;
	dlg.m_nordinates = options_viewdata->vtResolution;
	dlg.m_bgraphics = options_viewdata->bgraphics;
	dlg.m_ioption = options_viewdata->bcontours;
	dlg.m_iunit = options_viewdata->bunits;

	// invoke dialog box
	if (IDOK == dlg.DoModal())
	{
		options_viewdata->bgraphics = dlg.m_bgraphics;
		options_viewdata->bcontours = dlg.m_ioption;
		options_viewdata->bunits = dlg.m_iunit;
		options_viewdata->hzResolution = dlg.m_nabcissa;
		options_viewdata->vtResolution = dlg.m_nordinates;

		// output rectangle requested by user
		CRect rect(0, 0, options_viewdata->hzResolution, options_viewdata->vtResolution);

		// create metafile
		CMetaFileDC mDC;
		auto rect_bound = rect;
		rect_bound.right *= 32;
		rect_bound.bottom *= 30;
		auto p_dc_ref = GetDC();
		const auto old_dc = p_dc_ref->SaveDC();		// save DC

		auto cs_title = _T("dbWave\0") + GetDocument()->GetTitle();
		cs_title += _T("\0\0");
		const auto hm_dc = mDC.CreateEnhanced(p_dc_ref, nullptr, &rect_bound, cs_title);
		ASSERT(hm_dc != NULL);

		// Draw document in metafile.
		CPen black_pen(PS_SOLID, 0, RGB(0, 0, 0));
		const auto poldpen = mDC.SelectObject(&black_pen);
		if (!(CBrush*)mDC.SelectStockObject(BLACK_BRUSH))
			return;
		CClientDC attrib_dc(this);					// Create and attach attribute DC
		mDC.SetAttribDC(attrib_dc.GetSafeHdc());	// from current screen

		// print comments : set font
		m_pOldFont = nullptr;
		const auto oldsize = options_viewdata->fontsize;
		options_viewdata->fontsize = 10;
		memset(&m_logFont, 0, sizeof(LOGFONT));		// prepare font
		lstrcpy(m_logFont.lfFaceName, _T("Arial"));		// Arial font
		m_logFont.lfHeight = options_viewdata->fontsize;		// font height
		m_pOldFont = nullptr;
		m_fontPrint.CreateFontIndirect(&m_logFont);
		mDC.SetBkMode(TRANSPARENT);

		options_viewdata->fontsize = oldsize;
		m_pOldFont = mDC.SelectObject(&m_fontPrint);

		CString comments;
		// display data: source data and spikes
		auto iextent = m_spkClassListBox.GetYWExtent();			// get current extents
		const auto rheight = MulDiv(m_spkClassListBox.GetRowHeight(), rect.Width(),
			m_spkClassListBox.GetColsTimeWidth());
		auto rw_spikes = rect;							// adjust rect size
		rw_spikes.bottom = rheight;						// height ratio like screen
		auto rw_text = rw_spikes;
		auto rw_bars = rw_spikes;
		// horizontal size and position of the 3 rectangles
		const auto rseparator = rheight / 5;
		rw_text.right = rw_text.left + rheight;
		rw_spikes.left = rw_text.right + rseparator;
		rw_spikes.right = rw_spikes.left + rheight;
		rw_bars.left = rw_spikes.right + rseparator;

		// display data	if data file was found
		if (m_pDataDoc != nullptr)
		{
			m_ChartDataWnd.CenterChan(0);
			m_ChartDataWnd.Print(&mDC, &rw_bars);

			iextent = m_ChartDataWnd.GetChanlistYextent(0);
			rw_spikes.OffsetRect(0, rheight);
			rw_bars.OffsetRect(0, rheight);
			rw_text.OffsetRect(0, rheight);
		}

		// display spikes and bars
		AdjustYZoomToMaxMin(true);
		const auto ncount = m_spkClassListBox.GetCount();				// get nb of items in this file

		for (int icount = 0; icount < ncount; icount++)
		{
			m_spkClassListBox.PrintItem(&mDC, &rw_text, &rw_spikes, &rw_bars, icount);
			rw_spikes.OffsetRect(0, rheight);
			rw_bars.OffsetRect(0, rheight);
			rw_text.OffsetRect(0, rheight);
		}

		if (m_pOldFont != nullptr)
			mDC.SelectObject(m_pOldFont);
		m_fontPrint.DeleteObject();

		// restore oldpen
		mDC.SelectObject(poldpen);
		ReleaseDC(p_dc_ref);

		// Close metafile
		const auto h_emf_tmp = mDC.CloseEnhanced();
		ASSERT(h_emf_tmp != NULL);
		if (OpenClipboard())
		{
			EmptyClipboard();							// prepare clipboard
			SetClipboardData(CF_ENHMETAFILE, h_emf_tmp);	// put data
			CloseClipboard();							// close clipboard
		}
		else
		{
			// Someone else has the Clipboard open...
			DeleteEnhMetaFile(h_emf_tmp);					// delete data
			MessageBeep(0);							// tell user something is wrong!
			AfxMessageBox(IDS_CANNOT_ACCESS_CLIPBOARD, NULL, MB_OK | MB_ICONEXCLAMATION);
		}

		// restore initial conditions
		p_dc_ref->RestoreDC(old_dc);		// restore Display context
	}

	// restore screen in previous state
	UpdateSpikeFile(TRUE);
	UpdateScrollBar();
	m_spkClassListBox.Invalidate();
	if (m_pDataDoc != nullptr)
	{
		m_ChartDataWnd.GetDataFromDoc(m_lFirst, m_lLast);
		m_ChartDataWnd.ResizeChannels(m_ChartDataWnd.GetRectWidth(), m_lLast - m_lFirst);
		m_ChartDataWnd.Invalidate();
	}
}

void CViewSpikes::OnZoom()
{
	ZoomOnPresetInterval(0);
}

void CViewSpikes::ZoomOnPresetInterval(int iistart)
{
	if (iistart < 0)
		iistart = 0;
	m_lFirst = iistart;
	const auto acqrate = m_pSpkDoc->GetAcqRate();
	m_lLast = static_cast<long>((m_lFirst / acqrate + m_zoom) * acqrate);
	UpdateLegends(TRUE);
	// display data
	m_spkClassListBox.Invalidate();
	m_ChartDataWnd.Invalidate();
}

void CViewSpikes::OnFormatPreviousframe()
{
	ZoomOnPresetInterval(m_lFirst * 2 - m_lLast);
}

void CViewSpikes::OnFormatNextframe()
{
	const long len = m_lLast - m_lFirst;
	auto last = m_lLast + len;
	if (last > m_pSpkDoc->GetAcqSize())
		last = m_lLast - len;
	ZoomOnPresetInterval(last);
}

void CViewSpikes::OnGAINbutton()
{
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(0);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(1);
	SetVBarMode(BAR_GAIN);
}

void CViewSpikes::OnBIASbutton()
{
	((CButton*)GetDlgItem(IDC_BIAS_button))->SetState(1);
	((CButton*)GetDlgItem(IDC_GAIN_button))->SetState(0);
	SetVBarMode(BAR_BIAS);
}

void CViewSpikes::SetVBarMode(short bMode)
{
	if (bMode == BAR_BIAS)
		m_VBarMode = bMode;
	else
		m_VBarMode = BAR_GAIN;
	UpdateBiasScroll();
}

void CViewSpikes::UpdateGainScroll()
{
	m_scrolly.SetScrollPos(
		MulDiv(m_ChartDataWnd.GetChanlistYextent(0),
			100,
			YEXTENT_MAX)
		+ 50,
		TRUE);
}

void CViewSpikes::OnGainScroll(UINT nSBCode, UINT nPos)
{
	int lSize = m_ChartDataWnd.GetChanlistYextent(0);
	// get corresponding data
	switch (nSBCode)
	{
		// .................scroll to the start
	case SB_LEFT:		lSize = YEXTENT_MIN; break;
		// .................scroll one line left
	case SB_LINELEFT:	lSize -= lSize / 10 + 1; break;
		// .................scroll one line right
	case SB_LINERIGHT:	lSize += lSize / 10 + 1; break;
		// .................scroll one page left
	case SB_PAGELEFT:	lSize -= lSize / 2 + 1; break;
		// .................scroll one page right
	case SB_PAGERIGHT:	lSize += lSize + 1; break;
		// .................scroll to end right
	case SB_RIGHT:		lSize = YEXTENT_MAX; break;
		// .................scroll to pos = nPos or drag scroll box -- pos = nPos
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:	lSize = MulDiv(nPos - 50, YEXTENT_MAX, 100); break;
		// .................NOP: set position only
	default:			break;
	}

	// change y extent
	if (lSize > 0) //&& lSize<=YEXTENT_MAX)
	{
		m_ChartDataWnd.SetChanlistYextent(0, lSize);
		UpdateLegends(TRUE);
		m_ChartDataWnd.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_GAIN)
		UpdateGainScroll();
}

// --------------------------------------------------------------------------
// UpdateBiasScroll()
// -- not very nice code; interface counter intuitive
// --------------------------------------------------------------------------

void CViewSpikes::UpdateBiasScroll()
{
	const auto i_pos = static_cast<int>((m_ChartDataWnd.GetChanlistYzero(0)
		- m_ChartDataWnd.GetChanlistBinZero(0))
		* 100 / static_cast<int>(YZERO_SPAN)) + static_cast<int>(50);
	m_scrolly.SetScrollPos(i_pos, TRUE);
}

void CViewSpikes::OnBiasScroll(UINT nSBCode, UINT nPos)
{
	auto l_size = m_ChartDataWnd.GetChanlistYzero(0) - m_ChartDataWnd.GetChanlistBinZero(0);
	const auto yextent = m_ChartDataWnd.GetChanlistYextent(0);
	// get corresponding data
	switch (nSBCode)
	{
	case SB_LEFT:			// scroll to the start
		l_size = YZERO_MIN;
		break;
	case SB_LINELEFT:		// scroll one line left
		l_size -= yextent / 100 + 1;
		break;
	case SB_LINERIGHT:		// scroll one line right
		l_size += yextent / 100 + 1;
		break;
	case SB_PAGELEFT:		// scroll one page left
		l_size -= yextent / 10 + 1;
		break;
	case SB_PAGERIGHT:		// scroll one page right
		l_size += yextent / 10 + 1;
		break;
	case SB_RIGHT:			// scroll to end right
		l_size = YZERO_MAX;
		break;
	case SB_THUMBPOSITION:	// scroll to pos = nPos
	case SB_THUMBTRACK:		// drag scroll box -- pos = nPos
		l_size = (nPos - 50) * (YZERO_SPAN / 100);
		break;
	default:				// NOP: set position only
		break;
	}

	// try to read data with this new size
	if (l_size > YZERO_MIN && l_size < YZERO_MAX)
	{
		m_ChartDataWnd.SetChanlistYzero(0, l_size + m_ChartDataWnd.GetChanlistBinZero(0));
		m_ChartDataWnd.Invalidate();
	}
	// update scrollBar
	if (m_VBarMode == BAR_BIAS)
		UpdateBiasScroll();
}

void CViewSpikes::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		CDaoRecordView::OnVScroll(nSBCode, nPos, pScrollBar);
		return;
	}

	// CViewData scroll: vertical scroll bar
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

void CViewSpikes::SetAddspikesMode(int mousecursorType)
{
	// display or hide corresponding controls within this view
	const BOOL bSetAddspikemode = (mousecursorType == CURSOR_MEASURE);
	int n_cmd_show = SW_SHOW;
	if (!bSetAddspikemode)
	{
		n_cmd_show = SW_HIDE;
	}
	if (m_baddspikemode == bSetAddspikemode)
		return;
	m_baddspikemode = bSetAddspikemode;

	// tell parent window (childframe) that we want to change the mode
	// so that it updates the toolbar
	auto command_id = ID_VIEW_CURSORMODE_NORMAL;
	if (m_baddspikemode)
		command_id = ID_VIEW_CURSORMODE_MEASURE;
	GetParent()->SendMessage(WM_COMMAND, command_id, NULL);
	auto hwnd = GetSafeHwnd();
	if (!m_baddspikemode)
		hwnd = nullptr;
	m_ChartDataWnd.ReflectMouseMoveMessg(hwnd);
	m_spkClassListBox.ReflectBarsMouseMoveMessg(hwnd);
	m_ChartDataWnd.SetTrackSpike(m_baddspikemode, m_pspkDP->extractNpoints, m_pspkDP->prethreshold, m_pspkDP->extractChan);

	// display or not controls used when add_spike_mode is selected
	GetDlgItem(IDC_SOURCECLASS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_DESTCLASS)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT4)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDIT5)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_GROUPBOX)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_JITTER)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_JITTERSTATIC)->ShowWindow(n_cmd_show);
}

void CViewSpikes::OnArtefact()
{
	UpdateData(TRUE);				// load value from control
	if (m_spikeno < 0)
	{
		m_bartefact = FALSE;		// no action if spike index < 0
	}
	else
	{								// load old class nb
		auto spkclass = m_pSpkList->GetSpikeClass(m_spikeno);
		// if artefact: set class to negative value
		if (m_bartefact && spkclass >= 0)
			spkclass = -(spkclass + 1);

		// if not artefact: if spike has negative class, set to positive value
		else if (spkclass < 0)
			spkclass = -(spkclass + 1);
		m_pSpkList->SetSpikeClass(m_spikeno, spkclass);
	}
	CheckDlgButton(IDC_ARTEFACT, m_bartefact);
	m_pSpkDoc->SetModifiedFlag(TRUE);
	UpdateLegends(TRUE);
	m_spkClassListBox.Invalidate();
}

void CViewSpikes::OnEnChangeJitter()
{
	if (mm_jitter_ms.m_bEntryDone)
	{
		switch (mm_jitter_ms.m_nChar)
		{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_jitter_ms++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_jitter_ms--;	break;
		default:;
		}

		// check boundaries
		mm_jitter_ms.m_bEntryDone = FALSE;// clear flag
		mm_jitter_ms.m_nChar = 0;			// empty buffer
		mm_jitter_ms.SetSel(0, -1);		// select all text
		UpdateData(FALSE);
	}
}

void CViewSpikes::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_baddspikemode)
	{
		if (m_bdummy && m_rectVTtrack.PtInRect(point))
			m_ptVT = point.x - m_rectVTtrack.left;
		else
			m_ptVT = -1;
		m_bdummy = TRUE;
		m_ChartDataWnd.XorTempVTtag(m_ptVT);
		m_spkClassListBox.XorTempVTtag(m_ptVT);
	}
	// pass message to parent
	CDaoRecordView::OnMouseMove(nFlags, point);
}

void CViewSpikes::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	if (m_rectVTtrack.PtInRect(point))
	{
		const int iitime = m_ChartDataWnd.GetDataOffsetfromPixel(point.x - m_rectVTtrack.left);
		m_jitter = m_jitter_ms;
		auto b_check = TRUE;
		if (nFlags & MK_CONTROL)
			b_check = FALSE;
		AddSpiketoList(iitime, b_check);
		m_bdummy = FALSE;
	}
	CDaoRecordView::OnLButtonUp(nFlags, point);
}

void CViewSpikes::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_rectVTtrack.PtInRect(point))
		SetCapture();
	CDaoRecordView::OnLButtonDown(nFlags, point);
}

void CViewSpikes::OnHScrollLeft()
{
	OnHScroll(SB_PAGELEFT, NULL, (CScrollBar*)GetDlgItem(IDC_SCROLLBAR1));
}

void CViewSpikes::OnHScrollRight()
{
	OnHScroll(SB_PAGERIGHT, NULL, (CScrollBar*)GetDlgItem(IDC_SCROLLBAR1));
}

void CViewSpikes::OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	SelectSpkList(m_tabCtrl.GetCurSel());
	*pResult = 0;
}

void CViewSpikes::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	SelectSpkList(m_tabCtrl.GetCurSel());
	*pResult = 0;
}

void CViewSpikes::OnBnClickedSameclass()
{
	m_bKeepSameClass = ((CButton*)GetDlgItem(IDC_SAMECLASS))->GetCheck();
}
