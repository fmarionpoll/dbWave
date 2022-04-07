#include "StdAfx.h"
#include <math.h>
#include "dbWave.h"
#include "resource.h"
#include "Editctrl.h"
#include "ChartWnd.h"
#include "dbWaveDoc.h"
#include "Spikedoc.h"
#include "ChartSpikeShape.h"
#include "ChartSpikeHist.h"
#include "ChartSpikeXY.h"
#include "MainFrm.h"
#include "ViewSpikeSort.h"

#include "DlgProgress.h"
#include "DlgSpikeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// TODO limit size of measure array to nbspikes within currently selected spikelist

IMPLEMENT_DYNCREATE(ViewSpikeSort, dbTableView)

ViewSpikeSort::ViewSpikeSort()
	: dbTableView(IDD)
{
	m_bEnableActiveAccessibility = FALSE; // workaround to crash / accessibility
}

ViewSpikeSort::~ViewSpikeSort()
{
	// save spkD list i	 changed
	if (m_pSpkDoc != nullptr)
		saveCurrentSpkFile(); // save file if modified
	// save current spike detection parameters
	m_psC->bChanged = TRUE;
	m_psC->sourceclass = m_sourceclass;
	m_psC->destclass = m_destinationclass;
	m_psC->mvmax = m_mVMax;
	m_psC->mvmin = m_mVMin;
}

void ViewSpikeSort::DoDataExchange(CDataExchange* pDX)
{
	dbTableView::DoDataExchange(pDX);

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
	DDX_Check(pDX, IDC_CHECK1, m_bAllFiles);
	DDX_Text(pDX, IDC_NSPIKES, m_spikeno);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spikenoclass);
	DDX_Text(pDX, IDC_EDITRIGHT2, m_txyright);
	DDX_Text(pDX, IDC_EDITLEFT2, m_txyleft);

	DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
}

BEGIN_MESSAGE_MAP(ViewSpikeSort, dbTableView)

	ON_MESSAGE(WM_MYMESSAGE, &ViewSpikeSort::OnMyMessage)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_SOURCECLASS, &ViewSpikeSort::OnEnChangeSourceclass)
	ON_EN_CHANGE(IDC_DESTINATIONCLASS, &ViewSpikeSort::OnEnChangeDestinationclass)
	ON_CBN_SELCHANGE(IDC_PARAMETER, &ViewSpikeSort::OnSelchangeParameter)
	ON_EN_CHANGE(IDC_LIMITLOWER, &ViewSpikeSort::OnEnChangelower)
	ON_EN_CHANGE(IDC_LIMITUPPER, &ViewSpikeSort::OnEnChangeupper)
	ON_EN_CHANGE(IDC_T1, &ViewSpikeSort::OnEnChangeT1)
	ON_EN_CHANGE(IDC_T2, &ViewSpikeSort::OnEnChangeT2)
	ON_BN_CLICKED(IDC_EXECUTE, &ViewSpikeSort::OnSort)
	ON_BN_CLICKED(IDC_MEASURE, &ViewSpikeSort::OnMeasure)
	ON_BN_CLICKED(IDC_CHECK1, &ViewSpikeSort::OnSelectAllFiles)
	ON_COMMAND(ID_FORMAT_ALLDATA, &ViewSpikeSort::OnFormatAlldata)
	ON_COMMAND(ID_FORMAT_CENTERCURVE, &ViewSpikeSort::OnFormatCentercurve)
	ON_COMMAND(ID_FORMAT_GAINADJUST, &ViewSpikeSort::OnFormatGainadjust)
	ON_COMMAND(ID_TOOLS_EDITSPIKES, &ViewSpikeSort::OnToolsEdittransformspikes)
	ON_COMMAND(ID_TOOLS_ALIGNSPIKES, &ViewSpikeSort::OnToolsAlignspikes)
	ON_EN_CHANGE(IDC_EDIT2, &ViewSpikeSort::OnEnChangetimeFirst)
	ON_EN_CHANGE(IDC_EDIT3, &ViewSpikeSort::OnEnChangetimeLast)
	ON_EN_CHANGE(IDC_EDIT7, &ViewSpikeSort::OnEnChangemVMin)
	ON_EN_CHANGE(IDC_EDIT6, &ViewSpikeSort::OnEnChangemVMax)
	ON_EN_CHANGE(IDC_EDITLEFT2, &ViewSpikeSort::OnEnChangeEditleft2)
	ON_EN_CHANGE(IDC_EDITRIGHT2, &ViewSpikeSort::OnEnChangeEditright2)
	ON_EN_CHANGE(IDC_NSPIKES, &ViewSpikeSort::OnEnChangeNOspike)
	ON_BN_DOUBLECLICKED(IDC_DISPLAYPARM, &ViewSpikeSort::OnToolsEdittransformspikes)
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewSpikeSort::OnEnChangeSpikenoclass)
	ON_EN_CHANGE(IDC_BINMV, &ViewSpikeSort::OnEnChangeNBins)
END_MESSAGE_MAP()

void ViewSpikeSort::defineSubClassedItems()
{
	// subclass some controls
	VERIFY(yhistogram_wnd_.SubclassDlgItem(IDC_HISTOGRAM, this));
	VERIFY(xygraph_wnd_.SubclassDlgItem(IDC_DISPLAYPARM, this));
	VERIFY(m_ChartSpkWnd_Shape.SubclassDlgItem(IDC_DISPLAYSPIKE, this));
	VERIFY(m_ChartSpkWnd_Bar.SubclassDlgItem(IDC_DISPLAYBARS, this));

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

	VERIFY(m_filescroll.SubclassDlgItem(IDC_FILESCROLL, this));
	m_filescroll.SetScrollRange(0, 100, FALSE);
}

void ViewSpikeSort::defineStretchParameters()
{
	m_stretch.AttachParent(this);

	m_stretch.newProp(IDC_EDIT7, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_TAB1, XLEQ_XREQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_DISPLAYPARM, XLEQ_XREQ, YTEQ_YBEQ);
	m_stretch.newProp(IDC_DISPLAYBARS, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_FILESCROLL, XLEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_EDIT3, SZEQ_XREQ, SZEQ_YTEQ);
	m_stretch.newProp(IDC_STATICRIGHT, SZEQ_XREQ, SZEQ_YBEQ);

	m_stretch.newProp(IDC_STATICLEFT, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_STATIC12, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDITLEFT2, SZEQ_XLEQ, SZEQ_YBEQ);
	m_stretch.newProp(IDC_EDITRIGHT2, SZEQ_XLEQ, SZEQ_YBEQ);
}

void ViewSpikeSort::OnInitialUpdate()
{
	dbTableView::OnInitialUpdate();
	defineSubClassedItems();
	defineStretchParameters();
	m_binit = TRUE;
	m_autoIncrement = true;
	m_autoDetect = true;

	// load global parameters
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_psC = &(p_app->spkC);
	m_pOptionsViewData = &(p_app->options_viewdata);

	// assign values to controls
	m_CBparameter.SetCurSel(m_psC->iparameter);
	m_mVMax = m_psC->mvmax;
	m_mVMin = m_psC->mvmin;

	m_sourceclass = m_psC->sourceclass;
	m_destinationclass = m_psC->destclass;

	m_ChartSpkWnd_Shape.DisplayAllFiles(false, GetDocument());
	m_ChartSpkWnd_Shape.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);
	m_ChartSpkWnd_Shape.SetScopeParameters(&(m_pOptionsViewData->spksort1spk));
	m_spkformtagleft = m_ChartSpkWnd_Shape.m_VTtags.AddTag(m_psC->ileft, 0);
	m_spkformtagright = m_ChartSpkWnd_Shape.m_VTtags.AddTag(m_psC->iright, 0);

	xygraph_wnd_.DisplayAllFiles(false, GetDocument());
	xygraph_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	xygraph_wnd_.SetScopeParameters(&(m_pOptionsViewData->spksort1parms));
	m_itagup = xygraph_wnd_.m_HZtags.AddTag(m_psC->iupper, 0);
	m_itaglow = xygraph_wnd_.m_HZtags.AddTag(m_psC->ilower, 0);

	m_ChartSpkWnd_Bar.DisplayAllFiles(false, GetDocument());
	m_ChartSpkWnd_Bar.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	m_ChartSpkWnd_Bar.SetScopeParameters(&(m_pOptionsViewData->spksort1bars));

	yhistogram_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	yhistogram_wnd_.SetScopeParameters(&(m_pOptionsViewData->spksort1hist));

	// display tag lines at proper places
	m_spkhistupper = yhistogram_wnd_.m_VTtags.AddTag(m_psC->iupper, 0);
	m_spkhistlower = yhistogram_wnd_.m_VTtags.AddTag(m_psC->ilower, 0);

	updateFileParameters();
	if (nullptr != m_pSpkList)
	{
		m_delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
		m_lower = float(m_psC->ilower) * m_delta;
		m_upper = float(m_psC->iupper) * m_delta;
		UpdateData(false);
	}
	activateMode4();
}

void ViewSpikeSort::activateMode4()
{
	auto n_cmd_show = SW_HIDE;
	if (4 == m_psC->iparameter)
	{
		n_cmd_show = SW_SHOW;
		if (1 > xygraph_wnd_.m_VTtags.GetNTags())
		{
			m_ixyright = xygraph_wnd_.m_VTtags.AddTag(m_psC->ixyright, 0);
			m_ixyleft = xygraph_wnd_.m_VTtags.AddTag(m_psC->ixyleft, 0);
			const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
			m_txyright = static_cast<float>(m_psC->ixyright) / delta;
			m_txyleft = static_cast<float>(m_psC->ixyleft) / delta;
		}
		xygraph_wnd_.SetNxScaleCells(2, 0, 0);
		xygraph_wnd_.GetScopeParameters()->crScopeGrid = RGB(128, 128, 128);

		if (nullptr != m_pSpkList)
		{
			const auto spikelen_ms = (static_cast<float>(m_pSpkList->GetSpikeLength()) * m_tunit) / m_pSpkList->GetAcqSampRate();
			CString cs_dummy;
			cs_dummy.Format(_T("%0.1f ms"), spikelen_ms);
			GetDlgItem(IDC_STATICRIGHT)->SetWindowText(cs_dummy);
			cs_dummy.Format(_T("%0.1f ms"), -spikelen_ms);
			GetDlgItem(IDC_STATICLEFT)->SetWindowText(cs_dummy);
		}
	}
	else
	{
		xygraph_wnd_.m_VTtags.RemoveAllTags();
		xygraph_wnd_.SetNxScaleCells(0, 0, 0);
	}
	GetDlgItem(IDC_STATICRIGHT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATICLEFT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC12)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITRIGHT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITLEFT2)->ShowWindow(n_cmd_show);
	xygraph_wnd_.Invalidate();
}

void ViewSpikeSort::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	if (bActivate)
	{
		auto* p_mainframe = static_cast<CMainFrame*>(AfxGetMainWnd());
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_ACTIVATEVIEW,
		                         reinterpret_cast<LPARAM>(pActivateView->GetDocument()));
	}
	else
	{
		saveCurrentSpkFile();
		auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		if (nullptr == p_app->m_psort1spikesMemFile)
		{
			p_app->m_psort1spikesMemFile = new CMemFile;
			ASSERT(p_app->m_psort1spikesMemFile != NULL);
		}
		CArchive ar(p_app->m_psort1spikesMemFile, CArchive::store);
		p_app->m_psort1spikesMemFile->SeekToBegin();
		ar.Close();
	}
	dbTableView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void ViewSpikeSort::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	if (m_binit)
	{
		switch (LOWORD(lHint))
		{
		case HINT_DOCHASCHANGED: // file has changed?
		case HINT_DOCMOVERECORD:
		case HINT_REQUERY:
			updateFileParameters();
			break;
		case HINT_CLOSEFILEMODIFIED: // close modified file: save
			saveCurrentSpkFile();
			break;
		case HINT_REPLACEVIEW:
		default:
			break;
		}
	}
}

BOOL ViewSpikeSort::OnMove(UINT nIDMoveCommand)
{
	saveCurrentSpkFile();
	return dbTableView::OnMove(nIDMoveCommand);
}

void ViewSpikeSort::updateSpikeFile()
{
	m_pSpkDoc = GetDocument()->OpenCurrentSpikeFile();

	if (nullptr != m_pSpkDoc)
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->GetDB_CurrentSpkFileName(), FALSE);
		int icur = GetDocument()->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(icur);

		// update Tab at the bottom
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);
		m_tabCtrl.SetCurSel(icur);
	}
}

void ViewSpikeSort::updateFileParameters()
{
	// reset parms ? flag = single file or file list has changed
	if (!m_bAllFiles)
	{
		yhistogram_wnd_.RemoveHistData();
	}

	const BOOL bfirstupdate = (m_pSpkDoc == nullptr);
	updateSpikeFile();

	if (bfirstupdate || m_pOptionsViewData->bEntireRecord)
	{
		if (m_pSpkDoc == nullptr)
			return;
		m_timeFirst = 0.f;
		m_timeLast = (m_pSpkDoc->GetAcqSize() - 1.f) / m_pSpkList->GetAcqSampRate();
	}
	m_lFirst = static_cast<long>(m_timeFirst * m_pSpkList->GetAcqSampRate());
	m_lLast = static_cast<long>(m_timeLast * m_pSpkList->GetAcqSampRate());

	// spike and classes
	auto spike_index = m_pSpkList->m_selected_spike;
	if (m_pSpkList->GetTotalSpikes() < spike_index || 0 > spike_index)
	{
		spike_index = -1;
		m_sourceclass = 0;
	}
	else
	{
		m_sourceclass = m_pSpkList->GetSpike(spike_index)->get_class();
		m_psC->sourceclass = m_sourceclass;
	}
	ASSERT(m_sourceclass < 32768);

	if (0 == m_psC->ileft && 0 == m_psC->iright)
	{
		m_psC->ileft = m_pSpkList->GetDetectParms()->prethreshold;
		m_psC->iright = m_psC->ileft + m_pSpkList->GetDetectParms()->refractory;
	}
	m_t1 = (m_psC->ileft * m_tunit) / m_pSpkList->GetAcqSampRate();
	m_t2 = (m_psC->iright * m_tunit) / m_pSpkList->GetAcqSampRate();

	m_ChartSpkWnd_Bar.SetSourceData_spklist_dbwavedoc(m_pSpkList, GetDocument());
	m_ChartSpkWnd_Bar.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);

	m_ChartSpkWnd_Shape.SetSourceData(m_pSpkList, GetDocument());
	m_ChartSpkWnd_Shape.m_VTtags.SetTagVal(m_spkformtagleft, m_psC->ileft);
	m_ChartSpkWnd_Shape.m_VTtags.SetTagVal(m_spkformtagright, m_psC->iright);
	m_ChartSpkWnd_Shape.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);

	xygraph_wnd_.SetSourceData(m_pSpkList, GetDocument());
	xygraph_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
	yhistogram_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);

	m_filescroll_infos.fMask = SIF_ALL;
	m_filescroll_infos.nMin = 0;
	m_filescroll_infos.nMax = m_pSpkDoc->GetAcqSize() - 1;
	m_filescroll_infos.nPos = 0;
	m_filescroll_infos.nPage = m_pSpkDoc->GetAcqSize();
	m_filescroll.SetScrollInfo(&m_filescroll_infos);

	updateLegends();

	// display & compute parameters
	if (!m_bAllFiles || !m_bMeasureDone)
	{
		if (4 != m_psC->iparameter)
		{
			xygraph_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
			if (xygraph_wnd_.m_VTtags.GetNTags() > 0)
			{
				xygraph_wnd_.m_VTtags.RemoveAllTags();
				xygraph_wnd_.Invalidate();
			}
		}
		else
		{
			xygraph_wnd_.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
			if (1 > xygraph_wnd_.m_VTtags.GetNTags())
			{
				m_ixyright = xygraph_wnd_.m_VTtags.AddTag(m_psC->ixyright, 0);
				m_ixyleft = xygraph_wnd_.m_VTtags.AddTag(m_psC->ixyleft, 0);
				const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_txyright = static_cast<float>(m_psC->ixyright) / delta;
				m_txyleft = static_cast<float>(m_psC->ixyleft) / delta;
				xygraph_wnd_.Invalidate();
			}
		}
		// update text , display and compute histogram
		m_bMeasureDone = FALSE; // no parameters yet
		OnMeasure();
	}

	selectSpikeFromCurrentList(spike_index);
}

void ViewSpikeSort::updateLegends()
{
	// update text abscissa and horizontal scroll position
	m_timeFirst = m_lFirst / m_pSpkList->GetAcqSampRate();
	m_timeLast = m_lLast / m_pSpkList->GetAcqSampRate();
	updateFileScroll();

	if (4 != m_psC->iparameter)
		xygraph_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
	else
		xygraph_wnd_.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
	xygraph_wnd_.Invalidate();

	m_ChartSpkWnd_Shape.SetTimeIntervals(m_lFirst, m_lLast);
	m_ChartSpkWnd_Shape.Invalidate();

	m_ChartSpkWnd_Bar.SetTimeIntervals(m_lFirst, m_lLast);
	m_ChartSpkWnd_Bar.Invalidate();

	yhistogram_wnd_.Invalidate();

	UpdateData(FALSE); // copy view object to controls
}

void ViewSpikeSort::OnSort()
{
	// set file indexes - assume only one file selected
	auto pdb_doc = GetDocument();
	const int currentfile = pdb_doc->GetDB_CurrentRecordPosition();
	auto firstfile = currentfile;
	auto lastfile = firstfile;
	const auto nfiles = pdb_doc->GetDB_NRecords();
	const auto currentlist = m_pSpkDoc->GetSpkList_CurrentIndex();

	// change indexes if ALL files selected
	DlgProgress* pdlg = nullptr;
	auto istep = 0;
	CString cscomment;
	if (m_bAllFiles)
	{
		firstfile = 0; // index first file
		lastfile = pdb_doc->GetDB_NRecords() - 1; // index last file
		pdlg = new DlgProgress;
		pdlg->Create();
		pdlg->SetStep(1);
	}

	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// load spike file
		BOOL flagchanged;
		pdb_doc->SetDB_CurrentRecordPosition(ifile);
		m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		if (nullptr == m_pSpkDoc)
			continue;

		// update screen if multi-file requested
		if (m_bAllFiles)
		{
			if (pdlg->CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cscomment.Format(_T("Processing file [%i / %i]"), ifile + 1, nfiles);
			pdlg->SetStatus(cscomment);
			if (MulDiv(ifile, 100, nfiles) > istep)
			{
				pdlg->StepIt();
				istep = MulDiv(ifile, 100, nfiles);
			}
		}

		// load spike list
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
		if ((nullptr == m_pSpkList) || (0 == m_pSpkList->GetSpikeLength()))
			continue;

		// loop over all spikes of the list and compare to a single parameter
		const CSize limits1(m_psC->ilower, m_psC->iupper);
		const CSize fromclass_toclass(m_sourceclass, m_destinationclass);
		const CSize timewindow(m_lFirst, m_lLast);
		// sort on 1 parameter
		if (4 != m_psC->iparameter)
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
			m_pSpkDoc->OnSaveDocument(pdb_doc->GetDB_CurrentSpkFileName(FALSE));
			pdb_doc->SetDB_n_spikes(m_pSpkList->GetTotalSpikes());
		}
	}

	// end of loop, select current file again if necessary
	if (m_bAllFiles)
	{
		delete pdlg;
		pdb_doc->SetDB_CurrentRecordPosition(currentfile);
		m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	}

	// refresh data windows
	buildHistogram();

	xygraph_wnd_.Invalidate();
	m_ChartSpkWnd_Shape.Invalidate();
	m_ChartSpkWnd_Bar.Invalidate();
	yhistogram_wnd_.Invalidate();

	m_pSpkDoc->SetModifiedFlag(TRUE);
}

LRESULT ViewSpikeSort::OnMyMessage(WPARAM code, LPARAM lParam)
{
	short shortValue = LOWORD(lParam);
	switch (code)
	{
	case HINT_SETMOUSECURSOR: // ------------- change mouse cursor (all 3 items)
		if (CURSOR_ZOOM < shortValue)
			shortValue = 0;
		SetViewMouseCursor(shortValue);
		GetParent()->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(shortValue, 0));
		break;

	case HINT_CHANGEHZLIMITS: // -------------  abscissa have changed
		if (4 != m_psC->iparameter)
		{
			m_lFirst = xygraph_wnd_.GetTimeFirst();
			m_lLast = xygraph_wnd_.GetTimeLast();
		}
		else
		{
			m_lFirst = m_ChartSpkWnd_Bar.GetTimeFirst();
			m_lLast = m_ChartSpkWnd_Bar.GetTimeLast();
		}
		updateLegends();
		break;

	case HINT_HITSPIKE: // -------------  spike is selected or deselected
		{
			if (m_pSpkList->GetSpikeFlagArrayCount() > 0)
				unflagAllSpikes();
			auto spikeno = 0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spikeno = m_ChartSpkWnd_Shape.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spikeno = m_ChartSpkWnd_Bar.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spikeno = xygraph_wnd_.GetHitSpike();

			selectSpikeFromCurrentList(spikeno);
		}
		break;

	case HINT_SELECTSPIKES:
		xygraph_wnd_.Invalidate();
		m_ChartSpkWnd_Shape.Invalidate();
		m_ChartSpkWnd_Bar.Invalidate();
		break;

	case HINT_DBLCLKSEL:
		{
			auto spikeno = 0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spikeno = m_ChartSpkWnd_Shape.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spikeno = m_ChartSpkWnd_Bar.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spikeno = xygraph_wnd_.GetHitSpike();
			// if m_bAllFiles, spikeno is global, otherwise it comes from a single file...
			selectSpikeFromCurrentList(spikeno);
			OnToolsEdittransformspikes();
		}
		break;

	//case HINT_MOVEVERTTAG: // -------------  vertical tag has moved lowp = tag index
	case HINT_CHANGEVERTTAG: // -------------  vertical tag value has changed
		if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
		{
			if (shortValue == m_spkformtagleft) // first tag
			{
				m_psC->ileft = m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagleft);
				m_t1 = static_cast<float>(m_psC->ileft) * m_tunit / m_pSpkList->GetAcqSampRate();
				mm_t1.m_bEntryDone = TRUE;
				OnEnChangeT1();
			}
			else if (shortValue == m_spkformtagright) // second tag
			{
				m_psC->iright = m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagright);
				m_t2 = m_psC->iright * m_tunit / m_pSpkList->GetAcqSampRate();
				mm_t2.m_bEntryDone = TRUE;
				OnEnChangeT2();
			}
		}
		else if (HIWORD(lParam) == IDC_HISTOGRAM)
		{
			if (shortValue == m_spkhistlower) // first tag
			{
				m_psC->ilower = yhistogram_wnd_.m_VTtags.GetValue(m_spkhistlower);
				m_lower = m_psC->ilower * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				UpdateData(false);
			}
			else if (shortValue == m_spkhistupper) // second tag
			{
				m_psC->iupper = yhistogram_wnd_.m_VTtags.GetValue(m_spkhistupper); // load new value
				m_upper = m_psC->iupper * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				UpdateData(false);
			}
		}
		else if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (shortValue == m_ixyright)
			{
				const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_psC->ixyright = xygraph_wnd_.m_VTtags.GetValue(m_ixyright);
				m_txyright = static_cast<float>(m_psC->ixyright) / delta;
				mm_txyright.m_bEntryDone = TRUE;
				OnEnChangeEditright2();
			}
			else if (shortValue == m_ixyleft)
			{
				const auto delta = m_pSpkList->GetAcqSampRate() / m_tunit;
				m_psC->ixyleft = xygraph_wnd_.m_VTtags.GetValue(m_ixyleft);
				m_txyleft = static_cast<float>(m_psC->ixyleft) / delta;
				mm_txyleft.m_bEntryDone = TRUE;
				OnEnChangeEditleft2();
			}
		}
		break;

	case HINT_CHANGEHZTAG: // ------------- change horizontal tag value
		//case HINT_MOVEHZTAG:	// ------------- move horizontal tag
		if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (shortValue == m_itaglow) // first tag
			{
				m_psC->ilower = xygraph_wnd_.m_HZtags.GetValue(m_itaglow);
				m_lower = m_psC->ilower * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				mm_lower.m_bEntryDone = TRUE;
				OnEnChangelower();
			}
			else if (shortValue == m_itagup) // second tag
			{
				m_psC->iupper = xygraph_wnd_.m_HZtags.GetValue(m_itagup); 
				m_upper = m_psC->iupper * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				mm_upper.m_bEntryDone = TRUE;
				OnEnChangeupper();
			}
		}
		break;

	case HINT_VIEWSIZECHANGED: // ------------- change zoom
		updateLegends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		m_pOptionsViewData->spksort1spk = *m_ChartSpkWnd_Shape.GetScopeParameters();
		m_pOptionsViewData->spksort1parms = *xygraph_wnd_.GetScopeParameters();
		m_pOptionsViewData->spksort1hist = *yhistogram_wnd_.GetScopeParameters();
		m_pOptionsViewData->spksort1bars = *m_ChartSpkWnd_Bar.GetScopeParameters();
		break;
	case HINT_VIEWTABHASCHANGED:
		selectSpkList(shortValue);
		break;

	default:
		break;
	}
	return 0L;
}

void ViewSpikeSort::unflagAllSpikes()
{
	if (m_bAllFiles)
	{
		auto pdb_doc = GetDocument();
		for (auto ifile = 0; ifile < pdb_doc->GetDB_NRecords(); ifile++)
		{
			pdb_doc->SetDB_CurrentRecordPosition(ifile);
			m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();

			for (auto j = 0; j < m_pSpkDoc->GetSpkList_Size(); j++)
			{
				m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(j);
				m_pSpkList->RemoveAllSpikeFlags();
			}
		}
	}
	else
		m_pSpkList->RemoveAllSpikeFlags();
	xygraph_wnd_.Invalidate();
	m_ChartSpkWnd_Shape.Invalidate();
	m_ChartSpkWnd_Bar.Invalidate();
}

void ViewSpikeSort::OnMeasure()
{
	// set file indexes - assume only one file selected
	auto pdb_doc = GetDocument();
	int currentfile = pdb_doc->GetDB_CurrentRecordPosition(); // index current file
	const int nfiles = pdb_doc->GetDB_NRecords();
	const auto currentlist = m_pSpkDoc->GetSpkList_CurrentIndex();
	int firstfile = currentfile;
	int lastfile = currentfile;
	// change size of arrays and prepare temporary dialog
	selectSpikeFromCurrentList(-1);
	if (m_bAllFiles)
	{
		firstfile = 0; // index first file
		lastfile = nfiles - 1; // index last file
	}

	// loop over all selected files (or only one file currently selected)
	for (auto ifile = firstfile; ifile <= lastfile; ifile++)
	{
		// check if user wants to continue
		//if (m_bAllfiles)
		//{
		pdb_doc->SetDB_CurrentRecordPosition(ifile);
		m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		//}
		// check if this file is ok
		if (m_pSpkDoc == nullptr)
			continue;
		m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
		if (m_pSpkList == nullptr)
			continue;

		const auto nspikes = m_pSpkList->GetTotalSpikes();
		if (nspikes <= 0 || m_pSpkList->GetSpikeLength() == 0)
			continue;

		switch (m_psC->iparameter)
		{
		case 1: // value at t1
			m_pSpkList->Measure_case1_AmplitudeAtT(m_psC->ileft);
			m_bMeasureDone = TRUE;
			break;
		case 2: // value at t2
			m_pSpkList->Measure_case1_AmplitudeAtT(m_psC->iright);
			m_bMeasureDone = TRUE;
			break;

		case 3: // value at t2- value at t1
			m_pSpkList->Measure_case2_AmplitudeAtT2MinusAtT1(m_psC->ileft, m_psC->iright);
			m_bMeasureDone = TRUE;
			break;

		case 0: // max - min between t1 and t2
		case 4: // max-min vs tmax-tmin
		default:
			m_pSpkList->Measure_case0_AmplitudeMinToMax(m_psC->ileft, m_psC->iright);
			break;
		}

		//save only if changed?
		m_pSpkDoc->OnSaveDocument(pdb_doc->GetDB_CurrentSpkFileName(FALSE));
	}

	if (m_bAllFiles)
	{
		currentfile = pdb_doc->GetDB_CurrentRecordPosition();
		pdb_doc->SetDB_CurrentRecordPosition(currentfile);
		m_pSpkDoc = pdb_doc->OpenCurrentSpikeFile();
		m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	}

	m_ChartSpkWnd_Shape.SetSourceData(m_pSpkList, GetDocument());
	m_ChartSpkWnd_Bar.SetSourceData_spklist_dbwavedoc(m_pSpkList, GetDocument());

	xygraph_wnd_.m_HZtags.SetTagVal(m_itaglow, m_psC->ilower);
	xygraph_wnd_.m_HZtags.SetTagVal(m_itagup, m_psC->iupper);

	buildHistogram();
	yhistogram_wnd_.m_VTtags.SetTagVal(m_itaglow, m_psC->ilower);
	yhistogram_wnd_.m_VTtags.SetTagVal(m_itagup, m_psC->iupper);

	updateGain();
	UpdateData(FALSE);
}

void ViewSpikeSort::updateGain()
{
	const auto delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
	const auto max = static_cast<int>(m_mVMax / delta);
	const auto min = static_cast<int>(m_mVMin / delta);
	if (max == min)
		return;

	const auto y_we = max - min;
	const auto y_wo = (max + min) / 2;

	xygraph_wnd_.SetYWExtOrg(y_we, y_wo);
	xygraph_wnd_.Invalidate();

	yhistogram_wnd_.SetXWExtOrg(y_we, y_wo - y_we / 2);
	yhistogram_wnd_.Invalidate();
}

void ViewSpikeSort::OnFormatAlldata()
{
	// build new histogram only if necessary
	auto build_histogram = FALSE;

	// dots: spk file length
	if (m_lFirst != 0 || m_lLast != m_pSpkDoc->GetAcqSize() - 1)
	{
		m_lFirst = 0;
		m_lLast = m_pSpkDoc->GetAcqSize() - 1;

		if (m_psC->iparameter != 4) // then, we need imax imin ...
			xygraph_wnd_.SetTimeIntervals(m_lFirst, m_lLast);
		else
			xygraph_wnd_.SetTimeIntervals(-m_pSpkList->GetSpikeLength(), m_pSpkList->GetSpikeLength());
		xygraph_wnd_.Invalidate();

		m_ChartSpkWnd_Shape.SetTimeIntervals(m_lFirst, m_lLast);
		m_ChartSpkWnd_Shape.Invalidate();

		m_ChartSpkWnd_Bar.SetTimeIntervals(m_lFirst, m_lLast);
		m_ChartSpkWnd_Bar.Invalidate();
		build_histogram = TRUE;
	}

	// spikes: center spikes horizontally and adjust hz size of display

	const auto x_we = m_pSpkList->GetSpikeLength();
	if (x_we != m_ChartSpkWnd_Shape.GetXWExtent() || 0 != m_ChartSpkWnd_Shape.GetXWOrg())
		m_ChartSpkWnd_Shape.SetXWExtOrg(x_we, 0);

	// change spk_hist_wnd_
	if (build_histogram)
	{
		buildHistogram();
	}
	updateLegends();
}

void ViewSpikeSort::buildHistogram()
{
	auto pdb_doc = GetDocument();
	if (pdb_doc == nullptr)
		return;

	const auto delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
	m_parmmax = static_cast<int>(m_mVMax / delta);
	m_parmmin = static_cast<int>(m_mVMin / delta);
	const int nbins = static_cast<int>((m_mVMax - m_mVMin) / m_mVbin);
	if (nbins <= 0)
		return;

	yhistogram_wnd_.BuildHistFromDocument(pdb_doc, m_bAllFiles, m_lFirst, m_lLast, m_parmmax, m_parmmin, nbins, TRUE);
}

void ViewSpikeSort::OnFormatCentercurve()
{
	const auto n_spikes = m_pSpkList->GetTotalSpikes();
	for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
		m_pSpkList->GetSpike(i_spike)->CenterSpikeAmplitude( m_psC->ileft, m_psC->iright, 1);

	int max, min;
	m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	m_ChartSpkWnd_Shape.SetYWExtOrg(m_ChartSpkWnd_Shape.GetYWExtent(), middle);
	m_ChartSpkWnd_Bar.SetYWExtOrg(m_ChartSpkWnd_Shape.GetYWExtent(), middle);

	updateLegends();
}

void ViewSpikeSort::OnFormatGainadjust()
{
	// adjust gain of spkform and spkbar: data = raw signal
	int maxval, minval;
	GetDocument()->GetAllSpkMaxMin(m_bAllFiles, TRUE, &maxval, &minval);

	auto y_we = MulDiv(maxval - minval + 1, 10, 9);
	auto y_wo = (maxval + minval) / 2;
	m_ChartSpkWnd_Shape.SetYWExtOrg(y_we, y_wo);
	m_ChartSpkWnd_Bar.SetYWExtOrg(y_we, y_wo);
	//m_ChartSpkWnd_Bar.MaxCenter();

	// adjust gain for spk_hist_wnd_ and XYp: data = computed values
	// search max min of parameter values
	CSize measure = GetDocument()->GetSpkMaxMin_y1(m_bAllFiles);
	maxval = measure.cx;
	minval = measure.cy;

	const auto delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
	const auto max2 = static_cast<int>(m_upper / delta);
	const auto min2 = static_cast<int>(m_lower / delta);
	if (max2 > maxval)
		maxval = max2;
	if (min2 < minval)
		minval = min2;
	y_we = MulDiv(maxval - minval + 1, 10, 8);
	y_wo = (maxval + minval) / 2;

	// update display
	xygraph_wnd_.SetYWExtOrg(y_we, y_wo);
	const auto ymax = static_cast<int>(yhistogram_wnd_.GetHistMax());
	yhistogram_wnd_.SetXWExtOrg(y_we, y_wo - y_we / 2);
	yhistogram_wnd_.SetYWExtOrg(MulDiv(ymax, 10, 8), 0);

	// update edit controls
	m_mVMax = static_cast<float>(maxval) * delta;
	m_mVMin = static_cast<float>(minval) * delta;
	buildHistogram();
	updateLegends();
}

void ViewSpikeSort::selectSpikeFromCurrentList(int spikeno)
{
	const auto ispike_local = spikeno;

	m_ChartSpkWnd_Shape.SelectSpikeShape(ispike_local);
	m_ChartSpkWnd_Bar.SelectSpike(ispike_local);
	xygraph_wnd_.SelectSpike(ispike_local);
	m_pSpkList->m_selected_spike = ispike_local;

	m_spikenoclass = -1;
	auto n_cmd_show = SW_HIDE;
	if (ispike_local >= 0)
	{
		const auto spike_elemt = m_pSpkList->GetSpike(ispike_local);
		m_spikenoclass = spike_elemt->get_class();
		n_cmd_show = SW_SHOW;
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_SPIKECLASS)->ShowWindow(n_cmd_show);
	m_spikeno = ispike_local;
	UpdateData(FALSE);
}

void ViewSpikeSort::OnToolsEdittransformspikes()
{
	DlgSpikeEdit dlg;
	dlg.m_yextent = m_ChartSpkWnd_Shape.GetYWExtent();
	dlg.m_yzero = m_ChartSpkWnd_Shape.GetYWOrg();
	dlg.m_xextent = m_ChartSpkWnd_Shape.GetXWExtent();
	dlg.m_xzero = m_ChartSpkWnd_Shape.GetXWOrg();
	dlg.m_spikeno = m_spikeno;
	dlg.m_parent = this;
	dlg.m_pdbWaveDoc = GetDocument();

	// refresh pointer to data file because it not used elsewhere in the view
	const auto docname = GetDocument()->GetDB_CurrentDatFileName();
	auto b_doc_exists = FALSE;
	if (!docname.IsEmpty())
	{
		CFileStatus status;
		b_doc_exists = CFile::GetStatus(docname, status);
	}
	if (b_doc_exists)
	{
		const auto flag = (GetDocument()->OpenCurrentDataFile() != nullptr);
		ASSERT(flag);
	}

	// run dialog box
	dlg.DoModal();
	if (dlg.m_bchanged)
	{
		m_pSpkDoc->SetModifiedFlag(TRUE);
		const auto currentlist = m_tabCtrl.GetCurSel();
		m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
	}

	if (!dlg.m_bartefact && m_spikeno != dlg.m_spikeno)
		selectSpikeFromCurrentList(dlg.m_spikeno);

	updateLegends();
}

void ViewSpikeSort::OnSelectAllFiles()
{
	m_bAllFiles = dynamic_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	m_ChartSpkWnd_Bar.DisplayAllFiles(m_bAllFiles, GetDocument());
	m_ChartSpkWnd_Shape.DisplayAllFiles(m_bAllFiles, GetDocument());
	xygraph_wnd_.DisplayAllFiles(m_bAllFiles, GetDocument());

	m_bMeasureDone = FALSE;
	OnMeasure();
}

void ViewSpikeSort::OnToolsAlignspikes()
{
	// get source data
	auto b_doc_exist = FALSE;
	auto data_file_name = m_pSpkDoc->GetAcqFilename();
	if (!data_file_name.IsEmpty())
	{
		CFileStatus status;
		b_doc_exist = CFile::GetStatus(data_file_name, status);
	}
	if (!b_doc_exist)
	{
		AfxMessageBox(_T("Source data not found - operation aborted"));
		return;
	}

	// first prepare array with SUM

	const auto spikelen = m_pSpkList->GetSpikeLength(); // length of one spike
	const auto totalspikes = m_pSpkList->GetTotalSpikes(); // total nb of spikes /record
	const auto p_sum0 = new double[spikelen]; // array with results / SUMy
	const auto p_cxy0 = new double[spikelen]; // temp array to store correlat
	auto* const p_mean0 = new short[spikelen]; // mean (template) / at scale
	const auto p_dummy0 = new short[spikelen]; // results of correlation / at scale

	// init pSUM with zeros
	auto p_sum = p_sum0;
	for (auto i = 0; i < spikelen; i++, p_sum++)
		*p_sum = 0;

	// compute mean
	auto nbspk_selclass = 0;
	short* p_spk;
	for (auto ispk = 0; ispk < totalspikes; ispk++)
	{
		if (m_pSpkList->GetSpike(ispk)->get_class() != m_sourceclass)
			continue;
		nbspk_selclass++;
		p_spk = m_pSpkList->GetSpike(ispk)->get_p_data();
		p_sum = p_sum0;
		for (auto i = 0; i < spikelen; i++, p_spk++, p_sum++)
			*p_sum += *p_spk;
	}

	// build avg and avg autocorrelation, then display
	auto p_mean = p_mean0;
	p_sum = p_sum0;

	for (auto i = 0; i < spikelen; i++, p_mean++, p_sum++)
		*p_mean = static_cast<short>(*p_sum / nbspk_selclass);

	m_ChartSpkWnd_Shape.DisplayExData(p_mean0);

	// for each spike, compute correlation and take max value correlation
	const auto kstart = m_psC->ileft; // start of template match
	const auto kend = m_psC->iright; // end of template match
	if (kend <= kstart)
		return;
	const auto j0 = kstart - (kend - kstart) / 2; // start time lag
	const auto j1 = kend - (kend - kstart) / 2 + 1; // last lag

	// compute autocorrelation for mean;
	double cxx_mean = 0;
	p_mean = p_mean0 + kstart;
	for (auto i = kstart; i < kend; i++, p_mean++)
	{
		const auto val = static_cast<double>(*p_mean);
		cxx_mean += val * val;
	}

	// get parameters from document
	auto p_dat_doc = GetDocument()->m_pDat;
	p_dat_doc->OnOpenDocument(data_file_name);
	const auto doc_chan = m_pSpkList->GetDetectParms()->extractChan; 
	const auto number_channels = static_cast<int>(p_dat_doc->GetpWaveFormat()->scan_count); 
	const auto method = m_pSpkList->GetDetectParms()->extractTransform;
	const auto spike_pre_trigger = m_pSpkList->GetDetectParms()->prethreshold;
	const int offset = (method > 0) ? 1 : number_channels; 
	const int span = p_dat_doc->GetTransfDataSpan(method); 

	// pre-load data
	auto iitime0 = m_pSpkList->GetSpike(0)->get_time(); //-pretrig;
	auto l_rw_first0 = iitime0 - spikelen;
	auto l_rw_last0 = iitime0 + spikelen;
	if (!p_dat_doc->LoadRawData(&l_rw_first0, &l_rw_last0, span))
		return; // exit if error reported
	auto p_data = p_dat_doc->LoadTransfData(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto spkpretrig = m_pSpkList->GetDetectParms()->prethreshold;
	for (auto ispk = 0; ispk < totalspikes; ispk++)
	{
		Spike* pSpike = m_pSpkList->GetSpike(ispk);

		// exclude spikes that do not fall within time limits
		if (pSpike->get_class() != m_sourceclass)
			continue;

		iitime0 = pSpike->get_time();
		iitime0 -= spike_pre_trigger;

		// make sure that source data are loaded and get pointer to it (p_data)
		auto l_rw_first = iitime0 - spikelen; // first point (eventually) needed
		auto l_rw_last = iitime0 + spikelen; // last pt needed
		if (iitime0 > m_lLast || iitime0 < m_lFirst)
			continue;
		if (!p_dat_doc->LoadRawData(&l_rw_first, &l_rw_last, span))
			break; // exit if error reported

		// load data only if necessary
		if (l_rw_first != l_rw_first0 || l_rw_last != l_rw_last0)
		{
			p_data = p_dat_doc->LoadTransfData(l_rw_first, l_rw_last, method, doc_chan);
			l_rw_last0 = l_rw_last; // index las pt within p_data
			l_rw_first0 = l_rw_first; // index first pt within p_data
		}

		// pointer to first point of spike
		auto p_data_spike0 = p_data + (iitime0 - l_rw_first) * offset;

		// for spike ispk: loop over spikelen time lags centered over interval center

		// compute autocorrelation & cross correlation at first time lag
		auto p_cxy_lag = p_cxy0; // pointer to array with correl coeffs
		*p_cxy_lag = 0; // init cross corr
		auto pdat_k0 = p_data_spike0 + j0 * offset; // source data start

		// loop over all time lag requested
		for (auto j = j0; j < j1; j++, p_cxy_lag++, pdat_k0 += offset)
		{
			*p_cxy_lag = 0;

			// add cross product for each point: data * meanlong ii_time
			auto p_mean_k = p_mean0 + kstart; // first point / template
			short* pdat_k = pdat_k0; // first data point
			double cxx_spike = 0; // autocorrelation

			// loop over all points of source data and mean
			for (auto k = kstart; k < kend; k++, p_mean_k++, pdat_k += offset)
			{
				const auto val = static_cast<double>(*pdat_k);
				*p_cxy_lag += static_cast<double>(*p_mean_k) * val;
				cxx_spike += val * val;
			}

			*p_cxy_lag /= (static_cast<double>(kend) - kstart + 1);
			*p_cxy_lag = *p_cxy_lag / sqrt(cxx_mean * cxx_spike);
		}

		// get max and min of this correlation
		auto p_cxy = p_cxy0;
		auto cxy_max = *p_cxy; // correlation max value
		auto i_cxy_max = 0; // correlation max index
		for (auto i = 0; i < kend - kstart; p_cxy++, i++)
		{
			if (cxy_max < *p_cxy) // get max and max position
			{
				cxy_max = *p_cxy;
				i_cxy_max = i;
			}
		}

		// offset spike so that max is at spikelen/2
		const auto jdecal = i_cxy_max - (kend - kstart) / 2;
		if (jdecal != 0)
		{
			p_data_spike0 = p_data + static_cast<WORD>(iitime0 + jdecal - l_rw_first) * offset + doc_chan;
			pSpike->TransferDataToSpikeBuffer(p_data_spike0, number_channels);
			m_pSpkDoc->SetModifiedFlag(TRUE);
			pSpike->set_time(iitime0 + spkpretrig);
		}

		// now offset spike vertically to align it with the mean
		pSpike->OffsetSpikeDataToAverageEx(kstart, kend);
	}

	// exit : delete resources used locally
	if (m_pSpkDoc->IsModified())
	{
		m_ChartSpkWnd_Shape.Invalidate();
		m_ChartSpkWnd_Shape.DisplayExData(p_mean0);
	}

	delete[] p_sum0;
	delete[] p_mean0;
	delete[] p_cxy0;
	delete[] p_dummy0;

	OnMeasure();
}

void ViewSpikeSort::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// formview scroll: if pointer null
	if (pScrollBar == nullptr)
	{
		dbTableView::OnHScroll(nSBCode, nPos, pScrollBar);
		return;
	}
	// trap messages from ScrollBarEx
	CString cs;
	switch (nSBCode)
	{
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		m_filescroll.GetScrollInfo(&m_filescroll_infos, SIF_ALL);
		m_lFirst = m_filescroll_infos.nPos;
		m_lLast = m_lFirst + long(m_filescroll_infos.nPage) - 1;
		break;

	default:
		scrollFile(nSBCode, nPos);
		break;
	}
	updateLegends();
}

void ViewSpikeSort::scrollFile(UINT nSBCode, UINT nPos)
{
	// get corresponding data
	const auto total_scroll = m_pSpkDoc->GetAcqSize();
	const auto page_scroll = (m_lLast - m_lFirst);
	auto sb_scroll = MulDiv(page_scroll, 10, 100);
	if (sb_scroll == 0)
		sb_scroll = 1;
	auto l_first = m_lFirst;
	switch (nSBCode)
	{
	case SB_LEFT:
		l_first = 0;
		break; // Scroll to far left.
	case SB_LINELEFT:
		l_first -= sb_scroll;
		break; // Scroll left.
	case SB_LINERIGHT:
		l_first += sb_scroll;
		break; // Scroll right
	case SB_PAGELEFT:
		l_first -= page_scroll;
		break; // Scroll one page left
	case SB_PAGERIGHT:
		l_first += page_scroll;
		break; // Scroll one page right.
	case SB_RIGHT:
		l_first = total_scroll - page_scroll + 1;
		break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
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

	m_lFirst = l_first;
	m_lLast = l_last;
}

void ViewSpikeSort::updateFileScroll()
{
	m_filescroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_filescroll_infos.nPos = m_lFirst;
	m_filescroll_infos.nPage = m_lLast - m_lFirst + 1;
	m_filescroll.SetScrollInfo(&m_filescroll_infos);
}

void ViewSpikeSort::selectSpkList(int icursel)
{
	m_pSpkList = m_pSpkDoc->SetSpkList_AsCurrent(icursel);
	//GetDocument()->GetCurrent_Spk_Document()->SetSpkList_CurrentIndex(icursel);
	ASSERT(m_pSpkList != NULL);
	OnMeasure();

	// update source data: change data channel and update display
	yhistogram_wnd_.SetSpkList(m_pSpkList);
	m_ChartSpkWnd_Shape.SetSpkList(m_pSpkList);
	m_ChartSpkWnd_Bar.SetSpkList(m_pSpkList);
	xygraph_wnd_.SetSpkList(m_pSpkList);

	yhistogram_wnd_.Invalidate();
	xygraph_wnd_.Invalidate();
	m_ChartSpkWnd_Shape.Invalidate();
	m_ChartSpkWnd_Bar.Invalidate();
}

void ViewSpikeSort::OnEnChangeEditleft2()
{
	if (mm_txyleft.m_bEntryDone)
	{
		auto left = m_txyleft;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();
		switch (mm_txyleft.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			left = m_txyleft;
			break;
		case VK_UP:
		case VK_PRIOR: left += delta;
			break;
		case VK_DOWN:
		case VK_NEXT: left -= delta;
			break;
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
		if (itleft != xygraph_wnd_.m_VTtags.GetValue(m_ixyleft))
		{
			m_psC->ixyleft = itleft;
			xygraph_wnd_.MoveVTtagtoVal(m_ixyleft, itleft);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeEditright2()
{
	if (mm_txyright.m_bEntryDone)
	{
		auto right = m_txyright;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();
		switch (mm_txyright.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			right = m_txyright;
			break;
		case VK_UP:
		case VK_PRIOR: right += delta;
			break;
		case VK_DOWN:
		case VK_NEXT: right -= delta;
			break;
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
		if (itright != xygraph_wnd_.m_VTtags.GetValue(m_ixyright))
		{
			m_psC->ixyright = itright;
			xygraph_wnd_.MoveVTtagtoVal(m_ixyright, itright);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeSourceclass()
{
	if (mm_sourceclass.m_bEntryDone)
	{
		auto sourceclass = m_sourceclass;
		switch (mm_sourceclass.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			sourceclass = m_sourceclass;
			break;
		case VK_UP:
		case VK_PRIOR: sourceclass++;
			break;
		case VK_DOWN:
		case VK_NEXT: sourceclass--;
			break;
		default: ;
		}
		// change display if necessary
		mm_sourceclass.m_bEntryDone = FALSE; // clear flag
		mm_sourceclass.m_nChar = 0; // empty buffer
		mm_sourceclass.SetSel(0, -1); // select all text
		if (sourceclass != m_sourceclass)
		{
			m_sourceclass = sourceclass;
			m_ChartSpkWnd_Shape.SetPlotMode(PLOT_ONECOLOR, m_sourceclass);
			xygraph_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
			yhistogram_wnd_.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
			m_ChartSpkWnd_Bar.SetPlotMode(PLOT_CLASSCOLORS, m_sourceclass);
		}
		// change histogram accordingly
		m_ChartSpkWnd_Shape.Invalidate();
		m_ChartSpkWnd_Bar.Invalidate();
		xygraph_wnd_.Invalidate();
		yhistogram_wnd_.Invalidate();

		selectSpikeFromCurrentList(-1);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeDestinationclass()
{
	if (mm_destinationclass.m_bEntryDone)
	{
		short destinationclass = m_destinationclass;
		switch (mm_destinationclass.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			destinationclass = m_destinationclass;
			break;
		case VK_UP:
		case VK_PRIOR: destinationclass++;
			break;
		case VK_DOWN:
		case VK_NEXT: destinationclass--;
			break;
		default: ;
		}
		// change display if necessary
		mm_destinationclass.m_bEntryDone = FALSE; // clear flag
		mm_destinationclass.m_nChar = 0; // empty buffer
		mm_destinationclass.SetSel(0, -1); // select all text
		m_destinationclass = destinationclass;
		selectSpikeFromCurrentList(-1);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnSelchangeParameter()
{
	const auto iparameter = m_CBparameter.GetCurSel();
	if (iparameter != m_psC->iparameter)
	{
		m_psC->iparameter = iparameter;
		activateMode4();
		OnMeasure();
		OnFormatCentercurve();
	}

	//    STATIC3 lower STATIC4 upper STATIC5 T1 STATIC6 T2
	// 0  mV      vis    mV      vis    vis     vis  vis   vis
	// 1  mV      vis    mV      vis    vis     vis  NOT   NOT
	// 2  mS      vis    mS      vis    vis     vis  vis   vis
}

void ViewSpikeSort::OnEnChangelower()
{
	if (mm_lower.m_bEntryDone)
	{
		auto lower = m_lower;
		m_delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
		switch (mm_lower.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			lower = m_lower;
			break;
		case VK_UP:
		case VK_PRIOR: lower += m_delta;
			break;
		case VK_DOWN:
		case VK_NEXT: lower -= m_delta;
			break;
		default: ;
		}
		// check boundaries
		//if (lower < 0) lower = 0;
		if (lower >= m_upper)
			lower = m_upper - m_delta * 10.f;
		// change display if necessary
		mm_lower.m_bEntryDone = FALSE; 
		mm_lower.m_nChar = 0; 
		mm_lower.SetSel(0, -1);
		m_lower = lower;
		m_psC->ilower = static_cast<int>(m_lower / m_delta);
		if (m_psC->ilower != xygraph_wnd_.m_HZtags.GetValue(m_itaglow))
			xygraph_wnd_.MoveHZtagtoVal(m_itaglow, m_psC->ilower);
		if (m_psC->ilower != yhistogram_wnd_.m_VTtags.GetValue(m_spkhistlower))
			yhistogram_wnd_.MoveVTtagtoVal(m_spkhistlower, m_psC->ilower);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeupper()
{
	if (mm_upper.m_bEntryDone)
	{
		auto upper = m_upper;
		m_delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;

		switch (mm_upper.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN:
			UpdateData(TRUE);
			upper = m_upper;
			break;
		case VK_UP:
		case VK_PRIOR: upper += m_delta;
			break;
		case VK_DOWN:
		case VK_NEXT: upper -= m_delta;
			break;
		default: ;
		}

		// check boundaries
		//if (upper < 0) upper = 0;
		if (upper <= m_lower)
			upper = m_lower + m_delta * 10.f;

		// change display if necessary
		mm_upper.m_bEntryDone = FALSE;
		mm_upper.m_nChar = 0;
		mm_upper.SetSel(0, -1);
		m_upper = upper;
		m_psC->iupper = static_cast<int>(m_upper / m_delta);
		if (m_psC->iupper != xygraph_wnd_.m_HZtags.GetValue(m_itagup))
			xygraph_wnd_.MoveHZtagtoVal(m_itagup, m_psC->iupper);
		if (m_psC->ilower != yhistogram_wnd_.m_VTtags.GetValue(m_spkhistupper))
			yhistogram_wnd_.MoveVTtagtoVal(m_spkhistupper, m_psC->iupper);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeT1()
{
	if (mm_t1.m_bEntryDone)
	{
		auto t1 = m_t1;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();

		switch (mm_t1.m_nChar)
		{
		// load data from edit controls
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
		default: ;
		}
		// check boundaries
		if (t1 < 0)
			t1 = 0.0f;
		if (t1 >= m_t2)
			t1 = m_t2 - delta;
		// change display if necessary
		mm_t1.m_bEntryDone = FALSE; // clear flag
		mm_t1.m_nChar = 0; // empty buffer
		mm_t1.SetSel(0, -1); // select all text
		m_t1 = t1;
		const auto it1 = static_cast<int>(m_t1 / delta);
		if (it1 != m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagleft))
		{
			m_psC->ileft = it1;
			m_ChartSpkWnd_Shape.MoveVTtrack(m_spkformtagleft, m_psC->ileft);
			m_pSpkList->m_imaxmin1SL = m_psC->ileft;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeT2()
{
	if (mm_t2.m_bEntryDone)
	{
		auto t2 = m_t2;
		const auto delta = m_tunit / m_pSpkList->GetAcqSampRate();
		switch (mm_t2.m_nChar)
		{
		// load data from edit controls
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
		default: ;
		}

		// check boundaries
		if (t2 < m_t1)
			t2 = m_t1 + delta;
		const auto tmax = (float(m_pSpkList->GetSpikeLength()) - 1.f) * delta;
		if (t2 >= tmax)
			t2 = tmax;
		// change display if necessary
		mm_t2.m_bEntryDone = FALSE; // clear flag
		mm_t2.m_nChar = 0; // empty buffer
		mm_t2.SetSel(0, -1); // select all text
		m_t2 = t2;
		const auto it2 = static_cast<int>(m_t2 / delta);
		if (it2 != m_ChartSpkWnd_Shape.m_VTtags.GetValue(m_spkformtagright))
		{
			m_psC->iright = it2;
			m_ChartSpkWnd_Shape.MoveVTtrack(m_spkformtagright, m_psC->iright);
			m_pSpkList->m_imaxmin2SL = m_psC->iright;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangetimeFirst()
{
	if (mm_timeFirst.m_bEntryDone)
	{
		auto time_first = m_timeFirst;
		switch (mm_timeFirst.m_nChar)
		{
		case VK_RETURN: UpdateData(TRUE);
			time_first = m_timeFirst;
			break;
		case VK_UP:
		case VK_PRIOR: time_first++;
			break;
		case VK_DOWN:
		case VK_NEXT: time_first--;
			break;
		default: ;
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
		updateLegends();
	}
}

void ViewSpikeSort::OnEnChangetimeLast()
{
	if (mm_timeLast.m_bEntryDone)
	{
		auto time_last = m_timeLast;
		switch (mm_timeLast.m_nChar)
		{
		case VK_RETURN: UpdateData(TRUE);
			time_last = m_timeLast;
			break;
		case VK_UP:
		case VK_PRIOR: time_last++;
			break;
		case VK_DOWN:
		case VK_NEXT: time_last--;
			break;
		default: ;
		}

		// check boundaries
		if (time_last <= m_timeFirst)
			m_lLast = static_cast<long>(float((m_pSpkDoc->GetAcqSize()) - 1.f) / m_pSpkList->GetAcqSampRate());

		// change display if necessary
		mm_timeLast.m_bEntryDone = FALSE;
		mm_timeLast.m_nChar = 0;
		mm_timeLast.SetSel(0, -1);
		m_timeLast = time_last;
		m_lLast = static_cast<long>(m_timeLast * m_pSpkList->GetAcqSampRate());
		updateLegends();
	}
}

void ViewSpikeSort::OnEnChangemVMin()
{
	if (mm_mVMin.m_bEntryDone)
	{
		auto mv_min = m_mVMin;
		switch (mm_mVMin.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			mv_min = m_mVMin;
			break;
		case VK_UP:
		case VK_PRIOR: mv_min++;
			break;
		case VK_DOWN:
		case VK_NEXT: mv_min--;
			break;
		default: ;
		}
		// check boundaries
		if (mv_min >= m_mVMax)
			mv_min = m_mVMax - 1.f;

		// change display if necessary
		mm_mVMin.m_bEntryDone = FALSE;
		mm_mVMin.m_nChar = 0;
		mm_mVMin.SetSel(0, -1);
		m_mVMin = mv_min;

		updateGain();
		updateLegends();
	}
}

void ViewSpikeSort::OnEnChangemVMax()
{
	if (mm_mVMax.m_bEntryDone)
	{
		auto mv_max = m_mVMax;
		switch (mm_mVMax.m_nChar)
		{
		case VK_RETURN:
			UpdateData(TRUE);
			mv_max = m_mVMax;
			break;
		case VK_UP:
		case VK_PRIOR: mv_max++;
			break;
		case VK_DOWN:
		case VK_NEXT: mv_max--;
			break;
		default: ;
		}

		// check boundaries
		if (mv_max <= m_mVMin)
			mv_max = m_mVMin + 1.f;

		// change display if necessary
		mm_mVMax.m_bEntryDone = FALSE;
		mm_mVMax.m_nChar = 0;
		mm_mVMax.SetSel(0, -1);
		m_mVMax = mv_max;

		updateGain();
		updateLegends();
	}
}

void ViewSpikeSort::OnEnChangeNOspike()
{
	if (mm_spikeno.m_bEntryDone)
	{
		const auto spikeno = m_spikeno;
		switch (mm_spikeno.m_nChar)
		{
		// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_spikeno++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_spikeno--;
			break;
		default: ;
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
				const auto spike_element = m_pSpkList->GetSpike(m_spikeno);
				const auto spk_first = spike_element->get_time() - m_pSpkList->GetDetectParms()->prethreshold;
				const auto spk_last = spk_first + m_pSpkList->GetSpikeLength();

				if (spk_first < m_lFirst || spk_last > m_lLast)
				{
					const auto lspan = (m_lLast - m_lFirst) / 2;
					const auto lcenter = (spk_last + spk_first) / 2;
					m_lFirst = lcenter - lspan;
					m_lLast = lcenter + lspan;
					updateLegends();
				}
			}
		}
		selectSpikeFromCurrentList(m_spikeno);
	}
}

void ViewSpikeSort::OnEnChangeSpikenoclass()
{
	if (mm_spikenoclass.m_bEntryDone)
	{
		const auto spikenoclass = m_spikenoclass;
		switch (mm_spikenoclass.m_nChar)
		{
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_spikenoclass++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_spikenoclass--;
			break;
		default: ;
		}

		mm_spikenoclass.m_bEntryDone = FALSE;
		mm_spikenoclass.m_nChar = 0;
		mm_spikenoclass.SetSel(0, -1);

		if (m_spikenoclass != spikenoclass)
		{
			m_pSpkDoc->SetModifiedFlag(TRUE);
			const auto currentlist = m_tabCtrl.GetCurSel();
			auto* spike_list = m_pSpkDoc->SetSpkList_AsCurrent(currentlist);
			spike_list->GetSpike(m_spikeno)->set_class(m_spikenoclass);
			updateLegends();
		}
	}
}

void ViewSpikeSort::OnEnChangeNBins()
{
	if (mm_mVbin.m_bEntryDone)
	{
		auto mVbin = m_mVbin;
		const auto delta = (m_mVMax - m_mVMin) / 10.f;
		switch (mm_mVbin.m_nChar)
		{
		case VK_RETURN: UpdateData(TRUE);
			mVbin = m_mVbin;
			break;
		case VK_UP:
		case VK_PRIOR: mVbin += delta;
			break;
		case VK_DOWN:
		case VK_NEXT: mVbin -= delta;
			break;
		default: ;
		}

		mm_mVbin.m_bEntryDone = FALSE;
		mm_mVbin.m_nChar = 0;
		mm_mVbin.SetSel(0, -1);

		if (m_mVbin != mVbin)
		{
			m_mVbin = mVbin;
			buildHistogram();
			updateLegends();
		}
	}
}
