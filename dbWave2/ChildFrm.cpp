// ChildFrm.cpp : implementation of the CChildFrame class
//	accelerator

#include "StdAfx.h"
#include "resource.h"
#include "afxpriv.h"
#include "dbWave.h"
#include "dbWaveDoc.h"

#include "MainFrm.h"
#include "ChartData.h"
#include "ViewdbWave.h"
#include "ViewData.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"

#include "DlgFindFiles.h"
#include "DlgProgress.h"
#include "DlgRejectRecord.h"
#include "DlgGotoRecord.h"
#include "DlgImportFiles.h"
#include "DlgDeleteRecordOptions.h"

#include "CApplication.h"
#include "CPivotCell.h"
#include "CPivotField.h"
#include "CPivotTable.h"
#include "CRange.h"
#include "CWorkbook.h"
#include "CWorkbooks.h"
#include "CWorksheet.h"
#include "CWorksheets.h"
#include "DlgTransferFiles.h"
#include "DlgExportData.h"
#include "ViewADcontinuous.h"

#include "ViewSpikeSort.h"
#include "ViewSpikeDetect.h"
#include "ViewSpikes.h"
#include "ViewSpikeTemplate.h"
#include "ViewSpikeHist.h"
#include "ChildFrm.h"

#include "DlgBrowseFile.h"
#include "DlgDataComments.h"
#include "DlgExportSpikeInfos.h"
#include "DlgLoadSaveOptions.h"
#include "DlgPrintMargins.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)

	ON_COMMAND(ID_VIEW_CURSORMODE_NORMAL, &CChildFrame::OnViewCursormodeNormal)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CURSORMODE_NORMAL, &CChildFrame::OnUpdateViewCursormodeNormal)
	ON_COMMAND(ID_VIEW_CURSORMODE_MEASURE, &CChildFrame::OnViewCursormodeMeasure)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CURSORMODE_MEASURE, &CChildFrame::OnUpdateViewCursormodeMeasure)
	ON_COMMAND(ID_VIEW_CURSORMODE_ZOOMIN, &CChildFrame::OnViewCursormodeZoomin)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CURSORMODE_ZOOMIN, &CChildFrame::OnUpdateViewCursormodeZoomin)
	ON_COMMAND(ID_OPTIONS_BROWSEMODE, &CChildFrame::OnOptionsBrowsemode)
	ON_COMMAND(ID_OPTIONS_PRINTMARGINS, &CChildFrame::OnOptionsPrintmargins)
	ON_COMMAND(ID_OPTIONS_LOADSAVEOPTIONS, &CChildFrame::OnOptionsLoadsaveoptions)
	ON_MESSAGE(WM_MYMESSAGE, &CChildFrame::OnMyMessage)
	ON_WM_CREATE()
	ON_COMMAND_RANGE(ID_VIEW_DATABASE, ID_VIEW_ACQUIREDATA, &CChildFrame::ReplaceViewIndex)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_DATABASE, ID_VIEW_ACQUIREDATA, &CChildFrame::OnUpdateViewMenu)

	ON_COMMAND(ID_TOOLS_EXPORTDATACOMMENTS, &CChildFrame::OnToolsExportdatacomments)
	ON_COMMAND(ID_TOOLS_EXPORTDATAASTEXT, &CChildFrame::OnToolsExportdataAsText)
	ON_COMMAND(ID_TOOLS_EXPORTNUMBEROFSPIKES, &CChildFrame::OnToolsExportnumberofspikes)
	ON_COMMAND(ID_TOOLS_EXPORTDATAFILE, &CChildFrame::OnToolsExportdatafile)
	ON_COMMAND(ID_TOOLS_REMOVEMISSINGFILENAMES, &CChildFrame::OnToolsRemoveMissingFiles)
	ON_COMMAND(ID_TOOLS_REMOVEDUPLICATEFILES, &CChildFrame::OnToolsRemoveduplicatefiles)
	ON_COMMAND(ID_TOOLS_REMOVEARTEFACTFILES, &CChildFrame::OnToolsRemoveartefactfiles)
	ON_COMMAND(ID_TOOLS_RESTOREDELETEDFILES, &CChildFrame::OnToolsRestoredeletedfiles)
	ON_COMMAND(ID_TOOLS_CHECKCONSISTENCY, &CChildFrame::OnToolsCheckFilelistsConsistency)
	ON_COMMAND(ID_TOOLS_SYNCHRO_CURRENTFILE, &CChildFrame::OnToolsSynchronizesourceinformationsCurrentfile)
	ON_COMMAND(ID_TOOLS_SYNCHRO_ALLFILES, &CChildFrame::OnToolsSynchronizesourceinformationsAllfiles)
	ON_COMMAND(ID_TOOLS_TRANSFERFILES, &CChildFrame::OnToolsCopyAllProjectFiles)
	ON_COMMAND(ID_TOOLS_IMPORT_DATAFILES, &CChildFrame::OnToolsImportDatafiles)
	ON_COMMAND(ID_TOOLS_IMPORT_SPIKEFILES, &CChildFrame::OnToolsImportSpikefiles)
	ON_COMMAND(ID_TOOLS_IMPORT_DATABASE, &CChildFrame::OnToolsImportDatabase)
	ON_COMMAND(ID_TOOLS_IMPORT_ATFFILES, &CChildFrame::OnToolsImportATFfiles)

	ON_COMMAND(ID_RECORD_DELETECURRENT, &CChildFrame::OnRecordDeletecurrent)
	ON_COMMAND(ID_RECORD_GOTORECORD, &CChildFrame::OnRecordGotorecord)
	ON_COMMAND(ID_RECORD_ADD, &CChildFrame::OnRecordAdd)

	ON_WM_MDIACTIVATE()
	ON_COMMAND(ID_TOOLS_PATHS_RELATIVE, &CChildFrame::OnToolsPathsRelative)
	ON_COMMAND(ID_TOOLS_PATHS_ABSOLUTE, &CChildFrame::OnToolsPathsAbsolute)
	ON_COMMAND(ID_TOOLS_PATH, &CChildFrame::OnToolsPath)
	ON_COMMAND(ID_TOOLS_IMPORT, &CChildFrame::OnToolsImport)
	ON_COMMAND(ID_TOOLS_SYNCHRO, &CChildFrame::OnToolsSynchro)
	ON_COMMAND(ID_TOOLS_REMOVEUNUSED, &CChildFrame::OnToolsRemoveunused)
	ON_COMMAND(ID_TOOLS_GARBAGE, &CChildFrame::OnToolsGarbage)
	ON_COMMAND(ID_TOOLS_COMPACTDATABASE, &CChildFrame::OnToolsCompactdatabase)

END_MESSAGE_MAP()

CChildFrame::CChildFrame()
= default;

CChildFrame::~CChildFrame()
= default;

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIChildWndEx::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}

#endif //_DEBUG

void CChildFrame::OnViewCursormodeNormal()
{
	m_cursor_state = CURSOR_ARROW;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursor_state, NULL));
}

void CChildFrame::OnUpdateViewCursormodeNormal(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_cursor_state == CURSOR_ARROW);
}

void CChildFrame::OnViewCursormodeMeasure()
{
	m_cursor_state = CURSOR_CROSS;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursor_state, NULL));
}

void CChildFrame::OnUpdateViewCursormodeMeasure(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_cursor_state == CURSOR_CROSS);
}

void CChildFrame::OnViewCursormodeZoomin()
{
	m_cursor_state = CURSOR_ZOOM;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursor_state, NULL));
}

void CChildFrame::OnUpdateViewCursormodeZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_cursor_state == CURSOR_ZOOM);
}

void CChildFrame::OnOptionsBrowsemode()
{
	DlgBrowseFile dlg;
	dlg.mfBR = &(dynamic_cast<CdbWaveApp*>(AfxGetApp())->options_viewdata);
	dlg.DoModal();
}

void CChildFrame::OnOptionsPrintmargins()
{
	DlgPrintMargins dlg;
	const auto psource = &(dynamic_cast<CdbWaveApp*>(AfxGetApp())->options_viewdata);
	dlg.mdPM = psource;
	dlg.DoModal();
}

void CChildFrame::OnOptionsLoadsaveoptions()
{
	DlgLoadSaveOptions dlg;
	if (IDOK == dlg.DoModal())
	{
		const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		auto p_parm_files = &(p_app->m_csParmFiles);
		p_parm_files->RemoveAll();
		for (auto i = 0; i < dlg.pFiles.GetSize(); i++)
			p_parm_files->Add(dlg.pFiles.GetAt(i));
	}
}

void CChildFrame::OnToolsExportdatacomments()
{
	DlgDataComments dlg;
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	dlg.m_pvO = &(p_app->options_viewdata);
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		exportASCII(0);
	}
}

void CChildFrame::OnToolsExportnumberofspikes()
{
	DlgExportSpikeInfos dlg;
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	dlg.m_pvdS = &p_app->options_viewspikes;
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		exportASCII(1);
	}
}

void CChildFrame::OnToolsExportdataAsText()
{
	const auto pdb_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	pdb_doc->Export_DatafilesAsTXTfiles();
	pdb_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, NULL);
}

void CChildFrame::exportASCII(int option)
{
	CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	auto* pdb_doc = static_cast<CdbWaveDoc*>(GetActiveDocument());
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	switch (option)
	{
	case 0:
		pdb_doc->Export_DataAsciiComments(&sf);
		break;
	case 1:
		pdb_doc->Export_NumberOfSpikes(&sf);
		break;
	default:
		break;
	}

	const auto dw_len = static_cast<SIZE_T>(sf.GetLength());
	auto h_mem = sf.Detach();
	if (!h_mem)
		return;
	h_mem = GlobalReAlloc(h_mem, dw_len, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	if (!h_mem)
		return;
	auto* p_source = new COleDataSource();
	p_source->CacheGlobalData(CF_UNICODETEXT, h_mem);
	p_source->SetClipboard();

	//pass data to excel - if it does not work, paste to a new text document
	switch (option)
	{
	case 0: // export CAcqData
		{
			auto flag = FALSE;
			if (p_app->options_viewdata.btoExcel)
				flag = exportToExcel();
			if (!p_app->options_viewdata.btoExcel || !flag)
			{
				CMultiDocTemplate* note_view_template = p_app->m_NoteView_Template;
				const auto pdb_doc_export = note_view_template->OpenDocumentFile(nullptr);
				auto pos = pdb_doc_export->GetFirstViewPosition();
				const auto p_view = dynamic_cast<ViewNoteDoc*>(pdb_doc_export->GetNextView(pos));
				auto& p_edit = p_view->GetRichEditCtrl();
				p_edit.Paste();
			}
		}
		break;
	case 1:
		{
			auto flag = FALSE;
			if (p_app->options_viewspikes.bexporttoExcel)
				flag = exportToExcelAndBuildPivot(option);

			if (!p_app->options_viewspikes.bexporttoExcel || !flag)
			{
				CMultiDocTemplate* note_view_template = p_app->m_NoteView_Template;
				const auto pdb_doc_export = note_view_template->OpenDocumentFile(nullptr);
				auto pos = pdb_doc_export->GetFirstViewPosition();
				const auto p_view = dynamic_cast<ViewNoteDoc*>(pdb_doc_export->GetNextView(pos));
				auto& p_edit = p_view->GetRichEditCtrl();
				p_edit.Paste();
			}
		}
		break;
	default:
		break;
	}
}

void CChildFrame::ActivateFrame(int n_cmd_show)
{
	if (n_cmd_show == -1)
		n_cmd_show = SW_SHOWMAXIMIZED;
	CMDIChildWndEx::ActivateFrame(n_cmd_show);
}

LRESULT CChildFrame::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		{
			const short lowp = LOWORD(lParam);
			//switch (lowp)
			//{
			//case CURSOR_ZOOM:
			//case CURSOR_MEASURE:
			//case CURSOR_VERTICAL:
			//	break;
			//default:
			//	m_cursor_state=CURSOR_ARROW;
			//	break;
			//}
			m_cursor_state = lowp;
		}
		break;

	case HINT_SHAREDMEMFILLED:
		{
			auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
			if (p_app->m_psf != nullptr)
			{
				CMultiDocTemplate* note_view_template = p_app->m_NoteView_Template;
				const auto p_doc_export = note_view_template->OpenDocumentFile(nullptr);
				auto pos = p_doc_export->GetFirstViewPosition();
				const auto* p_view = dynamic_cast<ViewNoteDoc*>(p_doc_export->GetNextView(pos));
				auto& p_edit = p_view->GetRichEditCtrl();
				OpenClipboard();
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, (p_app->m_psf)->Detach());
				CloseClipboard();
				p_edit.Paste();
			}
		}
		break;

	default:
		break;
	}
	return 0L;
}

void CChildFrame::ReplaceViewIndex(UINT iID)
{
	const auto pdb_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	const auto p_mainframe = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	auto b_active_panes = TRUE;
	switch (iID)
	{
	case ID_VIEW_DATABASE:
		replaceView(RUNTIME_CLASS(ViewdbWave), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hDBView);
		break;
	case ID_VIEW_DATAFILE:
		if (!pdb_doc->GetDB_CurrentDatFileName(TRUE).IsEmpty())
			replaceView(RUNTIME_CLASS(ViewData), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hDataView);
		break;
	case ID_VIEW_SPIKEDETECTION:
		if (!pdb_doc->GetDB_CurrentDatFileName(TRUE).IsEmpty())
			replaceView(RUNTIME_CLASS(ViewSpikeDetection), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hDataView);
		break;
	case ID_VIEW_SPIKEDISPLAY:
		if (!pdb_doc->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
			replaceView(RUNTIME_CLASS(ViewSpikes), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKESORTINGAMPLITUDE:
		if (!pdb_doc->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
			replaceView(RUNTIME_CLASS(ViewSpikeSort), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKESORTINGTEMPLATES:
		if (!pdb_doc->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
			replaceView(RUNTIME_CLASS(ViewSpikeTemplates), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKETIMESERIES:
		if (!pdb_doc->GetDB_CurrentSpkFileName(TRUE).IsEmpty())
			replaceView(RUNTIME_CLASS(ViewSpikeHist), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_ACQUIREDATA:
		replaceView(RUNTIME_CLASS(ViewADcontinuous), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hDataView);
		b_active_panes = FALSE;
		break;

	default:
		iID = 0;
		replaceView(RUNTIME_CLASS(ViewdbWave), dynamic_cast<CdbWaveApp*>(AfxGetApp())->m_hDataView);
		break;
	}
	p_mainframe->ActivatePropertyPane(b_active_panes);
	p_mainframe->ActivateFilterPane(b_active_panes);
	m_previous_view_ON = m_view_ON;
	m_view_ON = iID;

	// update all views
	auto doctype = 1;
	if (iID < ID_VIEW_SPIKEDISPLAY || iID == ID_VIEW_ACQUIREDATA)
		doctype = 0;
	pdb_doc->UpdateAllViews(nullptr, MAKELPARAM(HINT_REPLACEVIEW, doctype), nullptr);
}

void CChildFrame::OnUpdateViewMenu(CCmdUI* pCmdUI)
{
	const auto db_wave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	ASSERT(db_wave_doc);
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	BOOL flag = (db_wave_doc != nullptr);

	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_SPIKEDISPLAY:
	case ID_VIEW_SPIKESORTINGAMPLITUDE:
	case ID_VIEW_SPIKESORTINGTEMPLATES:
	case ID_VIEW_SPIKETIMESERIES:
		flag = (flag
			&& !db_wave_doc->GetDB_CurrentSpkFileName(TRUE).IsEmpty()
			&& m_view_ON != ID_VIEW_ACQUIREDATA);
		break;

	case ID_VIEW_ACQUIREDATA:
		flag = p_app->m_bADcardFound;
		break;

	default:
		flag = (flag && !db_wave_doc->GetDB_CurrentDatFileName().IsEmpty());
		break;
	}

	pCmdUI->Enable(flag);
	pCmdUI->SetCheck(m_view_ON == pCmdUI->m_nID);
}

void CChildFrame::replaceView(CRuntimeClass* pViewClass, HMENU hmenu)
{
	// assume that the views replaced are of dbTableView type
	const auto p_current_view = GetActiveView();
	if ((p_current_view->IsKindOf(pViewClass)) == TRUE)
		return;

	auto* dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	ASSERT(dbwave_doc);
	dbwave_doc->m_hMyMenu = hmenu;

	CSize size;
	CRect rect;
	p_current_view->GetClientRect(rect);
	size.cx = rect.right;
	size.cy = rect.bottom;

	// delete old view without deleting document
	const auto bautodel = dbwave_doc->m_bAutoDelete;
	dbwave_doc->m_bAutoDelete = FALSE;
	dbwave_doc->CloseCurrentDataFile();
	p_current_view->DestroyWindow();
	dbwave_doc->m_bAutoDelete = bautodel;

	// create new view
	CCreateContext context;
	context.m_pNewViewClass = pViewClass;
	context.m_pCurrentDoc = dbwave_doc;
	context.m_pNewDocTemplate = nullptr;
	context.m_pLastView = nullptr;
	context.m_pCurrentFrame = nullptr;

	// create view inside a splitter
	const auto p_new_view = dynamic_cast<CView*>(CreateView(&context));
	p_new_view->SendMessage(WM_INITIALUPDATE, 0, 0);
	GetMDIFrame()->MDISetMenu(CMenu::FromHandle(hmenu), nullptr);
	GetMDIFrame()->DrawMenuBar();
	RecalcLayout();

	// adjust size of the view replacing the previous one
	p_new_view->SendMessage(WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(size.cx, size.cy));
	SetActiveView(p_new_view);
}

void CChildFrame::OnToolsRemoveMissingFiles()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument()); 
	ASSERT(p_dbwave_doc); // debug: check that doc is defined

	p_dbwave_doc->Remove_MissingFiles();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsRemoveduplicatefiles()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	ASSERT(p_dbwave_doc);
	p_dbwave_doc->Remove_DuplicateFiles();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsCheckFilelistsConsistency()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	ASSERT(p_dbwave_doc);
	p_dbwave_doc->Remove_FalseSpkFiles();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsRestoredeletedfiles()
{
	// scan directories and rename files *.datdel into *.dat and *.spkdel into *.spk
	CStringArray file_names;
	DlgFindFiles dlg;
	dlg.m_pfilenames = &file_names;
	dlg.m_selinit = 0;
	dlg.m_ioption = 1;
	dlg.m_pdbDoc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	const auto i_result = dlg.DoModal();

	if (IDOK == i_result)
	{
		DlgProgress dlg_progress;
		dlg_progress.Create();
		dlg_progress.SetStep(1);
		auto istep = 0;
		CString cscomment;

		const auto nfiles = file_names.GetSize();
		for (auto i = 0; i < nfiles; i++)
		{
			auto csoldname = file_names[i];
			auto csnewname = csoldname.Left(csoldname.GetLength() - 3);
			cscomment.Format(_T("Rename file [%i / %i] "), i + 1, nfiles);
			cscomment += csoldname;
			dlg_progress.SetStatus(cscomment);
			if (MulDiv(i, 100, nfiles) > istep)
			{
				dlg_progress.StepIt();
				istep = MulDiv(i, 100, nfiles);
			}
			CFile::Rename(csoldname, csnewname);
		}
	}
	auto* p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	ASSERT(p_dbwave_doc != NULL);
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsSynchronizesourceinformationsCurrentfile()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	ASSERT(p_dbwave_doc);
	p_dbwave_doc->SynchronizeSourceInfos(FALSE);
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsSynchronizesourceinformationsAllfiles()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	ASSERT(p_dbwave_doc);
	p_dbwave_doc->SynchronizeSourceInfos(TRUE);
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsRemoveartefactfiles()
{
	DlgRejectRecord dlg1;
	long n_consecutive_points = 1000;
	short jitter = 4;
	BOOL flag_rejected_file_as;

	dlg1.m_bconsecutivepoints = TRUE;
	dlg1.m_Nconsecutivepoints = n_consecutive_points;
	dlg1.m_jitter = jitter;
	if (IDOK == dlg1.DoModal())
	{
		n_consecutive_points = dlg1.m_Nconsecutivepoints;
		jitter = static_cast<short>(dlg1.m_jitter);
		flag_rejected_file_as = dlg1.m_flag;
	}
	else
		return;

	// search
	DlgProgress dlg;
	dlg.Create();
	dlg.SetStep(1);
	auto i_step = 0;
	CString cs_comment;
	auto* p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	const int n_files = p_dbwave_doc->GetDB_NRecords();

	for (int i_file = 0; i_file < n_files; i_file++)
	{
		// check if user wants to stop
		if (dlg.CheckCancelButton())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;

		cs_comment.Format(_T("Processing file [%i / %i] "), i_file + 1, n_files);
		cs_comment += p_dbwave_doc->GetDB_CurrentDatFileName();
		dlg.SetStatus(cs_comment);

		// load file
		p_dbwave_doc->set_db_current_record_position(i_file);
		const auto b_ok = p_dbwave_doc->OpenCurrentDataFile();
		if (!b_ok)
			continue;

		auto p_dat = p_dbwave_doc->m_pDat;
		if (p_dat == nullptr)
			continue;

		auto nconsecutivepoints = 0;
		long l_data_first = 0;
		const auto l_data_last = p_dat->GetDOCchanLength() - 1;

		while (l_data_first < l_data_last)
		{
			// read a chunk of data
			auto l_read_write_first = l_data_first;
			auto l_read_write_last = l_data_last;
			if (!p_dat->LoadRawData(&l_read_write_first, &l_read_write_last, 0))
				break; // exit if error reported
			if (l_read_write_last > l_data_last)
				l_read_write_last = l_data_last;
			const auto p_data0 = p_dat->LoadTransformedData(l_data_first, l_read_write_last, 0, 0);

			// compute initial offset (address of first point)
			// assume that detection is on channel 1
			auto p_data = p_data0;
			short lastvalue = 0;
			for (auto cx = l_data_first; cx <= l_read_write_last; cx++)
			{
				if (abs(lastvalue - *p_data) <= jitter)
				{
					nconsecutivepoints++;
					if (nconsecutivepoints > n_consecutive_points)
						break;
				}
				else
					nconsecutivepoints = 0;
				lastvalue = *p_data;
				p_data++;
			}
			l_data_first = l_read_write_last + 1;
		}
		// change flag if condition met
		if (nconsecutivepoints >= n_consecutive_points)
			p_dbwave_doc->SetDB_CurrentRecordFlag(flag_rejected_file_as);

		// update interface
		if (MulDiv(i_file, 100, n_files) > i_step)
		{
			dlg.StepIt();
			i_step = MulDiv(i_file, 100, n_files);
		}
	}
	// exit: update all views
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnRecordGotorecord()
{
	DlgGotoRecord dlg;
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	dlg.m_recordPos = p_dbwave_doc->GetDB_CurrentRecordPosition();
	dlg.m_recordID = p_dbwave_doc->GetDB_CurrentRecordID();
	dlg.m_bGotoRecordID = dynamic_cast<CdbWaveApp*>(AfxGetApp())->options_viewdata.bGotoRecordID;

	if (IDOK == dlg.DoModal())
	{
		dynamic_cast<CdbWaveApp*>(AfxGetApp())->options_viewdata.bGotoRecordID = dlg.m_bGotoRecordID;
		if (!dlg.m_bGotoRecordID)
			p_dbwave_doc->set_db_current_record_position(dlg.m_recordPos);
		else
			p_dbwave_doc->DBMoveToID(dlg.m_recordID);
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	}
}

void CChildFrame::OnToolsImportfiles(int ifilter)
{
	DlgFindFiles dlg;
	CStringArray file_names;
	dlg.m_pfilenames = &file_names;
	dlg.m_selinit = ifilter;
	dlg.m_pdbDoc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	if (IDOK == dlg.DoModal())
	{
		auto* p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument()); 
		p_dbwave_doc->ImportFileList(file_names);
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
		// display files which were discarded in a separate document
		PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, NULL);
	}
}

void CChildFrame::OnToolsImportATFfiles()
{
	DlgFindFiles dlg;
	CStringArray file_names;
	dlg.m_pfilenames = &file_names;
	dlg.m_selinit = 6;
	dlg.m_pdbDoc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	if (IDOK == dlg.DoModal())
	{
		DlgImportFiles dlg2;
		CStringArray convertedFiles;
		dlg2.m_pconvertedFiles = &convertedFiles;
		dlg2.m_pfilenameArray = &file_names;
		dlg2.m_option = ATFFILE;
		auto* p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
		dlg2.m_pdbDoc = p_dbwave_doc;
		if (IDOK == dlg2.DoModal())
		{
			p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
			p_dbwave_doc->ImportFileList(convertedFiles);
			p_dbwave_doc->DBMoveLast();
			p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
			// display files which were discarded in a separate document
			PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, NULL);
		}
	}
}

void CChildFrame::OnRecordDeletecurrent()
{
	const auto p_view = GetActiveView();
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument()); 

	// save index current file
	auto current_index = p_dbwave_doc->GetDB_CurrentRecordPosition();
	if (current_index < 0)
		current_index = 0;

	auto b_delete = TRUE;
	if (!m_bKeepChoice)
	{
		b_delete = FALSE;
		DlgDeleteRecordOptions dlg;
		dlg.m_bDeleteFile = m_bDeleteFile;
		dlg.m_bKeepChoice = m_bKeepChoice;

		if (IDOK == dlg.DoModal())
		{
			m_bDeleteFile = dlg.m_bDeleteFile;
			m_bKeepChoice = dlg.m_bKeepChoice;
			b_delete = TRUE;
		}
	}

	if (b_delete)
	{
		// delete records from the database and collect names of files to change
		// save list of data files to delete into a temporary array
		if (p_view->IsKindOf(RUNTIME_CLASS(ViewdbWave)))
			dynamic_cast<ViewdbWave*>(p_view)->DeleteRecords();
		else
			p_dbwave_doc->DBDeleteCurrentRecord();

		// update views and rename "erased" files
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_DOCHASCHANGED, nullptr);
		p_dbwave_doc->set_db_current_record_position(current_index);
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);

		// delete erased files
		if (m_bDeleteFile)
			p_dbwave_doc->Delete_ErasedFiles();
	}
}

BOOL CChildFrame::exportToExcel()
{
	//define a few constants
	COleVariant v_opt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant cov_true(static_cast<short>(TRUE));
	COleVariant cov_false(static_cast<short>(FALSE));
	COleVariant cov_xl_database(static_cast<short>(1));
	COleVariant cov_zero(static_cast<short>(0));

	//Start a new workbook in Excel using COleDispatchDriver (_Application) client side
	CApplication o_app;
	o_app.CreateDispatch(_T("Excel.Application"));
	if (!o_app)
		return FALSE;

	// 1 - create workbook and paste data
	CWorkbooks o_books = o_app.get_Workbooks();
	CWorkbook o_book = o_books.Add(v_opt);
	CWorksheets o_sheets = o_book.get_Worksheets();
	auto index = COleVariant(static_cast<short>(1));
	CWorksheet odata_sheet = o_sheets.get_Item(index); // select first sheet
	auto cell1 = COleVariant(_T("A1"));
	CRange o_range = odata_sheet.get_Range(cell1, v_opt); // select first cell

	odata_sheet.Paste(v_opt, v_opt); // paste data
	//odataSheet._PasteSpecial(COleVariant(_T("Text")), vOpt, vOpt, vOpt, vOpt, vOpt);

	odata_sheet.put_Name(_T("data")); // change name of sheet

	//Make Excel visible and give the user control
	o_app.put_Visible(TRUE);
	o_app.put_UserControl(TRUE);
	o_app.DetachDispatch();
	o_app.ReleaseDispatch();
	return TRUE;
}

BOOL CChildFrame::exportToExcelAndBuildPivot(int option)
{
	//define a few constants
	COleVariant v_opt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant cov_true(static_cast<short>(TRUE));
	COleVariant cov_false(static_cast<short>(FALSE));
	COleVariant cov_xl_database(static_cast<short>(1));
	COleVariant cov_zero(static_cast<short>(0));

	//Start a new workbook in Excel using COleDispatchDriver (_Application) client side
	CApplication o_app;
	o_app.CreateDispatch(_T("Excel.Application"));
	if (!o_app)
		return FALSE;

	// 1 - create workbook and paste data
	CWorkbooks o_books = o_app.get_Workbooks();
	CWorkbook o_book = o_books.Add(v_opt);
	CWorksheets o_sheets = o_book.get_Worksheets();
	auto item = COleVariant(static_cast<short>(1));
	CWorksheet odata_sheet = o_sheets.get_Item(item); // select first sheet
	auto cell1 = COleVariant(_T("A1"));
	CRange o_range = odata_sheet.get_Range(cell1, v_opt); // select first cell
	odata_sheet.Paste(v_opt, v_opt); // paste data
	odata_sheet.put_Name(_T("data")); // change name of sheet
	if (option == 1)
	{
		auto lcell1 = COleVariant(_T("A5"));
		CRange o_range1 = odata_sheet.get_Range(lcell1, v_opt); // select first cell of the table
		const auto col1 = o_range1.get_Column();
		const auto row1 = o_range1.get_Row();
		o_range1.Select();

		CRange o_select = o_range1.get_End(-4121); //xlDown); XlDirection.xlDown
		const auto row2 = o_select.get_Row();
		o_select = o_range1.get_End(-4161); //xlToRight); XlDirection.xlToRight
		const auto col2 = o_select.get_Column();

		auto rowsize = COleVariant(row2 - row1 + 1);
		auto columnsize = COleVariant(col2 - col1 + 1);
		o_range1 = o_range1.get_Resize(rowsize, columnsize);
		o_range1.Select(); // select data area

		// build range address as text (I was unable to pass the range into PivotTableWizard)
		CString cs2;
		const auto ialphabet = 26;
		if (col2 > ialphabet)
		{
			const int decimalcol = col2 / ialphabet;
			const int unicol = col2 - (decimalcol * ialphabet);
			cs2.Format(_T("%c%c%d"), _T('A') + (decimalcol - 1) % ialphabet, _T('A') + (unicol - 1) % ialphabet, row2);
		}
		else
			cs2.Format(_T("%c%d"), 'A' + (col2 - 1) % ialphabet, row2);
		CString cs1;
		cs1.Format(_T("%c%d"), 'A' + (col1 - 1) % ialphabet, row1);
		cs1 = cs1 + _T(":") + cs2;
		cs2 = odata_sheet.get_Name();
		cs1 = cs2 + _T("!") + cs1;

		auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		if (p_app->options_viewspikes.bexportPivot)
		{
			CString cs_bin;
			buildExcelPivot(&o_app, &odata_sheet, cs1, _T("pivot_cnt"), static_cast<short>(-4112), col2);
			buildExcelPivot(&o_app, &odata_sheet, cs1, _T("pivot_std"), static_cast<short>(-4156), col2);
			buildExcelPivot(&o_app, &odata_sheet, cs1, _T("pivot_avg"), static_cast<short>(-4106), col2);
		}
	}

	//Make Excel visible and give the user control
	o_app.put_Visible(TRUE);
	o_app.put_UserControl(TRUE);
	o_app.DetachDispatch();
	o_app.ReleaseDispatch();
	return TRUE;
}

void CChildFrame::buildExcelPivot(void* poApp, void* podataSheet, CString csSourceDataAddress, CString csNameSheet,
                                  short XlConsolidationFunction, int col2)
{
	COleVariant cov_xl_database(static_cast<short>(1));
	COleVariant v_opt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant cov_true(static_cast<short>(TRUE));
	COleVariant cov_false(static_cast<short>(FALSE));
	auto* o_app = static_cast<CApplication*>(poApp);
	auto* odata_sheet = static_cast<CWorksheet*>(podataSheet);

	odata_sheet->Activate();
	auto source_data = COleVariant(csSourceDataAddress);
	CPivotTable o_pivot1 = odata_sheet->PivotTableWizard(
		cov_xl_database, //const VARIANT& SourceType,
		source_data, //const VARIANT& SourceData,
		v_opt, //const VARIANT& TableDestination,
		v_opt, //const VARIANT& TableName,
		cov_false, //const VARIANT& RowGrand,
		cov_false, //const VARIANT& ColumnGrand,
		v_opt, //const VARIANT& SaveData,
		v_opt, //const VARIANT& HasAutoFormat,
		v_opt, //const VARIANT& AutoPage,
		v_opt, //const VARIANT& Reserved,
		v_opt, //const VARIANT& BackgroundQuery,
		v_opt, //const VARIANT& OptimizeCache,
		v_opt, //const VARIANT& PageFieldOrder,
		v_opt, //const VARIANT& PageFieldWrapCount,
		v_opt, //const VARIANT& ReadData,
		v_opt); //const VARIANT& Connection)

	CWorksheet pivot_sheet1 = o_app->get_ActiveSheet();
	pivot_sheet1.put_Name(csNameSheet);

	// get options
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	const auto option_view_spikes = &(p_app->options_viewspikes);

	// add fields to pivot table
	if (option_view_spikes->bacqcomments)
	{
		auto row_field = COleVariant(_T("type"));
		o_pivot1.AddFields(row_field, v_opt, v_opt, cov_false);
		row_field = COleVariant(_T("stim1"));
		o_pivot1.AddFields(row_field, v_opt, v_opt, cov_true);
		row_field = COleVariant(_T("conc1"));
		o_pivot1.AddFields(row_field, v_opt, v_opt, cov_true);
		for (auto i = 1; i <= 3; i++)
		{
			auto index = COleVariant(static_cast<short>(i));
			CPivotField o_field = o_pivot1.get_RowFields(index);
			index = COleVariant(static_cast<short>(1));
			o_field.put_Subtotals(index, cov_false);
		}
	}

	// loop over the bins
	CString cs_bin;
	auto ibin = 0;
	const auto col1 = option_view_spikes->ncommentcolumns + 1;
	for (auto i = col1; i <= col2; i++, ibin++)
	{
		cs_bin.Format(_T("bin_%i"), ibin);
		auto index = COleVariant(static_cast<short>(i));
		const auto field = o_pivot1.PivotFields(index);
		auto caption = COleVariant(cs_bin);
		auto function = COleVariant(XlConsolidationFunction);

		o_pivot1.AddDataField(field, caption, function);
	}

	if (col1 < col2) // error fired if only 1 bin is measured
	{
		CPivotField oField = o_pivot1.get_DataPivotField();
		oField.put_Orientation(2);
	}
}


void CChildFrame::OnRecordAdd()
{
	ReplaceViewIndex(ID_VIEW_ACQUIREDATA);
}

void CChildFrame::OnToolsImportDatafiles()
{
	OnToolsImportfiles(0);
}

void CChildFrame::OnToolsImportSpikefiles()
{
	OnToolsImportfiles(1);
}

void CChildFrame::OnToolsImportDatabase()
{
	CFileDialog dlgFile(TRUE);
	CString fileName;
	const int c_cMaxFiles = 100;
	const int c_cbBuffSize = (c_cMaxFiles * (MAX_PATH + 1)) + 1;
	dlgFile.GetOFN().lpstrFile = fileName.GetBuffer(c_cbBuffSize);
	dlgFile.GetOFN().nMaxFile = c_cbBuffSize;
	dlgFile.GetOFN().lpstrFilter = _T("Database Files\0*.mdb");
	dlgFile.GetOFN().lpstrTitle = _T("Select a database to be merged with current database...");
	if (IDOK == dlgFile.DoModal())
	{
		const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
		p_dbwave_doc->ImportDatabase(fileName);
		p_dbwave_doc->DBMoveLast();
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	}
}

void CChildFrame::OnToolsCopyAllProjectFiles()
{
	CString destination_path = _T("c:\\Temp");
	DlgTransferFiles dlg;
	dlg.m_csPathname = destination_path;
	if (IDOK == dlg.DoModal())
	{
		destination_path = dlg.m_csPathname;
		auto* p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
		p_dbwave_doc->CopyAllFilesToDirectory(destination_path);
	}
}

void CChildFrame::OnToolsExportdatafile()
{
	DlgExportData dlg;
	dlg.m_dbDoc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	dlg.DoModal();
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	const auto p_mainframe = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (bActivate)
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_MDIACTIVATE, NULL);
}

void CChildFrame::OnToolsPathsRelative()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	p_dbwave_doc->SetDB_PathsRelative_to_DataBaseFile();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsPathsAbsolute()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	p_dbwave_doc->SetDB_PathsAbsolute();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}

void CChildFrame::OnToolsPath()
{
	// TODO: Add your command handler code here
}

void CChildFrame::OnToolsRemoveunused()
{
	const auto p_dbwave_doc = dynamic_cast<CdbWaveDoc*>(GetActiveDocument());
	p_dbwave_doc->DBDeleteUnusedEntries();
	AfxMessageBox(_T("Accessory tables cleaned of all un-used entries"));
}

void CChildFrame::OnToolsImport()
{
	// TODO: Add your command handler code here
}

void CChildFrame::OnToolsSynchro()
{
	// TODO: Add your command handler code here
}

void CChildFrame::OnToolsGarbage()
{
	// TODO: Add your command handler code here
}

#define MAX_CFileDialog_FILE_COUNT 99
#define FILE_LIST_BUFFER_SIZE ((MAX_CFileDialog_FILE_COUNT * (MAX_PATH + 1)) + 1)

void CChildFrame::OnToolsCompactdatabase()
{
	CString file_name;
	auto p = file_name.GetBuffer(FILE_LIST_BUFFER_SIZE);
	CFileDialog dlg(TRUE);

	auto& ofn = dlg.GetOFN();
	ofn.Flags |= OFN_ALLOWMULTISELECT;
	ofn.lpstrFile = p;
	ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;
	ofn.lpstrFilter = _T("MDB Files (*.mdb)\0*.mdb\0\0");
	ofn.lpstrTitle = _T("Select MDB File");
	const auto result = dlg.DoModal();
	file_name.ReleaseBuffer();

	if (result == IDOK)
	{
		file_name = dlg.GetPathName(); 
		const auto ipos = file_name.ReverseFind('.');
		const auto file_name_new = file_name.Left(ipos) + _T("_new.mdb");

		// compact database and save new file
		CdbTable::CompactDataBase(file_name, file_name_new);

		const auto cs = file_name + _T(" database compacted and saved as ") + file_name_new;
		AfxMessageBox(cs);
	}
}
