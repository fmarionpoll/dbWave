// FindFilesDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "FindFilesDlg.h"
#include <shlobj.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CFindFilesDlg dialog


CFindFilesDlg::CFindFilesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFindFilesDlg::IDD, pParent)
	  , m_pfilenames(nullptr), m_banyformat(FALSE), m_bexcludecloud(TRUE), m_bSubtreeSearch(0)
{
	m_path = _T("");
	m_selinit = 0;
	m_ioption = 0;
	m_pdbDoc = nullptr;
	m_nfound = 0;
}

// -----------------------------------------------------------------------------

void CFindFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_fileext);
	DDX_Check(pDX, IDC_CHECKOTHERFORMATS, m_banyformat);
	DDX_Check(pDX, IDC_EXCLUDECLOUD, m_bexcludecloud);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_mfcbrowsecontrol);
}

// -----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CFindFilesDlg, CDialog)

	ON_BN_CLICKED(IDC_BUTTON2, OnSearch)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindFilesDlg message handlers

// browse directories - get root path


// -----------------------------------------------------------------------------

BOOL CFindFilesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();				// call class original routine

	m_pfilenames->RemoveAll();				// clear output CStringArray
	m_nfound = 0;							// no file found yet

	// assume user wants to explore subfolders
	((CButton*) GetDlgItem(IDC_CHECK1))->SetCheck(1);

	m_path = ((CdbWaveApp*) AfxGetApp())->ivO.path;
	if (m_pdbDoc)
		m_path = m_pdbDoc->m_ProposedDataPathName;
	m_mfcbrowsecontrol.SetWindowTextW(m_path);

	// hide yet undefined infos
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
	// update numbering option
	{
		CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();	// load browse parameters
		((CButton*) GetDlgItem(IDC_CHECKDISCARD))->SetCheck(pApp->ivO.bImportDuplicateFiles);
	}
	UpdateData(FALSE);
	m_fileext.SetCurSel(m_selinit);			// select first item / file extensions

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// -----------------------------------------------------------------------------

void CFindFilesDlg::OnOK() 
{
	if (m_pfilenames->GetSize() == 0)
		OnSearch();
	
	m_mfcbrowsecontrol.GetWindowTextW(m_path);
	CdbWaveApp* pApp = (CdbWaveApp*) AfxGetApp();
	pApp->ivO.path = m_path;
	pApp->ivO.bImportDuplicateFiles = ((CButton*) GetDlgItem(IDC_CHECKDISCARD))->GetCheck();

	CDialog::OnOK();
}

// -----------------------------------------------------------------------------

void CFindFilesDlg::OnSearch() 
{
	UpdateData(TRUE);		// update m_path

	m_ppath.RemoveAll();							// clean list of paths
	m_pfilenames->RemoveAll();						// remove all file names
	m_nfound = 0;									// reset nb of files found
	m_fileext.GetWindowText(m_searchString);		// get search string (filter)

	m_bSubtreeSearch = ((CButton*) GetDlgItem(IDC_CHECK1))->GetCheck();
	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
	m_mfcbrowsecontrol.GetWindowTextW(m_path);
	m_ppath.Add(m_path);							// add at least one path (root)
	int i=0;
	int ilast = 0;

	// scan subdirectories
	if (m_bSubtreeSearch)
		TraverseDirectory(m_path);

	// scan for files within the directories
	GetDlgItem(IDC_STATIC1)->ShowWindow(SW_SHOW);
	for (i= 0; i<= m_ppath.GetUpperBound(); i++)
	{
		CString csDir = m_ppath.GetAt(i);
		GetDlgItem(IDC_STATIC3)->SetWindowText(csDir);
		FindFiles(csDir);
		DisplaynFound();
	} 

	GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);	
}

// -----------------------------------------------------------------------------
void CFindFilesDlg::DisplaynFound()
{
	TCHAR sz[50];
	wsprintf(sz, _T("n found = %i"), m_nfound);
	GetDlgItem(IDC_STATIC1)->SetWindowText(sz);	
}

// -----------------------------------------------------------------------------
void CFindFilesDlg::TraverseDirectory (CString path)
{
	CFileFind finder;
	CString strWildcard = path;
	strWildcard += _T("\\*.*"); 

   // start working for files
   BOOL bWorking = finder.FindFile(strWildcard);
   while (bWorking)
   {
	  bWorking = finder.FindNextFile();
	  // skip . and .. files; otherwise, we'd recur infinitely!
	  if (finder.IsDots())
		 continue;
	  // if it's a directory, recursively search it
	  if (finder.IsDirectory())
	  {
		 CString str = finder.GetFilePath();
		 m_ppath.Add(str);
		 TraverseDirectory(str);
	  }
   }
}

// -----------------------------------------------------------------------------
void CFindFilesDlg::FindFiles (CString path)
{
	CFileFind finder;
	CString strWildcard = path + _T("\\") + m_searchString;
	CString csDummy;
	BOOL bWorking = finder.FindFile(strWildcard);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		csDummy = finder.GetFilePath();
		if (1 != m_ioption && 0 == (csDummy.Right(3)).CompareNoCase(_T("del")) )
			continue;
		if (m_bexcludecloud)
		{
			if (csDummy.Find(_T("\\Dropbox"), 0) >= 0)
				continue;
			if (csDummy.Find(_T("\\Google"), 0) >= 0)
				continue;
			if (csDummy.Find(_T("\\Skydrive"), 0) >= 0)
				continue;
		}
		m_pfilenames->Add(csDummy);
		m_nfound++;
	}
}
