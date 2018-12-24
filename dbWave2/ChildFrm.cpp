// ChildFrm.cpp : implementation of the CChildFrame class
//	accelerator

#include "StdAfx.h"
#include "resource.h"
#include "afxpriv.h"
#include "dbWave.h"
#include "dbWaveDoc.h"
//#include "dbIndexTable.h"

#include "MainFrm.h"
//#include "dbMainTable.h"

//#include "Cscale.h"
//#include "scopescr.h"
#include "Lineview.h"
#include "ViewdbWave.h"
//#include "Editctrl.h"
#include "ViewData.h"
#include "Browsefi.h"
#include "Printmar.h"
#include "Lsoption.h"

//#include "TemplateWnd.h"
//#include "TemplateListWnd.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"
#include "Datacomm.h"
#include "Exportsp.h"

#include "FindFilesDlg.h"
#include "ProgDlg.h"
#include "RejectRecordDlg.h"
#include "GotoRecordDlg.h"
#include "ImportFilesDlg.h"
#include "DeleteRecordOptionsDlg.h"

//#include <process.h>
#include "CApplication.h"
#include "CPivotCell.h"
#include "CPivotField.h"
//#include "CPivotFields.h"
//#include "CPivotItem.h"
//#include "CPivotItemList.h"
//#include "CPivotItems.h"
#include "CPivotTable.h"
//#include "CPivotTables.h"
#include "CRange.h"
#include "CWorkbook.h"
#include "CWorkbooks.h"
#include "CWorksheet.h"
#include "CWorksheets.h"
#include "TransferFilesDlg.h"
//#include "afxdialogex.h" 
#include "ExportDataDlg.h"
#include "ViewADcontinuous.h"

//#include "Spikebar.h"
//#include "spikeshape.h"
//#include "Spikebar.h"
//#include "spikeshape.h"
//#include "spikehistp.h"
//#include "spikexyp.h"
//#include "SpikeClassListBox.h"
#include "ViewSpikeSort.h"
#include "ViewSpikeDetect.h"
#include "ViewSpikes.h"
#include "ViewSpikeTemplate.h"
#include "ViewSpikeHist.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)

ON_COMMAND(ID_VIEW_CURSORMODE_NORMAL, OnViewCursormodeNormal)
ON_UPDATE_COMMAND_UI(ID_VIEW_CURSORMODE_NORMAL, OnUpdateViewCursormodeNormal)
ON_COMMAND(ID_VIEW_CURSORMODE_MEASURE, OnViewCursormodeMeasure)
ON_UPDATE_COMMAND_UI(ID_VIEW_CURSORMODE_MEASURE, OnUpdateViewCursormodeMeasure)
ON_COMMAND(ID_VIEW_CURSORMODE_ZOOMIN, OnViewCursormodeZoomin)
ON_UPDATE_COMMAND_UI(ID_VIEW_CURSORMODE_ZOOMIN, OnUpdateViewCursormodeZoomin)
ON_COMMAND(ID_OPTIONS_BROWSEMODE, OnOptionsBrowsemode)
ON_COMMAND(ID_OPTIONS_PRINTMARGINS, OnOptionsPrintmargins)
ON_COMMAND(ID_OPTIONS_LOADSAVEOPTIONS, OnOptionsLoadsaveoptions)
ON_MESSAGE(WM_MYMESSAGE, OnMyMessage)
ON_WM_CREATE()
ON_COMMAND_RANGE(ID_VIEW_DATABASE,ID_VIEW_ACQUIREDATA, ReplaceViewIndex)
ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_DATABASE,ID_VIEW_ACQUIREDATA, OnUpdateViewmenu)

ON_COMMAND(ID_TOOLS_EXPORTDATACOMMENTS, OnToolsExportdatacomments)						// ok
ON_COMMAND(ID_TOOLS_EXPORTDATAASTEXT, OnToolsExportdataAsText)
ON_COMMAND(ID_TOOLS_EXPORTNUMBEROFSPIKES, OnToolsExportnumberofspikes)					// ok
ON_COMMAND(ID_TOOLS_EXPORTDATAFILE, &CChildFrame::OnToolsExportdatafile)
ON_COMMAND(ID_TOOLS_REMOVEMISSINGFILENAMES, OnToolsRemoveMissingFiles)					// ok
ON_COMMAND(ID_TOOLS_REMOVEDUPLICATEFILES, OnToolsRemoveduplicatefiles)					// ok
ON_COMMAND(ID_TOOLS_REMOVEARTEFACTFILES, &CChildFrame::OnToolsRemoveartefactfiles)
ON_COMMAND(ID_TOOLS_RESTOREDELETEDFILES, &CChildFrame::OnToolsRestoredeletedfiles)
ON_COMMAND(ID_TOOLS_CHECKCONSISTENCY,  &CChildFrame::OnToolsCheckFilelistsConsistency)
ON_COMMAND(ID_TOOLS_SYNCHRO_CURRENTFILE, &CChildFrame::OnToolsSynchronizesourceinformationsCurrentfile)
ON_COMMAND(ID_TOOLS_SYNCHRO_ALLFILES, &CChildFrame::OnToolsSynchronizesourceinformationsAllfiles)
ON_COMMAND(ID_TOOLS_TRANSFERFILES, &CChildFrame::OnToolsCopyAllProjectFiles)
ON_COMMAND(ID_TOOLS_IMPORT_DATAFILES, &CChildFrame::OnToolsImportDatafiles)
ON_COMMAND(ID_TOOLS_IMPORT_SPIKEFILES, &CChildFrame::OnToolsImportSpikefiles)
ON_COMMAND(ID_TOOLS_IMPORT_DATABASE, &CChildFrame::OnToolsImportDatabase)
ON_COMMAND(ID_TOOLS_IMPORT_ATFFILES, &CChildFrame::OnToolsImportATFfiles)
//ON_COMMAND(ID_TOOLS_IMPORT_SYNTECHASPKFILES, &CChildFrame::OnToolsImportSyntechaspkfiles)

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

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_viewON		= ID_VIEW_DATABASE;
	m_previousviewON= ID_VIEW_DATABASE;
	m_cursorstate	= 0;
	m_bDeleteFile	= FALSE;
	m_bKeepChoice	= FALSE;
	m_nStatus = 0;
}


CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

// CChildFrame diagnostics

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

// CChildFrame message handlers

void CChildFrame::OnViewCursormodeNormal() 
{
	m_cursorstate=CURSOR_ARROW;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, NULL));
}


void CChildFrame::OnUpdateViewCursormodeNormal(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_cursorstate == CURSOR_ARROW);
}

void CChildFrame::OnViewCursormodeMeasure() 
{
	m_cursorstate = CURSOR_MEASURE;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, NULL));
}

void CChildFrame::OnUpdateViewCursormodeMeasure(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_cursorstate == CURSOR_MEASURE);
}

void CChildFrame::OnViewCursormodeZoomin() 
{
	m_cursorstate = CURSOR_ZOOM;
	(GetActiveView())->PostMessage(WM_MYMESSAGE, HINT_SETMOUSECURSOR, MAKELPARAM(m_cursorstate, NULL));
}

void CChildFrame::OnUpdateViewCursormodeZoomin(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_cursorstate == CURSOR_ZOOM);
}

void CChildFrame::OnOptionsBrowsemode() 
{
	CBrowseFileDlg dlg;
	dlg.mfBR = &(dynamic_cast<CdbWaveApp*> (AfxGetApp())->vdP);
	dlg.DoModal();	
}

void CChildFrame::OnOptionsPrintmargins() 
{
	CPrintMarginsDlg dlg;
	const auto psource = &(dynamic_cast<CdbWaveApp*>(AfxGetApp())->vdP);
	dlg.mdPM = psource;
	dlg.DoModal();		
}


void CChildFrame::OnOptionsLoadsaveoptions() 
{
	CLoadSaveOptionsDlg dlg;
	if (IDOK == dlg.DoModal())
	{
		const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
		auto p_parm_files=&(p_app->m_csParmFiles);
		p_parm_files->RemoveAll();
		for (auto i=0; i<dlg.pFiles.GetSize(); i++)
			p_parm_files->Add(dlg.pFiles.GetAt(i));
	}
}


void CChildFrame::OnToolsExportdatacomments() 
{
	CDataCommentsDlg dlg;
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	dlg.m_pvO = &(p_app->vdP);	
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		ExportASCII(0);
	}
}


void CChildFrame::OnToolsExportnumberofspikes() 
{
	CExportSpikeInfosDlg dlg;
	const auto p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	dlg.m_pvdS =  &p_app->vdS;
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		ExportASCII(1);
	}
}


void CChildFrame::OnToolsExportdataAsText() 
{
	auto pdb_doc = static_cast<CdbWaveDoc*>(GetActiveDocument());
	pdb_doc->ExportDatafilesAsTXTfiles();
	pdb_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);	
	PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, static_cast<LPARAM>(NULL));
}

// --------------------------------------------------------------------


void CChildFrame::ExportASCII(int option)
{
	CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	auto* pdb_doc = (CdbWaveDoc*) GetActiveDocument();
	auto* p_app = dynamic_cast<CdbWaveApp*>(AfxGetApp());
	switch (option)
	{
	case 0:	
		pdb_doc->ExportDataAsciiComments(&sf);
		break;
	case 1:
		pdb_doc->ExportNumberofSpikes(&sf);
		break;
	default:
		break;
	}

	const auto dw_len = static_cast<SIZE_T>(sf.GetLength());
	auto h_mem = sf.Detach();
	if (!h_mem)
		return;
	h_mem = ::GlobalReAlloc(h_mem, dw_len, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	if (!h_mem)
		return;
	auto* p_source = new COleDataSource();
	p_source->CacheGlobalData(CF_UNICODETEXT, h_mem);
	p_source->SetClipboard();

	//pass data to excel - if it does not work, paste to a new text document
	switch (option)
	{
	case 0:			// export CAcqData
		{
			auto flag = FALSE;
			if (p_app->vdP.btoExcel) 
				flag = ExportToExcel();
			if (!p_app->vdP.btoExcel || !flag)
			{
				CMultiDocTemplate* p_templ = p_app->m_pNoteViewTemplate;
				const auto pdb_doc_export = p_templ->OpenDocumentFile(nullptr);
				auto pos = pdb_doc_export->GetFirstViewPosition();
				const auto p_view = (CViewNoteDoc*) pdb_doc_export->GetNextView(pos);
				auto& p_edit = p_view->GetRichEditCtrl();
				p_edit.Paste();
			}
		}
		break;
	case 1:
		{
			auto flag = FALSE;
			if (p_app->vdS.bexporttoExcel) 
				flag = ExportToExcelAndBuildPivot(option);

			if (!p_app->vdS.bexporttoExcel || !flag)
			{
				CMultiDocTemplate* p_templ = p_app->m_pNoteViewTemplate;
				const auto pdb_doc_export = p_templ->OpenDocumentFile(nullptr);
				auto pos = pdb_doc_export->GetFirstViewPosition();
				const auto p_view = (CViewNoteDoc*)pdb_doc_export->GetNextView(pos);
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

// --------------------------------------------------------------------------

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
			//	m_cursorstate=CURSOR_ARROW;
			//	break;
			//}
			m_cursorstate = lowp;
		}
		break;	

	case HINT_SHAREDMEMFILLED:
		{
			auto* p_app= dynamic_cast<CdbWaveApp*>(AfxGetApp());
			if (p_app->m_psf != nullptr)
			{
				CMultiDocTemplate* p_templ = p_app->m_pNoteViewTemplate;
				const auto p_doc_export = p_templ->OpenDocumentFile(nullptr);
				auto pos = p_doc_export->GetFirstViewPosition();
				auto* p_view = (CViewNoteDoc*)(p_doc_export->GetNextView(pos));
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

////////////////////////////////////////////////////////////////////////////////////
// switch between different views or create a new one if it exists
// sample routine extracted from viewplace (MSDN CD9)
// VWRPLC: Replaces Views for a CMDIChildWndEx-derived Class

void CChildFrame::ReplaceViewIndex(UINT iID)
{
	auto pdb_doc = (CdbWaveDoc*) GetActiveDocument();
	auto p_mainframe = (CMainFrame*)AfxGetMainWnd();
	auto b_active_panes = TRUE;
	switch (iID)
	{
	case ID_VIEW_DATABASE:
		ReplaceView(RUNTIME_CLASS(CViewdbWave), ((CdbWaveApp*)AfxGetApp())->m_hDBView);
		break;
	case ID_VIEW_DATAFILE:
		if (!pdb_doc->DBGetCurrentDatFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CViewData), ((CdbWaveApp*)AfxGetApp())->m_hDataView);	
		break;
	case ID_VIEW_SPIKEDETECTION:
		if (!pdb_doc->DBGetCurrentDatFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CViewSpikeDetection), ((CdbWaveApp*)AfxGetApp())->m_hDataView);
		break;
	case ID_VIEW_SPIKEDISPLAY:
		if (!pdb_doc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CViewSpikes), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKESORTINGAMPLITUDE:
		if (!pdb_doc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CViewSpikeSort), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKESORTINGTEMPLATES:
		if (!pdb_doc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CViewSpikeTemplates), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKETIMESERIES:
		if (!pdb_doc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CViewSpikeHist), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_ACQUIREDATA:
		ReplaceView(RUNTIME_CLASS(CViewADContinuous), ((CdbWaveApp*)AfxGetApp())->m_hDataView);
		b_active_panes = FALSE;
		break;

	default:
		iID = 0;
		ReplaceView(RUNTIME_CLASS(CViewdbWave), ((CdbWaveApp*)AfxGetApp())->m_hDataView);
		break;
	}
	p_mainframe->ActivatePropertyPane(b_active_panes);
	p_mainframe->ActivateFilterPane(b_active_panes);
	m_previousviewON = m_viewON;
	m_viewON=iID;

	// update all views
	auto doctype=1;
	if (iID <ID_VIEW_SPIKEDISPLAY || iID == ID_VIEW_ACQUIREDATA)
		doctype = 0;
	pdb_doc->UpdateAllViews(nullptr, MAKELPARAM(HINT_REPLACEVIEW, doctype), nullptr);
}


void CChildFrame::OnUpdateViewmenu(CCmdUI* pCmdUI)
{
	auto p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();
	ASSERT(p_dbwave_doc);
	const auto p_app = (CdbWaveApp*) AfxGetApp();
	BOOL flag = (p_dbwave_doc != nullptr);
		
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_SPIKEDISPLAY:
	case ID_VIEW_SPIKESORTINGAMPLITUDE:
	case ID_VIEW_SPIKESORTINGTEMPLATES:
	case ID_VIEW_SPIKETIMESERIES:
		flag = (flag	
			&& !p_dbwave_doc->DBGetCurrentSpkFileName(TRUE).IsEmpty()	
			&& m_viewON != ID_VIEW_ACQUIREDATA);
		break;

	case ID_VIEW_ACQUIREDATA:
		flag = p_app->m_bADcardFound ;
		break;

	default:
		flag = (flag && !p_dbwave_doc->DBGetCurrentDatFileName().IsEmpty());
		break;
	}

	pCmdUI->Enable(flag);
	pCmdUI->SetCheck(m_viewON==pCmdUI->m_nID);
}


// assume that the views replaced are of CDaoRecordView type

void CChildFrame::ReplaceView(CRuntimeClass* pViewClass, HMENU hmenu)
{
	auto p_current_view = GetActiveView();
	if ((p_current_view->IsKindOf(pViewClass))==TRUE)
		return;

	auto* p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();	
	ASSERT(p_dbwave_doc);
	p_dbwave_doc->m_hMyMenu = hmenu;

	CSize size;
	CRect rect;
	p_current_view->GetClientRect(rect);
	size.cx = rect.right;	// or: rect.Width()
	size.cy = rect.bottom;	// or: rect.Height()

	// delete old view without deleting document
	const auto bautodel	= p_dbwave_doc->m_bAutoDelete;
	p_dbwave_doc->m_bAutoDelete = FALSE;
	p_current_view->DestroyWindow();
	p_dbwave_doc->m_bAutoDelete = bautodel;

	// create new view
	CCreateContext context;	
	context.m_pNewViewClass = pViewClass;
	context.m_pCurrentDoc = p_dbwave_doc;
	context.m_pNewDocTemplate = nullptr;
	context.m_pLastView = nullptr;	
	context.m_pCurrentFrame = nullptr;	

	// create view inside a splitter
	const auto p_new_view = (CView*) CreateView(&context);
	p_new_view->SendMessage(WM_INITIALUPDATE, 0, 0);
	GetMDIFrame()->MDISetMenu(CMenu::FromHandle(hmenu), nullptr);
	GetMDIFrame()->DrawMenuBar();
	RecalcLayout();

	// adjust size of the view replacing the previous one
	p_new_view->SendMessage(WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(size.cx, size.cy)); 	// MAKELPARAM = low, high
	SetActiveView(p_new_view);
}


void CChildFrame::OnToolsRemoveMissingFiles()
{
	auto p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
	ASSERT(p_dbwave_doc);										// debug: check that doc is defined
	
	p_dbwave_doc->RemoveMissingFiles();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnToolsRemoveduplicatefiles()
{
	auto p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
	ASSERT(p_dbwave_doc);										// debug: check that doc is defined
	
	p_dbwave_doc->RemoveDuplicateFiles();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnToolsCheckFilelistsConsistency()
{
	auto p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
	ASSERT(p_dbwave_doc);										// debug: check that doc is defined
	
	p_dbwave_doc->RemoveFalseSpkFiles();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnToolsRestoredeletedfiles()
{
	// scan directories and rename files *.datdel into *.dat and *.spkdel into *.spk
	CStringArray filenames;			// store file names in this array
	CFindFilesDlg dlg;
	dlg.m_pfilenames = &filenames;
	dlg.m_selinit = 0;
	dlg.m_ioption=1;
	dlg.m_pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	const auto i_result = dlg.DoModal();		

	if (IDOK == i_result)
	{
		CProgressDlg dlg_progress;
		dlg_progress.Create();
		dlg_progress.SetStep (1);
		auto istep = 0;
		CString cscomment;

		const auto nfiles = filenames.GetSize();
		for (auto i=0; i<nfiles; i++)
		{
			auto csoldname = filenames[i];
			auto csnewname = csoldname.Left(csoldname.GetLength()-3);
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
	auto* p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	ASSERT(p_dbwave_doc != NULL);		
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnToolsSynchronizesourceinformationsCurrentfile()
{
	auto p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	ASSERT(p_dbwave_doc);		
	p_dbwave_doc->SynchronizeSourceInfos(FALSE);
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnToolsSynchronizesourceinformationsAllfiles()
{
	CdbWaveDoc* p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	ASSERT(p_dbwave_doc);		
	p_dbwave_doc->SynchronizeSourceInfos(TRUE);
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnToolsRemoveartefactfiles()
{
	CRejectRecordDlg dlg1;
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
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetStep (1);
	auto istep = 0;
	CString cscomment;
	auto* p_dbwave_doc = (CdbWaveDoc*)GetActiveDocument();
	const int nfiles = p_dbwave_doc->DBGetNRecords();

	for (int ifile = 0; ifile < nfiles; ifile ++)
	{
		// check if user wants to stop
		if(dlg.CheckCancelButton())
			if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				break;

		cscomment.Format(_T("Processing file [%i / %i] "), ifile + 1, nfiles);
		cscomment += p_dbwave_doc->DBGetCurrentDatFileName();
		dlg.SetStatus(cscomment);

		// load file
		p_dbwave_doc->DBSetCurrentRecordPosition(ifile);
		const auto b_ok = p_dbwave_doc->OpenCurrentDataFile();
		if (!b_ok)
			continue;

		auto p_dat = p_dbwave_doc->m_pDat;
		if (p_dat == nullptr)
			continue;

		auto nconsecutivepoints = 0;
		long l_data_first = 0;
		const auto l_data_last = p_dat->GetDOCchanLength() -1;

		while (l_data_first < l_data_last)
		{
			// read a chunk of data 
			auto l_read_write_first=l_data_first;
			auto l_read_write_last =l_data_last;
			if (!p_dat->LoadRawData(&l_read_write_first, &l_read_write_last, 0))
				break;								// exit if error reported
			if (l_read_write_last > l_data_last)
				l_read_write_last = l_data_last;
			const auto p_data0 = p_dat->LoadTransfData(l_data_first, l_read_write_last, 0, 0);

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
			l_data_first = l_read_write_last+1;
		}
		// change flag if condition met
		if (nconsecutivepoints >= n_consecutive_points)
			p_dbwave_doc->DBSetCurrentRecordFlag(flag_rejected_file_as);

		// update interface
		if (MulDiv(ifile, 100, nfiles) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfiles);
		}
	}
	// exit: update all views
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnRecordGotorecord()
{
	CGotoRecordDlg dlg;
	auto p_dbwave_doc = (CdbWaveDoc*)GetActiveDocument();
	dlg.m_recordPos = p_dbwave_doc->DBGetCurrentRecordPosition();
	dlg.m_recordID = p_dbwave_doc->DBGetCurrentRecordID();
	dlg.m_bGotoRecordID = ((CdbWaveApp*) AfxGetApp())->vdP.bGotoRecordID;

	if (IDOK == dlg.DoModal())
	{
		((CdbWaveApp*) AfxGetApp())->vdP.bGotoRecordID = dlg.m_bGotoRecordID;		
		if (!dlg.m_bGotoRecordID)
			p_dbwave_doc->DBSetCurrentRecordPosition(dlg.m_recordPos);
		else
			p_dbwave_doc->DBMoveToID(dlg.m_recordID);
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);
	}
}


void CChildFrame::OnToolsImportfiles(int ifilter)
{
	CFindFilesDlg dlg;
	CStringArray filenames;
	dlg.m_pfilenames = &filenames;
	dlg.m_selinit = ifilter;
	dlg.m_pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	if (IDOK == dlg.DoModal())
	{
		auto* p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
		const BOOL b_only_genuine = !dlg.m_banyformat;
		p_dbwave_doc->ImportDescFromFileList(filenames, b_only_genuine);
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);	
		// display files which were discarded in a separate document
		PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, static_cast<LPARAM>(NULL));
	}
}


void CChildFrame::OnToolsImportATFfiles()
{
	CFindFilesDlg dlg;
	CStringArray filenames;			// store file names in this array
	dlg.m_pfilenames = &filenames;
	dlg.m_selinit = 6;
	dlg.m_pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	if (IDOK == dlg.DoModal())
	{
		CImportFilesDlg dlg2;
		CStringArray convertedFiles;
		dlg2.m_pconvertedFiles = &convertedFiles;
		dlg2.m_pfilenameArray = &filenames;
		dlg2.m_option = ATFFILE;
		auto* p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
		dlg2.m_pdbDoc = p_dbwave_doc;
		if (IDOK == dlg2.DoModal())
			{
				p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
				p_dbwave_doc->ImportDescFromFileList(convertedFiles);
				p_dbwave_doc->DBMoveLast();
				p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);	
				// display files which were discarded in a separate document
				PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, static_cast<LPARAM>(NULL));
			}
	}
}


void CChildFrame::OnRecordDeletecurrent()
{
	const auto p_view = GetActiveView();
	auto p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	
	// save index current file
	auto currentindex = p_dbwave_doc->DBGetCurrentRecordPosition();
	if (currentindex <0)
		currentindex = 0;

	auto b_delete = TRUE;
	if(!m_bKeepChoice)
	{
		b_delete = FALSE;
		CDeleteRecordOptionsDlg dlg;
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
		if (p_view->IsKindOf(RUNTIME_CLASS(CViewdbWave)))
			((CViewdbWave*)p_view)->DeleteRecords();
		else
			p_dbwave_doc->DBDeleteCurrentRecord();

		// update views and rename "erased" files
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_DOCHASCHANGED, nullptr);
		p_dbwave_doc->DBSetCurrentRecordPosition(currentindex);
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_DOCMOVERECORD, nullptr);

		// delete erased files
		if (m_bDeleteFile)
			p_dbwave_doc->DeleteErasedFiles();
	}
}


BOOL CChildFrame::ExportToExcel()
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
	CWorksheet odata_sheet = o_sheets.get_Item(index);		// select first sheet
	auto cell1 = COleVariant(_T("A1"));
	CRange o_range = odata_sheet.get_Range(cell1, v_opt);	// select first cell

	odata_sheet.Paste(v_opt, v_opt);						// paste data
	//odataSheet._PasteSpecial(COleVariant(_T("Text")), vOpt, vOpt, vOpt, vOpt, vOpt);

	odata_sheet.put_Name(_T("data"));						// change name of sheet
	
	//Make Excel visible and give the user control
	o_app.put_Visible(TRUE);
	o_app.put_UserControl(TRUE);
	o_app.DetachDispatch();
	o_app.ReleaseDispatch();
	return TRUE;
}


BOOL CChildFrame::ExportToExcelAndBuildPivot(int option)
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
	CWorksheet odata_sheet = o_sheets.get_Item(item);		// select first sheet
	auto cell1 = COleVariant(_T("A1"));
	CRange o_range = odata_sheet.get_Range(cell1, v_opt);	// select first cell
	odata_sheet.Paste(v_opt, v_opt);						// paste data 
	odata_sheet.put_Name(_T("data"));						// change name of sheet
	if (option == 1)
	{
		auto lcell1 = COleVariant(_T("A5"));
		CRange o_range1 = odata_sheet.get_Range(lcell1, v_opt);		// select first cell of the table
		const auto col1 = o_range1.get_Column();
		const auto row1 = o_range1.get_Row();
		o_range1.Select();

		CRange o_select = o_range1.get_End(-4121);			//xlDown); XlDirection.xlDown
		const auto row2 = o_select.get_Row();
		o_select = o_range1.get_End(-4161);					//xlToRight); XlDirection.xlToRight
		const auto col2 = o_select.get_Column();

		auto rowsize = COleVariant(row2 - row1 + 1);
		auto columnsize = COleVariant(col2 - col1 + 1);
		o_range1 = o_range1.get_Resize(rowsize, columnsize);
		o_range1.Select();									// select data area

		// build range address as text (I was unable to pass the range into PivotTableWizard)
		CString cs2;
		const auto ialphabet = 26;
		if (col2 > ialphabet)
		{
			const int decimalcol = col2/ialphabet;
			const int unicol = col2 - (decimalcol*ialphabet);
			cs2.Format(_T("%c%c%d"),_T('A')+(decimalcol -1) % ialphabet, _T('A') + (unicol - 1) %ialphabet, row2);
		}
		else
			cs2.Format(_T("%c%d"), 'A' + (col2-1)%ialphabet, row2);
		CString cs1;
		cs1.Format(_T("%c%d"), 'A' + (col1-1)%ialphabet, row1);
		cs1 = cs1+_T(":")+cs2;
		cs2 = odata_sheet.get_Name();
		cs1 = cs2 + _T("!") + cs1;

		auto* p_app = (CdbWaveApp*) AfxGetApp();
		if (p_app->vdS.bexportPivot) 
		{
			CString cs_bin;
			BuildPivot(&o_app, &odata_sheet, cs1, _T("pivot_cnt"), static_cast<short>(-4112), col2);
			BuildPivot(&o_app, &odata_sheet, cs1, _T("pivot_std"), static_cast<short>(-4156), col2);
			BuildPivot(&o_app, &odata_sheet, cs1, _T("pivot_avg"), static_cast<short>(-4106), col2);
		}
	}

	//Make Excel visible and give the user control
	o_app.put_Visible(TRUE);
	o_app.put_UserControl(TRUE);
	o_app.DetachDispatch();
	o_app.ReleaseDispatch();
	return TRUE;
}


void CChildFrame::BuildPivot(void* poApp, void* podataSheet, CString csSourceDataAddress, CString csNameSheet, short XlConsolidationFunction, int col2)
{
	COleVariant cov_xl_database(static_cast<short>(1));
	COleVariant v_opt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant cov_true(static_cast<short>(TRUE));
	COleVariant cov_false(static_cast<short>(FALSE));
	auto* o_app = static_cast<CApplication*>(poApp);
	auto* odata_sheet = static_cast<CWorksheet*>(podataSheet);

	odata_sheet->Activate();
	auto sourcedata = COleVariant(csSourceDataAddress);
	CPivotTable o_pivot1 = odata_sheet->PivotTableWizard(
			cov_xl_database,						//const VARIANT& SourceType,
			sourcedata,	//const VARIANT& SourceData,
			v_opt,								//const VARIANT& TableDestination,
			v_opt,								//const VARIANT& TableName,
			cov_false,							//const VARIANT& RowGrand, 
			cov_false,							//const VARIANT& ColumnGrand, 
			v_opt,								//const VARIANT& SaveData, 
			v_opt,								//const VARIANT& HasAutoFormat, 
			v_opt,								//const VARIANT& AutoPage, 
			v_opt,								//const VARIANT& Reserved, 
			v_opt,								//const VARIANT& BackgroundQuery, 
			v_opt,								//const VARIANT& OptimizeCache, 
			v_opt,								//const VARIANT& PageFieldOrder, 
			v_opt,								//const VARIANT& PageFieldWrapCount, 
			v_opt,								//const VARIANT& ReadData, 
			v_opt);								//const VARIANT& Connection)

	CWorksheet pivot_sheet1 = o_app->get_ActiveSheet();
	pivot_sheet1.put_Name(csNameSheet);

	// get options
	auto* p_app = (CdbWaveApp*) AfxGetApp();
	const auto p_option_viewspikes = &(p_app->vdS);

	// add fields to pivottable
	if (p_option_viewspikes->bacqcomments)
	{
		auto rowfield = COleVariant(_T("type"));
		o_pivot1.AddFields(rowfield, v_opt, v_opt, cov_false);
		rowfield = COleVariant(_T("stim1"));
		o_pivot1.AddFields(rowfield, v_opt, v_opt, cov_true);
		rowfield = COleVariant(_T("conc1"));
		o_pivot1.AddFields(rowfield, v_opt, v_opt, cov_true);
		for (auto i=1; i <= 3; i++)
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
	const auto col1 = p_option_viewspikes->ncommentcolumns +1;
	for (auto i=col1; i <= col2; i++, ibin++)
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


//#include "autospike_files\appdata.h"
//#include "autospike_files\ASpkDoc.h"
//void CChildFrame::OnToolsImportSyntechaspkfiles()
//{
//	CStringArray filenames;			// store file names in this array
//	int iResult;					// local variable
//	CFindFilesDlg dlg;
//	dlg.m_pfilenames = &filenames;
//	dlg.m_selinit = 7;
//	iResult = dlg.DoModal();
//	if (IDOK == iResult)
//	{
//		//CImportFilesDlg dlg2;
//		//CStringArray convertedFiles;
//		//dlg2.m_pconvertedFiles = &convertedFiles;
//		//dlg2.m_pfilenameArray = &filenames;
//		//dlg2.m_option = ATFFILE;
//		//CdbWaveDoc* p_dbwave_doc = (CdbWaveDoc*) GetMDIActiveDocument();// get pointer to document
//		//dlg2.m_pdbDoc = p_dbwave_doc;
//		//
//		//iResult = dlg2.DoModal();
//		//if (IDOK == iResult)
//		//	{
//		//		CdbWaveDoc* p_dbwave_doc = (CdbWaveDoc*) GetMDIActiveDocument();// get pointer to document
//		//		p_dbwave_doc->ImportDescFromFileList(convertedFiles);
//		//		p_dbwave_doc->BuilInsectAndSensillumIDArrays();
//		//		p_dbwave_doc->MoveRecord(ID_RECORD_LAST);
//		//		p_dbwave_doc->UpdateAllViews(NULL, HINT_REQUERY, NULL);	
//		//		// display files which were discarded in a separate document
//		//		PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, (LPARAM) NULL);
//		//	}
//		CAutoSpkDoc* pautospkDoc = new CAutoSpkDoc;
//		CString cs = filenames.GetAt(0);
//		pautospkDoc->ReadAspkFile(cs, NULL, FALSE);
//		delete	pautospkDoc;
//	}
//}

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
		auto p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();
		p_dbwave_doc->ImportDatabase(fileName);
		p_dbwave_doc->DBMoveLast();
		p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);	
	}
}


void CChildFrame::OnToolsCopyAllProjectFiles()
{
	CString destination_path = _T("c:\\Temp");
	CTransferFilesDlg dlg;
	dlg.m_csPathname = destination_path;
	if (IDOK== dlg.DoModal())
	{
		destination_path = dlg.m_csPathname;
		auto* p_dbwave_doc = (CdbWaveDoc*) GetActiveDocument();
		p_dbwave_doc->CopyAllFilesintoDirectory(destination_path);
	}
}


void CChildFrame::OnToolsExportdatafile()
{
	CExportDataDlg dlg;
	dlg.m_dbDoc = (CdbWaveDoc*) GetActiveDocument();
	dlg.DoModal();
}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	auto p_mainframe = (CMainFrame*)AfxGetMainWnd();
	if (bActivate)
		p_mainframe->PostMessage(WM_MYMESSAGE, HINT_MDIACTIVATE, NULL);

}


void CChildFrame::OnToolsPathsRelative()
{
	auto p_dbwave_doc = (CdbWaveDoc*)GetActiveDocument();
	p_dbwave_doc->DBSetPathsRelative_to_DataBaseFile();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}


void CChildFrame::OnToolsPathsAbsolute()
{
	auto p_dbwave_doc = (CdbWaveDoc*)GetActiveDocument();
	p_dbwave_doc->DBSetPathsAbsolute();
	p_dbwave_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
}



void CChildFrame::OnToolsPath()
{
	// TODO: Add your command handler code here
}



void CChildFrame::OnToolsRemoveunused()
{
	auto p_dbwave_doc = (CdbWaveDoc*)GetActiveDocument();
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
		file_name = dlg.GetPathName(); // return full path and filename
		const auto ipos = file_name.ReverseFind('.');
		const auto file_name_new = file_name.Left(ipos) + _T("_new.mdb");

		// compact database and save new file
		CDaoWorkspace::CompactDatabase(file_name, file_name_new, dbLangGeneral, 0);

		const auto cs = file_name + _T(" database compacted and saved as ") + file_name_new;
		AfxMessageBox(cs);
	}
}