#include "StdAfx.h"
#include "ViewSpikeSort.h"

#include "dbWave.h"
#include "DlgProgress.h"
#include "DlgSpikeEdit.h"
#include "MainFrm.h"


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
	m_psC->sourceclass = m_source_class;
	m_psC->destclass = m_destination_class;
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
	DDX_Text(pDX, IDC_SOURCECLASS, m_source_class);
	DDX_Text(pDX, IDC_DESTINATIONCLASS, m_destination_class);
	DDX_Text(pDX, IDC_EDIT2, m_timeFirst);
	DDX_Text(pDX, IDC_EDIT3, m_timeLast);
	DDX_Text(pDX, IDC_EDIT6, m_mVMax);
	DDX_Text(pDX, IDC_EDIT7, m_mVMin);
	DDX_Text(pDX, IDC_BINMV, m_mV_bin);
	DDX_Check(pDX, IDC_CHECK1, m_bAllFiles);
	DDX_Text(pDX, IDC_NSPIKES, m_spike_index);
	DDX_Text(pDX, IDC_SPIKECLASS, m_spike_index_class);
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
	ON_EN_CHANGE(IDC_SPIKECLASS, &ViewSpikeSort::OnEnChangespike_indexclass)
	ON_EN_CHANGE(IDC_BINMV, &ViewSpikeSort::OnEnChangeNBins)
END_MESSAGE_MAP()

void ViewSpikeSort::define_sub_classed_items()
{
	// subclass some controls
	VERIFY(m_chart_histogram.SubclassDlgItem(IDC_HISTOGRAM, this));
	VERIFY(m_chart_measures.SubclassDlgItem(IDC_DISPLAYPARM, this));
	VERIFY(m_chart_spike_shapes.SubclassDlgItem(IDC_DISPLAYSPIKE, this));
	VERIFY(m_chart_spike_bars.SubclassDlgItem(IDC_DISPLAYBARS, this));

	VERIFY(mm_t1.SubclassDlgItem(IDC_T1, this));
	VERIFY(mm_t2.SubclassDlgItem(IDC_T2, this));
	VERIFY(mm_lower.SubclassDlgItem(IDC_LIMITLOWER, this));
	VERIFY(mm_upper.SubclassDlgItem(IDC_LIMITUPPER, this));
	VERIFY(mm_source_class.SubclassDlgItem(IDC_SOURCECLASS, this));
	VERIFY(mm_destination_class.SubclassDlgItem(IDC_DESTINATIONCLASS, this));
	VERIFY(mm_timeFirst.SubclassDlgItem(IDC_EDIT2, this));
	VERIFY(mm_timeLast.SubclassDlgItem(IDC_EDIT3, this));
	VERIFY(mm_mVMax.SubclassDlgItem(IDC_EDIT6, this));
	VERIFY(mm_mVMin.SubclassDlgItem(IDC_EDIT7, this));
	VERIFY(mm_txyright.SubclassDlgItem(IDC_EDITRIGHT2, this));
	VERIFY(mm_txyleft.SubclassDlgItem(IDC_EDITLEFT2, this));
	VERIFY(mm_mV_bin.SubclassDlgItem(IDC_BINMV, this));

	VERIFY(mm_spike_index.SubclassDlgItem(IDC_NSPIKES, this));
	mm_spike_index.ShowScrollBar(SB_VERT);
	VERIFY(mm_spike_index_class.SubclassDlgItem(IDC_SPIKECLASS, this));
	mm_spike_index_class.ShowScrollBar(SB_VERT);

	VERIFY(m_file_scroll.SubclassDlgItem(IDC_FILESCROLL, this));
	m_file_scroll.SetScrollRange(0, 100, FALSE);
}

void ViewSpikeSort::define_stretch_parameters()
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
	define_sub_classed_items();
	define_stretch_parameters();
	m_binit = TRUE;
	m_autoIncrement = true;
	m_autoDetect = true;

	// load global parameters
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	m_psC = &(p_app->spk_classif);
	m_pOptionsViewData = &(p_app->options_viewdata);

	// assign values to controls
	m_CBparameter.SetCurSel(m_psC->iparameter);
	m_mVMax = m_psC->mvmax;
	m_mVMin = m_psC->mvmin;

	m_source_class = m_psC->sourceclass;
	m_destination_class = m_psC->destclass;

	m_chart_spike_shapes.DisplayAllFiles(false, GetDocument());
	m_chart_spike_shapes.set_plot_mode(PLOT_ONECOLOR, m_source_class);
	m_chart_spike_shapes.SetScopeParameters(&(m_pOptionsViewData->spksort1spk));
	m_spkform_tag_left = m_chart_spike_shapes.m_VTtags.AddTag(m_psC->ileft, 0);
	m_spkform_tag_right = m_chart_spike_shapes.m_VTtags.AddTag(m_psC->iright, 0);

	m_chart_measures.DisplayAllFiles(false, GetDocument());
	m_chart_measures.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	m_chart_measures.SetScopeParameters(&(m_pOptionsViewData->spksort1parms));
	m_itagup = m_chart_measures.m_HZtags.AddTag(m_psC->iupper, 0);
	m_itaglow = m_chart_measures.m_HZtags.AddTag(m_psC->ilower, 0);

	m_chart_spike_bars.DisplayAllFiles(false, GetDocument());
	m_chart_spike_bars.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	m_chart_spike_bars.SetScopeParameters(&(m_pOptionsViewData->spksort1bars));

	m_chart_histogram.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	m_chart_histogram.SetScopeParameters(&(m_pOptionsViewData->spksort1hist));

	// display tag lines at proper places
	m_spkhist_upper = m_chart_histogram.m_VTtags.AddTag(m_psC->iupper, 0);
	m_spkhist_lower = m_chart_histogram.m_VTtags.AddTag(m_psC->ilower, 0);

	update_file_parameters();
	if (nullptr != m_pSpkList)
	{
		m_delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
		m_lower = static_cast<float>(m_psC->ilower) * m_delta;
		m_upper = static_cast<float>(m_psC->iupper) * m_delta;
		UpdateData(false);
	}
	activate_mode4();
}

void ViewSpikeSort::activate_mode4()
{
	auto n_cmd_show = SW_HIDE;
	if (4 == m_psC->iparameter)
	{
		n_cmd_show = SW_SHOW;
		if (1 > m_chart_measures.m_VTtags.GetNTags())
		{
			m_ixyright = m_chart_measures.m_VTtags.AddTag(m_psC->ixyright, 0);
			m_ixyleft = m_chart_measures.m_VTtags.AddTag(m_psC->ixyleft, 0);
			const auto delta = m_pSpkList->GetAcqSampRate() / m_time_unit;
			m_txyright = static_cast<float>(m_psC->ixyright) / delta;
			m_txyleft = static_cast<float>(m_psC->ixyleft) / delta;
		}
		m_chart_measures.SetNxScaleCells(2, 0, 0);
		m_chart_measures.GetScopeParameters()->crScopeGrid = RGB(128, 128, 128);

		if (nullptr != m_pSpkList)
		{
			const auto spike_length_ms = (static_cast<double>(static_cast<float>(m_pSpkList->get_spike_length()) * m_time_unit / m_pSpkList->GetAcqSampRate()));
			CString cs_dummy;
			cs_dummy.Format(_T("%0.1f ms"), spike_length_ms);
			GetDlgItem(IDC_STATICRIGHT)->SetWindowText(cs_dummy);
			cs_dummy.Format(_T("%0.1f ms"), -spike_length_ms);
			GetDlgItem(IDC_STATICLEFT)->SetWindowText(cs_dummy);
		}
	}
	else
	{
		m_chart_measures.m_VTtags.RemoveAllTags();
		m_chart_measures.SetNxScaleCells(0, 0, 0);
	}
	GetDlgItem(IDC_STATICRIGHT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATICLEFT)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_STATIC12)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITRIGHT2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_EDITLEFT2)->ShowWindow(n_cmd_show);
	m_chart_measures.Invalidate();
}

void ViewSpikeSort::OnActivateView(const BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
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
		const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
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
			update_file_parameters();
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

void ViewSpikeSort::update_spike_file()
{
	m_pSpkDoc = GetDocument()->open_current_spike_file();

	if (nullptr != m_pSpkDoc)
	{
		m_pSpkDoc->SetModifiedFlag(FALSE);
		m_pSpkDoc->SetPathName(GetDocument()->GetDB_CurrentSpkFileName(), FALSE);
		const int current_index = GetDocument()->GetCurrent_Spk_Document()->GetSpkList_CurrentIndex();
		m_pSpkList = m_pSpkDoc->set_spk_list_as_current(current_index);

		// update Tab at the bottom
		m_tabCtrl.InitctrlTabFromSpikeDoc(m_pSpkDoc);
		m_tabCtrl.SetCurSel(current_index);
	}
}

void ViewSpikeSort::update_file_parameters()
{
	// reset parameters ? flag = single file or file list has changed
	if (!m_bAllFiles)
	{
		m_chart_histogram.RemoveHistData();
	}

	const BOOL first_update = (m_pSpkDoc == nullptr);
	update_spike_file();

	if (first_update || m_pOptionsViewData->bEntireRecord)
	{
		if (m_pSpkDoc == nullptr)
			return;
		m_timeFirst = 0.f;
		m_timeLast = (static_cast<float>(m_pSpkDoc->GetAcqSize()) - 1) / m_pSpkList->GetAcqSampRate();
	}
	m_lFirst = static_cast<long>(m_timeFirst * m_pSpkList->GetAcqSampRate());
	m_lLast = static_cast<long>(m_timeLast * m_pSpkList->GetAcqSampRate());

	// spike and classes
	auto spike_index = m_pSpkList->m_selected_spike;
	if (m_pSpkList->get_spikes_count() < spike_index || 0 > spike_index)
	{
		spike_index = -1;
		m_source_class = 0;
	}
	else
	{
		m_source_class = m_pSpkList->get_spike(spike_index)->get_class_id();
		m_psC->sourceclass = m_source_class;
	}
	ASSERT(m_source_class < 32768);

	if (0 == m_psC->ileft && 0 == m_psC->iright)
	{
		m_psC->ileft = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
		m_psC->iright = m_psC->ileft + m_pSpkList->get_detection_parameters()->detect_refractory_period;
	}
	m_t1 = static_cast<float>(m_psC->ileft) * m_time_unit / m_pSpkList->GetAcqSampRate();
	m_t2 = static_cast<float>(m_psC->iright) * m_time_unit / m_pSpkList->GetAcqSampRate();

	m_chart_spike_bars.set_source_data(m_pSpkList, GetDocument());
	m_chart_spike_bars.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);

	m_chart_spike_shapes.set_source_data(m_pSpkList, GetDocument());
	m_chart_spike_shapes.m_VTtags.SetTagVal(m_spkform_tag_left, m_psC->ileft);
	m_chart_spike_shapes.m_VTtags.SetTagVal(m_spkform_tag_right, m_psC->iright);
	m_chart_spike_shapes.set_plot_mode(PLOT_ONECOLOR, m_source_class);

	m_chart_measures.set_source_data(m_pSpkList, GetDocument());
	m_chart_measures.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
	m_chart_histogram.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);

	m_file_scroll_infos.fMask = SIF_ALL;
	m_file_scroll_infos.nMin = 0;
	m_file_scroll_infos.nMax = m_pSpkDoc->GetAcqSize() - 1;
	m_file_scroll_infos.nPos = 0;
	m_file_scroll_infos.nPage = m_pSpkDoc->GetAcqSize();
	m_file_scroll.SetScrollInfo(&m_file_scroll_infos);

	update_legends();

	// display & compute parameters
	if (!m_bAllFiles || !m_bMeasureDone)
	{
		if (4 != m_psC->iparameter)
		{
			m_chart_measures.SetTimeIntervals(m_lFirst, m_lLast);
			if (m_chart_measures.m_VTtags.GetNTags() > 0)
			{
				m_chart_measures.m_VTtags.RemoveAllTags();
				m_chart_measures.Invalidate();
			}
		}
		else
		{
			m_chart_measures.SetTimeIntervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
			if (1 > m_chart_measures.m_VTtags.GetNTags())
			{
				m_ixyright = m_chart_measures.m_VTtags.AddTag(m_psC->ixyright, 0);
				m_ixyleft = m_chart_measures.m_VTtags.AddTag(m_psC->ixyleft, 0);
				const auto delta = m_pSpkList->GetAcqSampRate() / m_time_unit;
				m_txyright = static_cast<float>(m_psC->ixyright) / delta;
				m_txyleft = static_cast<float>(m_psC->ixyleft) / delta;
				m_chart_measures.Invalidate();
			}
		}
		// update text , display and compute histogram
		m_bMeasureDone = FALSE; 
		OnMeasure();
	}

	select_spike_from_current_list(spike_index);
}

void ViewSpikeSort::update_legends()
{
	// update text abscissa and horizontal scroll position
	m_timeFirst = static_cast<float>(m_lFirst) / m_pSpkList->GetAcqSampRate();
	m_timeLast = static_cast<float>(m_lLast) / m_pSpkList->GetAcqSampRate();
	update_file_scroll();

	if (4 != m_psC->iparameter)
		m_chart_measures.SetTimeIntervals(m_lFirst, m_lLast);
	else
		m_chart_measures.SetTimeIntervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
	m_chart_measures.Invalidate();

	m_chart_spike_shapes.SetTimeIntervals(m_lFirst, m_lLast);
	m_chart_spike_shapes.Invalidate();

	m_chart_spike_bars.SetTimeIntervals(m_lFirst, m_lLast);
	m_chart_spike_bars.Invalidate();

	m_chart_histogram.Invalidate();

	UpdateData(FALSE);
}

void ViewSpikeSort::OnSort()
{
	// set file indexes - assume only one file selected
	const auto pdb_doc = GetDocument();
	const int current_file = pdb_doc->GetDB_CurrentRecordPosition();
	auto first_file = current_file;
	auto last_file = first_file;
	const auto n_files = pdb_doc->GetDB_NRecords();
	const auto current_list = m_pSpkDoc->GetSpkList_CurrentIndex();

	// change indexes if ALL files selected
	DlgProgress* dlg_progress = nullptr;
	auto i_step = 0;
	CString cs_comment;
	if (m_bAllFiles)
	{
		first_file = 0; // index first file
		last_file = pdb_doc->GetDB_NRecords() - 1; // index last file
		dlg_progress = new DlgProgress;
		dlg_progress->Create();
		dlg_progress->SetStep(1);
	}

	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		// load spike file
		BOOL flag_changed;
		pdb_doc->set_db_current_record_position(i_file);
		m_pSpkDoc = pdb_doc->open_current_spike_file();
		if (nullptr == m_pSpkDoc)
			continue;

		// update screen if multi-file requested
		if (m_bAllFiles)
		{
			if (dlg_progress->CheckCancelButton())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cs_comment.Format(_T("Processing file [%i / %i]"), i_file + 1, n_files);
			dlg_progress->SetStatus(cs_comment);
			if (MulDiv(i_file, 100, n_files) > i_step)
			{
				dlg_progress->StepIt();
				i_step = MulDiv(i_file, 100, n_files);
			}
		}

		// load spike list
		m_pSpkList = m_pSpkDoc->set_spk_list_as_current(current_list);
		if ((nullptr == m_pSpkList) || (0 == m_pSpkList->get_spike_length()))
			continue;

		// loop over all spikes of the list and compare to a single parameter
		const CSize limits1(m_psC->ilower, m_psC->iupper);
		const CSize from_class_id_to_class_id(m_source_class, m_destination_class);
		const CSize time_window(m_lFirst, m_lLast);
		// sort on 1 parameter
		if (4 != m_psC->iparameter)
		{
			flag_changed = m_pSpkList->SortSpikeWithY1(from_class_id_to_class_id, time_window, limits1);
		}
		// sort on 2 parameters
		else
		{
			const CSize limits2(m_psC->ixyleft, m_psC->ixyright);
			flag_changed = m_pSpkList->SortSpikeWithY1AndY2(from_class_id_to_class_id, time_window, limits1, limits2);
		}

		if (flag_changed)
		{
			m_pSpkDoc->OnSaveDocument(pdb_doc->GetDB_CurrentSpkFileName(FALSE));
			pdb_doc->SetDB_n_spikes(m_pSpkList->get_spikes_count());
		}
	}

	// end of loop, select current file again if necessary
	if (m_bAllFiles)
	{
		delete dlg_progress;
		pdb_doc->set_db_current_record_position(current_file);
		m_pSpkDoc = pdb_doc->open_current_spike_file();
		m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	}

	// refresh data windows
	build_histogram();

	m_chart_measures.Invalidate();
	m_chart_spike_shapes.Invalidate();
	m_chart_spike_bars.Invalidate();
	m_chart_histogram.Invalidate();

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
			m_lFirst = m_chart_measures.GetTimeFirst();
			m_lLast = m_chart_measures.GetTimeLast();
		}
		else
		{
			m_lFirst = m_chart_spike_bars.GetTimeFirst();
			m_lLast = m_chart_spike_bars.GetTimeLast();
		}
		update_legends();
		break;

	case HINT_HITSPIKE: // -------------  spike is selected or deselected
		{
			if (m_pSpkList->GetSpikeFlagArrayCount() > 0)
				unflag_all_spikes();
			auto spike_index = 0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spike_index = m_chart_spike_shapes.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spike_index = m_chart_spike_bars.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spike_index = m_chart_measures.GetHitSpike();

			select_spike_from_current_list(spike_index);
		}
		break;

	case HINT_SELECTSPIKES:
		m_chart_measures.Invalidate();
		m_chart_spike_shapes.Invalidate();
		m_chart_spike_bars.Invalidate();
		break;

	case HINT_DBLCLKSEL:
		{
			auto spike_index = 0;
			if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
				spike_index = m_chart_spike_shapes.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYBARS)
				spike_index = m_chart_spike_bars.GetHitSpike();
			else if (HIWORD(lParam) == IDC_DISPLAYPARM)
				spike_index = m_chart_measures.GetHitSpike();
			// if m_bAllFiles, spike_index is global, otherwise it comes from a single file...
			select_spike_from_current_list(spike_index);
			OnToolsEdittransformspikes();
		}
		break;

	//case HINT_MOVEVERTTAG: // -------------  vertical tag has moved lowp = tag index
	case HINT_CHANGEVERTTAG: // -------------  vertical tag value has changed
		if (HIWORD(lParam) == IDC_DISPLAYSPIKE)
		{
			if (shortValue == m_spkform_tag_left) // first tag
			{
				m_psC->ileft = m_chart_spike_shapes.m_VTtags.GetValue(m_spkform_tag_left);
				m_t1 = static_cast<float>(m_psC->ileft) * m_time_unit / m_pSpkList->GetAcqSampRate();
				mm_t1.m_bEntryDone = TRUE;
				OnEnChangeT1();
			}
			else if (shortValue == m_spkform_tag_right) // second tag
			{
				m_psC->iright = m_chart_spike_shapes.m_VTtags.GetValue(m_spkform_tag_right);
				m_t2 = static_cast<float>(m_psC->iright) * m_time_unit / m_pSpkList->GetAcqSampRate();
				mm_t2.m_bEntryDone = TRUE;
				OnEnChangeT2();
			}
		}
		else if (HIWORD(lParam) == IDC_HISTOGRAM)
		{
			if (shortValue == m_spkhist_lower) // first tag
			{
				m_psC->ilower = m_chart_histogram.m_VTtags.GetValue(m_spkhist_lower);
				m_lower = static_cast<float>(m_psC->ilower) * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				UpdateData(false);
			}
			else if (shortValue == m_spkhist_upper) // second tag
			{
				m_psC->iupper = m_chart_histogram.m_VTtags.GetValue(m_spkhist_upper); // load new value
				m_upper = static_cast<float>(m_psC->iupper) * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				UpdateData(false);
			}
		}
		else if (HIWORD(lParam) == IDC_DISPLAYPARM)
		{
			if (shortValue == m_ixyright)
			{
				const auto delta = m_pSpkList->GetAcqSampRate() / m_time_unit;
				m_psC->ixyright = m_chart_measures.m_VTtags.GetValue(m_ixyright);
				m_txyright = static_cast<float>(m_psC->ixyright) / delta;
				mm_txyright.m_bEntryDone = TRUE;
				OnEnChangeEditright2();
			}
			else if (shortValue == m_ixyleft)
			{
				const auto delta = m_pSpkList->GetAcqSampRate() / m_time_unit;
				m_psC->ixyleft = m_chart_measures.m_VTtags.GetValue(m_ixyleft);
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
				m_psC->ilower = m_chart_measures.m_HZtags.GetValue(m_itaglow);
				m_lower = static_cast<float>(m_psC->ilower) * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				mm_lower.m_bEntryDone = TRUE;
				OnEnChangelower();
			}
			else if (shortValue == m_itagup) // second tag
			{
				m_psC->iupper = m_chart_measures.m_HZtags.GetValue(m_itagup); 
				m_upper = static_cast<float>(m_psC->iupper) * m_pSpkList->GetAcqVoltsperBin() * m_vunit;
				mm_upper.m_bEntryDone = TRUE;
				OnEnChangeupper();
			}
		}
		break;

	case HINT_VIEWSIZECHANGED: // ------------- change zoom
		update_legends();
		break;

	case HINT_WINDOWPROPSCHANGED:
		m_pOptionsViewData->spksort1spk = *m_chart_spike_shapes.GetScopeParameters();
		m_pOptionsViewData->spksort1parms = *m_chart_measures.GetScopeParameters();
		m_pOptionsViewData->spksort1hist = *m_chart_histogram.GetScopeParameters();
		m_pOptionsViewData->spksort1bars = *m_chart_spike_bars.GetScopeParameters();
		break;
	case HINT_VIEWTABHASCHANGED:
		select_spike_list(shortValue);
		break;

	default:
		break;
	}
	return 0L;
}

void ViewSpikeSort::unflag_all_spikes()
{
	if (m_bAllFiles)
	{
		const auto pdb_doc = GetDocument();
		for (auto i_file = 0; i_file < pdb_doc->GetDB_NRecords(); i_file++)
		{
			pdb_doc->set_db_current_record_position(i_file);
			m_pSpkDoc = pdb_doc->open_current_spike_file();

			for (auto j = 0; j < m_pSpkDoc->GetSpkList_Size(); j++)
			{
				m_pSpkList = m_pSpkDoc->set_spk_list_as_current(j);
				m_pSpkList->RemoveAllSpikeFlags();
			}
		}
	}
	else
		m_pSpkList->RemoveAllSpikeFlags();
	m_chart_measures.Invalidate();
	m_chart_spike_shapes.Invalidate();
	m_chart_spike_bars.Invalidate();
}

void ViewSpikeSort::OnMeasure()
{
	// set file indexes - assume only one file selected
	const auto pdb_doc = GetDocument();
	int index_current_file = pdb_doc->GetDB_CurrentRecordPosition(); // index current file
	const int n_files = pdb_doc->GetDB_NRecords();
	const auto current_spike_list = m_pSpkDoc->GetSpkList_CurrentIndex();
	int first_file = index_current_file;
	int last_file = index_current_file;
	// change size of arrays and prepare temporary dialog
	select_spike_from_current_list(-1);
	if (m_bAllFiles)
	{
		first_file = 0; // index first file
		last_file = n_files - 1; // index last file
	}

	// loop over all selected files (or only one file currently selected)
	for (auto i_file = first_file; i_file <= last_file; i_file++)
	{
		// check if user wants to continue
		//if (m_bAllfiles)
		//{
		pdb_doc->set_db_current_record_position(i_file);
		m_pSpkDoc = pdb_doc->open_current_spike_file();
		//}
		// check if this file is ok
		if (m_pSpkDoc == nullptr)
			continue;
		m_pSpkList = m_pSpkDoc->set_spk_list_as_current(current_spike_list);
		if (m_pSpkList == nullptr)
			continue;

		const auto n_spikes = m_pSpkList->get_spikes_count();
		if (n_spikes <= 0 || m_pSpkList->get_spike_length() == 0)
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
		index_current_file = pdb_doc->GetDB_CurrentRecordPosition();
		pdb_doc->set_db_current_record_position(index_current_file);
		m_pSpkDoc = pdb_doc->open_current_spike_file();
		m_pSpkList = m_pSpkDoc->GetSpkList_Current();
	}

	m_chart_spike_shapes.set_source_data(m_pSpkList, GetDocument());
	m_chart_spike_bars.set_source_data(m_pSpkList, GetDocument());

	m_chart_measures.m_HZtags.SetTagVal(m_itaglow, m_psC->ilower);
	m_chart_measures.m_HZtags.SetTagVal(m_itagup, m_psC->iupper);

	build_histogram();
	m_chart_histogram.m_VTtags.SetTagVal(m_itaglow, m_psC->ilower);
	m_chart_histogram.m_VTtags.SetTagVal(m_itagup, m_psC->iupper);

	update_gain();
	UpdateData(FALSE);
}

void ViewSpikeSort::update_gain()
{
	const auto delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
	const auto max = static_cast<int>(m_mVMax / delta);
	const auto min = static_cast<int>(m_mVMin / delta);
	if (max == min)
		return;

	const auto y_we = max - min;
	const auto y_wo = (max + min) / 2;

	m_chart_measures.SetYWExtOrg(y_we, y_wo);
	m_chart_measures.Invalidate();

	m_chart_histogram.SetXWExtOrg(y_we, y_wo - y_we / 2);
	m_chart_histogram.Invalidate();
}

void ViewSpikeSort::OnFormatAlldata()
{
	// build new histogram only if necessary
	auto calculate_histogram = FALSE;

	// dots: spk file length
	if (m_lFirst != 0 || m_lLast != m_pSpkDoc->GetAcqSize() - 1)
	{
		m_lFirst = 0;
		m_lLast = m_pSpkDoc->GetAcqSize() - 1;

		if (m_psC->iparameter != 4) // then, we need imax imin ...
			m_chart_measures.SetTimeIntervals(m_lFirst, m_lLast);
		else
			m_chart_measures.SetTimeIntervals(-m_pSpkList->get_spike_length(), m_pSpkList->get_spike_length());
		m_chart_measures.Invalidate();

		m_chart_spike_shapes.SetTimeIntervals(m_lFirst, m_lLast);
		m_chart_spike_shapes.Invalidate();

		m_chart_spike_bars.SetTimeIntervals(m_lFirst, m_lLast);
		m_chart_spike_bars.Invalidate();
		calculate_histogram = TRUE;
	}

	// spikes: center spikes horizontally and adjust hz size of display

	const auto x_we = m_pSpkList->get_spike_length();
	if (x_we != m_chart_spike_shapes.GetXWExtent() || 0 != m_chart_spike_shapes.GetXWOrg())
		m_chart_spike_shapes.SetXWExtOrg(x_we, 0);

	// change spk_hist_wnd_
	if (calculate_histogram)
	{
		build_histogram();
	}
	update_legends();
}

void ViewSpikeSort::build_histogram()
{
	const auto pdb_doc = GetDocument();
	if (pdb_doc == nullptr)
		return;

	const auto delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
	m_parmmax = static_cast<int>(m_mVMax / delta);
	m_parmmin = static_cast<int>(m_mVMin / delta);
	const int n_bins = static_cast<int>((m_mVMax - m_mVMin) / m_mV_bin);
	if (n_bins <= 0)
		return;
	m_chart_histogram.BuildHistFromDocument(pdb_doc, m_bAllFiles, m_lFirst, m_lLast, m_parmmax, m_parmmin, n_bins, TRUE);
}

void ViewSpikeSort::OnFormatCentercurve()
{
	const auto n_spikes = m_pSpkList->get_spikes_count();
	for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
	{
		auto spike = m_pSpkList->get_spike(i_spike);
		spike->set_spike_length(m_pSpkList->get_spike_length());
		spike->CenterSpikeAmplitude(m_psC->ileft, m_psC->iright, 1);
	}
		

	short max, min;
	m_pSpkList->GetTotalMaxMin(TRUE, &max, &min);
	const auto middle = (max + min) / 2;
	m_chart_spike_shapes.SetYWExtOrg(m_chart_spike_shapes.GetYWExtent(), middle);
	m_chart_spike_bars.SetYWExtOrg(m_chart_spike_shapes.GetYWExtent(), middle);

	update_legends();
}

void ViewSpikeSort::OnFormatGainadjust()
{
	// adjust gain
	short maxvalue, minvalue;
	GetDocument()->GetAllSpkMaxMin(m_bAllFiles, TRUE, &maxvalue, &minvalue);

	auto y_we = MulDiv(maxvalue - minvalue + 1, 10, 9);
	auto y_wo = (maxvalue + minvalue) / 2;
	m_chart_spike_shapes.SetYWExtOrg(y_we, y_wo);
	m_chart_spike_bars.SetYWExtOrg(y_we, y_wo);
	//m_chart_spike_bars.MaxCenter();

	// adjust gain for spk_hist_wnd_ and XYp: data = computed values
	// search max min of parameter values
	const CSize measure = GetDocument()->GetSpkMaxMin_y1(m_bAllFiles);
	maxvalue = static_cast<short>(measure.cx);
	minvalue = static_cast<short>(measure.cy);

	const auto delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
	const auto max2 = static_cast<short>(m_upper / delta);
	const auto min2 = static_cast<short>(m_lower / delta);
	if (max2 > maxvalue)
		maxvalue = max2;
	if (min2 < minvalue)
		minvalue = min2;
	y_we = MulDiv(maxvalue - minvalue + 1, 10, 8);
	y_wo = (maxvalue + minvalue) / 2;

	// update display
	m_chart_measures.SetYWExtOrg(y_we, y_wo);
	const auto y_max = static_cast<int>(m_chart_histogram.GetHistMax());
	m_chart_histogram.SetXWExtOrg(y_we, y_wo - y_we / 2);
	m_chart_histogram.SetYWExtOrg(MulDiv(y_max, 10, 8), 0);

	// update edit controls
	m_mVMax = static_cast<float>(maxvalue) * delta;
	m_mVMin = static_cast<float>(minvalue) * delta;
	build_histogram();
	update_legends();
}

void ViewSpikeSort::select_spike_from_current_list(int spike_index)
{
	m_chart_spike_shapes.SelectSpikeShape(spike_index);
	m_chart_spike_bars.SelectSpike(spike_index);
	m_chart_measures.SelectSpike(spike_index);
	m_pSpkList->m_selected_spike = spike_index;

	m_spike_index_class = -1;
	auto n_cmd_show = SW_HIDE;
	if (spike_index >= 0)
	{
		const auto spike = m_pSpkList->get_spike(spike_index);
		m_spike_index_class = spike->get_class_id();
		n_cmd_show = SW_SHOW;
	}
	GetDlgItem(IDC_STATIC2)->ShowWindow(n_cmd_show);
	GetDlgItem(IDC_SPIKECLASS)->ShowWindow(n_cmd_show);
	m_spike_index = spike_index;
	UpdateData(FALSE);
}

void ViewSpikeSort::OnToolsEdittransformspikes()
{
	DlgSpikeEdit dlg;
	dlg.m_yextent = m_chart_spike_shapes.GetYWExtent();
	dlg.m_yzero = m_chart_spike_shapes.GetYWOrg();
	dlg.m_xextent = m_chart_spike_shapes.GetXWExtent();
	dlg.m_xzero = m_chart_spike_shapes.GetXWOrg();
	dlg.m_spike_index = m_spike_index;
	dlg.m_parent = this;
	dlg.m_pdbWaveDoc = GetDocument();

	// refresh pointer to data file because it not used elsewhere in the view
	const auto dat_name = GetDocument()->GetDB_CurrentDatFileName();
	auto b_doc_exists = FALSE;
	if (!dat_name.IsEmpty())
	{
		CFileStatus status;
		b_doc_exists = CFile::GetStatus(dat_name, status);
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
		const auto current_list = m_tabCtrl.GetCurSel();
		m_pSpkDoc->set_spk_list_as_current(current_list);
	}

	if (!dlg.m_bartefact && m_spike_index != dlg.m_spike_index)
		select_spike_from_current_list(dlg.m_spike_index);

	update_legends();
}

void ViewSpikeSort::OnSelectAllFiles()
{
	m_bAllFiles = dynamic_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	m_chart_spike_bars.DisplayAllFiles(m_bAllFiles, GetDocument());
	m_chart_spike_shapes.DisplayAllFiles(m_bAllFiles, GetDocument());
	m_chart_measures.DisplayAllFiles(m_bAllFiles, GetDocument());

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

	const auto spike_length = m_pSpkList->get_spike_length(); 
	const auto total_spikes = m_pSpkList->get_spikes_count(); 
	const auto p_sum0 = new double[spike_length]; // array with results / SUMy
	const auto p_cxy0 = new double[spike_length]; // temp array to store correlation
	auto* const p_mean0 = new short[spike_length]; // mean (template) / at scale
	const auto p_dummy0 = new short[spike_length]; // results of correlation / at scale

	// init pSUM with zeros
	auto p_sum = p_sum0;
	for (auto i = 0; i < spike_length; i++, p_sum++)
		*p_sum = 0;

	// compute mean
	auto nb_spk_selected_class = 0;
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		if (m_pSpkList->get_spike(i_spike)->get_class_id() != m_source_class)
			continue;
		nb_spk_selected_class++;
		short* p_spk = m_pSpkList->get_spike(i_spike)->get_p_data();
		p_sum = p_sum0;
		for (auto i = 0; i < spike_length; i++, p_spk++, p_sum++)
			*p_sum += *p_spk;
	}

	// build avg and avg autocorrelation, then display
	auto p_mean = p_mean0;
	p_sum = p_sum0;

	for (auto i = 0; i < spike_length; i++, p_mean++, p_sum++)
		*p_mean = static_cast<short>(*p_sum / nb_spk_selected_class);

	m_chart_spike_shapes.DisplayExData(p_mean0);

	// for each spike, compute correlation and take max value correlation
	const auto k_start = m_psC->ileft; // start of template match
	const auto k_end = m_psC->iright; // end of template match
	if (k_end <= k_start)
		return;
	const auto j0 = k_start - (k_end - k_start) / 2; // start time lag
	const auto j1 = k_end - (k_end - k_start) / 2 + 1; // last lag

	// compute autocorrelation for mean;
	double cxx_mean = 0;
	p_mean = p_mean0 + k_start;
	for (auto i = k_start; i < k_end; i++, p_mean++)
	{
		const auto val = static_cast<double>(*p_mean);
		cxx_mean += val * val;
	}

	// get parameters from document
	const auto p_dat_doc = GetDocument()->m_pDat;
	p_dat_doc->OnOpenDocument(data_file_name);
	const auto doc_chan = m_pSpkList->get_detection_parameters()->extract_channel; 
	const auto number_channels = static_cast<int>(p_dat_doc->GetpWaveFormat()->scan_count); 
	const auto method = m_pSpkList->get_detection_parameters()->extract_transform;
	const auto spike_pre_trigger = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	const int offset = (method > 0) ? 1 : number_channels; 
	const int span = p_dat_doc->GetTransfDataSpan(method); 

	// pre-load data
	auto ii_time0 = m_pSpkList->get_spike(0)->get_time(); 
	auto l_rw_first0 = ii_time0 - spike_length;
	auto l_rw_last0 = ii_time0 + spike_length;
	if (!p_dat_doc->LoadRawData(&l_rw_first0, &l_rw_last0, span))
		return;

	auto p_data = p_dat_doc->LoadTransfData(l_rw_first0, l_rw_last0, method, doc_chan);

	// loop over all spikes now
	const auto pre_trigger = m_pSpkList->get_detection_parameters()->detect_pre_threshold;
	for (auto i_spike = 0; i_spike < total_spikes; i_spike++)
	{
		Spike* spike = m_pSpkList->get_spike(i_spike);

		// exclude spikes that do not fall within time limits
		if (spike->get_class_id() != m_source_class)
			continue;

		ii_time0 = spike->get_time();
		ii_time0 -= spike_pre_trigger;

		// make sure that source data are loaded and get pointer to it (p_data)
		auto l_rw_first = ii_time0 - spike_length; 
		auto l_rw_last = ii_time0 + spike_length; 
		if (ii_time0 > m_lLast || ii_time0 < m_lFirst)
			continue;
		if (!p_dat_doc->LoadRawData(&l_rw_first, &l_rw_last, span))
			break; 

		// load data only if necessary
		if (l_rw_first != l_rw_first0 || l_rw_last != l_rw_last0)
		{
			p_data = p_dat_doc->LoadTransfData(l_rw_first, l_rw_last, method, doc_chan);
			l_rw_last0 = l_rw_last; 
			l_rw_first0 = l_rw_first;
		}

		// pointer to first point of spike
		auto p_data_spike0 = p_data + (ii_time0 - l_rw_first) * offset;

		// for spike i_spk: loop over spike_len time lags centered over interval center

		// compute autocorrelation & cross correlation at first time lag
		auto p_cxy_lag = p_cxy0; // pointer to array with correlation coefficients
		*p_cxy_lag = 0;
		auto p_data_k0 = p_data_spike0 + j0 * offset; // source data start

		// loop over all time lag requested
		for (auto j = j0; j < j1; j++, p_cxy_lag++, p_data_k0 += offset)
		{
			*p_cxy_lag = 0;

			// add cross product for each point: data * meanlong ii_time
			auto p_mean_k = p_mean0 + k_start; // first point / template
			short* p_data_k = p_data_k0; // first data point
			double cxx_spike = 0; // autocorrelation

			// loop over all points of source data and mean
			for (auto k = k_start; k < k_end; k++, p_mean_k++, p_data_k += offset)
			{
				const auto val = static_cast<double>(*p_data_k);
				*p_cxy_lag += static_cast<double>(*p_mean_k) * val;
				cxx_spike += val * val;
			}

			*p_cxy_lag /= (static_cast<double>(k_end) - k_start + 1);
			*p_cxy_lag = *p_cxy_lag / sqrt(cxx_mean * cxx_spike);
		}

		// get max and min of this correlation
		auto p_cxy = p_cxy0;
		auto cxy_max = *p_cxy; // correlation max value
		auto i_cxy_max = 0; // correlation max index
		for (auto i = 0; i < k_end - k_start; p_cxy++, i++)
		{
			if (cxy_max < *p_cxy) // get max and max position
			{
				cxy_max = *p_cxy;
				i_cxy_max = i;
			}
		}

		// offset spike so that max is at spike_len/2
		const auto j_decal = i_cxy_max - (k_end - k_start) / 2;
		if (j_decal != 0)
		{
			p_data_spike0 = p_data + static_cast<WORD>(ii_time0 + j_decal - l_rw_first) * offset + doc_chan;
			spike->TransferDataToSpikeBuffer(p_data_spike0, number_channels, spike_length);
			m_pSpkDoc->SetModifiedFlag(TRUE);
			spike->set_time(ii_time0 + pre_trigger);
		}

		// now offset spike vertically to align it with the mean
		spike->OffsetSpikeDataToAverageEx(k_start, k_end);
	}

	// exit : delete resources used locally
	if (m_pSpkDoc->IsModified())
	{
		m_chart_spike_shapes.Invalidate();
		m_chart_spike_shapes.DisplayExData(p_mean0);
	}

	delete[] p_sum0;
	delete[] p_mean0;
	delete[] p_cxy0;
	delete[] p_dummy0;

	OnMeasure();
}

void ViewSpikeSort::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// view scroll: if pointer null
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
		m_file_scroll.GetScrollInfo(&m_file_scroll_infos, SIF_ALL);
		m_lFirst = m_file_scroll_infos.nPos;
		m_lLast = m_lFirst + long(m_file_scroll_infos.nPage) - 1;
		break;

	default:
		scroll_file(nSBCode, nPos);
		break;
	}
	update_legends();
}

void ViewSpikeSort::scroll_file(UINT nSBCode, UINT nPos)
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
	case SB_LEFT: l_first = 0; break; // Scroll to far left.
	case SB_LINELEFT: l_first -= sb_scroll; break; // Scroll left.
	case SB_LINERIGHT: l_first += sb_scroll; break; // Scroll right
	case SB_PAGELEFT: l_first -= page_scroll; break; // Scroll one page left
	case SB_PAGERIGHT: l_first += page_scroll; break; // Scroll one page right.
	case SB_RIGHT: l_first = total_scroll - page_scroll + 1;break;
	case SB_THUMBPOSITION: // scroll to pos = nPos
	case SB_THUMBTRACK: // drag scroll box -- pos = nPos
		l_first = static_cast<int>(nPos);break;
	default:return;
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

void ViewSpikeSort::update_file_scroll()
{
	m_file_scroll_infos.fMask = SIF_PAGE | SIF_POS;
	m_file_scroll_infos.nPos = m_lFirst;
	m_file_scroll_infos.nPage = m_lLast - m_lFirst + 1;
	m_file_scroll.SetScrollInfo(&m_file_scroll_infos);
}

void ViewSpikeSort::select_spike_list(int current_index)
{
	m_pSpkList = m_pSpkDoc->set_spk_list_as_current(current_index);
	//GetDocument()->GetCurrent_Spk_Document()->SetSpkList_CurrentIndex(current_index);
	ASSERT(m_pSpkList != NULL);
	OnMeasure();

	// update source data: change data channel and update display
	m_chart_histogram.SetSpkList(m_pSpkList);
	m_chart_spike_shapes.SetSpkList(m_pSpkList);
	m_chart_spike_bars.SetSpkList(m_pSpkList);
	m_chart_measures.SetSpkList(m_pSpkList);

	m_chart_histogram.Invalidate();
	m_chart_measures.Invalidate();
	m_chart_spike_shapes.Invalidate();
	m_chart_spike_bars.Invalidate();
}

void ViewSpikeSort::OnEnChangeEditleft2()
{
	if (mm_txyleft.m_bEntryDone)
	{
		auto left = m_txyleft;
		const auto delta = m_time_unit / m_pSpkList->GetAcqSampRate();
		mm_txyleft.OnEnChange(this, m_txyleft, delta, -delta);
		// check boundaries
		if (m_txyleft >= m_txyright)
			m_txyleft = m_txyright - delta;

		// change display if necessary
		left = m_txyleft / delta;
		const auto it_left = static_cast<int>(left);
		if (it_left != m_chart_measures.m_VTtags.GetValue(m_ixyleft))
		{
			m_psC->ixyleft = it_left;
			m_chart_measures.MoveVTtagtoVal(m_ixyleft, it_left);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeEditright2()
{
	if (mm_txyright.m_bEntryDone)
	{
		auto right = m_txyright;
		const auto delta = m_time_unit / m_pSpkList->GetAcqSampRate();
		mm_txyright.OnEnChange(this, m_txyright, delta, -delta);

		// check boundaries
		if (m_txyright <= m_txyleft)
			m_txyright = m_txyleft + delta;

		// change display if necessary
		right = m_txyright / delta;
		const auto i_right = static_cast<int>(right);
		if (i_right != m_chart_measures.m_VTtags.GetValue(m_ixyright))
		{
			m_psC->ixyright = i_right;
			m_chart_measures.MoveVTtagtoVal(m_ixyright, i_right);
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeSourceclass()
{
	if (mm_source_class.m_bEntryDone)
	{
		const auto source_class = m_source_class;
		mm_source_class.OnEnChange(this, m_source_class, 1, -1);
		if (source_class != m_source_class)
		{
			m_chart_spike_shapes.set_plot_mode(PLOT_ONECOLOR, m_source_class);
			m_chart_measures.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
			m_chart_histogram.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
			m_chart_spike_bars.set_plot_mode(PLOT_CLASSCOLORS, m_source_class);
		}
		// change histogram accordingly
		m_chart_spike_shapes.Invalidate();
		m_chart_spike_bars.Invalidate();
		m_chart_measures.Invalidate();
		m_chart_histogram.Invalidate();

		select_spike_from_current_list(-1);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeDestinationclass()
{
	if (mm_destination_class.m_bEntryDone)
	{
		mm_destination_class.OnEnChange(this, m_destination_class, 1, -1);
		select_spike_from_current_list(-1);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnSelchangeParameter()
{
	const auto i_parameter = m_CBparameter.GetCurSel();
	if (i_parameter != m_psC->iparameter)
	{
		m_psC->iparameter = i_parameter;
		activate_mode4();
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
		m_delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
		mm_lower.OnEnChange(this, m_lower, m_delta, -m_delta);
		if (m_lower >= m_upper)
			m_lower = m_upper - m_delta * 10.f;

		m_psC->ilower = static_cast<int>(m_lower / m_delta);
		if (m_psC->ilower != m_chart_measures.m_HZtags.GetValue(m_itaglow))
			m_chart_measures.MoveHZtagtoVal(m_itaglow, m_psC->ilower);
		if (m_psC->ilower != m_chart_histogram.m_VTtags.GetValue(m_spkhist_lower))
			m_chart_histogram.MoveVTtagtoVal(m_spkhist_lower, m_psC->ilower);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeupper()
{
	if (mm_upper.m_bEntryDone)
	{
		m_delta = m_pSpkList->GetAcqVoltsperBin() * m_vunit;
		mm_upper.OnEnChange(this, m_upper,m_delta, -m_delta);
		// check boundaries
		if (m_upper <= m_lower)
			m_upper = m_lower + m_delta * 10.f;

	
		m_psC->iupper = static_cast<int>(m_upper / m_delta);
		if (m_psC->iupper != m_chart_measures.m_HZtags.GetValue(m_itagup))
			m_chart_measures.MoveHZtagtoVal(m_itagup, m_psC->iupper);
		if (m_psC->ilower != m_chart_histogram.m_VTtags.GetValue(m_spkhist_upper))
			m_chart_histogram.MoveVTtagtoVal(m_spkhist_upper, m_psC->iupper);
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeT1()
{
	if (mm_t1.m_bEntryDone)
	{
		const auto delta = m_time_unit / m_pSpkList->GetAcqSampRate();
		
		mm_t1.OnEnChange(this, m_t1, delta, -delta);
		// check boundaries
		if (m_t1 < 0)
			m_t1 = 0.0f;
		if (m_t1 >= m_t2)
			m_t1 = m_t2 - delta;
	;
		const auto it1 = static_cast<int>(m_t1 / delta);
		if (it1 != m_chart_spike_shapes.m_VTtags.GetValue(m_spkform_tag_left))
		{
			m_psC->ileft = it1;
			m_chart_spike_shapes.MoveVTtrack(m_spkform_tag_left, m_psC->ileft);
			m_pSpkList->m_imaxmin1SL = m_psC->ileft;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangeT2()
{
	if (mm_t2.m_bEntryDone)
	{
		const auto delta = m_time_unit / m_pSpkList->GetAcqSampRate();
		mm_t2.OnEnChange(this, m_t2, delta, -delta);

		// check boundaries
		if (m_t2 < m_t1)
			m_t2 = m_t1 + delta;
		const auto t_max = (static_cast<float>(m_pSpkList->get_spike_length()) - 1.f) * delta;
		if (m_t2 >= t_max)
			m_t2 = t_max;

		const auto it2 = static_cast<int>(m_t2 / delta);
		if (it2 != m_chart_spike_shapes.m_VTtags.GetValue(m_spkform_tag_right))
		{
			m_psC->iright = it2;
			m_chart_spike_shapes.MoveVTtrack(m_spkform_tag_right, m_psC->iright);
			m_pSpkList->m_imaxmin2SL = m_psC->iright;
		}
		UpdateData(FALSE);
	}
}

void ViewSpikeSort::OnEnChangetimeFirst()
{
	if (mm_timeFirst.m_bEntryDone)
	{
		mm_timeFirst.OnEnChange(this, m_timeFirst, 1.f, -1.f);

		// check boundaries
		if (m_timeFirst < 0.f)
			m_timeFirst = 0.f;
		if (m_timeFirst >= m_timeLast)
			m_timeFirst = 0.f;

		m_lFirst = static_cast<long>(m_timeFirst * m_pSpkList->GetAcqSampRate());
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangetimeLast()
{
	if (mm_timeLast.m_bEntryDone)
	{
		mm_timeLast.OnEnChange(this, m_timeLast, 1.f, -1.f);

		// check boundaries
		if (m_timeLast <= m_timeFirst)
			m_lLast = static_cast<long>(static_cast<float>(m_pSpkDoc->GetAcqSize() - 1) / m_pSpkList->GetAcqSampRate());

		m_lLast = static_cast<long>(m_timeLast * m_pSpkList->GetAcqSampRate());
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangemVMin()
{
	if (mm_mVMin.m_bEntryDone)
	{
		mm_mVMin.OnEnChange(this, m_mVMin, 1.f, -1.f);

		// check boundaries
		if (m_mVMin >= m_mVMax)
			m_mVMin = m_mVMax - 1.f;

		// change display if necessary
		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangemVMax()
{
	if (mm_mVMax.m_bEntryDone)
	{
		mm_mVMax.OnEnChange(this, m_mVMax, 1.f, -1.f);

		// check boundaries
		if (m_mVMax <= m_mVMin)
			m_mVMax = m_mVMin + 1.f;

		// change display if necessary
		update_gain();
		update_legends();
	}
}

void ViewSpikeSort::OnEnChangeNOspike()
{
	if (mm_spike_index.m_bEntryDone)
	{
		const int spike_index = m_spike_index;
		mm_spike_index.OnEnChange(this, m_spike_index, 1, -1);

		// check boundaries
		if (m_spike_index < 0)
			m_spike_index = -1;
		if (m_spike_index >= m_pSpkList->get_spikes_count())
			m_spike_index = m_pSpkList->get_spikes_count() - 1;

		if (m_spike_index != spike_index)
		{
			if (m_spike_index >= 0)
			{
				// test if spike visible in the current time interval
				const auto spike_element = m_pSpkList->get_spike(m_spike_index);
				const auto spk_first = spike_element->get_time() - m_pSpkList->get_detection_parameters()->detect_pre_threshold;
				const auto spk_last = spk_first + m_pSpkList->get_spike_length();

				if (spk_first < m_lFirst || spk_last > m_lLast)
				{
					const auto l_span = (m_lLast - m_lFirst) / 2;
					const auto l_center = (spk_last + spk_first) / 2;
					m_lFirst = l_center - l_span;
					m_lLast = l_center + l_span;
					update_legends();
				}
			}
		}
		select_spike_from_current_list(m_spike_index);
	}
}

void ViewSpikeSort::OnEnChangespike_indexclass()
{
	if (mm_spike_index_class.m_bEntryDone)
	{
		const auto spike_index_class = m_spike_index_class;
		mm_spike_index_class.OnEnChange(this, m_spike_index_class, 1, -1);

		if (m_spike_index_class != spike_index_class)
		{
			m_pSpkDoc->SetModifiedFlag(TRUE);
			const auto current_list = m_tabCtrl.GetCurSel();
			auto* spike_list = m_pSpkDoc->set_spk_list_as_current(current_list);
			spike_list->get_spike(m_spike_index)->set_class_id(m_spike_index_class);
			update_legends();
		}
	}
}

void ViewSpikeSort::OnEnChangeNBins()
{
	if (mm_mV_bin.m_bEntryDone)
	{
		const auto mV_bin = m_mV_bin;
		const auto delta = (m_mVMax - m_mVMin) / 10.f;
		mm_mV_bin.OnEnChange(this, m_mV_bin, delta, -delta);

		if (m_mV_bin != mV_bin)
		{
			build_histogram();
			update_legends();
		}
	}
}
