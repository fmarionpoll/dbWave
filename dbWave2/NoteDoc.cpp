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
= default;

BOOL CNoteDoc::OnNewDocument()
{
	return CRichEditDoc::OnNewDocument();
}

CRichEditCntrItem* CNoteDoc::CreateClientItem(REOBJECT* p_re_object) const
{
	return new CNotedocCntrItem(p_re_object, const_cast<CNoteDoc*>(this));
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

BOOL CNoteDoc::OnOpenDocument( LPCTSTR lpsz_path_name)
{
	if (!CRichEditDoc::OnOpenDocument(lpsz_path_name))
		return FALSE;

	// load project file and corresponding data
	CString cs_path_name = lpsz_path_name;
	cs_path_name.MakeLower();
	if (cs_path_name.Find(_T(".prj")) > 0)
		open_project_files(cs_path_name);

	m_bRTF = FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// open data files corresponding to a list of files stored into a text window
// use of OFN_EXPLORER flag allows the use of standard routines
// current version (20-3-96) approach the pb by extracting names within CNoteDoc
// it assumes that CR LF TAB ";" are separators and that files have the same ext.

#include "dbWave.h"

BOOL CNoteDoc::open_project_files(CString& cs_path_name)
{
	auto flag = FALSE; // pessimistic flag
	CRichEditCtrl& p_edit = static_cast<CRichEditView*>(m_viewList.GetHead())->GetRichEditCtrl();

	// make sure the correct import options are selected
	DlgImportOptions dlg;
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	dlg.m_b_allow_duplicate_files = p_app->options_import.discard_duplicate_files;
	dlg.m_b_header = p_app->options_import.header_present;
	dlg.m_b_read_columns = p_app->options_import.read_columns;
	if (IDOK == dlg.DoModal())
	{
		p_app->options_import.discard_duplicate_files =  static_cast<boolean>(dlg.m_b_allow_duplicate_files);
		p_app->options_import.header_present = static_cast<boolean>(dlg.m_b_header);
		p_app->options_import.read_columns = static_cast<boolean>(dlg.m_b_read_columns);
	}

	// open data files
	CStringArray cs_file_list;
	CStringArray cs_descriptors_list;
	const int n_columns = extract_list(p_edit, cs_file_list, cs_descriptors_list);
	if (cs_file_list.GetCount() > 0)
		flag = open_file_list(cs_path_name, cs_file_list, cs_descriptors_list, n_columns);

	return flag;
}

BOOL CNoteDoc::open_file_list(CString& cs_path_name, CStringArray& cs_array_files, CStringArray& cs_descriptions_array,
                              const int n_columns) const
{
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	const auto p_dbwave_doc = static_cast<CdbWaveDoc*>((p_app->m_db_wave_view_template)->CreateNewDocument());
	BOOL flag = FALSE;
	if (p_dbwave_doc != nullptr)
	{
		flag = TRUE;
		if (cs_path_name.IsEmpty())
		{
			cs_path_name = GetTitle();
			if (cs_path_name.IsEmpty())
				cs_path_name = "dummy.mdb";
		}

		const auto i1 = cs_path_name.ReverseFind('\\');
		const auto i2 = cs_path_name.ReverseFind('.');
		const auto dbname = cs_path_name.Mid(i1 + 1, i2 - i1 - 1);

		if (p_dbwave_doc->open_new_document(dbname))
		{
			if (p_app->options_import.read_columns)
				p_dbwave_doc->import_file_list(cs_descriptions_array, n_columns, p_app->options_import.header_present);
			else
				p_dbwave_doc->import_file_list(cs_array_files);
			const auto p_wave_format = (p_app->m_db_wave_view_template)->CreateNewFrame(p_dbwave_doc, nullptr);
			ASSERT(p_wave_format != NULL);
			p_app->m_db_wave_view_template->InitialUpdateFrame(p_wave_format, p_dbwave_doc, TRUE);
		}
	}
	return flag;
}

int CNoteDoc::extract_list(CRichEditCtrl& p_edit, CStringArray& cs_array_file_names, CStringArray& cs_descriptors_array)
{
	CString text_from_rich_edit_view{};
	p_edit.GetWindowText(text_from_rich_edit_view);
	if (text_from_rich_edit_view.IsEmpty())
		return 0;

	boolean b_files_not_found = false;
	CStringArray cs_files_tested;
	int i_first = 0;
	if (static_cast<CdbWaveApp*>(AfxGetApp())->options_import.header_present)
		i_first = 1;

	int cur_pos = 0;
	int row_index = 0;
	int count_columns = 0;
	const auto separator = L"\r\n";
	CString cs_row = text_from_rich_edit_view.Tokenize(separator, cur_pos);
	while (!cs_row.IsEmpty())
	{
		row_index++;
		if (cs_row.GetLength() > 2)
		{
			CStringArray row_array;
			count_columns = parse_row2(cs_row, row_array);

			if (row_index > i_first)
			{
				if (add_file_name(row_array.GetAt(0), cs_array_file_names, cs_files_tested))
					add_row_to_array(row_array, cs_descriptors_array);
				else
					b_files_not_found = true;
			}
			else if (i_first == 1)
			{
				add_row_to_array(row_array, cs_descriptors_array);
			}
		}
		cs_row = text_from_rich_edit_view.Tokenize(separator, cur_pos);
	}

	if (b_files_not_found)
		display_files_imported(p_edit, cs_files_tested);
	return count_columns;
}

void CNoteDoc::add_row_to_array(const CStringArray& row_array, CStringArray& cs_descriptors_array)
{
	for (int i = 0; i < row_array.GetCount(); i++)
		cs_descriptors_array.Add(row_array.GetAt(i));
}

int CNoteDoc::parse_row2(const CString& cs_row, CStringArray& cs_columns)
{
	int cur_pos = 0;
	const CString tokens = _T("\t;,");
	CString res_token = cs_row.Tokenize(tokens, cur_pos);
	while (res_token != _T(""))
	{
		cs_columns.Add(res_token);
		res_token = cs_row.Tokenize(tokens, cur_pos);
	}
	return cs_columns.GetCount();
}

BOOL CNoteDoc::add_file_name(const CString& res_token, CStringArray& cs_array_ok, CStringArray& cs_array_tested)
{
	BOOL b_found = true;
	CString token = res_token;
	if (!is_file_present(res_token))
	{
		// not found: add a question mark
		token = _T("? ") + res_token;
		b_found = false;
	}
	else
	{
		token.MakeLower();
		cs_array_ok.Add(token);
	}
	cs_array_tested.Add(token);
	return b_found;
}

void CNoteDoc::display_files_imported(CRichEditCtrl& p_edit, const CStringArray& cs_descriptors_list)
{
	CString all;
	for (int i = 0; i < cs_descriptors_list.GetCount(); i++)
		all += cs_descriptors_list.GetAt(i) + _T("\n");
	p_edit.SetWindowText(all);
	AfxMessageBox(_T("Some files were not found\n\nThese are marked\nby a '?' in the project"));
}
