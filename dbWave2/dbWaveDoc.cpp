#include "StdAfx.h"
#include "dbWave.h"

#include "dbTableMain.h"
#include "Spikedoc.h"
#include "dbWaveDoc.h"	
#include "DlgdbNewFileDuplicate.h"
#include "DlgProgress.h"
#include "NoteDoc.h"
#include "ViewNotedoc.h"
#include "dbWave_constants.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

number_id_to_text CdbWaveDoc::headers_[] = {
	{0, _T("path")}, {1, _T("Expt")}, {2, _T("insectID")}, {3, _T("ssID")}, {4, _T("insect")},
	{5, _T("strain")}, {6, _T("sex")}, {7, _T("location")}, {8, _T("operator")}, {9, _T("more")},
	{10, _T("stim1")}, {11, _T("conc1")}, {12, _T("repeat1")}, {13, _T("stim2")}, {14, _T("conc2")},
	{15, _T("repeat2")},
	{16, _T("type")}, {17, _T("sensillum")},
	{18, _T("flag")}
};

IMPLEMENT_DYNCREATE(CdbWaveDoc, COleDocument)

BEGIN_MESSAGE_MAP(CdbWaveDoc, COleDocument)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, COleDocument::OnUpdatePasteMenu)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE_LINK, COleDocument::OnUpdatePasteLinkMenu)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_CONVERT, COleDocument::OnUpdateObjectVerbMenu)
	ON_COMMAND(ID_OLE_EDIT_CONVERT, COleDocument::OnEditConvert)
	ON_UPDATE_COMMAND_UI(ID_OLE_EDIT_LINKS, COleDocument::OnUpdateEditLinksMenu)
	ON_COMMAND(ID_OLE_EDIT_LINKS, COleDocument::OnEditLinks)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OLE_VERB_FIRST, ID_OLE_VERB_LAST, COleDocument::OnUpdateObjectVerbMenu)
END_MESSAGE_MAP()

CdbWaveDoc::CdbWaveDoc()
= default;

CdbWaveDoc::~CdbWaveDoc()
{
	SAFE_DELETE(db_table)
		SAFE_DELETE(m_p_data_doc)
		SAFE_DELETE(m_p_spk_doc)

		// store temp mdb filename to delete on exit within the application
		// (it seems it is not possible to erase the file from here)
		if (clean_database_on_exit_)
		{
			const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
			p_app->m_temporary_mdb_files.Add(db_filename);
		}
}

void CdbWaveDoc::update_all_views_db_wave(CView* p_sender, LPARAM l_hint, CObject* p_hint)
{
	CDocument::UpdateAllViews(p_sender, l_hint, p_hint);
	// passes message OnUpdate() to the mainframe and add a reference to the document that sends it
	const auto main_frame = static_cast<CMainFrame*>(AfxGetMainWnd());
	main_frame->OnUpdate(reinterpret_cast<CView*>(this), l_hint, p_hint);
}

// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

BOOL CdbWaveDoc::OnNewDocument()
{
	if (!COleDocument::OnNewDocument())
		return FALSE;

	b_call_new_ = FALSE;
	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	auto cs_path = p_app->get_my_documents_my_dbwave_path();
	TCHAR sz_entry[MAX_PATH];
	unsigned long len = MAX_PATH;
	GetUserName(&sz_entry[0], &len);
	CString cs_ext = sz_entry;
	cs_ext += _T("_1");
	cs_path += _T('\\') + cs_ext;
	return open_new_document(cs_path);
}

// TODO here: ask where data are to be saved (call make directory/explore directory)
// ask for name of a database, then create a directory of the same name where the database will be put

BOOL CdbWaveDoc::open_new_document(LPCTSTR lpsz_path_name)
{
	if (b_call_new_ && !COleDocument::OnNewDocument())
		return FALSE;
	b_call_new_ = TRUE;

	CString cs_name = lpsz_path_name;
	cs_name += _T(".mdb");
	if (cs_name.Find(_T(':')) < 0
		&& cs_name.Find(_T("\\\\")) < 0)
	{
		const auto cs_path = static_cast<CdbWaveApp*>(AfxGetApp())->get_my_documents_my_dbwave_path();
		cs_name = cs_path + _T('\\') + cs_name;
	}

	// check if this file is already present, exit if not...
	CFileStatus status;
	auto b_exist = CFile::GetStatus(cs_name, status);

	while (b_exist)
	{
		DlgdbNewFileDuplicate dlg;
		dlg.m_p_file_in = cs_name;
		if (IDOK == dlg.DoModal())
		{
			switch (dlg.m_option)
			{
			case 0: // open file
				db_table = new CdbTable;
				db_table->attach(current_data_file_name_, current_spike_file_name_);
				return open_database(cs_name);
				//break;
			case 1: // overwrite file
				CFile::Remove(cs_name);
				b_exist = FALSE;
				break;
			case 2: // try another name
				cs_name = dlg.m_file_out;
				b_exist = CFile::GetStatus(cs_name, status);
				break;
			case 3: // abort process
			default:
				valid_tables = FALSE;
				return FALSE;
				//break;
			}
		}
		else
		{
			valid_tables = FALSE;
			return FALSE;
		}
	}

	// create dbFile
	db_table = new CdbTable;
	db_table->attach(current_data_file_name_, current_spike_file_name_);
	db_table->Create(cs_name, dbLangGeneral, dbVersion30);
	db_filename = cs_name;
	proposed_data_path_name = cs_name.Left(cs_name.ReverseFind('.'));
	db_table->create_all_tables();

	// save file name
	valid_tables = db_table->open_tables();
	SetTitle(cs_name);
	SetPathName(cs_name, TRUE);

	return valid_tables;
}

void CdbWaveDoc::Serialize(CArchive & ar)
{
	//if (ar.IsStoring())
	//{
	//	// TODO: add storing code here
	//}
	//else
	//{
	//	// TODO: add loading code here
	//}
}

#ifdef _DEBUG
void CdbWaveDoc::AssertValid() const
{
	COleDocument::AssertValid();
}

void CdbWaveDoc::Dump(CDumpContext & dc) const
{
	COleDocument::Dump(dc);
}
#endif

/*
 * Knowledge Base articles Q108587 "HOWTO: Get Current CDocument or CView from Anywhere" and
 * Q111814 "HOWTO: Get the Current Document in an MDI Application".
 */
CdbWaveDoc* CdbWaveDoc::get_active_mdi_document()
{
	CMDIChildWnd* pChild = static_cast<CMDIFrameWnd*>(AfxGetApp()->m_pMainWnd)->MDIGetActive();

	if (!pChild)
		return nullptr;

	CDocument* p_doc = pChild->GetActiveDocument();
	if (!p_doc)
		return nullptr;

	// Fail if doc is of wrong kind
	if (!p_doc->IsKindOf(RUNTIME_CLASS(CdbWaveDoc)))
		return nullptr;

	return static_cast<CdbWaveDoc*>(p_doc);
}

BOOL CdbWaveDoc::OnOpenDocument(LPCTSTR lpsz_path_name)
{
	// open database document
	CString cs_new = lpsz_path_name;
	CFileStatus status;
	if (!CFile::GetStatus(lpsz_path_name, status))
		return FALSE;

	cs_new.MakeLower();
	CString cs_ext;
	cs_ext.Empty();
	const auto i_ext = cs_new.ReverseFind('.');
	if (i_ext > 0)
		cs_ext = cs_new.Right(cs_new.GetLength() - i_ext - 1);

	if ((cs_ext.Compare(_T("mdb")) == 0) || (cs_ext.Compare(_T("accdb")) == 0))
	{
		if (!COleDocument::OnOpenDocument(lpsz_path_name))
			return FALSE;
		return open_database(lpsz_path_name);
	}

	// open spike or dat documents
	if ((cs_ext.Compare(_T("dat")) == 0) || (cs_ext.Compare(_T("spk")) == 0)
		|| (cs_ext.Compare(_T("asd")) == 0) || (cs_ext.Compare(_T("smr")) == 0)
		|| (cs_ext.Compare(_T("smrx")) == 0))
	{
		if (i_ext > 0)
			cs_new = cs_new.Left(i_ext);
		const auto flag = open_new_document(cs_new);
		if (flag)
		{
			CStringArray file_names;
			file_names.Add(lpsz_path_name);
			import_file_list(file_names);
		}
		return flag;
	}
	return FALSE;
}

BOOL CdbWaveDoc::open_database(const LPCTSTR lpsz_path_name)
{
	const auto tmp_db = new CdbTable;
	tmp_db->attach(current_data_file_name_, current_spike_file_name_);

	// Attempt to open the new database before replacing our ptr
	try
	{
		tmp_db->Open(lpsz_path_name);
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 4);
		e->Delete();
		tmp_db->Close();
		delete tmp_db;
		return FALSE;
	}
	db_filename = lpsz_path_name;
	proposed_data_path_name = db_filename.Left(db_filename.ReverseFind('.'));

	// pass file pointer to document pointer
	if (db_table != nullptr)
	{
		if (db_table->IsOpen())
			db_table->Close();
		delete db_table;
	}
	db_table = tmp_db;

	// save file name
	valid_tables = db_table->open_tables();
	SetPathName(lpsz_path_name, TRUE);

	return valid_tables;
}

BOOL CdbWaveDoc::OnSaveDocument(const LPCTSTR lpsz_path_name)
{
	// now duplicate file
	const auto cs_old_name = GetPathName();
	constexpr auto b_fail_if_exists = TRUE;

	const auto b_done = ::CopyFile(cs_old_name, lpsz_path_name, b_fail_if_exists);
	if (b_done == 0)
	{
		LPVOID lp_msg_buf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			reinterpret_cast<LPTSTR>(&lp_msg_buf),
			0,
			nullptr);

		// Process any inserts in lpMsgBuf.
		CString cs_error = static_cast<LPTSTR>(lp_msg_buf);
		cs_error = _T("Error when copying:\n") + cs_old_name + _T("\ninto:\n") + lpsz_path_name + _T("\n\n") + cs_error;

		// Display the string.
		MessageBox(nullptr, cs_error, _T("Error"), MB_OK | MB_ICONINFORMATION);
		// Free the buffer.
		LocalFree(lp_msg_buf);
		return FALSE;
	}
	return TRUE;
}

CString CdbWaveDoc::db_set_current_spike_file_name()
{
	if (!db_get_current_spk_file_name(TRUE).IsEmpty())
		return current_spike_file_name_;

	// not found? derive name from data file (same directory)
	current_spike_file_name_ = current_data_file_name_.Left(current_data_file_name_.ReverseFind(_T('.')) + 1) + _T("spk");

	const auto i_count = current_spike_file_name_.ReverseFind(_T('\\'));
	const auto cs_name = current_spike_file_name_.Right(current_spike_file_name_.GetLength() - i_count - 1);

	try
	{
		db_table->m_main_table_set.Edit();
		db_table->m_main_table_set.SetFieldNull(&(db_table->m_main_table_set.m_file_spk), TRUE);
		db_table->m_main_table_set.m_file_spk = cs_name;
		db_table->m_main_table_set.m_path2_id = db_table->m_main_table_set.m_path_id;
		db_table->m_main_table_set.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 10);
		e->Delete();
	}
	return current_spike_file_name_;
}

CString CdbWaveDoc::db_get_current_dat_file_name(const BOOL b_test)
{
	current_data_file_name_ = db_table->get_current_record_data_file_name();
	auto filename = current_data_file_name_;
	if (b_test && !is_file_present(filename))
		filename.Empty();
	return filename;
}

CString CdbWaveDoc::db_get_current_spk_file_name(const BOOL b_test)
{
	current_spike_file_name_ = db_table->get_current_record_spike_file_name();
	auto file_name = current_spike_file_name_;
	if (b_test && !is_file_present(file_name))
		file_name.Empty();
	return file_name;
}

long CdbWaveDoc::db_get_data_len() const
{
	long data_length = 0;
	try { data_length = db_table->m_main_table_set.m_data_len; }
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 11);
		e->Delete();
	}

	return data_length;
}

AcqDataDoc* CdbWaveDoc::open_current_data_file()
{
	// data doc to read data files
	if (m_p_data_doc == nullptr)
	{
		m_p_data_doc = new AcqDataDoc;
		ASSERT(m_p_data_doc != NULL);
	}

	// open document; erase object if operation failed
	db_get_current_dat_file_name(TRUE);
	if (current_data_file_name_.IsEmpty()
		|| !m_p_data_doc->open_document(current_data_file_name_))
	{
		delete m_p_data_doc;
		m_p_data_doc = nullptr;
	}
	else
		m_p_data_doc->SetPathName(current_data_file_name_, FALSE);
	return m_p_data_doc;
}

void CdbWaveDoc::close_current_data_file() const
{
	if (m_p_data_doc != nullptr)
		m_p_data_doc->acq_close_file();
}

CSpikeDoc* CdbWaveDoc::open_current_spike_file()
{
	// spike doc to read data files
	if (m_p_spk_doc == nullptr)
	{
		m_p_spk_doc = new CSpikeDoc;
		ASSERT(m_p_spk_doc != NULL);
	}
	// open document; erase object if operation fails
	if (db_get_current_record_position() >= 0)
		db_get_current_spk_file_name(TRUE);
	if (current_spike_file_name_.IsEmpty()
		|| !m_p_spk_doc->OnOpenDocument(current_spike_file_name_))
	{
		delete m_p_spk_doc;
		m_p_spk_doc = nullptr;
	}
	else
		m_p_spk_doc->SetPathName(current_spike_file_name_, FALSE);
	return m_p_spk_doc;
}

Spike* CdbWaveDoc::get_spike(db_spike& spike_coords)
{
	if (spike_coords.spike_index < 0)
		return nullptr;

	if (spike_coords.record_id < 0)
	{
		spike_coords.record_id = db_get_current_record_id();
		spike_coords.spike_list_index = m_p_spk_doc->get_index_current_spike_list();
	}
	else if (spike_coords.record_id != db_get_current_record_id()) 
	{
		if (db_move_to_id(spike_coords.record_id))
		{
			if (nullptr == open_current_spike_file())
				return nullptr;
		}	
	}

	SpikeList* p_spike_list;
	if (spike_coords.spike_list_index >= 0)
		p_spike_list = m_p_spk_doc->get_spike_list_at(spike_coords.spike_list_index);
	else
		p_spike_list = m_p_spk_doc->get_spike_list_current();
	
	return p_spike_list->get_spike(spike_coords.spike_index);
}

boolean CdbWaveDoc::get_max_min_amplitude_of_all_spikes(const BOOL b_all_files, const BOOL b_recalculate, int& max, int& min)
{
	boolean spikes_found = false;
	const long n_files = b_all_files ? db_get_records_count() : 1;
	const int current_spike_list_index = m_p_spk_doc->get_index_current_spike_list();
	boolean initialized = false;

	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (b_all_files)
		{
			if (db_set_current_record_position(i_file))
				open_current_spike_file();
			if (m_p_spk_doc == nullptr)
				continue;
			m_p_spk_doc->set_index_current_spike_list(current_spike_list_index);
		}

		const auto p_spk_list = m_p_spk_doc->get_spike_list_current();
		if (p_spk_list->get_spikes_count() > 0) 
		{
			spikes_found = true;
			int max_file_i = 0;
			int min_file_i = 0;
			p_spk_list->get_total_max_min(b_recalculate, &max_file_i, &min_file_i);
			if (spikes_found)
			{
				if (!initialized)
				{
					max = max_file_i;
					min = min_file_i;
					initialized = true;
					continue;
				}
				if (max < max_file_i)
					max = max_file_i;
				if (min > min_file_i)
					min = min_file_i;
			}
		}
	}

	return spikes_found;
}

boolean CdbWaveDoc::get_max_min_y1_of_all_spikes(const boolean b_all_files, int& max, int& min)
{
	boolean spikes_found = false;
	const long n_files = b_all_files ? db_get_records_count() : 1;
	const int current_spike_list_index = m_p_spk_doc->get_index_current_spike_list();
	boolean initialized = false;

	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (b_all_files)
		{
			if (db_set_current_record_position(i_file))
				open_current_spike_file();
			if (m_p_spk_doc == nullptr)
				continue;
			m_p_spk_doc->set_index_current_spike_list(current_spike_list_index);
		}

		const auto p_spk_list = m_p_spk_doc->get_spike_list_current();
		if (p_spk_list->get_spikes_count() > 0)
		{
			int max_file_i = 0;
			int min_file_i = 0;
			const int n_spikes_found = p_spk_list->get_total_max_min_of_y1( &max_file_i, &min_file_i);
			if (n_spikes_found > 0)
			{
				if (!initialized)
				{
					max = max_file_i;
					min = min_file_i;
					initialized = true;
					spikes_found = true;
					continue;
				}
				if (max < max_file_i) 
					max = max_file_i;
				if (min > min_file_i) 
					min = min_file_i;
			}
		}
	}

	return spikes_found;
}

void CdbWaveDoc::center_spike_amplitude_all_spikes_between_t1_and_t2(const boolean b_all_files, const int spike_class, const int t1, const int t2)
{
	const long n_files = b_all_files ? db_get_records_count() : 1;
	const int current_spike_list_index = m_p_spk_doc->get_index_current_spike_list();
	boolean initialized = false;

	for (long i_file = 0; i_file < n_files; i_file++)
	{
		if (b_all_files)
		{
			if (db_set_current_record_position(i_file))
				open_current_spike_file();
			if (m_p_spk_doc == nullptr)
				continue;
			m_p_spk_doc->set_index_current_spike_list(current_spike_list_index);
		}

		const auto p_spk_list = m_p_spk_doc->get_spike_list_current();
		const auto n_spikes = p_spk_list->get_spikes_count();
		if (n_spikes > 0)
		{
			for (auto i_spike = 0; i_spike < n_spikes; i_spike++)
			{
				const auto spike = p_spk_list->get_spike(i_spike);
				if (spike_class < 0 || spike_class == spike->get_class_id())
				{
					spike->set_spike_length(p_spk_list->get_spike_length());
					spike->center_spike_amplitude(t1, t2, 1);
				}
			}
		}
	}
}

long CdbWaveDoc::db_get_current_record_position() const
{
	long i_file = -1;
	try
	{
		i_file = db_table->m_main_table_set.GetAbsolutePosition();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 14);
		e->Delete();
	}
	return i_file;
}

BOOL CdbWaveDoc::db_set_current_record_position(const long i_file)
{
	BOOL flag = db_table->set_index_current_file(i_file);
	if (flag)
	{
		current_data_file_name_ = db_table->current_data_filename;
		current_spike_file_name_ = db_table->current_spike_filename;
	}
	return flag;
}

long CdbWaveDoc::db_get_current_record_id() const
{
	return db_table->m_main_table_set.m_id;
}

void CdbWaveDoc::db_set_current_record_flag(const int flag) const
{
	db_table->m_main_table_set.Edit();
	db_table->m_main_table_set.m_flag = flag;
	db_table->m_main_table_set.Update();
}

void CdbWaveDoc::db_set_paths_relative() const
{
	db_table->set_path_relative();
}

void CdbWaveDoc::db_set_paths_absolute() const
{
	db_table->set_path_absolute();
}

void CdbWaveDoc::db_transfer_dat_path_to_spk_path() const
{
	db_table->m_main_table_set.copy_path_to_path2();
}

void CdbWaveDoc::db_delete_unused_entries() const
{
	db_table->delete_unused_entries_in_accessory_tables();
}

void CdbWaveDoc::set_db_n_spikes(const long n_spikes) const
{
	try
	{
		db_table->m_main_table_set.Edit();
		db_table->m_main_table_set.m_n_spikes = n_spikes;
		db_table->m_main_table_set.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 15);
		e->Delete();
	}
}

long CdbWaveDoc::get_db_n_spikes() const
{
	return db_table->m_main_table_set.m_n_spikes;
}

long CdbWaveDoc::get_db_n_spike_classes() const
{
	return db_table->m_main_table_set.m_n_spike_classes;
}

void CdbWaveDoc::set_db_n_spike_classes(const long n_classes) const
{
	try
	{
		db_table->m_main_table_set.Edit();
		db_table->m_main_table_set.m_n_spike_classes = n_classes;
		db_table->m_main_table_set.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 16);
		e->Delete();
	}
}

void CdbWaveDoc::export_data_ascii_comments(CSharedFile * p_shared_file)
{
	DlgProgress dlg;
	dlg.Create();
	int i_step = 0;
	dlg.set_step(1);
	CString cs_comment;
	CString cs_file_comment = _T("Analyze file: ");
	CString cs_dummy;
	const auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	const auto p_view_data_options = &(p_app->options_view_data);
	const int index_current = db_get_current_record_position();
	const int n_files = db_get_records_count();

	// memory allocated -- get pointer to it
	cs_dummy.Format(_T("n files = %i\r\n\r\n"), n_files);
	p_shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

	// loop over all files of the multi-document
	for (auto i_file = 0; i_file < n_files; i_file++)
	{
		if (!db_set_current_record_position(i_file))
			continue;

		cs_dummy.Format(_T("%i\t%i\t"), i_file + 1, db_table->m_main_table_set.m_id);
		p_shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		// check if user wants to stop
		if (dlg.check_cancel_button())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		cs_comment.Format(_T("Processing file [%i / %i] %s"), i_file + 1, n_files, (LPCTSTR)current_data_file_name_);
		dlg.set_status(cs_comment);

		cs_dummy.Empty();
		if (open_current_data_file() != nullptr)
		{
			cs_dummy += m_p_data_doc->get_data_file_infos(p_view_data_options); 
			if (p_view_data_options->b_data_base_columns)
				cs_dummy += export_database_data();
		}
		else
			cs_dummy += _T("file not found\t");

		cs_dummy += _T("\r\n"); // next line
		p_shared_file->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		if (MulDiv(i_file, 100, n_files) > i_step)
		{
			dlg.step_it();
			i_step = MulDiv(i_file, 100, n_files);
		}
	}

	if (db_set_current_record_position(index_current))
		open_current_data_file();
}

boolean CdbWaveDoc::create_directory_if_does_not_exists(const CString & path)
{
	// first decompose the path into subdirectories
	CStringArray cs_sub_paths;
	auto cs_new_path = path;
	auto f_pos = 1;
	BOOL b_exists;
	do
	{
		// test if directory already present
		b_exists = PathFileExists(cs_new_path);

		// if not, strip the chain down from one sub_directory
		if (!b_exists)
		{
			f_pos = cs_new_path.ReverseFind(_T('\\'));
			if (f_pos != -1)
			{
				auto sub_path = cs_new_path.Right(cs_new_path.GetLength() - f_pos);
				cs_sub_paths.Add(sub_path);
				cs_new_path = cs_new_path.Left(f_pos);
			}
		}
	} while (f_pos > 0 && b_exists == FALSE);

	// now, newPath should be something like "c:" (if data are on a disk) or "" (if data are on a network like "\\folder" )
	for (auto i = cs_sub_paths.GetUpperBound(); i >= 0; i--)
	{
		cs_new_path = cs_new_path + cs_sub_paths.GetAt(i);
		if (!CreateDirectory(cs_new_path, nullptr))
		{
			AfxMessageBox(IDS_DIRECTORYFAILED);
			return FALSE;
		}
	}
	// then loop through array to create directories
	return TRUE;
}

BOOL CdbWaveDoc::copy_files_to_directory(const CString & path)
{
	// prepare dialog box to tell what is going on ----------------------
	DlgProgress dlg;
	dlg.Create();
	auto i_step = 0;
	dlg.set_step(1);
	CString cs_comment;
	CString cs_file_comment = _T("Copy files");
	CString cs_dummy;

	// read all data paths and create new array --------------------------
	dlg.set_status(_T("Create destination directories..."));
	CStringArray cs_source_path_array;
	CStringArray cs_dest_path_array;
	CUIntArray ui_id_array;
	const auto cs_path = path + _T('\\');
	auto flag = create_directory_if_does_not_exists(path); // create root destination directory if necessary
	if (!flag)
		return FALSE;

	// loop over database pathSet : copy paths into 2 string arrays and create subdirectories
	db_table->m_path_set.MoveFirst();
	auto n_records = 0;
	while (!db_table->m_path_set.IsEOF())
	{
		cs_source_path_array.Add(db_table->m_path_set.m_cs);
		ui_id_array.Add(db_table->m_path_set.m_id);
		n_records++;
		db_table->m_path_set.MoveNext();
	}

	// find root & build new directories
	auto cs_root = cs_source_path_array[0];
	auto cs_root_length = cs_root.GetLength();
	for (auto i = 1; i < n_records; i++)
	{
		// trim strings to the same length from the left
		auto cs_current = cs_source_path_array[i];
		if (cs_root_length > cs_current.GetLength())
		{
			cs_root = cs_root.Left(cs_current.GetLength());
			cs_root_length = cs_root.GetLength();
		}
		if (cs_current.GetLength() > cs_root_length)
			cs_current = cs_current.Left(cs_root_length);

		// loop to find the smallest common string
		for (auto j = cs_root_length; j > 1; j--)
		{
			if (0 == cs_current.CompareNoCase(cs_root))
				break;
			// not found, search next subfolder in root string and clip current
			const auto f_pos = cs_root.ReverseFind(_T('\\'));
			if (f_pos != -1)
			{
				cs_root = cs_root.Left(f_pos);
				cs_root_length = f_pos;
			}
			cs_current = cs_current.Left(cs_root_length);
		}
	}

	// create new paths and create directories
	if (cs_root_length <= 2)
	{
		CString cs_buf_temp;
		for (auto i = 0; i < n_records; i++)
		{
			cs_buf_temp.Format(_T("%06.6lu"), i);
			auto cs_dummy2 = cs_path + cs_buf_temp;
			cs_dest_path_array.Add(cs_dummy2);
		}
	}
	else
	{
		for (auto i = 0; i < n_records; i++)
		{
			auto sub_path = cs_source_path_array.GetAt(i);
			sub_path = sub_path.Right(sub_path.GetLength() - cs_root_length - 1);
			auto cs_dummy3 = cs_path + sub_path;
			cs_dest_path_array.Add(cs_dummy3);
		}
	}

	// copy database into new database ------------------------------------
	dlg.set_status(_T("Copy database ..."));
	auto old_database = GetPathName();
	const auto i_count = old_database.ReverseFind(_T('\\')) + 1;
	auto new_path = path;
	auto new_name = old_database.Right(old_database.GetLength() - i_count);
	const auto f_pos = path.ReverseFind(_T('\\'));
	if (f_pos != -1)
	{
		new_name = path.Right(path.GetLength() - f_pos) + _T(".mdb");
		new_path = path.Left(f_pos);
	}
	const auto destination_database = new_path + new_name;

	// create database and copy all records
	flag = static_cast<boolean>(OnSaveDocument(destination_database));
	if (!flag)
		return FALSE;

	// read all data and spike file names and store them into an array ------------
	if (db_table->m_main_table_set.GetRecordCount() < 1)
		return FALSE;

	// create new document and open it to update the paths
	auto* p_new = new CdbWaveDoc;
	flag = static_cast<boolean>(p_new->OnOpenDocument(destination_database));
	if (!flag)
	{
		delete p_new;
		return FALSE;
	}

	// read fileList & copy records into target tableSet
	dlg.set_status(_T("Build a list of files to copy..."));

	CStringArray old_names_array;
	CStringArray new_names_array;
	CUIntArray ui_id_new_path_array;
	try
	{
		db_table->m_main_table_set.MoveFirst();

		while (!db_table->m_main_table_set.IsEOF())
		{
			db_table->get_current_record_file_names();
			// data file
			if (!current_data_file_name_.IsEmpty())
			{
				old_names_array.Add(current_data_file_name_);

				const UINT uid = db_table->m_main_table_set.m_path_id;
				auto j = 0;
				for (auto i = 0; i < ui_id_array.GetSize(); i++, j++)
					if (ui_id_array.GetAt(i) == uid)
						break;
				ui_id_new_path_array.Add(j);
				new_names_array.Add(db_table->m_main_table_set.m_file_dat);
			}
			// spike file
			if (!current_spike_file_name_.IsEmpty())
			{
				old_names_array.Add(current_spike_file_name_);
				const UINT uid = db_table->m_main_table_set.m_path2_id;
				auto j = 0;
				for (auto i = 0; i < ui_id_array.GetSize(); i++, j++)
					if (ui_id_array.GetAt(i) == uid)
						break;
				ui_id_new_path_array.Add(j);
				new_names_array.Add(db_table->m_main_table_set.m_file_spk);
			}
			// move to next pRecord
			db_table->m_main_table_set.MoveNext();
		}
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 13);
		e->Delete();
	}

	// replace the destination paths
	dlg.set_status(_T("Update directories in the destination database..."));

	// change paths
	try
	{
		p_new->db_table->m_path_set.MoveFirst();
		while (!p_new->db_table->m_path_set.IsEOF())
		{
			auto cs_path_local = p_new->db_table->m_path_set.m_cs;
			// find corresponding path in old paths
			auto i_found = -1;
			for (auto i = 0; i < cs_source_path_array.GetSize(); i++)
			{
				if (cs_path_local.Compare(cs_source_path_array.GetAt(i)) == 0)
				{
					i_found = i;
					break;
				}
			}
			if (i_found > -1)
			{
				p_new->db_table->m_path_set.Edit();
				p_new->db_table->m_path_set.m_cs = cs_dest_path_array.GetAt(i_found);
				p_new->db_table->m_path_set.Update();
			}
			p_new->db_table->m_path_set.MoveNext();
		}
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 13);
		e->Delete();
	}

	//-------------------------------------------------------
	// memory allocated -- get pointer to it
	const auto n_files = old_names_array.GetSize();
	cs_dummy.Format(_T("n files (*.dat & *.spk) = %i\r\n\r\n"), n_files);

	// loop over all files of the multi-document
	for (auto i_file = 0; i_file < n_files; i_file++)
	{
		// set message for dialog box and check if user wants to stop
		if (dlg.check_cancel_button())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		// get source name
		const auto& source_file = old_names_array.GetAt(i_file);
		cs_comment.Format(_T("Processing file [%i / %i] %s"), i_file + 1, n_files, (LPCTSTR)source_file);
		dlg.set_status(cs_comment);

		// get destination directory
		const int i_path = static_cast<int>(ui_id_new_path_array.GetAt(i_file));
		const auto& path_local = cs_dest_path_array.GetAt(i_path);
		// create directory if necessary or exit if not possible
		if (!create_directory_if_does_not_exists(path_local))
			return FALSE;

		// copy file
		auto destination_file = path_local + _T("\\") + new_names_array.GetAt(i_file);
		if (binary_file_copy(source_file, destination_file))
		{
			// update count
			if (MulDiv(i_file, 100, n_files) > i_step)
			{
				dlg.step_it();
					i_step = MulDiv(i_file, 100, n_files);
			}
		}
	}

	//-------------------------------------------------------
	// remove non-relevant files from the new database
	if (p_new->db_table->m_main_table_set.GetEditMode() != dbEditNone)
		p_new->db_table->m_main_table_set.Update();
	p_new->db_table->m_main_table_set.Close(); // close dynaset and open as datatable

	if (!p_new->db_table->m_main_table_set.open_table(dbOpenTable, nullptr, 0))
		return FALSE;

	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	p_new->db_table->m_main_table_set.MoveFirst();

	if (!p_new->db_table->m_main_table_set.IsBOF())
	{
		while (!p_new->db_table->m_main_table_set.IsEOF())
		{
			p_new->db_table->get_current_record_file_names();
			CFileStatus status;
			const auto b_dat_file = CFile::GetStatus(p_new->current_data_file_name_, status);
			// check if data file is present
			const auto b_spk_file = CFile::GetStatus(p_new->current_spike_file_name_, status);
			// check if spike file is present
			if (!b_dat_file && !b_spk_file) // if none of them found, remove pRecord
				p_new->db_table->m_main_table_set.Delete();
			p_new->db_table->m_main_table_set.MoveNext();
		}
		p_new->db_table->m_main_table_set.MoveFirst();
	}

	// close database and document
	delete p_new;
	return true;
}

CString CdbWaveDoc::copy_file_to_directory(const LPCTSTR psz_source, const CString & directory)
{
	CFileException ex;
	CFile source_file;

	// open the source file for reading
	if (!source_file.Open(psz_source, CFile::modeRead | CFile::shareDenyNone, &ex))
	{
		// complain if an error happened
		TCHAR sz_error[1024];
		CString str_formatted = _T("Couldn't open source file:");
		if (ex.GetErrorMessage(sz_error, 1024))
			str_formatted += sz_error;
		AfxMessageBox(str_formatted);
		return nullptr;
	}

	CString destination_name = directory + "\\" + source_file.GetFileName();
	if (is_file_present(destination_name))
	{
		auto prompt = destination_name;
		prompt += _T("\nThis file seems to exist already.\nDelete the old file?");
		if (AfxMessageBox(prompt, MB_YESNO) == IDYES)
			CFile::Remove(destination_name);
		else
			return destination_name;
	}
	if (!binary_file_copy(psz_source, destination_name))
		return nullptr;
	return destination_name;
}

bool CdbWaveDoc::is_file_present(const CString & cs_new_name)
{
	CFileStatus status;
	return CFile::GetStatus(cs_new_name, status);
}

bool CdbWaveDoc::binary_file_copy(const LPCTSTR psz_source, const LPCTSTR psz_dest)
{
	// constructing these file objects doesn't open them
	CFile source_file;
	CFile destination_file;

	// we'll use a CFileException object to get error information
	CFileException ex;

	// open the source file for reading
	if (!source_file.Open(psz_source, CFile::modeRead | CFile::shareDenyNone, &ex))
	{
		// complain if an error happened
		TCHAR sz_error[1024];
		CString str_formatted = _T("Couldn't open source file:");
		if (ex.GetErrorMessage(sz_error, 1024))
			str_formatted += sz_error;
		AfxMessageBox(str_formatted);
		return false;
	}

	if (!destination_file.Open(psz_dest, CFile::modeWrite | CFile::shareExclusive | CFile::modeCreate, &ex))
	{
		TCHAR sz_error[1024];
		CString str_formatted = _T("Couldn't open source file:");
		if (ex.GetErrorMessage(sz_error, 1024))
			str_formatted += sz_error;
		AfxMessageBox(str_formatted);
		source_file.Close();
		return false;
	}

	DWORD dw_read;

	// Read in 4096-byte blocks,
	// remember how many bytes were actually read,
	// and try to write that many out. This loop ends
	// when there are no more bytes to read.
	do
	{
		BYTE buffer[4096];
		dw_read = source_file.Read(buffer, 4096);
		destination_file.Write(buffer, dw_read);
	} while (dw_read > 0);

	// Close both files

	destination_file.Close();
	source_file.Close();

	return true;
}

HMENU CdbWaveDoc::GetDefaultMenu()
{
	return h_my_menu; // just use original default
}

source_data CdbWaveDoc::get_wave_format_from_either_file(CString cs_filename)
{
	source_data record;

	record.i_last_backslash_position = cs_filename.ReverseFind('\\');
	const auto i_dot_position = cs_filename.ReverseFind('.');
	const auto name_length = i_dot_position - record.i_last_backslash_position - 1;
	record.cs_path = cs_filename.Left(record.i_last_backslash_position);
	record.cs_path.MakeLower();
	const auto cs_extent = cs_filename.Right(cs_filename.GetLength() - i_dot_position - 1);
	auto cs_root_name = cs_filename.Mid(record.i_last_backslash_position + 1, name_length);

	const auto b_is_dat_file = is_extension_recognized_as_data_file(cs_extent);
	if (b_is_dat_file)
	{
		record.cs_dat_file = cs_filename;
		record.cs_spk_file = cs_filename.Left(i_dot_position) + _T(".spk");
	}
	else
	{
		record.cs_dat_file = cs_filename.Left(i_dot_position) + _T(".") + cs_extent;
		record.cs_spk_file = cs_filename;
	}

	// test  files
	CFileStatus status;
	record.data_file_present = static_cast<boolean>(CFile::GetStatus(record.cs_dat_file, status));
	record.spike_file_present = static_cast<boolean>(CFile::GetStatus(record.cs_spk_file, status));
	if (record.data_file_present)
		cs_filename = record.cs_dat_file;
	else if (record.spike_file_present)
		cs_filename = record.cs_spk_file;
	record.p_wave_format = get_wave_format(cs_filename, record.data_file_present);
	m_p_data_doc->acq_close_file();

	return record;
}

int CdbWaveDoc::find_column_associated_to_header(const CString & text)
{
	int found = -1;
	for (int i = 0; i < 18; i++)
	{
		CString header = headers_[i].cs_text;
		if (text.CompareNoCase(header) == 0)
		{
			found = headers_[i].value;
			break;
		}
	}
	return found;
}

void CdbWaveDoc::get_infos_from_string_array(const source_data * p_record, const CStringArray & file_names_array, int const i_record, const int n_columns, const boolean b_header)
{
	CWaveFormat* p_wave_format = p_record->p_wave_format;
	const int index = index_2d_array(i_record, n_columns, b_header);

	for (int i = 1; i < n_columns; i++)
	{
		int i_column = i;
		if (b_header)
			i_column = find_column_associated_to_header(file_names_array.GetAt(i));

		const int index_i_record = index + i;
		const CString& cs_item = file_names_array.GetAt(index_i_record);
		switch (i_column)
		{
		case 1: p_wave_format->cs_comment = cs_item;
			break;
		case 2: p_wave_format->insect_id = _ttoi(cs_item);
			break;
		case 3: p_wave_format->sensillum_id = _ttoi(cs_item);
			break;
		case 4: p_wave_format->cs_insect_name = cs_item;
			break;
		case 5: p_wave_format->cs_strain = cs_item;
			break;
		case 6: p_wave_format->cs_sex = cs_item;
			break;
		case 7: p_wave_format->cs_location = cs_item;
			break;
		case 8: p_wave_format->cs_operator = cs_item;
			break;
		case 9: p_wave_format->cs_more_comment = cs_item;
			break;
		case 10: p_wave_format->cs_stimulus = cs_item;
			break;
		case 11: p_wave_format->cs_concentration = cs_item;
			break;
		case 12: p_wave_format->repeat = _ttoi(cs_item);
			break;
		case 13: p_wave_format->cs_stimulus2 = cs_item;
			break;
		case 14: p_wave_format->cs_concentration2 = cs_item;
			break;
		case 15: p_wave_format->repeat2 = _ttoi(cs_item);
			break;
		case 16: p_wave_format->cs_sensillum = cs_item;
			break;
		case 17: p_wave_format->cs_ad_card_name = cs_item; 
			break;
		case 18: p_wave_format->flag = _ttoi(cs_item);
			break;
		default:
			break;
		}
	}
}

void CdbWaveDoc::set_record_file_names(const source_data * record) const
{
	// save file names
	if (record->data_file_present)
	{
		db_table->m_main_table_set.m_path_id = db_table->m_path_set.get_string_in_linked_table(record->cs_path);
		db_table->m_main_table_set.SetFieldNull(&(db_table->m_main_table_set.m_file_dat), FALSE);
		db_table->m_main_table_set.m_file_dat = record->cs_dat_file.Right(
			record->cs_dat_file.GetLength() - record->i_last_backslash_position - 1);
		db_table->m_main_table_set.m_data_len = m_p_data_doc->get_doc_channel_length();
	}

	if (record->spike_file_present)
	{
		db_table->m_main_table_set.m_path2_id = db_table->m_path_set.get_string_in_linked_table(record->cs_path);
		db_table->m_main_table_set.SetFieldNull(&(db_table->m_main_table_set.m_file_spk), FALSE);
		db_table->m_main_table_set.m_file_spk = record->cs_spk_file.Right(
			record->cs_spk_file.GetLength() - record->i_last_backslash_position - 1);
	}
}

boolean CdbWaveDoc::set_record_spk_classes(const source_data * record) const
{
	const boolean flag = static_cast<boolean>(m_p_spk_doc->OnOpenDocument(record->cs_spk_file));
	if (flag)
	{
		db_table->m_main_table_set.m_n_spikes = m_p_spk_doc->get_spike_list_current()->get_spikes_count();
		if (m_p_spk_doc->get_spike_list_current()->get_classes_count() <= 0)
			m_p_spk_doc->get_spike_list_current()->update_class_list();
		db_table->m_main_table_set.m_n_spike_classes = m_p_spk_doc->get_spike_list_current()->get_classes_count();
		db_table->m_main_table_set.m_data_len = m_p_spk_doc->m_wave_format.get_nb_points_sampled_per_channel();
	}
	return flag;
}

void CdbWaveDoc::set_record_wave_format(const source_data * record) const
{
	db_table->transfer_wave_format_data_to_record(record->p_wave_format);
}

void CdbWaveDoc::import_file_list(CStringArray& cs_descriptors_array, const int n_columns, const boolean b_header)
{
	// exit if no data to import
	const int n_files = get_size_2d_array(cs_descriptors_array, n_columns, b_header);
	if (n_files == 0)
		return;

	CSharedFile* shared_file = static_cast<CdbWaveApp*>(AfxGetApp())->m_psf;
	SAFE_DELETE(shared_file)
		static_cast<CdbWaveApp*>(AfxGetApp())->m_psf = nullptr;
	shared_file = new CSharedFile(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);

	// -------------------------- cancel any pending edit or add operation
	db_table->update_all_database_tables();
	db_table->m_main_table_set.Close();
	if (!db_table->m_main_table_set.open_table(dbOpenTable, nullptr, 0))
	{
		delete shared_file;
		return;
	}

	// browse existing database array - collect data file acquisition time and IDs already used
	db_table->m_main_table_set.get_max_id();
	long m_id = db_table->m_main_table_set.max_id;
	const auto n_files_ok = check_files_can_be_opened(cs_descriptors_array, shared_file, n_columns, b_header);

	// ---------------------------------------------file loop: read infos --------------------------------
	DlgProgress dlg;
	dlg.Create();
	dlg.set_step(1);
	dlg.set_pos(0);
	dlg.set_range(0, n_files_ok);

	for (auto i_record = 0; i_record < n_files_ok; i_record++)
	{
		// check if user wants to stop and update progression bar
		if (dlg.check_cancel_button())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		// get file name
		const int index = index_2d_array(i_record, n_columns, b_header);
		auto cs_filename = CString(cs_descriptors_array[index]);
		CString cs_comment;
		cs_comment.Format(_T("Import file [%i / %i] %s"), i_record + 1, n_files_ok, (LPCTSTR)cs_filename);
		dlg.set_status(cs_comment);
		dlg.step_it();

		if (!import_file_single(cs_filename, m_id, i_record, cs_descriptors_array, n_columns, b_header))
			shared_file = file_discarded_message(shared_file, cs_filename, i_record);
	}

	// open dynaset
	db_table->m_main_table_set.Close();
	if (db_table->m_main_table_set.open_table(dbOpenDynaset, nullptr, 0))
	{
		db_table->m_main_table_set.Requery();
		db_table->m_main_table_set.build_and_sort_id_arrays();
		db_table->m_main_table_set.MoveLast();
	}

	// close files opened here
	SAFE_DELETE(m_p_data_doc)
		SAFE_DELETE(m_p_spk_doc)
		SAFE_DELETE(shared_file)
}

boolean CdbWaveDoc::import_file_single(const CString & cs_filename, long& m_id, const int i_record, const CStringArray & cs_array, const int n_columns,
                                       const boolean b_header)
{
	// open document and read data - go to next file if not readable
	source_data record = get_wave_format_from_either_file(cs_filename);
	if (record.p_wave_format == nullptr)
		return false;

	if (n_columns > 1)
		get_infos_from_string_array(&record, cs_array, i_record, n_columns, b_header);

	// check data acquisition time - go to next file if already exist and if flag set
	const auto t = record.p_wave_format->acquisition_time;
	COleDateTime o_time;
	o_time.SetDateTime(t.GetYear(), t.GetMonth(), t.GetDay(), t.GetHour(), t.GetMinute(), t.GetSecond());
	if (!static_cast<CdbWaveApp*>(AfxGetApp())->options_import.discard_duplicate_files)
	{
		if (!db_table->m_main_table_set.check_if_acq_date_time_is_unique(&o_time))
			return false;
	}

	// add new pRecord  -- mID
	db_table->m_main_table_set.AddNew();
	m_id++;
	db_table->m_main_table_set.m_id = m_id;
	set_record_file_names(&record);
	set_record_wave_format(&record);
	boolean flag = set_record_spk_classes(&record);
	
	try
	{
		db_table->m_main_table_set.Update();
	}
	catch (CDaoException* e)
	{
		DisplayDaoException(e, 17);
		e->Delete();
		return false;
	}
	return true;
}

BOOL CdbWaveDoc::is_extension_recognized_as_data_file(CString string)
{
	string.MakeLower();
	return (string.Find(_T("dat")) != -1
		|| string.Find(_T("mcid")) != -1
		|| string.Find(_T("asd")) != -1
		|| string.Find(_T("smr")) != -1
		|| string.Find(_T("smrx")) != -1);
}

CWaveFormat* CdbWaveDoc::get_wave_format(CString cs_filename, const BOOL is_dat_file)
{
	// new data doc used to read data files
	if (m_p_data_doc == nullptr)
	{
		m_p_data_doc = new AcqDataDoc;
		ASSERT(m_p_data_doc != NULL);
	}

	// new spike doc used to read data files
	if (m_p_spk_doc == nullptr)
	{
		m_p_spk_doc = new CSpikeDoc;
		ASSERT(m_p_spk_doc != NULL);
	}

	CWaveFormat* p_wave_format = nullptr;
	if (is_dat_file)
	{
		const auto b_is_read_ok = m_p_data_doc->open_document(cs_filename);
		if (b_is_read_ok)
			p_wave_format = m_p_data_doc->get_wave_format();
	}
	else if (cs_filename.Find(_T(".spk")) > 0)
	{
		const auto b_is_read_ok = m_p_spk_doc->OnOpenDocument(cs_filename);
		if (b_is_read_ok)
			p_wave_format = &(m_p_spk_doc->m_wave_format);
	}
	return p_wave_format;
}

BOOL CdbWaveDoc::import_database(const CString & filename) const
{
	// create dbWaveDoc, open database from this new document, then
	// read all files from this and import them into current document
	const auto p_new_doc = new CdbWaveDoc;
	if (!p_new_doc->OnOpenDocument(filename))
		return FALSE;

	// browse existing database array - collect data file acquisition time and IDs already used
	const auto p_new_database = p_new_doc->db_table;
	p_new_database->m_main_table_set.MoveFirst();
	auto n_added_records = 0;
	while (!p_new_database->m_main_table_set.IsEOF())
	{
		this->db_table->import_record_from_database(p_new_database);
		n_added_records++;
		p_new_database->m_main_table_set.MoveNext();
	}
	p_new_database->m_main_table_set.Close();
	delete p_new_database;
	delete p_new_doc;

	// open dynaset
	db_table->m_main_table_set.Close();
	if (!db_table->m_main_table_set.open_table(dbOpenDynaset, nullptr, 0))
		return FALSE;

	db_table->m_main_table_set.Requery();
	db_table->m_main_table_set.build_and_sort_id_arrays();

	return TRUE;
}

CString CdbWaveDoc::get_full_path_name_without_extension() const
{
	const CString path_name = GetPathName();
	const auto i_position_of_extension = path_name.ReverseFind('.');
	return path_name.Left(i_position_of_extension);
}

CString CdbWaveDoc::get_path_directory(const CString & full_name)
{
	const auto i_position_of_extension = full_name.ReverseFind('\\');
	return full_name.Left(i_position_of_extension);
}

void CdbWaveDoc::copy_files_to_directory(CStringArray & files_to_copy_array, const CString& mdb_directory)
{
	for (int i = 0; i < files_to_copy_array.GetCount(); i++) 
		files_to_copy_array[i] = copy_file_to_directory(files_to_copy_array[i], mdb_directory);
}

BOOL CdbWaveDoc::import_data_files_from_another_data_base(const CString& other_data_base_file_name, const boolean copy_data_to_new_sub_directory) const
{
	const auto p_new_doc = new CdbWaveDoc;
	if (!p_new_doc->OnOpenDocument(other_data_base_file_name))
		return FALSE;

	CString path_to_mdb_sub_directory{};
	if (copy_data_to_new_sub_directory)
	{
		const CString cs1 = get_full_path_name_without_extension();
		const CString file_name = PathFindFileName(other_data_base_file_name);
		const auto i_position_of_extension = file_name.ReverseFind('.');
		const CString cs2 = file_name.Left(i_position_of_extension);
		path_to_mdb_sub_directory = cs1 + _T("\\") + cs2;
		if (!create_directory_if_does_not_exists(path_to_mdb_sub_directory))
			return FALSE;
	}

	// get names of data files of otherDataBase
	const auto p_new_database = p_new_doc->db_table;
	p_new_database->m_main_table_set.MoveFirst();
	while (!p_new_database->m_main_table_set.IsEOF())
	{
		if (db_table->is_record_time_unique(p_new_database->m_main_table_set.m_table_acq_date))
		{
			if (copy_data_to_new_sub_directory)
			{
				CString dat_name = p_new_database->get_current_record_data_file_name();
				if (!dat_name.IsEmpty() && file_exists(dat_name))
					dat_name = copy_file_to_directory(dat_name, path_to_mdb_sub_directory);
				CString spk_name = p_new_database->get_current_record_spike_file_name();
				if (!spk_name.IsEmpty() && file_exists(spk_name))
					spk_name = copy_file_to_directory(spk_name, path_to_mdb_sub_directory);
			}
			db_table->import_record_from_database(p_new_database);
		}
		p_new_database->m_main_table_set.MoveNext();
	}
	p_new_database->m_main_table_set.Close();
	delete p_new_doc;

	// TODO change associated table "old directory" to "new directory" 

	// open dynaset
	db_table->m_main_table_set.Close();
	if (!db_table->m_main_table_set.open_table(dbOpenDynaset, nullptr, 0))
		return FALSE;

	db_table->m_main_table_set.Requery();
	db_table->m_main_table_set.build_and_sort_id_arrays();

	return TRUE;
}

/*
 // change paths
	try
 {
	 p_new->db_table->m_path_set.MoveFirst();
	 while (!p_new->db_table->m_path_set.IsEOF())
	 {
		 auto cs_path_local = p_new->db_table->m_path_set.m_cs;
		 // find corresponding path in old paths
		 auto i_found = -1;
		 for (auto i = 0; i < cs_source_path_array.GetSize(); i++)
		 {
			 if (cs_path_local.Compare(cs_source_path_array.GetAt(i)) == 0)
			 {
				 i_found = i;
				 break;
			 }
		 }
		 if (i_found > -1)
		 {
			 p_new->db_table->m_path_set.Edit();
			 p_new->db_table->m_path_set.m_cs = cs_dest_path_array.GetAt(i_found);
			 p_new->db_table->m_path_set.Update();
		 }
		 p_new->db_table->m_path_set.MoveNext();
	 }
 }
 catch (CDaoException* e)
 {
	 DisplayDaoException(e, 13);
	 e->Delete();
 }
 */

boolean CdbWaveDoc::file_exists(const CString & file_name)
{
	CFileStatus status;
	return static_cast<boolean>(CFile::GetStatus(file_name, status));
}

void CdbWaveDoc::synchronize_source_infos(const BOOL b_all)
{
	// save current index position - restore on exit
	const auto current_file = db_get_current_record_position();

	// make sure there are objects to read / write data and spike files
	if (m_p_data_doc == nullptr) 
	{
		m_p_data_doc = new AcqDataDoc;
		ASSERT(m_p_data_doc != NULL);
	}
	if (m_p_spk_doc == nullptr) // spike doc
	{
		m_p_spk_doc = new CSpikeDoc;
		ASSERT(m_p_spk_doc != NULL);
	}

	// do only one iteration?
	if (!b_all)
	{
		CWaveFormat* wave_format;
		// process data file
		if (!current_data_file_name_.IsEmpty())
		{
			const auto p_dat = open_current_data_file();
			ASSERT(p_dat != nullptr);
			wave_format = p_dat->get_wave_format();
			if (update_waveformat_from_database(wave_format)) {
				if (!p_dat->acq_save_data_descriptors()) {
					AfxMessageBox(_T("Error saving data descriptors\n"), MB_OK);
				}
			}
		}
		// process spike file
		if (!current_spike_file_name_.IsEmpty())
		{
			const auto p_spk = open_current_spike_file();
			ASSERT(p_spk != nullptr);
			wave_format = &(m_p_spk_doc->m_wave_format);
			if (update_waveformat_from_database(wave_format))
				m_p_spk_doc->OnSaveDocument(current_spike_file_name_);
		}
		return;
	}

	// prepare progress dialog box
	DlgProgress dlg;
	dlg.Create();
	dlg.set_step(1);
	auto i_step = 0;
	CString cs_comment;
	const auto n_files = db_get_records_count();
	auto i_file = 1;

	// got to the first pRecord
	db_table->m_main_table_set.MoveFirst();
	dlg.set_pos(0);

	while (!db_table->m_main_table_set.IsEOF())
	{
		db_table->get_current_record_file_names();
		// check if user wants to stop
		if (dlg.check_cancel_button())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		cs_comment.Format(_T("Processing file [%i / %i]"), i_file, n_files);
		dlg.set_status(cs_comment);

		CWaveFormat* p_wave_format;
		// process data file
		if (!current_data_file_name_.IsEmpty())
		{
			const auto p_dat = open_current_data_file();
			ASSERT(p_dat != nullptr);
			p_wave_format = m_p_data_doc->get_wave_format();
			if (update_waveformat_from_database(p_wave_format))
			{
				if (!m_p_data_doc->acq_save_data_descriptors())
					AfxMessageBox(_T("Error saving data descriptors\n"), MB_OK);
			}
		}
		// process spike file
		if (!current_spike_file_name_.IsEmpty())
		{
			const auto p_spk = open_current_spike_file();
			ASSERT(p_spk != nullptr);
			p_wave_format = &(m_p_spk_doc->m_wave_format);
			if (update_waveformat_from_database(p_wave_format))
				m_p_spk_doc->OnSaveDocument(current_spike_file_name_);
		}
		// move to next pRecord
		db_table->m_main_table_set.MoveNext();
		i_file++;
		if (MulDiv(i_file, 100, n_files) > i_step)
		{
			dlg.step_it();
			i_step = MulDiv(i_file, 100, n_files);
		}
	}
	db_table->m_main_table_set.MoveFirst();
	db_table->get_current_record_file_names();

	// restore current data position
	db_set_current_record_position(current_file);
}

BOOL CdbWaveDoc::update_waveformat_from_database(CWaveFormat * p_wave_format) const
{
	auto b_changed = FALSE;

	b_changed = db_table->get_record_value_string(CH_EXPT_ID, p_wave_format->cs_comment);
	b_changed |= db_table->get_record_value_string(CH_MORE, p_wave_format->cs_more_comment);
	b_changed |= db_table->get_record_value_string(CH_OPERATOR_ID, p_wave_format->cs_operator);
	b_changed |= db_table->get_record_value_string(CH_INSECT_ID, p_wave_format->cs_insect_name);
	b_changed |= db_table->get_record_value_string(CH_STRAIN_ID, p_wave_format->cs_strain);
	b_changed |= db_table->get_record_value_string(CH_SEX_ID, p_wave_format->cs_sex);
	b_changed |= db_table->get_record_value_string(CH_LOCATION_ID, p_wave_format->cs_location);
	b_changed |= db_table->get_record_value_string(CH_SENSILLUM_ID, p_wave_format->cs_sensillum);
	b_changed |= db_table->get_record_value_string(CH_STIM_ID, p_wave_format->cs_stimulus);
	b_changed |= db_table->get_record_value_string(CH_CONC_ID, p_wave_format->cs_concentration);
	b_changed |= db_table->get_record_value_string(CH_STIM2_ID, p_wave_format->cs_stimulus2);
	b_changed |= db_table->get_record_value_string(CH_CONC2_ID, p_wave_format->cs_concentration2);
	b_changed |= db_table->get_record_value_long(CH_IDINSECT, p_wave_format->insect_id);
	b_changed |= db_table->get_record_value_long(CH_IDSENSILLUM, p_wave_format->sensillum_id);
	b_changed |= db_table->get_record_value_long(CH_REPEAT, p_wave_format->repeat);
	b_changed |= db_table->get_record_value_long(CH_REPEAT2, p_wave_format->repeat2);

	const auto n_per_cycle = static_cast<int>(static_cast<float>(m_p_spk_doc->m_stimulus_intervals.n_items) / 2.f
		/ m_p_spk_doc->get_acq_duration() / 8.192f);
	b_changed |= (n_per_cycle != m_p_spk_doc->m_stimulus_intervals.n_per_cycle);
	m_p_spk_doc->m_stimulus_intervals.n_per_cycle = n_per_cycle;

	return b_changed;
}

void CdbWaveDoc::export_spk_descriptors(CSharedFile * p_sf, SpikeList * p_spike_list, const int k_class) const
{
	CString cs_dummy;
	const CString cs_tab = _T("\t");

	const auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	const auto options_view_spikes = &(p_app->options_view_spikes);

	const auto cs_file_comment = _T("\r\n") + export_database_data();
	p_sf->Write(cs_file_comment, cs_file_comment.GetLength() * sizeof(TCHAR));

	// spike file additional comments
	if (options_view_spikes->b_spk_comments)
	{
		p_sf->Write(cs_tab, cs_tab.GetLength() * sizeof(TCHAR));
		const auto cs_temp = m_p_spk_doc->get_comment();
		p_sf->Write(cs_temp, cs_temp.GetLength() * sizeof(TCHAR));
	}

	// number of spikes
	if (options_view_spikes->b_total_spikes)
	{
		cs_dummy.Format(_T("%s%f"), (LPCTSTR)cs_tab, p_spike_list->get_detection_parameters()->detect_threshold_mv);
		p_sf->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));

		cs_dummy.Format(_T("%s%i"), (LPCTSTR)cs_tab, p_spike_list->get_spikes_count());
		p_sf->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		cs_dummy.Format(_T("%s%i"), (LPCTSTR)cs_tab, p_spike_list->get_classes_count());
		p_sf->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
		const auto t_duration = m_p_spk_doc->get_acq_duration();
		cs_dummy.Format(_T("%s%f"), (LPCTSTR)cs_tab, t_duration);
		p_sf->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	}

	// spike list iColumn, spike class
	if (options_view_spikes->spike_class_option != 0)
	{
		cs_dummy.Format(_T("%s%i %s%s %s%i"), (LPCTSTR)cs_tab, options_view_spikes->i_chan,
		   (LPCTSTR)cs_tab, (LPCTSTR)p_spike_list->get_detection_parameters()->comment,
		   (LPCTSTR)cs_tab, k_class);
		// spike class descriptor
		const CString cs = _T("\t") + p_spike_list->get_class_description_from_id(k_class);
		cs_dummy = cs_dummy + cs;
	}
	else
	{
		cs_dummy.Format(_T("%s%i %s%s \t(all)"), (LPCTSTR)cs_tab, options_view_spikes->i_chan,
		   (LPCTSTR)cs_tab, (LPCTSTR)p_spike_list->get_detection_parameters()->comment);
	}
	p_sf->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
}

CString CdbWaveDoc::export_database_data(const int option) const
{
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	const auto options_view_spikes = &(p_app->options_view_spikes);
	CString separator = _T("\t");
	if (option == 1)
		separator = _T(" | ");

	// export data comment into csFileComment
	CString cs_file_comment;
	CString cs_dummy;

	DB_ITEMDESC desc;
	db_table->get_record_item_value(CH_PATH2_ID, &desc);
	auto filename = desc.cs_val;
	db_table->get_record_item_value(CH_FILESPK, &desc);
	filename = filename + _T('\\') + desc.cs_val;
	cs_file_comment.Format(_T("%i%s%s"), db_table->m_main_table_set.m_id, (LPCTSTR)separator, (LPCTSTR)filename);

	// source data file items
	if (options_view_spikes->b_acq_date) // source data time and date
	{
		cs_dummy = separator + db_table->m_main_table_set.m_table_acq_date.Format(VAR_DATEVALUEONLY);
		cs_file_comment += cs_dummy;
		cs_dummy = separator + db_table->m_main_table_set.m_table_acq_date.Format(VAR_TIMEVALUEONLY);
		cs_file_comment += cs_dummy;
	}
	// source data comments
	if (options_view_spikes->b_acq_comments)
	{
		db_table->get_record_item_value(CH_EXPT_ID, &desc);
		cs_file_comment += separator + desc.cs_val;

		db_table->get_record_item_value(CH_IDINSECT, &desc);
		cs_dummy.Format(_T("%i"), desc.l_val);
		cs_file_comment += separator + cs_dummy;
		db_table->get_record_item_value(CH_IDSENSILLUM, &desc);
		cs_dummy.Format(_T("%i"), desc.l_val);
		cs_file_comment += separator + cs_dummy;

		db_table->get_record_item_value(CH_INSECT_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_STRAIN_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_SEX_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_LOCATION_ID, &desc);
		cs_file_comment += separator + desc.cs_val;

		db_table->get_record_item_value(CH_OPERATOR_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_MORE, &desc);
		cs_file_comment += separator + desc.cs_val;

		db_table->get_record_item_value(CH_STIM_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_CONC_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_REPEAT, &desc);
		cs_dummy.Format(_T("%i"), desc.l_val);
		cs_file_comment += separator + cs_dummy;

		db_table->get_record_item_value(CH_STIM2_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_CONC2_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_REPEAT2, &desc);
		cs_dummy.Format(_T("%i"), desc.l_val);
		cs_file_comment += separator + cs_dummy;

		db_table->get_record_item_value(CH_SENSILLUM_ID, &desc);
		cs_file_comment += separator + desc.cs_val;
		db_table->get_record_item_value(CH_FLAG, &desc);
		cs_dummy.Format(_T("%i"), desc.l_val);
		cs_file_comment += separator + cs_dummy;
	}
	return cs_file_comment;
}

void CdbWaveDoc::export_number_of_spikes(CSharedFile * p_sf)
{
	DlgProgress dlg;
	dlg.Create();
	dlg.set_step(1);
	CString file_comment = _T("Analyze file: ");

	// save current selection and export header of the table
	const int i_old_index = db_get_current_record_position();
	const int n_files = db_get_records_count();
	if (nullptr == m_p_spk_doc)
	{
		m_p_spk_doc = new CSpikeDoc;
		ASSERT(m_p_spk_doc != NULL);
		m_p_spk_doc->set_index_current_spike_list(get_current_spike_file()->get_index_current_spike_list());
	}

	auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	const auto options_view_spikes = &(p_app->options_view_spikes);

	// export description of the type of data exported
	const auto i_old_list = m_p_spk_doc->get_index_current_spike_list();
	m_p_spk_doc->export_table_title(p_sf, options_view_spikes, n_files);

	// export header of all columns
	CSpikeDoc::export_headers_descriptors(p_sf, options_view_spikes);
	m_p_spk_doc->export_headers_data(p_sf, options_view_spikes); 

	// single file export operation: EXTREMA, amplitude, SPIKE_POINTS - change orientation of table
	transpose_ = FALSE;
	if (options_view_spikes->export_data_type == EXPORT_INTERV)
		transpose_ = TRUE;

	if (options_view_spikes->export_data_type == EXPORT_EXTREMA
		|| options_view_spikes->export_data_type == EXPORT_AMPLIT
		|| options_view_spikes->export_data_type == EXPORT_SPIKEPOINTS) 
	{
		const CString cs_file_desc;
		m_p_spk_doc->export_spk_file_comment(p_sf, options_view_spikes, 0, cs_file_desc);
		m_p_spk_doc->export_spk_attributes_one_file(p_sf, options_view_spikes);
	}

	// multiple file export operations: ISI, AUTOCORRELATION, HIST_AMPL, AVERAGE, INTERVALS, PSTH
	else
	{
		auto i_step = 0;
		CString cs_comment;
		auto n_bins = 0;
		double* p_double = nullptr;
		switch (options_view_spikes->export_data_type)
		{
		case EXPORT_ISI: 
		case EXPORT_AUTOCORR: 
			n_bins = options_view_spikes->n_bins_isi;
			break;

		case EXPORT_HISTAMPL: 
			n_bins = options_view_spikes->hist_ampl_n_bins + 2;
			break;

		case EXPORT_AVERAGE: 
			p_double = new double[m_p_spk_doc->get_spike_list_current()->get_spike_length() * 2 + 1 + 2];
			*p_double = m_p_spk_doc->get_spike_list_current()->get_spike_length();
			break;

		case EXPORT_INTERV: // feb 23, 2009
			break;

		case EXPORT_PSTH:
		default:
			n_bins = options_view_spikes->n_bins;
			break;
		}
		const auto p_hist0 = new long[n_bins + 2]; // create array (dimension = n_bins) to store results
		*p_hist0 = n_bins;
		ASSERT(p_hist0 != NULL);
		CString cs_file_comment;
		cs_file_comment.Empty();

		// in each spike list, loop over spike classes as defined in the options
		auto class1 = options_view_spikes->class_nb;
		auto class2 = options_view_spikes->class_nb_2;
		if (options_view_spikes->spike_class_option == 0)
			class2 = class1;
		else
		{
			if (class2 < class1)
			{
				const auto i_class = class1;
				class1 = class2;
				class2 = i_class;
			}
		}

		// loop (1) from file 1 to file n ---------------------------------------------
		for (auto i_file = 0; i_file < n_files; i_file++)
		{
			// check if user wants to stop
			if (dlg.check_cancel_button())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			cs_comment.Format(_T("Processing file [%i / %i]"), i_file + 1, n_files);
			dlg.set_status(cs_comment);

			// open document
			const BOOL flag0 = db_set_current_record_position(i_file);
			if (!flag0 || current_spike_file_name_.IsEmpty())
				continue;

			// check if file is still present and open it
			CFileStatus status;
			if (!CFile::GetStatus(current_spike_file_name_, status))
			{
				cs_file_comment = _T("\r\n") + export_database_data();
				cs_file_comment += _T("\tERROR: MISSING FILE"); // next line
				p_sf->Write(cs_file_comment, cs_file_comment.GetLength() * sizeof(TCHAR));
				continue;
			}
			if (open_current_spike_file() == nullptr)
				continue;

			// loop over the spike lists stored in that file
			auto i_chan1 = 0;
			auto i_chan2 = m_p_spk_doc->get_spike_list_size();
			if (!options_view_spikes->b_all_channels)
			{
				i_chan1 = i_old_list;
				i_chan2 = i_chan1 + 1;
			}

			//----------------------------------------------------------
			for (auto i_spike_list = i_chan1; i_spike_list < i_chan2; i_spike_list++)
			{
				const auto p_spike_list = m_p_spk_doc->set_index_current_spike_list(i_spike_list);
				options_view_spikes->i_chan = i_spike_list;

				for (auto k_class = class1; k_class <= class2; k_class++)
				{
					export_spk_descriptors(p_sf, p_spike_list, k_class);
					// export data
					switch (options_view_spikes->export_data_type)
					{
					case EXPORT_HISTAMPL: 
						m_p_spk_doc->export_spk_amplitude_histogram(p_sf, options_view_spikes, p_hist0, i_spike_list, k_class);
						break;

					case EXPORT_LATENCY: 
						m_p_spk_doc->export_spk_latencies(p_sf, options_view_spikes, 10, i_spike_list, k_class);
						break;

					case EXPORT_INTERV: 
						m_p_spk_doc->export_spk_latencies(p_sf, options_view_spikes, -1, i_spike_list, k_class);
						break;

					case EXPORT_AVERAGE: 
						m_p_spk_doc->export_spk_average_wave(p_sf, options_view_spikes, p_double, i_spike_list, k_class);
						break;

					case EXPORT_PSTH: 
					case EXPORT_ISI: 
					case EXPORT_AUTOCORR: 
						m_p_spk_doc->export_spk_psth(p_sf, options_view_spikes, p_hist0, i_spike_list, k_class);
						break;

					default:
						ATLTRACE2(_T("option selected not implemented: %i /n"), options_view_spikes->export_data_type);
						break;
					}
				} // end of for: k_class
			} // end of for: spk_list

			// update progress bar
			if (MulDiv(i_file, 100, n_files) > i_step)
			{
				dlg.step_it();
				i_step = MulDiv(i_file, 100, n_files);
			}
		}

		delete[] p_hist0;
		delete[] p_double;
	}

	// transpose file
	if (transpose_)
		transpose_file_for_excel(p_sf);

	// restore initial file name and channel
	if (db_set_current_record_position(i_old_index)) 
	{
		if (open_current_spike_file() != nullptr)
			m_p_spk_doc->set_index_current_spike_list(i_old_list);
	}
	UpdateAllViews(nullptr, HINT_DOC_MOVE_RECORD, nullptr);
}

BOOL CdbWaveDoc::transpose_file_for_excel(CSharedFile * p_sf)
{
	// create dummy file on disk that duplicates the memory file
	CStdioFile data_dest; // destination file object
	const CString dummy_file1 = _T("dummyFile.txt"); // to read the clipboard
	const CString dummy_file2 = _T("dummyFile_transposed.txt"); // to store the transposed data

	CFileException fe; // trap exceptions
	if (!data_dest.Open(dummy_file1, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText,
		&fe))
	{
		data_dest.Abort(); // file not found
		return FALSE; // and return
	}
	auto ul_len_char_total = p_sf->GetLength();
	p_sf->SeekToBegin();
	auto ul_len_char = ul_len_char_total;
	char buffer[2048];
	while (ul_len_char > 0)
	{
		auto ul_len = ul_len_char;
		if (ul_len > 1024)
			ul_len = 1024;
		p_sf->Read(&buffer, static_cast<UINT>(ul_len));
		data_dest.Write(&buffer, static_cast<UINT>(ul_len));
		ul_len_char = ul_len_char - ul_len;
	}

	// create a transposed file
	CStdioFile data_transposed; // destination file object
	if (!data_transposed.Open(dummy_file2,
		CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText, &fe))
	{
		data_transposed.Abort(); // file not found
		return FALSE; // and return
	}

	// files are created, now read dataDest line by line
	data_dest.SeekToBegin();
	CString cs_read;
	// first find where the header is and copy the first lines of the table to the transposed file
	auto b_header_found = FALSE;
	ULONGLONG ul_position_header = 0;
	while (data_dest.ReadString(cs_read) && !b_header_found)
	{
		const auto ul_position_first = data_dest.GetPosition();
		if ((b_header_found = (cs_read.Find(_T("ID")) >= 0)))
			break;

		data_transposed.WriteString(cs_read);
		ul_position_header = ul_position_first;
	}
	// now: ulPositionHeader saves the file position of the table header
	//		dataTransposed is ready to get the first line of data

	// Copy data up to the header to dataTransposed
	// --------------------------------------------------------- first line of the table
	// first pass: browse through all lines from dataDest
	auto n_records = -1;
	data_dest.Seek(static_cast<LONGLONG>(ul_position_header), CFile::begin);
	CString cs_transposed;
	constexpr char c_sep = _T('\t');

	while (data_dest.ReadString(cs_read))
	{
		constexpr auto i_first = 0;
		constexpr auto i_start = 0;
		const auto i_found = cs_read.Find(_T('\t'), i_start);
		cs_transposed += cs_read.Mid(i_first, i_found - i_start) + c_sep;
		n_records++;
	}
	cs_transposed += _T('\n');
	data_transposed.WriteString(cs_transposed);

	// ---------------------------------------------------------
	// now we now how many lines are in dataDest, and we will scan line by line until all lines are exhausted
	auto col_dest = 0;
	auto n_found = n_records;

	while (n_found > 0)
	{
		data_dest.Seek(static_cast<LONGLONG>(ul_position_header), CFile::begin);
		cs_transposed.Empty();
		n_found = n_records + 1;
		col_dest++;

		for (auto i = 0; i <= n_records; i++)
		{
			data_dest.ReadString(cs_read);

			// find tab at i_col_dest position
			auto i_first_local = 0;
			for (auto j = 0; j < col_dest; j++)
			{
				i_first_local = cs_read.Find(c_sep, i_first_local + 1);
				if (i_first_local < 0) // not found: exit loop and go to next line
					break;
			}

			// write dummy space if no tab found
			if (i_first_local < 0)
			{
				cs_transposed += c_sep;
				n_found--;
				continue;
			}

			auto i_last = cs_read.Find(c_sep, i_first_local + 1);
			// extract value
			if (i_last < 0)
			{
				i_last = cs_read.GetLength();
				if (cs_read.Find(_T('\n')) > 0)
					i_last--;
				if (cs_read.Find(_T('\r')) > 0)
					i_last--;
			}

			auto cs_temp = cs_read.Mid(i_first_local, i_last - i_first_local);
			const auto test = cs_temp.Find(c_sep);
			if (test < 1)
				cs_temp = cs_read.Mid(i_first_local + 1, i_last - i_first_local - 1);
			if (cs_temp.Find(c_sep) < 0)
				cs_temp += c_sep;
			cs_transposed += cs_temp;
		}
		cs_transposed += _T('\n');
		data_transposed.WriteString(cs_transposed);
	}

	// write end-of-the-file character
	buffer[0] = 0;
	data_transposed.Write(&buffer, 1);

	// now that the transposed file has all data in the proper format,
	// we can copy it back to the clipboard
	ul_len_char_total = data_transposed.GetLength();
	data_transposed.SeekToBegin();
	ul_len_char = ul_len_char_total;

	// erase old clipboard data and create a new block of data
	p_sf->SetLength(ul_len_char_total);
	p_sf->SeekToBegin();

	while (ul_len_char > 0)
	{
		auto ul_len = ul_len_char;
		if (ul_len > 1024)
			ul_len = 1024;
		data_transposed.Read(&buffer, static_cast<UINT>(ul_len));
		p_sf->Write(&buffer, static_cast<UINT>(ul_len));
		ul_len_char -= ul_len;
	}

	// now delete the 2 dummy files
	data_transposed.Close();
	data_dest.Close();
	CFile::Remove(dummy_file1);
	CFile::Remove(dummy_file2);

	return TRUE;
}

void CdbWaveDoc::remove_row_at(CStringArray & file_name_array, const int i_row, const int n_columns, const boolean b_header)
{
	const int index = index_2d_array(i_row, n_columns, b_header);
	for (int i = n_columns - 1; i >= 0; i--)
	{
		file_name_array.RemoveAt(index + i);
	}
}

int CdbWaveDoc::check_files_can_be_opened(CStringArray & file_names_array, CSharedFile * psf, const int n_columns, const boolean b_header)
{
	// prepare progress dialog box
	auto n_files_ok = 0;
	const int n_files = get_size_2d_array(file_names_array, n_columns, b_header);
	DlgProgress dlg;
	dlg.Create();
	dlg.set_step(1);
	dlg.set_range(0, n_files);

	for (auto record_item = n_files - 1; record_item >= 0; record_item--)
	{
		// check if filename not already defined
		const int index = index_2d_array(record_item, n_columns, b_header);
		auto cs_filename = CString(file_names_array[index]);
		cs_filename.MakeLower();

		if (lstrlen(cs_filename) >= _MAX_PATH)
			continue;

		CString comment;
		comment.Format(_T("Checking file type and status on disk [%i / %i] %s"), n_files - record_item, n_files, (LPCTSTR)cs_filename);
		dlg.set_status(comment);
		dlg.step_it();

		// check if file of correct type
		CString cs_ext;
		const auto i_ext = cs_filename.ReverseFind('.');
		cs_ext = cs_filename.Right(cs_filename.GetLength() - i_ext - 1);
		const BOOL b_dat = is_extension_recognized_as_data_file(cs_ext);

		if ((!b_dat && cs_ext.Compare(_T("spk")) != 0) || (cs_filename.Find(_T("tmp.dat")) >= 0))
		{
			psf = file_discarded_message(psf, cs_filename, record_item);
			remove_row_at(file_names_array, record_item, n_columns, b_header);
			continue;
		}

		// open document and read data
		CFileStatus status;
		const BOOL flag = CFile::GetStatus(cs_filename, status);

		// GOTO next file if not possible to open the file either as a spk or a dat file
		if (!flag)
		{
			psf = file_discarded_message(psf, cs_filename, record_item);
			remove_row_at(file_names_array, record_item, n_columns, b_header);
			continue;
		}
		n_files_ok++;
	}
	return n_files_ok;
}

CSharedFile* CdbWaveDoc::file_discarded_message(CSharedFile * p_sf, const CString& cs_filename, const int i_record)
{
	CString cs_dummy;
	cs_dummy.Format(_T("file discarded=%i:\t%s\t error reading file \r\n"), i_record, (LPCTSTR)cs_filename);
	p_sf->Write(cs_dummy, cs_dummy.GetLength() * sizeof(TCHAR));
	return p_sf;
}

void CdbWaveDoc::delete_erased_files()
{
	// close current data file (no need to do that for spk file)(so far)
	if (m_p_data_doc != nullptr)
		m_p_data_doc->acq_close_file();

	const auto n_files_to_delete = names_of_files_to_delete_.GetSize() - 1;

	for (auto i = n_files_to_delete; i >= 0; i--)
	{
		// get data file name and rename file
		auto file_name = names_of_files_to_delete_.GetAt(i);
		auto file_new_name = file_name + _T("del");

		// check if same file already exists: if yes, search if up to 10 clones exist, otherwise, destroy the last clone
		CFileStatus status;
		if (CFile::GetStatus(file_new_name, status))
		{
			auto flag = TRUE;
			CString prompt;
			for (auto j = 0; j < 10; j++)
			{
				CString cs;
				prompt.Format(_T("%s%i"), (LPCTSTR)file_new_name, j);
				flag = CFile::GetStatus(prompt, status);
				if (!flag)
					break;
			}

			if (flag == TRUE)
			{
				prompt += _T("\nThis file seems to exist already.\nDelete it?");
				if (AfxMessageBox(prompt, MB_YESNO) == IDYES)
					CFile::Remove(prompt);
			}
			else
				file_new_name = prompt;
		}

		// rename file
		try
		{
			CFile::Rename(file_name, file_new_name);
		}

		catch (CFileException* p_ex)
		{
#ifdef _DEBUG
			afxDump << _T("File ") << file_name << _T(" not found, cause = ")
				<< p_ex->m_cause << _T("\n");
			switch (p_ex->m_cause)
			{
			case CFileException::fileNotFound:
				afxDump << _T("The file could not be located.");
				break;
			case CFileException::badPath:
				afxDump << _T("All or part of the path is invalid. ");
				break;
			case CFileException::tooManyOpenFiles:
				afxDump << _T("The permitted number of open files was exceeded. ");
				break;
			case CFileException::accessDenied:
				afxDump << _T("The file could not be accessed. ");
				break;
			case CFileException::invalidFile:
				afxDump << _T("There was an attempt to use an invalid file handle.  ");
				break;
			case CFileException::removeCurrentDir:
				afxDump << _T("The current working directory cannot be removed.  ");
				break;
			case CFileException::directoryFull:
				afxDump << _T("There are no more directory entries.  ");
				break;
			case CFileException::badSeek:
				afxDump << _T("There was an error trying to set the file pointer.  ");
				break;
			case CFileException::hardIO:
				afxDump << _T("There was a hardware error.  ");
				break;
			case CFileException::sharingViolation:
				afxDump << _T("SHARE.EXE was not loaded, or a shared region was locked.  ");
				break;
			case CFileException::lockViolation:
				afxDump << _T("There was an attempt to lock a region that was already locked.  ");
				break;
			case CFileException::diskFull:
				afxDump << _T("The disk is full.  ");
				break;
			case CFileException::endOfFile:
				afxDump << _T("The end of file was reached.  ");
				break;
			default: break;
			}
			afxDump << _T("\n");
#endif
			p_ex->Delete();
		}
		names_of_files_to_delete_.RemoveAt(i);
	}
}

void CdbWaveDoc::db_delete_current_record()
{
	// save data & spike file names, together with their full access path
	db_table->m_path_set.seek_id(db_table->m_main_table_set.m_path_id);
	CString cs;
	if (!db_table->m_main_table_set.m_file_dat.IsEmpty())
	{
		cs = db_table->m_path_set.m_cs + _T('\\') + db_table->m_main_table_set.m_file_dat;
		names_of_files_to_delete_.Add(cs);
	}
	if (!db_table->m_main_table_set.m_file_spk.IsEmpty())
	{
		cs = db_table->m_path_set.m_cs + _T('\\') + db_table->m_main_table_set.m_file_spk;
		names_of_files_to_delete_.Add(cs);
	}
	db_table->m_main_table_set.Delete();
	db_table->m_main_table_set.MoveNext();
}

void CdbWaveDoc::remove_duplicate_files()
{
	// loop through all files
	// discard if data acquisition the same day, same time

	DlgProgress dlg;
	dlg.Create();
	dlg.SetWindowText(_T("Scan database to discard duplicate (or missing) data files..."));

	const int n_files = db_get_records_count();
	dlg.set_range(0, n_files);
	dlg.set_step(1);

	CStringArray deleted_dat_names;
	CStringArray deleted_spk_names;
	CStringArray original_names;
	CStringArray all_names;
	CArray<CTime, CTime> o_time_array;
	deleted_dat_names.SetSize(n_files);
	deleted_spk_names.SetSize(n_files);
	original_names.SetSize(n_files);
	all_names.SetSize(n_files);
	o_time_array.SetSize(n_files);

	db_table->update_all_database_tables();
	db_table->m_main_table_set.Close();
	if (!db_table->m_main_table_set.open_table(dbOpenTable, nullptr, 0))
		return;

	// scan database to collect all file names
	dlg.set_status(_T("Collect names from database and read data acquisition times..."));

	auto nb_records_to_suppress = 0;

	if (!db_table->m_main_table_set.IsBOF())
	{
		auto index_file = 0;
		auto index_valid_records = 0;
		while (!db_table->m_main_table_set.IsEOF())
		{
			// load time for this pRecord and save bookmark
			const auto bookmark = db_table->m_main_table_set.GetBookmark();
			db_table->get_current_record_file_names();
			db_table->m_main_table_set.SetBookmark(bookmark);
			all_names.SetAt(index_valid_records, current_data_file_name_);

			// process file
			CString comment;
			comment.Format(_T("Processing file [%i / %i] %s"), index_valid_records, n_files, (LPCTSTR)current_data_file_name_);
			dlg.set_status(comment);
			if (dlg.check_cancel_button())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;

			// load file and read date and time of data acquisition
			CTime o_time_file_current = 0;
			auto b_ok = (open_current_data_file() != nullptr);
			auto i_duplicate_file = -1;
			if (b_ok)
			{
				const auto wave_format = m_p_data_doc->get_wave_format();
				o_time_file_current = wave_format->acquisition_time;
				// loop to find if current file has a duplicate in the list of previous files stored in the array
				for (auto i = 0; i < index_valid_records; i++)
				{
					auto o_time_file_i = o_time_array.GetAt(i);
					ASSERT(o_time_file_i != 0);
					if (o_time_file_current == o_time_file_i)
					{
						i_duplicate_file = i;
						b_ok = FALSE;
						break;
					}
				}
			}

			// update parameters array
			if (!b_ok) // suppress pRecord if data file not found or if duplicate
			{
				if (i_duplicate_file >= 0)
					original_names.SetAt(nb_records_to_suppress, all_names.GetAt(i_duplicate_file));
				else
					original_names.SetAt(nb_records_to_suppress, _T("(none)"));
				deleted_dat_names.SetAt(nb_records_to_suppress, current_data_file_name_);
				deleted_spk_names.SetAt(nb_records_to_suppress, current_spike_file_name_);
				db_table->m_main_table_set.Delete();
				nb_records_to_suppress++;
			}
			else // save time of this valid pRecord into array
			{
				o_time_array.SetAtGrow(index_valid_records, o_time_file_current);
				index_valid_records++;
			}

			// move to next pRecord & update dialog box
			db_table->m_main_table_set.MoveNext();
			index_file++;
			dlg.step_it();
		}
		db_table->m_main_table_set.MoveFirst();
	}

	// re-open dynaset
	db_table->m_main_table_set.Close();
	if (!db_table->m_main_table_set.open_table(dbOpenDynaset, nullptr, 0))
		return;
	db_table->m_main_table_set.Requery();

	// trace
	if (nb_records_to_suppress == 0)
		AfxMessageBox(_T("No duplicate files found"));
	else
	{
		CString cs;
		cs.Format(_T("Changes were made to the data base:\nNumber of records removed= %i\n"), nb_records_to_suppress);
		AfxMessageBox(cs);
		//---------------------------------------------
		deleted_dat_names.SetSize(nb_records_to_suppress);
		original_names.SetSize(nb_records_to_suppress);
		deleted_spk_names.SetSize(nb_records_to_suppress);

		CSharedFile sf(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
		CString cs_dummy = _T("Removed:\tOriginal:\n");
		sf.Write(cs_dummy, cs_dummy.GetLength());
		for (auto i = 0; i < nb_records_to_suppress; i++)
		{
			cs_dummy.Format(_T("%s\t%s\n"), (LPCTSTR)deleted_dat_names.GetAt(i), (LPCTSTR)original_names.GetAt(i));
			sf.Write(cs_dummy, cs_dummy.GetLength());
		}
		const auto dw_len = static_cast<DWORD>(sf.GetLength());
		auto h_mem = sf.Detach();
		if (!h_mem)
			return;
		h_mem = GlobalReAlloc(h_mem, dw_len, GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);
		if (!h_mem)
			return;

		auto p_source = new COleDataSource();
		p_source->CacheGlobalData(CF_TEXT, h_mem);
		p_source->SetClipboard();
		auto* p_app = static_cast<CdbWaveApp*>(AfxGetApp());
		CMultiDocTemplate* p_note_view_template = p_app->m_note_view_template;
		const auto pdb_doc_export = p_note_view_template->OpenDocumentFile(nullptr);
		auto pos = pdb_doc_export->GetFirstViewPosition();
		auto* p_view = static_cast<ViewNoteDoc*>(pdb_doc_export->GetNextView(pos));
		auto& p_edit = p_view->GetRichEditCtrl();
		p_edit.Paste();

		//---------------------------------------------
		const auto id_response = AfxMessageBox(_T("Do you want to erase these files from the disk?"), MB_YESNO);
		if (id_response == IDYES)
		{
			for (auto i = 0; i < nb_records_to_suppress; i++)
			{
				remove_file_from_disk(deleted_dat_names.GetAt(i));
				remove_file_from_disk(deleted_spk_names.GetAt(i));
			}
		}
	}
}

void CdbWaveDoc::remove_file_from_disk(const CString& file_name)
{
	if (!file_name.IsEmpty())
	{
		try
		{
			CFile::Remove(file_name);
		}
		catch (CFileException* p_ex)
		{
			const auto cs_local = _T("File") + file_name + _T(" %20s cannot be removed\n");
			AfxMessageBox(cs_local, MB_OK);
			p_ex->Delete();
		}
	}
}

void CdbWaveDoc::remove_missing_files() const
{
	DlgProgress dlg;
	dlg.Create();
	dlg.set_step(1);
	CString file_comment = _T("Checking database consistency - find and remove missing files: ");

	const int n_files = db_get_records_count();

	// loop through all files
	// discard if data file and spike file is missing

	// -------------------------- cancel any pending edit or add operation
	db_table->update_all_database_tables();

	db_table->m_main_table_set.Close(); // close dynaset and open as datatable
	if (!db_table->m_main_table_set.open_table(dbOpenTable, nullptr, 0))
		return;

	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	db_table->m_main_table_set.MoveFirst();

	if (!db_table->m_main_table_set.IsBOF())
	{
		auto i_file = 0;
		CString cs_dummy;
		CFileStatus status;
		CString cs_comment;
		auto i_step = 0;
		while (!db_table->m_main_table_set.IsEOF())
		{
			cs_dummy.Format(_T("%i\t"), i_file);
			if (dlg.check_cancel_button())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;

			db_table->get_current_record_file_names();
			cs_comment.Format(_T("Processing file [%i / %i] %s"), i_file, n_files, (LPCTSTR)current_data_file_name_);
			dlg.set_status(cs_comment);

			const auto b_dat_file = CFile::GetStatus(current_data_file_name_, status);
			const auto b_spk_file = CFile::GetStatus(current_spike_file_name_, status);
			if (!b_dat_file && !b_spk_file)
				db_table->m_main_table_set.Delete();

			db_table->m_main_table_set.MoveNext();

			i_file++;
			if (MulDiv(i_file, 100, n_files) > i_step)
			{
				dlg.step_it();
				i_step = MulDiv(i_file, 100, n_files);
			}
		}
		db_table->m_main_table_set.MoveFirst();
	}

	// open dynaset
	db_table->m_main_table_set.Close();
	if (!db_table->m_main_table_set.open_table(dbOpenDynaset, nullptr, 0))
		return;
	db_table->m_main_table_set.Requery();
}

void CdbWaveDoc::remove_false_spk_files() const
{
	DlgProgress dlg;
	dlg.Create();
	dlg.set_step(1);
	CString file_comment = _T("Checking database consistency - find and remove missing files: ");

	const int n_files = db_get_records_count();
	auto i_files_removed = 0;
	auto i_errors_corrected = 0;

	// loop through all files
	// discard if data file and spike file is missing

	// -------------------------- cancel any pending edit or add operation
	db_table->update_all_database_tables();

	db_table->m_main_table_set.Close(); // close dynaset and open as datatable
	if (!db_table->m_main_table_set.open_table(dbOpenTable, nullptr, 0))
		return;

	// load OleTime into array and avoid duplicating data acq file with the same A/D time
	db_table->m_main_table_set.MoveFirst();

	if (!db_table->m_main_table_set.IsBOF())
	{
		CFileStatus status;
		auto i_file = 0;
		CString cs_dummy;
		CString cs_comment;
		auto i_step = 0;
		while (!db_table->m_main_table_set.IsEOF())
		{
			cs_dummy.Format(_T("%i\t"), i_file);
			// check if user wants to stop
			if (dlg.check_cancel_button())
				if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
					break;
			db_table->get_current_record_file_names();
			cs_comment.Format(_T("Processing file [%i / %i] %s"), i_file, n_files, (LPCTSTR)current_data_file_name_);
			dlg.set_status(cs_comment);

			// check if spike file is present
			auto cs_spike_file_name = current_data_file_name_.Left(current_data_file_name_.ReverseFind('.') + 1) + _T("spk");
			const auto b_spk_file = CFile::GetStatus(cs_spike_file_name, status);

			// if spk file not found in the same directory, remove pRecord
			if (!b_spk_file)
			{
				db_table->m_main_table_set.SetFieldNull(&(db_table->m_main_table_set.m_file_spk), TRUE);
				i_files_removed++;
			}
			else
			{
				// compare the 2 file names - if different, copy data name & directory index into spike name & directory fields
				if (cs_spike_file_name.CompareNoCase(current_spike_file_name_) != 0)
				{
					cs_spike_file_name = db_table->m_main_table_set.m_file_dat.Left(
						db_table->m_main_table_set.m_file_dat.ReverseFind('.') + 1) + _T("spk");
					db_table->m_main_table_set.m_file_spk = cs_spike_file_name;
					i_errors_corrected++;
				}
			}

			db_table->m_main_table_set.MoveNext();
			i_file++;
			if (MulDiv(i_file, 100, n_files) > i_step)
			{
				dlg.step_it();
				i_step = MulDiv(i_file, 100, n_files);
			}
		}
		db_table->m_main_table_set.MoveFirst();
	}

	// open dynaset
	db_table->m_main_table_set.Close();
	if (!db_table->m_main_table_set.open_table(dbOpenDynaset, nullptr, 0))
		return;
	db_table->m_main_table_set.Requery();

	// trace
	if (i_errors_corrected == 0 && i_files_removed == 0)
		AfxMessageBox(_T("No errors found"));
	else
	{
		CString cs;
		cs.Format(
			_T("Changes were made to the data base:\nerrors corrected: %i, files removed: %i\n"), i_errors_corrected,
			i_files_removed);
		AfxMessageBox(cs);
	}
}

void CdbWaveDoc::export_datafiles_as_text_files()
{
	// save current index position - restore on exit
	const auto index_current_record = db_get_current_record_position();

	// make sure there are objects to read / write data and spike files
	if (m_p_data_doc == nullptr) // data doc
	{
		m_p_data_doc = new AcqDataDoc;
		ASSERT(m_p_data_doc != NULL);
	}

	// prepare clipboard to receive names of files exported
	auto psf = static_cast<CdbWaveApp*>(AfxGetApp())->m_psf;
	SAFE_DELETE(psf)

	static_cast<CdbWaveApp*>(AfxGetApp())->m_psf = nullptr;
	psf = new CSharedFile(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT);

	// prepare progress dialog box
	DlgProgress dlg;
	dlg.Create();
	CString cs_comment;
	const auto n_database_records = db_get_records_count();
	auto i_file = 1;
	dlg.set_range(0, n_database_records);
	CString cs_filename;

	// got to the first pRecord
	db_table->m_main_table_set.MoveFirst();
	dlg.set_pos(0);

	while (!db_table->m_main_table_set.IsEOF())
	{
		db_table->get_current_record_file_names();
		// check if user wants to stop
		if (dlg.check_cancel_button())
			if (AfxMessageBox(_T("Are you sure you want to Cancel?"), MB_YESNO) == IDYES)
				break;
		cs_comment.Format(_T("Processing file [%i / %i]"), i_file, n_database_records);
		dlg.set_status(cs_comment);

		// process data file
		if (!current_data_file_name_.IsEmpty())
		{
			// open file
			const auto p_dat = open_current_data_file();
			ASSERT(p_dat != nullptr);

			// create text file on disk with the same name as the data file_with dat
			CStdioFile data_dest; 
			auto cs_txt_file = current_data_file_name_ + _T("_.txt");
			CFileException fe; 
			if (!data_dest.Open(
				cs_txt_file, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText, &fe))
			{
				data_dest.Abort();
				continue;
			}

			// export data
			m_p_data_doc->export_data_file_to_txt_file(&data_dest);
			data_dest.Close();
			cs_txt_file += _T("\n");
			psf->Write(cs_txt_file, cs_txt_file.GetLength() * sizeof(TCHAR));
		}

		// move to next pRecord
		db_table->m_main_table_set.MoveNext();
		i_file++;
		dlg.set_pos(i_file);
	}
	db_table->m_main_table_set.MoveFirst();
	db_table->get_current_record_file_names();

	// restore current data position
	BOOL success = db_set_current_record_position(index_current_record);
	static_cast<CdbWaveApp*>(AfxGetApp())->m_psf = psf;
}
