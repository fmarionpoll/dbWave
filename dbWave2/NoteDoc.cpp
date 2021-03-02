
#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "NotedocCntrItem.h"
#include "NoteDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/*
map<int, int, string> int2String{ 
	{1, 0, "Expt"},
	{2, 2, "insectID"},
	{3, 2,  "ssID"},
	{4, 0, "insect"},
	{5, 0, "strain"},
	{6, 0, "sex"},
	{7, 0, "location"},
	{8, 0, "operator"},
	{9, 0, "more"},
	{10, 0, "stim1"},
	{11, 0, "conc1"},
	{12, 2, "repeat1"},
	{13, 0, "stim2"},
	{14, 0, "conc2"},
	{15, 2, "repeat2"},
	{16, 0, "type"}
};
*/

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
	return CRichEditDoc::OnNewDocument();
}

CRichEditCntrItem* CNoteDoc::CreateClientItem(REOBJECT* preo) const
{
	return new CNotedocCntrItem(preo, const_cast<CNoteDoc*>(this));
}

void CNoteDoc::Serialize(CArchive& ar)
{
	m_bRTF = FALSE;
	CRichEditDoc::Serialize(ar);
}

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
	auto flag = FALSE;					// pessimistic flag
	CRichEditCtrl& pEdit = static_cast<CRichEditView*>(m_viewList.GetHead())->GetRichEditCtrl();

	// make sure the correct import options are selected
	CImportOptionsDlg dlg;
	auto p_app = (CdbWaveApp*)AfxGetApp();
	dlg.m_bAllowDuplicateFiles = p_app->options_import.bImportDuplicateFiles;
	dlg.m_bHeader = p_app->options_import.bHeader;
	dlg.m_bReadColumns = p_app->options_import.bReadColumns;
	if (IDOK == dlg.DoModal()) {
		p_app->options_import.bImportDuplicateFiles = dlg.m_bAllowDuplicateFiles;
		p_app->options_import.bHeader = dlg.m_bHeader;
		p_app->options_import.bReadColumns = dlg.m_bReadColumns;
	}

	// open data files
	if (p_app->options_import.bReadColumns)
	{
	}
	else
	{
		CStringArray* cs_arrayfiles = extractListOfFilesSimple(cspathname, pEdit);
		if (cs_arrayfiles != nullptr)
			flag = openListOfFilesSimple(cspathname, cs_arrayfiles);
		delete cs_arrayfiles;
	}
	return flag;
}

BOOL CNoteDoc::openListOfFilesSimple(CString& cspathname, CStringArray* cs_arrayfiles)
{
	auto p_app = (CdbWaveApp*)AfxGetApp();
	CdbWaveDoc* p_dbwave_doc = (CdbWaveDoc*)(p_app->m_pdbWaveViewTemplate)->CreateNewDocument();
	BOOL flag = FALSE;
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

		if (p_dbwave_doc->OnNewDocument(dbname))
		{
			p_dbwave_doc->ImportFileList(*cs_arrayfiles);
			auto p_wave_format = (p_app->m_pdbWaveViewTemplate)->CreateNewFrame(p_dbwave_doc, nullptr);
			ASSERT(p_wave_format != NULL);
			p_app->m_pdbWaveViewTemplate->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
		}
	}
	return flag;
}

CStringArray* CNoteDoc::extractListOfFilesSimple_old(CString& cspathname, CRichEditCtrl& pEdit)
{
	CString file_list{};
	pEdit.GetWindowText(file_list);
	if (file_list.IsEmpty())
		return nullptr;

	CStringArray* cs_arrayfiles = new CStringArray();
	CString new_list = file_list;
	const auto pstring = file_list.GetBuffer(file_list.GetLength());
	const auto pnew = new_list.GetBuffer(new_list.GetLength());
	TCHAR seps[] = _T("\n\t;\r\f");
	TCHAR* next_token = nullptr;

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
			cs_arrayfiles->Add(filename);
		}
		// scan next line
		token = _tcstok_s(nullptr, seps, &next_token);
	}

	if (bchanged)
	{
		CRichEditCtrl& pEdit = static_cast<CRichEditView*>(m_viewList.GetHead())->GetRichEditCtrl();
		pEdit.SetWindowText(new_list);
		AfxMessageBox(_T("Some files were not found\n\nThese are marked\nby a '?' in the project"));
	}
	file_list.ReleaseBuffer();
	new_list.ReleaseBuffer();
	return cs_arrayfiles;
}

CStringArray* CNoteDoc::extractListOfFilesSimple(CString& cspathname, CRichEditCtrl& pEdit)
{
	int nlines = pEdit.GetLineCount();
	CStringArray* pArrayOK = new CStringArray();
	CStringArray* pArrayTested = new CStringArray();

	boolean bchanged = false;
	int ifirst = 0;
	if (((CdbWaveApp*)AfxGetApp())->options_import.bHeader)
		ifirst = 1;
		
	for (int i = ifirst; i < nlines; i++)
	{
		//int nLineIndex = pEdit.LineIndex(i);
		//int lineLength = pEdit.LineLength(nLineIndex);

		int lineLength = pEdit.LineLength(i);
		CString csLine{};
		pEdit.GetLine(i, csLine.GetBuffer(lineLength +1), lineLength);
		csLine.ReleaseBuffer(lineLength +1);
		TCHAR seps[] = _T("\t");
		int curPos = 0;
		CString resToken = csLine.Tokenize(seps, curPos);
		while (!resToken.IsEmpty())
		{
			bchanged |= addFileName(resToken, pArrayOK, pArrayTested);
			resToken = csLine.Tokenize(seps, curPos);
		}
	}
	if (bchanged)
	{
		displayFilesImported(pEdit, pArrayTested);
	}

	return pArrayOK;
}

BOOL CNoteDoc::addFileName(CString resToken, CStringArray* pArrayOK, CStringArray* pArrayTested)
{
	BOOL bChanged = false;
	CFileStatus status;
	CString token = resToken;
	if (!CFile::GetStatus(resToken, status))
	{	// not found: add a question mark
		token = _T("? ") + resToken;
		bChanged = TRUE;
	}
	else
	{
		token.MakeLower();
		pArrayOK->Add(token);
	}
	pArrayTested->Add(token);
	return bChanged;
}

void CNoteDoc::displayFilesImported(CRichEditCtrl& pEdit, CStringArray* pArrayTested)
{
	CString all;
	for (int i = 0; i < pArrayTested->GetCount(); i++) 
		all += pArrayTested->GetAt(i) + _T("\n");
	pEdit.SetWindowText(all);
	AfxMessageBox(_T("Some files were not found\n\nThese are marked\nby a '?' in the project"));
}
