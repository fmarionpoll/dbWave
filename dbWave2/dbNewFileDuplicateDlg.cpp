// dbNewFileDuplicateDlg.cpp : implementation file

#include "StdAfx.h"
#include "DlgdbNewFileDuplicate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CdbNewFileDuplicateDlg dialog

CdbNewFileDuplicateDlg::CdbNewFileDuplicateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CdbNewFileDuplicateDlg::IDD, pParent)
{
	m_option = -1;
	m_pfilein = nullptr;
	m_csExt.Empty();
}

void CdbNewFileDuplicateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO1, m_option);
}

BEGIN_MESSAGE_MAP(CdbNewFileDuplicateDlg, CDialog)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CdbNewFileDuplicateDlg message handlers

BOOL CdbNewFileDuplicateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CButton*)GetDlgItem(IDC_RADIO3))->SetCheck(TRUE);

	// check if file requested is already there
	CString csnew = m_pfilein;
	CFileStatus status;
	const auto b_exist = CFile::GetStatus(csnew, status);

	// decompose name
	const auto icount = csnew.ReverseFind('\\') + 1;
	m_csPath = csnew.Left(icount);
	m_csName = csnew.Right(csnew.GetLength() - icount);
	GetDlgItem(IDC_STATIC1)->SetWindowText(m_csName);
	GetDlgItem(IDC_STATIC14)->SetWindowText(m_csPath);

	// if it is already there, find the root of the name (clip off any trailing numbers)
	// and iterate numbers until root+number is not found on disk
	if (b_exist)
	{
		CString cs_root;	// root name of the file series

		// extract name without extension
		csnew = m_csName;
		const auto i = csnew.ReverseFind('.');
		ASSERT(i > 0);
		if (i > 0)
		{
			m_csExt = csnew.Right(csnew.GetLength() - i);
			cs_root = csnew.Left(i);
		}
		else
			cs_root = csnew;

		// get root name without numbers
		const auto j = cs_root.FindOneOf(_T("0123456789"));	// find the first numerical character
		// no numerical character is found, assume it is the first and add "1"
		if (j < 0)
			csnew = cs_root + _T("1");

		// numerical character found - iterate until a file name is found that is not used
		else
		{
			auto csnb = cs_root.Right(cs_root.GetLength() - j);
			auto nb = _ttoi(csnb);
			cs_root = cs_root.Left(j);

			auto b_exist2 = TRUE;
			auto jiterations = 50;		// limit nb of iterations to 50
			while (b_exist2 && (jiterations > 0))
			{
				nb++;
				csnb.Format(_T("%i"), nb);
				auto csdummy = m_csPath + cs_root + csnb;
				csdummy += m_csExt;
				b_exist2 = CFile::GetStatus(csdummy, status);
				jiterations--;
			}
			csnew = cs_root + csnb;
		}
	}

	// tentative name defined - display it and exit
	GetDlgItem(IDC_EDIT1)->SetWindowText(csnew);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CdbNewFileDuplicateDlg::OnOK()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT1)->GetWindowText(m_csName);
	m_fileout = m_csPath + m_csName;
	if (!m_csExt.IsEmpty())
	{
		m_fileout += m_csExt;
	}
	CDialog::OnOK();
}

void CdbNewFileDuplicateDlg::OnRadio3()
{
	GetDlgItem(IDC_EDIT1)->EnableWindow(((CButton*)GetDlgItem(IDC_RADIO3))->GetCheck());
}