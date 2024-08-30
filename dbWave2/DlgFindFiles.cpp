#include "StdAfx.h"
#include "DlgFindFiles.h"
#include "dbWave.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgFindFiles::DlgFindFiles(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgFindFiles::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_COMBO1, m_file_ext);
	DDX_Check(p_dx, IDC_CHECKOTHERFORMATS, m_b_any_format);
	DDX_Control(p_dx, IDC_MFCEDITBROWSE1, m_mfc_browse_control);
}

BEGIN_MESSAGE_MAP(DlgFindFiles, CDialog)

	ON_BN_CLICKED(IDC_BUTTON2, on_search)

END_MESSAGE_MAP()

BOOL DlgFindFiles::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_p_file_names->RemoveAll();
	m_path = static_cast<CdbWaveApp*>(AfxGetApp())->options_import.path;
	if (m_pdbDoc)
		m_path = m_pdbDoc->proposed_data_path_name;
	m_mfc_browse_control.SetWindowTextW(m_path);

	// hide yet undefined infos
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(1);
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);

	// if option set to 1
	if (1 == m_i_option)
	{
		//  hide three other controls
		GetDlgItem(IDC_CHECKDISCARD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATICIMPORTOPTIONS)->ShowWindow(SW_HIDE);
		// delete items within IDC_COMBO1 and add 2 new items
		m_file_ext.ResetContent();
		m_file_ext.AddString(_T("*.datdel"));
		m_file_ext.AddString(_T("*.spkdel"));
	}
	else
	{
		auto p_app = static_cast<CdbWaveApp*>(AfxGetApp()); // load browse parameters
		static_cast<CButton*>(GetDlgItem(IDC_CHECKDISCARD))->SetCheck(p_app->options_import.discard_duplicate_files);
	}
	UpdateData(FALSE);
	m_file_ext.SetCurSel(m_sel_init); // select first item / file extensions

	return TRUE;
}

void DlgFindFiles::OnOK()
{
	if (m_p_file_names->GetSize() == 0)
		on_search();

	m_mfc_browse_control.GetWindowTextW(m_path);
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	p_app->options_import.path = m_path;
	p_app->options_import.discard_duplicate_files = static_cast<CButton*>(GetDlgItem(IDC_CHECKDISCARD))->GetCheck();

	CDialog::OnOK();
}

void DlgFindFiles::on_search()
{
	UpdateData(TRUE);

	m_p_path_.RemoveAll(); 
	m_p_file_names->RemoveAll(); 
	m_n_found = 0; 
	m_file_ext.GetWindowText(m_search_string_); 

	m_b_subtree_search_ = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
	m_mfc_browse_control.GetWindowTextW(m_path);
	m_p_path_.Add(m_path); // add at least one path (root)

	// scan subdirectories
	if (m_b_subtree_search_)
		traverse_directory(m_path);

	// scan for files within the directories
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
	for (auto i = 0; i <= m_p_path_.GetUpperBound(); i++)
	{
		CString cs_dir = m_p_path_.GetAt(i);
		GetDlgItem(IDC_STATIC3)->SetWindowText(cs_dir);
		find_files(cs_dir);
		display_n_found();
	}

	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);
}

void DlgFindFiles::display_n_found() const
{
	TCHAR sz[50];
	wsprintf(sz, _T("n found = %i"), m_n_found);
	GetDlgItem(IDC_STATIC1)->SetWindowText(sz);
}

void DlgFindFiles::traverse_directory(const CString& path)
{
	CFileFind finder;
	CString str_wildcard = path;
	str_wildcard += _T("\\*.*");

	// start working for files
	auto b_working = finder.FindFile(str_wildcard);
	while (b_working)
	{
		b_working = finder.FindNextFile();
		// skip . and .. files; otherwise, we'd recur infinitely!
		if (finder.IsDots())
			continue;
		// if it's a directory, recursively search it
		if (finder.IsDirectory())
		{
			auto str = finder.GetFilePath();
			m_p_path_.Add(str);
			traverse_directory(str);
		}
	}
}

void DlgFindFiles::find_files(const CString& path)
{
	CFileFind finder;
	const auto str_wildcard = path + _T("\\") + m_search_string_;
	auto b_working = finder.FindFile(str_wildcard);
	while (b_working)
	{
		b_working = finder.FindNextFile();
		auto cs_dummy = finder.GetFilePath();
		if (1 != m_i_option && 0 == (cs_dummy.Right(3)).CompareNoCase(_T("del")))
			continue;
		m_p_file_names->Add(cs_dummy);
		m_n_found++;
	}
}
