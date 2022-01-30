#include "StdAfx.h"
#include "DlgFindFiles.h"
#include "dbWave.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgFindFiles::DlgFindFiles(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgFindFiles::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_fileext);
	DDX_Check(pDX, IDC_CHECKOTHERFORMATS, m_banyformat);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_mfcbrowsecontrol);
}

BEGIN_MESSAGE_MAP(DlgFindFiles, CDialog)

	ON_BN_CLICKED(IDC_BUTTON2, OnSearch)

END_MESSAGE_MAP()

BOOL DlgFindFiles::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_pfilenames->RemoveAll();
	m_path = static_cast<CdbWaveApp*>(AfxGetApp())->options_import.path;
	if (m_pdbDoc)
		m_path = m_pdbDoc->m_ProposedDataPathName;
	m_mfcbrowsecontrol.SetWindowTextW(m_path);

	// hide yet undefined infos
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(1);
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);

	// if option set to 1
	if (1 == m_ioption)
	{
		//  hide three other controls
		GetDlgItem(IDC_CHECKDISCARD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATICIMPORTOPTIONS)->ShowWindow(SW_HIDE);
		// delete items within IDC_COMBO1 and add 2 new items
		m_fileext.ResetContent();
		m_fileext.AddString(_T("*.datdel"));
		m_fileext.AddString(_T("*.spkdel"));
	}
	else
	{
		auto p_app = static_cast<CdbWaveApp*>(AfxGetApp()); // load browse parameters
		static_cast<CButton*>(GetDlgItem(IDC_CHECKDISCARD))->SetCheck(p_app->options_import.bDiscardDuplicateFiles);
	}
	UpdateData(FALSE);
	m_fileext.SetCurSel(m_selinit); // select first item / file extensions

	return TRUE;
}

void DlgFindFiles::OnOK()
{
	if (m_pfilenames->GetSize() == 0)
		OnSearch();

	m_mfcbrowsecontrol.GetWindowTextW(m_path);
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	p_app->options_import.path = m_path;
	p_app->options_import.bDiscardDuplicateFiles = static_cast<CButton*>(GetDlgItem(IDC_CHECKDISCARD))->GetCheck();

	CDialog::OnOK();
}

void DlgFindFiles::OnSearch()
{
	UpdateData(TRUE);

	m_ppath.RemoveAll(); // clean list of paths
	m_pfilenames->RemoveAll(); // remove all file names
	m_nfound = 0; // reset nb of files found
	m_fileext.GetWindowText(m_searchString); // get search string (filter)

	m_bSubtreeSearch = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
	m_mfcbrowsecontrol.GetWindowTextW(m_path);
	m_ppath.Add(m_path); // add at least one path (root)

	// scan subdirectories
	if (m_bSubtreeSearch)
		TraverseDirectory(m_path);

	// scan for files within the directories
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
	for (auto i = 0; i <= m_ppath.GetUpperBound(); i++)
	{
		CString cs_dir = m_ppath.GetAt(i);
		GetDlgItem(IDC_STATIC3)->SetWindowText(cs_dir);
		FindFiles(cs_dir);
		DisplaynFound();
	}

	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);
}

void DlgFindFiles::DisplaynFound()
{
	TCHAR sz[50];
	wsprintf(sz, _T("n found = %i"), m_nfound);
	GetDlgItem(IDC_STATIC1)->SetWindowText(sz);
}

void DlgFindFiles::TraverseDirectory(CString path)
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
			m_ppath.Add(str);
			TraverseDirectory(str);
		}
	}
}

void DlgFindFiles::FindFiles(CString path)
{
	CFileFind finder;
	const auto str_wildcard = path + _T("\\") + m_searchString;
	auto b_working = finder.FindFile(str_wildcard);
	while (b_working)
	{
		b_working = finder.FindNextFile();
		auto cs_dummy = finder.GetFilePath();
		if (1 != m_ioption && 0 == (cs_dummy.Right(3)).CompareNoCase(_T("del")))
			continue;
		m_pfilenames->Add(cs_dummy);
		m_nfound++;
	}
}
