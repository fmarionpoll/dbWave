// NoteDoc.cpp : implementation of the CNoteDoc class
//

#include "stdafx.h"


#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "NotedocCntrItem.h"
#include "NoteDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CNoteDoc

IMPLEMENT_DYNCREATE(CNoteDoc, CRichEditDoc)

BEGIN_MESSAGE_MAP(CNoteDoc, CRichEditDoc)
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, CRichEditDoc::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, CRichEditDoc::OnEditLinks)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, CRichEditDoc::OnUpdateObjectVerbMenu)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNoteDoc construction/destruction

CNoteDoc::CNoteDoc()
{
	// add one-time construction code here
	m_bRTF=FALSE;
}

CNoteDoc::~CNoteDoc()
{
}

BOOL CNoteDoc::OnNewDocument()
{
	if (!CRichEditDoc::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

CRichEditCntrItem* CNoteDoc::CreateClientItem(REOBJECT* preo) const
{
	// cast away constness of this
	return new CNotedocCntrItem(preo, (CNoteDoc*) this);
}



/////////////////////////////////////////////////////////////////////////////
// CNoteDoc serialization

void CNoteDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}

	// Calling the base class CRichEditDoc enables serialization
	//  of the container document's COleClientItem objects.
	// TODO: set CRichEditDoc::m_bRTF = FALSE if you are serializing as text
	m_bRTF = FALSE;
	CRichEditDoc::Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CNoteDoc diagnostics

#ifdef _DEBUG
void CNoteDoc::AssertValid() const
{
	CRichEditDoc::AssertValid();
}

void CNoteDoc::Dump(CDumpContext& dc) const
{
	CRichEditDoc::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNoteDoc commands

BOOL CNoteDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CRichEditDoc::OnOpenDocument(lpszPathName))
		return FALSE;

	// load project file and corresponding data
	CString cspathname = lpszPathName;
	cspathname.MakeLower();
	if (cspathname.Find(_T(".prj")) >0)
		OpenProjectFiles(cspathname);

	m_bRTF = FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// open data files corresponding to a list of files stored into a text window
// use of OFN_EXPLORER flag allows the use of standard routines
// current version (20-3-96) approach the pb by extracting names within CNoteDoc
// it assumes that CR LF TAB ";" are separators and that files have the same ext.

#include "ImportOptions.h"
BOOL CNoteDoc::OpenProjectFiles(CString& cspathname)
{
	CWaitCursor wait;
	BOOL flag = FALSE;					// pessimistic flag
	CString fileList;					// load file list within this string
	CRichEditCtrl& pEdit = ((CRichEditView*)m_viewList.GetHead())->GetRichEditCtrl();
	pEdit.GetWindowText(fileList);		// copy content of window into CString
	if (fileList.IsEmpty())
		return flag;
	
	// assume that filenames are separated either by "RC", "TAB", ";" or "LF"	
	CString newList = fileList;
	TCHAR* pstring = fileList.GetBuffer(fileList.GetLength());
	TCHAR* pnew = newList.GetBuffer(newList.GetLength());	
	TCHAR seps[] = _T("\n\t;\r\f");
	TCHAR* token;
	TCHAR* next_token =NULL;
	CStringArray csArrayfiles;
	
	// loop through the string to extract data and build file names
	BOOL bchanged = FALSE;
	token = _tcstok_s(pstring, seps, &next_token);
	while (token != NULL)
	{
		CString filename = token;	// get filename into CString
		CFileStatus status;	
		// check if file is present 
		if(!CFile::GetStatus(filename, status ))
		{	// not found: replace the first letter with a question mark
			*(pnew + (token - pstring)) = '?';
			bchanged = TRUE;
		}

		// try to open the document if file was found
		else
		{
			filename.MakeLower();
			csArrayfiles.Add(filename);
		}
		// scan next line
		token = _tcstok_s(NULL, seps, &next_token);
	}
	if (bchanged)
	{
		pEdit.SetWindowText(newList);	// copy content to window
		AfxMessageBox(_T("Some files were not found\n\nThese are marked\nby a '?' in the project"));
	}
	fileList.ReleaseBuffer();
	newList.ReleaseBuffer();

	// make sure the correct import options are selected
	CImportOptionsDlg dlg;
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();	// load browse parameters
	dlg.m_bAllowDuplicateFiles = pApp->ivO.bImportDuplicateFiles;
	if (IDOK == dlg.DoModal())
	{
		pApp->ivO.bImportDuplicateFiles = dlg.m_bAllowDuplicateFiles;
	}

	// open data file
	if (csArrayfiles.GetSize() > 0)
	{
		CFrameWnd* pFrameWnd = (CFrameWnd*) ((CRichEditView*)m_viewList.GetHead())->GetParent();
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
		// create an empty document and then create a table with the same name as the project
		CdbWaveDoc* pdbDoc = (CdbWaveDoc*) (pApp->m_pdbWaveViewTemplate)->CreateNewDocument();
		if (pdbDoc != NULL) 
		{
			flag = TRUE;
			CString dbname;
			if (cspathname.IsEmpty())
			{
				cspathname = GetTitle();
				if (cspathname.IsEmpty())
					cspathname = "dummy.mdb";
			}
			
			int i1 = cspathname.ReverseFind('\\');
			int i2 = cspathname.ReverseFind('.');
			dbname = cspathname.Mid(i1+1, i2-i1 -1);

			if (pdbDoc->OnNewDocument(dbname))	// create table
			{
				pdbDoc->ImportDescFromFileList(csArrayfiles);
				CFrameWnd* pWF = (pApp->m_pdbWaveViewTemplate)->CreateNewFrame(pdbDoc, NULL);
				ASSERT(pWF != NULL);
				pApp->m_pdbWaveViewTemplate->InitialUpdateFrame(pWF, pdbDoc, TRUE);
			}
		}
	}
	return flag;
}
