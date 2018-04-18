// ChildFrm.cpp : implementation of the CChildFrame class
//	accelerator

#include "stdafx.h"
#include "resource.h"
#include "AFXPRIV.H"
#include "dbWave.h"
#include "dbWaveDoc.h"
#include "dbIndexTable.h"

#include "MainFrm.h"
#include "dbMainTable.h"

#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"
#include "spikebar.h"
#include "spikeshape.h"
#include "ViewdbWave.h"
#include "editctrl.h"
#include "viewdata.h"
#include "browsefi.h"
#include "printmar.h"
#include "lsoption.h"

#include "ViewSpikeDetect.h"
#include "SpikeClassListBox.h"
#include "ViewSpikes.h"
#include "spikehistp.h"
#include "spikexyp.h"
#include "ViewSpikeSort.h"
#include "TemplateWnd.h"
#include "TemplateListWnd.h"
#include "ViewSpikeTemplate.h"
#include "ViewSpikeHist.h"
#include "notedoc.h"
#include "ViewNotedoc.h"
#include "datacomm.h"
#include "exportsp.h"

#include "findfilesdlg.h"
#include "ProgDlg.h"
#include "RejectRecordDlg.h"
#include "GotoRecordDlg.h"
#include "ImportFilesDlg.h"
#include "DeleteRecordOptionsDlg.h"

#include "ChildFrm.h"

#include <process.h>
#include "CApplication.h"
#include "CPivotCell.h"
#include "CPivotField.h"
#include "CPivotFields.h"
#include "CPivotItem.h"
#include "CPivotItemList.h"
#include "CPivotItems.h"
#include "CPivotTable.h"
#include "CPivotTables.h"
#include "CRange.h"
#include "CWorkbook.h"
#include "CWorkbooks.h"
#include "CWorksheet.h"
#include "CWorksheets.h"
#include "TransferFilesDlg.h"
#include "afxdialogex.h" 
#include "ExportDataDlg.h"
//#include "excel9.h" 
#include "ViewADContinuous.h"
#include <process.h>

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
	m_pMenu			= NULL;
	m_cursorstate	= 0;
	m_bDeleteFile	= FALSE;
	m_bKeepChoice	= FALSE;
}


CChildFrame::~CChildFrame()
{
	SAFE_DELETE(m_pMenu);
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

////////////////////////////////////////////////////////////////////////////////////

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
	dlg.mfBR = &(((CdbWaveApp*) AfxGetApp())->vdP);
	dlg.DoModal();	
}


void CChildFrame::OnOptionsPrintmargins() 
{
	CPrintMarginsDlg dlg;	
	OPTIONS_VIEWDATA* psource = &(((CdbWaveApp*) AfxGetApp())->vdP);    
	dlg.mdPM = psource;
	dlg.DoModal();		
}


void CChildFrame::OnOptionsLoadsaveoptions() 
{
	CLoadSaveOptionsDlg dlg;
	if (IDOK == dlg.DoModal())
	{
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		CStringArray* pParmFiles=&(pApp->m_csParmFiles);
		pParmFiles->RemoveAll();
		for (int i=0; i<dlg.pFiles.GetSize(); i++)
			pParmFiles->Add(dlg.pFiles.GetAt(i));
	}
}


void CChildFrame::OnToolsExportdatacomments() 
{
	CDataCommentsDlg dlg;					// define dialog box
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	dlg.m_pvO = &(pApp->vdP);				// pass parameters address
	// open dialog box & action if OK
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		ExportASCII(0);
	}
}


void CChildFrame::OnToolsExportnumberofspikes() 
{
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	BOOL bDoit=FALSE;
	CExportSpikeInfosDlg dlg;		// create dialog
	dlg.m_pvdS =  &(pApp->vdS);		// pass address of spike parameters

	// open dialog & action if IDOK
	if (IDOK == dlg.DoModal())
	{
		UpdateWindow();
		CWaitCursor wait;
		ExportASCII(1);
	}
}


void CChildFrame::OnToolsExportdataAsText() 
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();
	pdbDoc->ExportDatafilesAsTXTfiles();
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);	
	// display files which were transformed in a separate document
	PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, (LPARAM) NULL);
}

// --------------------------------------------------------------------


void CChildFrame::ExportASCII(int option)
{
	CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	int nbcommentcolumns = 0;

	switch (option)
	{
	case 0:			// export CAcqData
		pdbDoc->ExportDataAsciiComments(&sf);
		break;
	case 1:
		pdbDoc->ExportNumberofSpikes(&sf);
		break;
	default:
		break;
	}

	DWORD dwLen = (DWORD) sf.GetLength();
	HGLOBAL hMem = sf.Detach();
	if (!hMem)
		return;
	hMem = ::GlobalReAlloc(hMem, dwLen, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
	if (!hMem)
		return;
	COleDataSource* pSource = new COleDataSource();
	pSource->CacheGlobalData(CF_UNICODETEXT, hMem);
	pSource->SetClipboard();

	//pass data to excel - if it does not work, paste to a new text document
	switch (option)
	{
	case 0:			// export CAcqData
		{
			BOOL flag = FALSE;
			if (pApp->vdP.btoExcel) 
				flag = ExportToExcel();
			if (!pApp->vdP.btoExcel || !flag)
			{
				CMultiDocTemplate* pTempl = pApp->m_pNoteViewTemplate;
				CDocument* pdbDocExport = pTempl->OpenDocumentFile(NULL);
				POSITION pos = pdbDocExport->GetFirstViewPosition();
				CNoteDocView* pView = (CNoteDocView*)pdbDocExport->GetNextView(pos);
				CRichEditCtrl& pEdit = pView->GetRichEditCtrl();
				pEdit.Paste();
			}
		}
		break;
	case 1:
		{
			BOOL flag = FALSE;
			if (pApp->vdS.bexporttoExcel) 
				flag = ExportToExcelAndBuildPivot(option);

			if (!pApp->vdS.bexporttoExcel || !flag)
			{
				CMultiDocTemplate* pTempl = pApp->m_pNoteViewTemplate;
				CDocument* pdbDocExport = pTempl->OpenDocumentFile(NULL);
				POSITION pos = pdbDocExport->GetFirstViewPosition();
				CNoteDocView* pView = (CNoteDocView*)pdbDocExport->GetNextView(pos);
				CRichEditCtrl& pEdit = pView->GetRichEditCtrl();
				pEdit.Paste();
			}
		}
		break;
	default:
		break;
	}
}


void CChildFrame::ActivateFrame(int nCmdShow) 
{
	if (nCmdShow == -1)
		nCmdShow = SW_SHOWMAXIMIZED;
	CMDIChildWndEx::ActivateFrame(nCmdShow);
}

// --------------------------------------------------------------------------

LRESULT CChildFrame::OnMyMessage(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case HINT_SETMOUSECURSOR:
		{
			short lowp = LOWORD(lParam);	
			short highp = HIWORD(lParam);
			WORD  menuID;
			switch (lowp)
			{
			case CURSOR_ZOOM:
				menuID= ID_VIEW_CURSORMODE_ZOOMIN;
				break;
			case CURSOR_MEASURE:
				menuID = ID_VIEW_CURSORMODE_MEASURE;
				break;
			case CURSOR_VERTICAL:
				menuID = ID_VIEW_CURSORMODE_ZOOMIN;
				break;
			default:
				menuID = ID_VIEW_CURSORMODE_NORMAL;
				m_cursorstate=CURSOR_ARROW;
				break;
			}
			m_cursorstate = lowp;
		}
		break;	

	case HINT_SHAREDMEMFILLED:
		{
			CdbWaveApp* pApp= (CdbWaveApp*) AfxGetApp();
			if (pApp->m_psf != NULL)
			{
				CMultiDocTemplate* pTempl = ((CdbWaveApp*) AfxGetApp())->m_pNoteViewTemplate;
				CDocument* pDocExport = pTempl->OpenDocumentFile(NULL);
				POSITION pos = pDocExport->GetFirstViewPosition();
				CNoteDocView* pView = (CNoteDocView*)pDocExport->GetNextView(pos);
				CRichEditCtrl& pEdit = pView->GetRichEditCtrl();
				OpenClipboard();
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, (pApp->m_psf)->Detach());
				CloseClipboard();
				pEdit.Paste();
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
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();
	CMainFrame* pmF = (CMainFrame*)AfxGetMainWnd();
	BOOL bActivePanes = TRUE;
	switch (iID)
	{
	case ID_VIEW_DATABASE:
		ReplaceView(RUNTIME_CLASS(CdbWaveView), ((CdbWaveApp*)AfxGetApp())->m_hDBView);
		break;
	case ID_VIEW_DATAFILE:
		if (!pdbDoc->DBGetCurrentDatFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CDataView), ((CdbWaveApp*)AfxGetApp())->m_hDataView);	
		break;
	case ID_VIEW_SPIKEDETECTION:
		if (!pdbDoc->DBGetCurrentDatFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CSpikeDetectionView), ((CdbWaveApp*)AfxGetApp())->m_hDataView);
		break;
	case ID_VIEW_SPIKEDISPLAY:
		if (!pdbDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CSpikeView), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKESORTINGAMPLITUDE:
		if (!pdbDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CSpikeSort1View), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKESORTINGTEMPLATES:
		if (!pdbDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CSpikeTemplateView), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_SPIKETIMESERIES:
		if (!pdbDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty())
			ReplaceView(RUNTIME_CLASS(CSpikeHistView), ((CdbWaveApp*)AfxGetApp())->m_hSpikeView);
		break;
	case ID_VIEW_ACQUIREDATA:
		ReplaceView(RUNTIME_CLASS(CADContView), ((CdbWaveApp*)AfxGetApp())->m_hDataView);
		bActivePanes = FALSE;
		break;

	default:
		iID = 0;
		ReplaceView(RUNTIME_CLASS(CdbWaveView), ((CdbWaveApp*)AfxGetApp())->m_hDataView);
		break;
	}
	pmF->ActivatePropertyPane(bActivePanes);
	pmF->ActivateFilterPane(bActivePanes);
	m_previousviewON = m_viewON;
	m_viewON=iID;

	// update all views
	int doctype=1;
	if (iID <ID_VIEW_SPIKEDISPLAY || iID == ID_VIEW_ACQUIREDATA)
		doctype = 0;
	else
		doctype = 1;
	pdbDoc->UpdateAllViews(NULL, MAKELPARAM(HINT_REPLACEVIEW, doctype), NULL);
}


void CChildFrame::OnUpdateViewmenu(CCmdUI* pCmdUI)
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();
	ASSERT(pdbDoc);
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	BOOL flag = (pdbDoc != NULL);
		
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_SPIKEDISPLAY:
	case ID_VIEW_SPIKESORTINGAMPLITUDE:
	case ID_VIEW_SPIKESORTINGTEMPLATES:
	case ID_VIEW_SPIKETIMESERIES:
		flag = (flag										// no data file?
			&& !pdbDoc->DBGetCurrentSpkFileName(TRUE).IsEmpty()	// no spike file?
			&& m_viewON != ID_VIEW_ACQUIREDATA);			// data acquisition acquisition mode? (no spike file)
		break;

	case ID_VIEW_ACQUIREDATA:
		flag = pApp->m_bADcardFound ;
		break;

	default:
		flag = (flag && !pdbDoc->DBGetCurrentDatFileName().IsEmpty());
		break;
	}

	pCmdUI->Enable(flag);
	pCmdUI->SetCheck(m_viewON==pCmdUI->m_nID);
}


//////////////////////////////////////////////////////////////////////////////////////////////
// assume that the views replaced are of CDaoRecordView type

void CChildFrame::ReplaceView(CRuntimeClass* pViewClass, HMENU hmenu)
{
	// is current View is already of the class requested?? YES, return without doing anything
	CView* pCurrentView = GetActiveView();
	if ((pCurrentView->IsKindOf(pViewClass))==TRUE)
		return;

	// tell document which menu is currently on
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
	ASSERT(pdbDoc);											// debug: check that doc is defined
	pdbDoc->m_hMyMenu = hmenu;

	// measure size of the view and save status
	CSize size;
	CRect rect;
	pCurrentView->GetClientRect(rect);
	size.cx = rect.right;	// or: rect.Width()
	size.cy = rect.bottom;	// or: rect.Height()

	// delete old view without deleting document
	BOOL bautodel		= pdbDoc->m_bAutoDelete;	// save doc flag telling that
	pdbDoc->m_bAutoDelete = FALSE;				// modify flag
	pCurrentView->DestroyWindow();				// delete existing view
	pdbDoc->m_bAutoDelete = bautodel;				// restore flag	

	// create new view
	CCreateContext context;						// prepare context structure for new view
	context.m_pNewViewClass = pViewClass;		// change view class
	context.m_pCurrentDoc = pdbDoc;				// assoc document
	context.m_pNewDocTemplate = NULL;			//
	context.m_pLastView = NULL;					//
	context.m_pCurrentFrame = NULL;				// this??

	// create view inside a splitter
	CView* pNewView = (CView*) CreateView(&context);
	pNewView->SendMessage(WM_INITIALUPDATE, 0, 0);

	// change menu
	if (m_pMenu == NULL)
		m_pMenu = new CMenu;
	GetMDIFrame()->MDISetMenu(m_pMenu->FromHandle(hmenu), NULL);
	GetMDIFrame()->DrawMenuBar();
	RecalcLayout();

	// adjust size of the view replacing the previous one
	pNewView->SendMessage(WM_SIZE, SIZE_MAXIMIZED, MAKELPARAM(size.cx, size.cy)); 	// MAKELPARAM = low, high
	SetActiveView(pNewView);
}


void CChildFrame::OnToolsRemoveMissingFiles()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
	ASSERT(pdbDoc);										// debug: check that doc is defined
	
	pdbDoc->RemoveMissingFiles();
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnToolsRemoveduplicatefiles()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
	ASSERT(pdbDoc);										// debug: check that doc is defined
	
	pdbDoc->RemoveDuplicateFiles();
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnToolsCheckFilelistsConsistency()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
	ASSERT(pdbDoc);										// debug: check that doc is defined
	
	pdbDoc->RemoveFalseSpkFiles();
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnToolsRestoredeletedfiles()
{
	// scan directories and rename files *.datdel into *.dat and *.spkdel into *.spk
	CStringArray filenames;			// store file names in this array
	int iResult;					// local variable
	CFindFilesDlg dlg;
	dlg.m_pfilenames = &filenames;
	dlg.m_selinit = 0;
	dlg.m_ioption=1;
	dlg.m_pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	iResult = dlg.DoModal();		

	if (IDOK == iResult)
	{
		CProgressDlg dlg;
		dlg.Create();
		dlg.SetStep (1);
		int istep = 0;
		CString cscomment;

		int nfiles = filenames.GetSize();
		for (int i=0; i<nfiles; i++)
		{
			CString csoldname = filenames[i];
			CString csnewname = csoldname.Left(csoldname.GetLength()-3);
			cscomment.Format(_T("Rename file [%i / %i] "), i + 1, nfiles);
			cscomment += csoldname;
			dlg.SetStatus(cscomment);
			if (MulDiv(i, 100, nfiles) > istep)
			{
				dlg.StepIt();
				istep = MulDiv(i, 100, nfiles);
			}
			CFile::Rename(csoldname, csnewname);
		}
	}
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	ASSERT(pdbDoc != NULL);		
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnToolsSynchronizesourceinformationsCurrentfile()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	ASSERT(pdbDoc);		
	pdbDoc->SynchronizeSourceInfos(FALSE);
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnToolsSynchronizesourceinformationsAllfiles()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	ASSERT(pdbDoc);		
	pdbDoc->SynchronizeSourceInfos(TRUE);
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnToolsRemoveartefactfiles()
{
	CRejectRecordDlg dlg1;
	long Nconsecutivepoints = 1000;
	short jitter = 4;
	int flagRejectedFileAs = 1;

	dlg1.m_bconsecutivepoints = TRUE;
	dlg1.m_Nconsecutivepoints = Nconsecutivepoints;
	dlg1.m_jitter = jitter;
	if (IDOK == dlg1.DoModal())
	{
		Nconsecutivepoints = dlg1.m_Nconsecutivepoints;
		jitter = (short) dlg1.m_jitter;
		flagRejectedFileAs = dlg1.m_flag;
	}
	else
		return;

	// search 
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	
	CProgressDlg dlg;
	dlg.Create();
	dlg.SetStep (1);
	int istep = 0;
	CString cscomment;
	int nfiles = pdbDoc->DBGetNRecords();

	for (int ifile = 0; ifile < nfiles; ifile ++)
	{
		// check if user wants to stop
		if(dlg.CheckCancelButton())
			if(AfxMessageBox(_T("Are you sure you want to Cancel?"),MB_YESNO)==IDYES)
				break;

		cscomment.Format(_T("Processing file [%i / %i] "), ifile + 1, nfiles);
		cscomment += pdbDoc->DBGetCurrentDatFileName();
		dlg.SetStatus(cscomment);

		// load file
		pdbDoc->DBSetCurrentRecordPosition(ifile);
		BOOL bOK = pdbDoc->OpenCurrentDataFile();
		if (!bOK)
			continue;

		CAcqDataDoc* pDat = pdbDoc->m_pDat;
		if (pDat == NULL)
			continue;

		int nconsecutivepoints = 0;
		int nchans;								// number of data chans / source buffer
		short* pBuf = pDat->LoadRawDataParams(&nchans);
		int offset = nchans;					// offset between points / detection
		int nspan = pDat->GetTransfDataSpan(0);	// nb pts to read before transf
		long lDataFirst = 0;
		long lDataLast = pDat->GetDOCchanLength() -1;

		while (lDataFirst < lDataLast)
		{
			// read a chunk of data 
			long lRWFirst=lDataFirst;
			long lRWLast =lDataLast;
			if (!pDat->LoadRawData(&lRWFirst, &lRWLast, 0))
				break;								// exit if error reported
			if (lRWLast > lDataLast)
				lRWLast = lDataLast;
			short* pData0 = pDat->LoadTransfData(lDataFirst, lRWLast, 0, 0);

			// compute initial offset (address of first point)
			// assume that detection is on channel 1
			short* pData = pData0;
			short lastvalue = 0;
			for (long cx = lDataFirst; cx <= lRWLast; cx++)
			{
				if (abs(lastvalue - *pData) <= jitter)
				{
					nconsecutivepoints++;
					if (nconsecutivepoints > Nconsecutivepoints)
						break;
				}
				else
					nconsecutivepoints = 0;
				lastvalue = *pData;
				pData++;
			}
			lDataFirst = lRWLast+1;
		}
		// change flag if condition met
		if (nconsecutivepoints >= Nconsecutivepoints)
			pdbDoc->DBSetCurrentRecordFlag(flagRejectedFileAs);

		// update interface
		if (MulDiv(ifile, 100, nfiles) > istep)
		{
			dlg.StepIt();
			istep = MulDiv(ifile, 100, nfiles);
		}
	}
	// exit: update all views
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnRecordGotorecord()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	
	CGotoRecordDlg dlg;
	dlg.m_recordPos = pdbDoc->DBGetCurrentRecordPosition();
	dlg.m_recordID = pdbDoc->DBGetCurrentRecordID();
	dlg.m_bGotoRecordID = ((CdbWaveApp*) AfxGetApp())->vdP.bGotoRecordID;
	
	if (IDOK == dlg.DoModal())
	{
		((CdbWaveApp*) AfxGetApp())->vdP.bGotoRecordID = dlg.m_bGotoRecordID;		
		if (!dlg.m_bGotoRecordID)
			pdbDoc->DBSetCurrentRecordPosition(dlg.m_recordPos);
		else
			pdbDoc->DBMoveToID(dlg.m_recordID);
		pdbDoc->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);
	}
}


void CChildFrame::OnToolsImportfiles(int ifilter)
{
	CStringArray filenames;			// store file names in this array
	int iResult;					// local variable
	CFindFilesDlg dlg;
	dlg.m_pfilenames = &filenames;
	dlg.m_selinit = ifilter;
	dlg.m_pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	iResult = dlg.DoModal();
	BOOL bOnlyGenuine = !dlg.m_banyformat;
	if (IDOK == iResult)
	{	
		CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
		pdbDoc->ImportDescFromFileList(filenames, bOnlyGenuine);
		pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);	
		// display files which were discarded in a separate document
		PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, (LPARAM) NULL);
	}
}


void CChildFrame::OnToolsImportATFfiles()
{
	CFindFilesDlg dlg;
	CStringArray filenames;			// store file names in this array
	dlg.m_pfilenames = &filenames;
	dlg.m_selinit = 6;
	dlg.m_pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	int iResult = dlg.DoModal();
	if (IDOK == iResult)
	{
		CImportFilesDlg dlg2;
		CStringArray convertedFiles;
		dlg2.m_pconvertedFiles = &convertedFiles;
		dlg2.m_pfilenameArray = &filenames;
		dlg2.m_option = ATFFILE;
		CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
		dlg2.m_pdbDoc = pdbDoc;
		
		iResult = dlg2.DoModal();
		if (IDOK == iResult)
			{
				CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
				pdbDoc->ImportDescFromFileList(convertedFiles);
				pdbDoc->DBMoveLast();
				pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);	
				// display files which were discarded in a separate document
				PostMessage(WM_MYMESSAGE, HINT_SHAREDMEMFILLED, (LPARAM) NULL);
			}
	}
}


void CChildFrame::OnRecordDeletecurrent()
{
	CView* pView = GetActiveView();
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	
	// save index current file
	long currentindex = pdbDoc->DBGetCurrentRecordPosition();
	if (currentindex <0)
		currentindex = 0;

	BOOL bDelete = TRUE;	// m_bKeepChoice = TRUE: delete files without showing the dialog box
	if(!m_bKeepChoice)
	{
		bDelete = FALSE;	// m_bKeepChoice = FALSE: display dialog box
		CDeleteRecordOptionsDlg dlg;
		dlg.m_bDeleteFile = m_bDeleteFile;
		dlg.m_bKeepChoice = m_bKeepChoice;
		
		if (IDOK == dlg.DoModal())
		{
			m_bDeleteFile = dlg.m_bDeleteFile;
			m_bKeepChoice = dlg.m_bKeepChoice;
			bDelete = TRUE;
		}
	}

	if (bDelete)
	{
		// delete records from the database and collect names of files to change
		// save list of data files to delete into a temporary array
		if (pView->IsKindOf(RUNTIME_CLASS(CdbWaveView)))
			((CdbWaveView*)pView)->DeleteRecords();
		else
			pdbDoc->DBDeleteCurrentRecord();

		// update views and rename "erased" files
		pdbDoc->UpdateAllViews(NULL, HINT_DOCHASCHANGED, NULL);
		pdbDoc->DBSetCurrentRecordPosition(currentindex);
		pdbDoc->UpdateAllViews(NULL, HINT_DOCMOVERECORD, NULL);

		// delete erased files
		if (m_bDeleteFile)
			pdbDoc->DeleteErasedFiles();
	}
}


BOOL CChildFrame::ExportToExcel()
{
	//define a few constants
	COleVariant vOpt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant covTrue((short)TRUE);
	COleVariant covFalse((short)FALSE);
	COleVariant cov_xlDatabase((short)1);
	COleVariant covZero((short)0);

	//Start a new workbook in Excel using COleDispatchDriver (_Application) client side
	CApplication oApp;
	oApp.CreateDispatch(_T("Excel.Application"));
	if (!oApp)
		return FALSE;

	// 1 - create workbook and paste data
	CWorkbooks oBooks = oApp.get_Workbooks();
	CWorkbook oBook = oBooks.Add(vOpt);
	CWorksheets oSheets = oBook.get_Worksheets();
	CWorksheet odataSheet = oSheets.get_Item(COleVariant((short) 1));		// select first sheet
	CRange oRange;
	oRange = odataSheet.get_Range(COleVariant(_T("A1")), vOpt);					// select first cell

	//odataSheet.Paste(vOpt, vOpt);											// paste data
	odataSheet._PasteSpecial(COleVariant(_T("Text")), vOpt, vOpt, vOpt, vOpt, vOpt);

	odataSheet.put_Name(_T("data"));											// change name of sheet
	
	//Make Excel visible and give the user control
	oApp.put_Visible(TRUE);
	oApp.put_UserControl(TRUE);
	oApp.DetachDispatch();
	oApp.ReleaseDispatch();
	return TRUE;
}


BOOL CChildFrame::ExportToExcelAndBuildPivot(int option)
{
	//define a few constants
	COleVariant vOpt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant covTrue((short)TRUE);
	COleVariant covFalse((short)FALSE);
	COleVariant cov_xlDatabase((short)1);
	COleVariant covZero((short)0);

	//Start a new workbook in Excel using COleDispatchDriver (_Application) client side
	CApplication oApp;
	oApp.CreateDispatch(_T("Excel.Application"));
	if (!oApp)
		return FALSE;

	// 1 - create workbook and paste data
	CWorkbooks oBooks = oApp.get_Workbooks();
	CWorkbook oBook = oBooks.Add(vOpt);
	CWorksheets oSheets = oBook.get_Worksheets();
	CWorksheet odataSheet = oSheets.get_Item(COleVariant((short) 1));		// select first sheet
	CRange oRange;
	oRange = odataSheet.get_Range(COleVariant(_T("A1")), vOpt);					// select first cell
	//odataSheet.Paste(vOpt, vOpt);											// paste data - does not work under windows 8.1 on the HP laptop
	odataSheet._PasteSpecial(COleVariant(_T("Text")), vOpt, vOpt, vOpt, vOpt, vOpt);
	odataSheet.put_Name(_T("data"));											// change name of sheet
	if (option == 1)
	{
		// select data area to create pivot
		long col2, row2, col1, row1;

		oRange = odataSheet.get_Range(COleVariant(_T("A5")), vOpt);			// select first cell of the table
		col1 = oRange.get_Column();
		row1 = oRange.get_Row();
		oRange.Select();

		CRange oSelect;
		oSelect = oRange.get_End(-4121);									//xlDown); XlDirection.xlDown
		row2 = oSelect.get_Row();
		oSelect = oRange.get_End(-4161);									//xlToRight); XlDirection.xlToRight
		col2 = oSelect.get_Column();
		
		oRange = oRange.get_Resize(COleVariant(row2-row1+1), COleVariant(col2-col1+1));
		oRange.Select();												// select data area

		// build range address as text (I was unable to pass the range into PivotTableWizard)
		CString cs2;
		const int ialphabet = 26;
		if (col2 > ialphabet)
		{
			int decimalcol = col2/ialphabet;
			int unicol = col2 - (decimalcol*ialphabet);
			cs2.Format(_T("%c%c%d"),_T('A')+(decimalcol -1) % ialphabet, _T('A') + (unicol - 1) %ialphabet, row2);
		}
		else
			cs2.Format(_T("%c%d"), 'A' + (col2-1)%ialphabet, row2);
		CString cs1;
		cs1.Format(_T("%c%d"), 'A' + (col1-1)%ialphabet, row1);
		cs1 = cs1+_T(":")+cs2;
		cs2 = odataSheet.get_Name();
		cs1 = cs2 + _T("!") + cs1;

		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		if (pApp->vdS.bexportPivot) 
		{
			// variables for creating pivottables
			CString csBin;
			int ibin = 0;

			//-------------------------------------------------------
			// create pivot tables
			BuildPivot(&oApp, &odataSheet, cs1, _T("pivot_cnt"), (short)-4112, col2);
			BuildPivot(&oApp, &odataSheet, cs1, _T("pivot_std"), (short)-4156, col2);
			BuildPivot(&oApp, &odataSheet, cs1, _T("pivot_avg"), (short)-4106, col2);
		}
	}

	//Make Excel visible and give the user control
	oApp.put_Visible(TRUE);
	oApp.put_UserControl(TRUE);
	oApp.DetachDispatch();
	oApp.ReleaseDispatch();
	return TRUE;
}


void CChildFrame::BuildPivot(void* poApp, void* podataSheet, CString csSourceDataAddress, CString csNameSheet, short XlConsolidationFunction, int col2)
{
	COleVariant cov_xlDatabase((short)1);
	COleVariant vOpt(DISP_E_PARAMNOTFOUND, VT_ERROR);
	COleVariant covTrue((short)TRUE);
	COleVariant covFalse((short)FALSE);
	CApplication* oApp = (CApplication*) poApp;
	CWorksheet*  odataSheet = (CWorksheet*) podataSheet;

	odataSheet->Activate();
	CPivotTable oPivot1 = odataSheet->PivotTableWizard(
			cov_xlDatabase,						//const VARIANT& SourceType,
			COleVariant(csSourceDataAddress),	//const VARIANT& SourceData,
			vOpt,								//const VARIANT& TableDestination,
			vOpt,								//const VARIANT& TableName,
			covFalse,							//const VARIANT& RowGrand, 
			covFalse,							//const VARIANT& ColumnGrand, 
			vOpt,								//const VARIANT& SaveData, 
			vOpt,								//const VARIANT& HasAutoFormat, 
			vOpt,								//const VARIANT& AutoPage, 
			vOpt,								//const VARIANT& Reserved, 
			vOpt,								//const VARIANT& BackgroundQuery, 
			vOpt,								//const VARIANT& OptimizeCache, 
			vOpt,								//const VARIANT& PageFieldOrder, 
			vOpt,								//const VARIANT& PageFieldWrapCount, 
			vOpt,								//const VARIANT& ReadData, 
			vOpt);								//const VARIANT& Connection)

		CWorksheet pivotSheet1 = oApp->get_ActiveSheet();
		pivotSheet1.put_Name(csNameSheet);

		// get options
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		OPTIONS_VIEWSPIKES* pvdS = &(pApp->vdS);

		// add fields to pivottable
		if (pvdS->bacqcomments)
		{
			oPivot1.AddFields(COleVariant(_T("type")), vOpt, vOpt, covFalse);
			oPivot1.AddFields(COleVariant(_T("stim1")), vOpt, vOpt, covTrue);
			oPivot1.AddFields(COleVariant(_T("conc1")), vOpt, vOpt, covTrue);
			for (int i=1; i <= 3; i++)
			{
				CPivotField oField = oPivot1.get_RowFields(COleVariant((short)i));
				oField.put_Subtotals(COleVariant((short)1), covFalse);
			}
		}

		// loop over the bins
		CString csBin;
		int ibin = 0;
		int col1 = pvdS->ncommentcolumns +1;
		for (int i=col1; i <= col2; i++, ibin++)
		{
			csBin.Format(_T("bin_%i"), ibin);
			oPivot1.AddDataField(
				oPivot1.PivotFields(COleVariant((short)i)), 
				COleVariant(csBin), 
				COleVariant(XlConsolidationFunction)); 
		}

		if (col1 < col2) // error fired if only 1 bin is measured
		{
			CPivotField oField = oPivot1.get_DataPivotField();
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
//		//CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetMDIActiveDocument();// get pointer to document
//		//dlg2.m_pdbDoc = pdbDoc;
//		//
//		//iResult = dlg2.DoModal();
//		//if (IDOK == iResult)
//		//	{
//		//		CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetMDIActiveDocument();// get pointer to document
//		//		pdbDoc->ImportDescFromFileList(convertedFiles);
//		//		pdbDoc->BuilInsectAndSensillumIDArrays();
//		//		pdbDoc->MoveRecord(ID_RECORD_LAST);
//		//		pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);	
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
		CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();	// get pointer to document
		pdbDoc->ImportDatabase(fileName);
		pdbDoc->DBMoveLast();
		pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);	
	}
}


void CChildFrame::OnToolsCopyAllProjectFiles()
{
	CString destinationPath = _T("c:\\Temp");
	CTransferFilesDlg dlg;
	dlg.m_csPathname = destinationPath;
	int iResult = dlg.DoModal();
	if (IDOK == iResult)
	{
		destinationPath = dlg.m_csPathname;
		CdbWaveDoc* pdbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
		pdbDoc->CopyAllFilesintoDirectory(destinationPath);
	}
}


void CChildFrame::OnToolsExportdatafile()
{
	CExportDataDlg dlg;
	dlg.m_dbDoc = (CdbWaveDoc*) GetActiveDocument();// get pointer to document
	dlg.DoModal();
}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
	CMainFrame* pmF = (CMainFrame*)AfxGetMainWnd();
	if (bActivate)
		pmF->PostMessage(WM_MYMESSAGE, HINT_MDIACTIVATE, NULL);

}


void CChildFrame::OnToolsPathsRelative()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	pdbDoc->DBSetPathsRelative_to_DataBaseFile();
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}


void CChildFrame::OnToolsPathsAbsolute()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	pdbDoc->DBSetPathsAbsolute();
	pdbDoc->UpdateAllViews(NULL, HINT_REQUERY, NULL);
}



void CChildFrame::OnToolsPath()
{
	// TODO: Add your command handler code here
}



void CChildFrame::OnToolsRemoveunused()
{
	CdbWaveDoc* pdbDoc = (CdbWaveDoc*)GetActiveDocument();
	pdbDoc->DBDeleteUnusedEntries();
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
	CString fileName;
	wchar_t* p = fileName.GetBuffer(FILE_LIST_BUFFER_SIZE);
	CFileDialog dlg(TRUE);

	OPENFILENAME& ofn = dlg.GetOFN();
	ofn.Flags |= OFN_ALLOWMULTISELECT;
	ofn.lpstrFile = p;
	ofn.nMaxFile = FILE_LIST_BUFFER_SIZE;
	ofn.lpstrFilter = _T("MDB Files (*.mdb)\0*.mdb\0\0");
	ofn.lpstrTitle = _T("Select MDB File");
	INT_PTR result = dlg.DoModal();
	fileName.ReleaseBuffer();

	if (result == IDOK)
	{
		fileName = dlg.GetPathName(); // return full path and filename
		int ipos = fileName.ReverseFind('.');
		CString fileName_new = fileName.Left(ipos) + _T("_new.mdb");

		// compact database and save new file
		CDaoWorkspace::CompactDatabase(fileName, fileName_new, dbLangGeneral, 0);

		CString cs;
		cs = fileName + _T(" database compacted and saved as ") + fileName_new;
		AfxMessageBox(cs);
	}
}