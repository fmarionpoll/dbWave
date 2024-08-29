#include "StdAfx.h"
#include "DlgdbNewFileDuplicate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgdbNewFileDuplicate::DlgdbNewFileDuplicate(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgdbNewFileDuplicate::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Radio(p_dx, IDC_RADIO1, m_option);
}

BEGIN_MESSAGE_MAP(DlgdbNewFileDuplicate, CDialog)
	ON_BN_CLICKED(IDC_RADIO3, on_radio3)
END_MESSAGE_MAP()

BOOL DlgdbNewFileDuplicate::OnInitDialog()
{
	CDialog::OnInitDialog();

	static_cast<CButton*>(GetDlgItem(IDC_RADIO3))->SetCheck(TRUE);

	// check if file requested is already there
	CString cs_new = m_p_file_in;
	CFileStatus status;
	const auto b_exist = CFile::GetStatus(cs_new, status);

	// decompose name
	const auto i_count = cs_new.ReverseFind('\\') + 1;
	m_cs_path_ = cs_new.Left(i_count);
	m_cs_name_ = cs_new.Right(cs_new.GetLength() - i_count);
	GetDlgItem(IDC_STATIC1)->SetWindowText(m_cs_name_);
	GetDlgItem(IDC_STATIC14)->SetWindowText(m_cs_path_);

	// if it is already there, find the root of the name (clip off any trailing numbers)
	// and iterate numbers until root+number is not found on disk
	if (b_exist)
	{
		CString cs_root; // root name of the file series

		// extract name without extension
		cs_new = m_cs_name_;
		const auto i = cs_new.ReverseFind('.');
		ASSERT(i > 0);
		if (i > 0)
		{
			m_cs_ext_ = cs_new.Right(cs_new.GetLength() - i);
			cs_root = cs_new.Left(i);
		}
		else
			cs_root = cs_new;

		// get root name without numbers
		const auto j = cs_root.FindOneOf(_T("0123456789")); // find the first numerical character
		// no numerical character is found, assume it is the first and add "1"
		if (j < 0)
			cs_new = cs_root + _T("1");

		// numerical character found - iterate until a file name is found that is not used
		else
		{
			auto cs_nb = cs_root.Right(cs_root.GetLength() - j);
			auto nb = _ttoi(cs_nb);
			cs_root = cs_root.Left(j);

			auto b_exist2 = TRUE;
			auto j_iterations = 50; // limit nb of iterations to 50
			while (b_exist2 && (j_iterations > 0))
			{
				nb++;
				cs_nb.Format(_T("%i"), nb);
				auto cs_dummy = m_cs_path_ + cs_root + cs_nb;
				cs_dummy += m_cs_ext_;
				b_exist2 = CFile::GetStatus(cs_dummy, status);
				j_iterations--;
			}
			cs_new = cs_root + cs_nb;
		}
	}

	// tentative name defined - display it and exit
	GetDlgItem(IDC_EDIT1)->SetWindowText(cs_new);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgdbNewFileDuplicate::OnOK()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_EDIT1)->GetWindowText(m_cs_name_);
	m_file_out = m_cs_path_ + m_cs_name_;
	if (!m_cs_ext_.IsEmpty())
	{
		m_file_out += m_cs_ext_;
	}
	CDialog::OnOK();
}

void DlgdbNewFileDuplicate::on_radio3() 
{
	GetDlgItem(IDC_EDIT1)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_RADIO3))->GetCheck());
}
