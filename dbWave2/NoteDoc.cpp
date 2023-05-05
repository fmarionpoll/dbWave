#include "StdAfx.h"
#include "dbWaveDoc.h"
#include "NotedocCntrItem.h"
#include "NoteDoc.h"

#include "DlgImportOptions.h"

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

#include "dbWave.h"

BOOL CNoteDoc::OpenProjectFiles(CString& cspathname)
{
	auto flag = FALSE; // pessimistic flag
	CRichEditCtrl& pEdit = static_cast<CRichEditView*>(m_viewList.GetHead())->GetRichEditCtrl();

	// make sure the correct import options are selected
	DlgImportOptions dlg;
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	dlg.m_bAllowDuplicateFiles = p_app->options_import.bDiscardDuplicateFiles;
	dlg.m_bHeader = p_app->options_import.bHeader;
	dlg.m_bReadColumns = p_app->options_import.bReadColumns;
	if (IDOK == dlg.DoModal())
	{
		p_app->options_import.bDiscardDuplicateFiles = dlg.m_bAllowDuplicateFiles;
		p_app->options_import.bHeader = dlg.m_bHeader;
		p_app->options_import.bReadColumns = dlg.m_bReadColumns;
	}

	// open data files
	CStringArray csFileList;
	CStringArray csDescriptorsList;
	int nColumns = extractList(pEdit, csFileList, csDescriptorsList);
	if (csFileList.GetCount() > 0)
		flag = openFileList(cspathname, csFileList, csDescriptorsList, nColumns);

	return flag;
}

BOOL CNoteDoc::openFileList(CString& cspathname, CStringArray& csFileList, CStringArray& csDescriptorsList,
                            int nColumns)
{
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	auto p_dbwave_doc = static_cast<CdbWaveDoc*>((p_app->m_dbWaveView_Template)->CreateNewDocument());
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
			if (p_app->options_import.bReadColumns)
			{
				int nColumns2 = csDescriptorsList.GetCount() / (csFileList.GetCount() + p_app->options_import.bHeader);
				p_dbwave_doc->Import_FileList(csDescriptorsList, nColumns, p_app->options_import.bHeader);
			}
			else
			{
				p_dbwave_doc->Import_FileList(csFileList);
			}

			auto p_wave_format = (p_app->m_dbWaveView_Template)->CreateNewFrame(p_dbwave_doc, nullptr);
			ASSERT(p_wave_format != NULL);
			p_app->m_dbWaveView_Template->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
		}
	}
	return flag;
}

int CNoteDoc::extractList(CRichEditCtrl& pEdit, CStringArray& csFileList, CStringArray& csDescriptorsList)
{
	CString textFromRichEditView{};
	pEdit.GetWindowText(textFromRichEditView);
	if (textFromRichEditView.IsEmpty())
		return 0;

	boolean bFilesNotFound = false;
	CStringArray csFilesTested;
	int ifirst = 0;
	if (static_cast<CdbWaveApp*>(AfxGetApp())->options_import.bHeader)
		ifirst = 1;

	int curPos = 0;
	int countRows = 0;
	int countColumns = 0;
	auto seps = L"\r\n";
	CString csRow = textFromRichEditView.Tokenize(seps, curPos);
	while (!csRow.IsEmpty())
	{
		countRows++;
		if (csRow.GetLength() > 2)
		{
			CStringArray csColumnsOneRow;
			countColumns = extractColumnsFromRow(csRow, csColumnsOneRow);
			if (countRows > ifirst)
			{
				CString name = csColumnsOneRow.GetAt(0);
				if (addFileName(name, csFileList, csFilesTested))
					addRowToArray(csColumnsOneRow, csDescriptorsList);
				else
					bFilesNotFound = true;
			}
			else if (ifirst == 1)
			{
				addRowToArray(csColumnsOneRow, csDescriptorsList);
			}
		}
		csRow = textFromRichEditView.Tokenize(seps, curPos);
	}

	if (bFilesNotFound)
		displayFilesImported(pEdit, csFilesTested);
	return countColumns;
}

void CNoteDoc::addRowToArray(CStringArray& csRow, CStringArray& csOut)
{
	for (int i = 0; i < csRow.GetCount(); i++)
		csOut.Add(csRow.GetAt(i));
}

int CNoteDoc::extractColumnsFromRow(CString& csRow, CStringArray& csColumns)
{
	auto seps = L"\t;,";
	int curPos = 0;
	int countColumns = 0;
	int newPos = 0;
	CString csExtract = csRow;
	while (newPos >= 0)
	{
		csExtract = csExtract.Right(csExtract.GetLength() - curPos);
		newPos = csExtract.FindOneOf(seps);
		int endPos = newPos;
		if (endPos < 0)
			endPos = csExtract.GetLength();
		CString resToken = csExtract.Left(endPos);
		csColumns.Add(resToken);
		countColumns++;
		curPos = newPos + 1;
	}
	return countColumns;
}

BOOL CNoteDoc::addFileName(CString& resToken, CStringArray& csFileList, CStringArray& csDescriptorsList)
{
	BOOL bFound = true;
	CString token = resToken;
	if (!isFilePresent(resToken))
	{
		// not found: add a question mark
		token = _T("? ") + resToken;
		bFound = false;
	}
	else
	{
		token.MakeLower();
		csFileList.Add(token);
	}
	csDescriptorsList.Add(token);
	return bFound;
}

void CNoteDoc::displayFilesImported(CRichEditCtrl& pEdit, CStringArray& csDescriptorsList)
{
	CString all;
	for (int i = 0; i < csDescriptorsList.GetCount(); i++)
		all += csDescriptorsList.GetAt(i) + _T("\n");
	pEdit.SetWindowText(all);
	AfxMessageBox(_T("Some files were not found\n\nThese are marked\nby a '?' in the project"));
}
