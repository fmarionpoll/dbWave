// TransferFilesDlg.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "resource.h"
#include "TransferFilesDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTransferFilesDlg dialog

IMPLEMENT_DYNAMIC(CTransferFilesDlg, CDialogEx)

CTransferFilesDlg::CTransferFilesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTransferFilesDlg::IDD, pParent)
	, m_csPathname(_T(""))
{
}

CTransferFilesDlg::~CTransferFilesDlg()
{
}

void CTransferFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_csPathname);
}

BEGIN_MESSAGE_MAP(CTransferFilesDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTONPATH, &CTransferFilesDlg::OnBnClickedButtonpath)
END_MESSAGE_MAP()

// CTransferFilesDlg message handlers
TCHAR sz_dir3[MAX_PATH];

int CALLBACK BrowseCallbackProc3(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM p_data)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(sz_dir3));
		break;
	default:
		break;
	}
	return 0;
}

void CTransferFilesDlg::OnBnClickedButtonpath()
{
	UpdateData(TRUE);
	lstrcpy(sz_dir3, m_csPathname);

	LPMALLOC g_pMalloc;					// special alloc mode (shell)
	 // Get the shell's allocator.
	if (!SUCCEEDED(SHGetMalloc(&g_pMalloc)))
		return; // 1;

	BROWSEINFO bi;						// parameter array
	LPTSTR lp_buffer;					// buffer

	// Allocate a buffer to receive browse information.
	if ((lp_buffer = static_cast<LPTSTR>(g_pMalloc->Alloc(MAX_PATH))) == nullptr)
		return;

	// Fill in the BROWSEINFO structure.
	bi.hwndOwner = GetSafeHwnd();		// owner of the dlg is this dlg box
	bi.pidlRoot = nullptr;				//pidlPrograms; ?
	bi.lpfn = BrowseCallbackProc3;
	bi.pszDisplayName = lp_buffer;		// output buffer
	bi.lpszTitle = _T("Choose a Folder");
	bi.ulFlags = BIF_USENEWUI;			// see doc for flags
	bi.lParam = 0;

	// Browse for a folder and return its PIDL.
	const auto pidl_browse = SHBrowseForFolder(&bi);
	if (pidl_browse != nullptr)			// check if a folder was chosen
	{
		// Show the display name, title, and file system path.
		if (SHGetPathFromIDList(pidl_browse, lp_buffer))
		{
			m_csPathname = lp_buffer;	// load folder name
			if (m_csPathname.Right(1) != _T("\\"))
				m_csPathname += _T("\\");
		}
		// Free the PIDL returned by SHBrowseForFolder.
		g_pMalloc->Free(pidl_browse);
	}

	// Clean up.
	g_pMalloc->Free(lp_buffer);
	UpdateData(FALSE);
}