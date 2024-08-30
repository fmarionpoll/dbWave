#include "StdAfx.h"
#include "resource.h"
#include "DlgLoadSaveOptions.h"

#include "dbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DlgLoadSaveOptions::DlgLoadSaveOptions(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgLoadSaveOptions::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_NAMELIST, m_cb_name_list);
	DDX_Text(p_dx, IDC_COMMENT, m_ddx_comment);
}

BEGIN_MESSAGE_MAP(DlgLoadSaveOptions, CDialog)
	ON_CBN_SELCHANGE(IDC_NAMELIST, on_sel_change_name_list)
	ON_CBN_KILLFOCUS(IDC_NAMELIST, on_kill_focus_name_list)
	ON_EN_CHANGE(IDC_COMMENT, on_en_change_comment)
	ON_CBN_EDITCHANGE(IDC_NAMELIST, on_edit_change_name_list)
	ON_BN_CLICKED(IDC_LOAD, on_load)
	ON_BN_CLICKED(IDC_SAVE, on_save)
	ON_EN_KILLFOCUS(IDC_COMMENT, on_kill_focus_comment)
END_MESSAGE_MAP()



// return -1 if filename not found

int DlgLoadSaveOptions::parameter_find_string(const CString& filename) const
{
	auto i_found = -1;
	for (auto i = 0; i <= p_files.GetUpperBound(); i++)
	{
		if (filename.CompareNoCase(p_files.GetAt(i)) == 0)
		{
			i_found = i;
			break;
		}
	}
	return i_found;
}

void DlgLoadSaveOptions::update_file_list()
{
	// no action if filename was not changed
	if (!m_b_file_name_changed && m_b_comment_changed)
		return;

	// file name has changed : SAVE current parms under new name
	//                      or LOAD new file
	CString dummy; // load name from edit control
	m_cb_name_list.GetWindowText(dummy); // get content of edit control

	// check if name present into parms
	if (parameter_find_string(dummy) < 0)
	{
		m_cb_name_list.AddString(dummy); // add into listbox
		m_current_selected = m_cb_name_list.GetCount() - 1;
		p_files.InsertAt(0, dummy); // add into parmfiles
	}
	GetDlgItem(IDC_COMMENT)->GetWindowText(m_ddx_comment);
	*p_comment = m_ddx_comment;

	// SAVE parameter file?
	if (static_cast<CButton*>(GetDlgItem(IDC_SAVE))->GetCheck())
		static_cast<CdbWaveApp*>(AfxGetApp())->archive_parameter_files(dummy, FALSE);
		// b_read=FALSE

		// LOAD parameter file?
	else if (!static_cast<CdbWaveApp*>(AfxGetApp())->archive_parameter_files(dummy, TRUE))
		// b_read=TRUE
		AfxMessageBox(_T("Parameter file not found!"), MB_ICONSTOP | MB_OK);

	m_b_comment_changed = FALSE;
	m_b_file_name_changed = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// DlgLoadSaveOptions message handlers

BOOL DlgLoadSaveOptions::OnInitDialog()
{
	CDialog::OnInitDialog();
	// get pointers to variables used in this dialog box
	p_parameter_files = &(static_cast<CdbWaveApp*>(AfxGetApp())->m_cs_parameter_files);
	p_comment = &(static_cast<CdbWaveApp*>(AfxGetApp())->m_comment);

	// load data into combobox
	for (int i = 0; i < p_parameter_files->GetSize(); i++) // browse through array
	{
		auto dummy = p_parameter_files->GetAt(i); // get string
		p_files.Add(dummy);
		m_cb_name_list.AddString(dummy); // add string to list box of combo box
	}
	m_current_file = p_files.GetAt(0); // save current file name

	m_current_selected = 0; // select first item
	m_cb_name_list.SetCurSel(m_current_selected); // of the combo box

	// load comment
	m_ddx_comment = *p_comment;

	// init parms to load
	static_cast<CButton*>(GetDlgItem(IDC_LOAD))->SetCheck(TRUE);
	on_load();
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------

void DlgLoadSaveOptions::OnOK()
{
	// save data from current selected parm file
	CString dummy;
	m_cb_name_list.GetWindowText(dummy); // get content of edit control
	auto i_file = parameter_find_string(dummy); // find corresponding file within parms
	if (i_file < 0)
	{
		update_file_list();
		i_file = parameter_find_string(dummy);
	}

	if (i_file > 0)
	{
		p_files.RemoveAt(i_file);
		p_files.InsertAt(0, dummy);
	}
	CDialog::OnOK();
}

void DlgLoadSaveOptions::OnCancel()
{
	static_cast<CdbWaveApp*>(AfxGetApp())->archive_parameter_files(m_current_file, TRUE); // b_read=TRUE
	CDialog::OnCancel();
}

void DlgLoadSaveOptions::on_load()
{
	static_cast<CEdit*>(GetDlgItem(IDC_COMMENT))->SetReadOnly(TRUE);
}

void DlgLoadSaveOptions::on_save()
{
	static_cast<CEdit*>(GetDlgItem(IDC_COMMENT))->SetReadOnly(FALSE);
}


void DlgLoadSaveOptions::on_en_change_comment()
{
	m_b_comment_changed = TRUE;
}

void DlgLoadSaveOptions::on_kill_focus_comment()
{
	UpdateData(TRUE);
}

void DlgLoadSaveOptions::on_edit_change_name_list()
{
	m_b_file_name_changed = TRUE;
}

void DlgLoadSaveOptions::on_sel_change_name_list()
{
	// save current set of parameters if
	if (m_b_comment_changed && static_cast<CButton*>(GetDlgItem(IDC_SAVE))->GetCheck())
	{
		CString dummy;
		m_cb_name_list.GetLBText(m_current_selected, dummy);
		GetDlgItem(IDC_COMMENT)->GetWindowText(m_ddx_comment);
		*p_comment = m_ddx_comment;
		static_cast<CdbWaveApp*>(AfxGetApp())->archive_parameter_files(dummy, FALSE); // b_read=FALSE
		m_b_comment_changed = FALSE;
	}

	update_file_list();

	// load new parameters
	CString dummy;
	m_current_selected = m_cb_name_list.GetCurSel();
	m_cb_name_list.GetLBText(m_current_selected, dummy);
	static_cast<CdbWaveApp*>(AfxGetApp())->archive_parameter_files(dummy, TRUE); // b_read=TRUE
	m_ddx_comment = *p_comment; // load comment into string
	UpdateData(FALSE); // display changes
}

void DlgLoadSaveOptions::on_kill_focus_name_list()
{
	update_file_list();
}
