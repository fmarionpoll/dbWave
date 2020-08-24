// NoteDoc.cpp : implementation of the CNoteDoc class
//

#include "StdAfx.h"
//#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "NotedocCntrItem.h"
#include "NoteDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CNoteDoc, CRichEditDoc)

BEGIN_MESSAGE_MAP(CNoteDoc, CRichEditDoc)
	// Enable default OLE container implementation
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, CRichEditDoc::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, CRichEditDoc::OnEditLinks)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, CRichEditDoc::OnUpdateObjectVerbMenu)
END_MESSAGE_MAP()

CNoteDoc::CNoteDoc()
{
	// add one-time construction code here
	m_bRTF = FALSE;
}

CNoteDoc::~CNoteDoc()
{
}

BOOL CNoteDoc::OnNewDocument()
{
	if (!CRichEditDoc::OnNewDocument())
		return FALSE;

	return TRUE;
}

CRichEditCntrItem* CNoteDoc::CreateClientItem(REOBJECT* preo) const
{
	return new CNotedocCntrItem(preo, const_cast<CNoteDoc*>(this));
}

/////////////////////////////////////////////////////////////////////////////
// CNoteDoc serialization

void CNoteDoc::Serialize(CArchive& ar)
{
	//if (ar.IsStoring())
	//{
	//	// TODO: add storing code here
	//}
	//else
	//{
	//	// TODO: add loading code here
	//}

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
	if (cspathname.Find(_T(".prj")) > 0)
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
	auto flag = FALSE;					// pessimistic flag
	CString file_list;					// load file list within this string
	auto& pEdit = static_cast<CRichEditView*>(m_viewList.GetHead())->GetRichEditCtrl();
	pEdit.GetWindowText(file_list);		// copy content of window into CString
	if (file_list.IsEmpty())
		return flag;

	// assume that filenames are separated either by "rc", "TAB", ";" or "LF"
	auto new_list = file_list;
	const auto pstring = file_list.GetBuffer(file_list.GetLength());
	const auto pnew = new_list.GetBuffer(new_list.GetLength());
	TCHAR seps[] = _T("\n\t;\r\f");
	TCHAR* next_token = nullptr;
	CStringArray cs_arrayfiles;

	// loop through the string to extract data and build file names
	auto bchanged = FALSE;
	auto token = _tcstok_s(pstring, seps, &next_token);
	while (token != nullptr)
	{
		CString filename = token;	// get filename into CString
		CFileStatus status;
		if (!CFile::GetStatus(filename, status))
		{	// not found: replace the first letter with a question mark
			*(pnew + (token - pstring)) = '?';
			bchanged = TRUE;
		}
		else
		{
			filename.MakeLower();
			cs_arrayfiles.Add(filename);
		}
		// scan next line
		token = _tcstok_s(nullptr, seps, &next_token);
	}
	if (bchanged)
	{
		pEdit.SetWindowText(new_list);	// copy content to window
		AfxMessageBox(_T("Some files were not found\n\nThese are marked\nby a '?' in the project"));
	}
	file_list.ReleaseBuffer();
	new_list.ReleaseBuffer();

	// make sure the correct import options are selected
	CImportOptionsDlg dlg;
	auto p_app = (CdbWaveApp*)AfxGetApp();	// load browse parameters
	dlg.m_bAllowDuplicateFiles = p_app->options_import.bImportDuplicateFiles;
	if (IDOK == dlg.DoModal())
	{
		p_app->options_import.bImportDuplicateFiles = dlg.m_bAllowDuplicateFiles;
	}

	// open data file
	if (cs_arrayfiles.GetSize() > 0)
	{
		//auto p_frame_wnd = (CFrameWnd*) ((CRichEditView*)m_viewList.GetHead())->GetParent();
		//auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		// create an empty document and then create a table with the same name as the project
		auto* p_dbwave_doc = (CdbWaveDoc*)(p_app->m_pdbWaveViewTemplate)->CreateNewDocument();
		if (p_dbwave_doc != nullptr)
		{
			flag = TRUE;
			if (cspathname.IsEmpty())
			{
				cspathname = GetTitle();
				if (cspathname.IsEmpty())
					cspathname = "dummy.mdb";
			}

			const auto i1 = cspathname.ReverseFind('\\');
			const auto i2 = cspathname.ReverseFind('.');
			const auto dbname = cspathname.Mid(i1 + 1, i2 - i1 - 1);

			if (p_dbwave_doc->OnNewDocument(dbname))	// create table
			{
				p_dbwave_doc->ImportDescFromFileList(cs_arrayfiles);
				auto p_wave_format = (p_app->m_pdbWaveViewTemplate)->CreateNewFrame(p_dbwave_doc, nullptr);
				ASSERT(p_wave_format != NULL);
				p_app->m_pdbWaveViewTemplate->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
			}
		}
	}
	return flag;
}