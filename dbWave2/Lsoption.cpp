// lsoptions : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "Lsoption.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoadSaveOptionsDlg dialog


CLoadSaveOptionsDlg::CLoadSaveOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoadSaveOptionsDlg::IDD, pParent)
{
	m_ddxcomment = _T("");
	pParmFiles= nullptr;
	pcomment= nullptr;
	m_bfilenamechanged=FALSE;		// file name was modified
	m_bcommentchanged=FALSE;		// comment was modified
	m_cursel=0;
}


void CLoadSaveOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NAMELIST, m_CBnamelist);
	DDX_Text(pDX, IDC_COMMENT, m_ddxcomment);
}

BEGIN_MESSAGE_MAP(CLoadSaveOptionsDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_NAMELIST, OnSelchangeNamelist)
	ON_CBN_KILLFOCUS(IDC_NAMELIST, OnKillfocusNamelist)
	ON_EN_CHANGE(IDC_COMMENT, OnEnChangeComment)
	ON_CBN_EDITCHANGE(IDC_NAMELIST, OnEditchangeNamelist)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_EN_KILLFOCUS(IDC_COMMENT, OnKillfocusComment)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// CLoadSaveOptionsDlg protected procedures

// ParmFindString() -- find parm
// return -1 if filename not found
int	CLoadSaveOptionsDlg::ParmFindString(CString& filename)
{
	auto ifound=-1;
	for (auto i=0; i<= pFiles.GetUpperBound(); i++)
	{
		if (filename.CompareNoCase(pFiles.GetAt(i)) == 0)
		{
			ifound=i;
			break;
		}
	}
	return ifound;
}

void CLoadSaveOptionsDlg::UpdateFileList()
{
	// no action if filename was not changed
	if (!m_bfilenamechanged && m_bcommentchanged)	
		return;
		
	// file name has changed : SAVE current parms under new name
	//                      or LOAD new file
	CString dummy;								// load name from edit control	
	m_CBnamelist.GetWindowText(dummy);			// get content of edit control

	// check if name present into parms
	if (ParmFindString(dummy) <0)
	{
		m_CBnamelist.AddString(dummy);		// add into listbox
		m_cursel = m_CBnamelist.GetCount()-1;
		pFiles.InsertAt(0, dummy);			// add into parmfiles
	}
	((CEdit*)GetDlgItem(IDC_COMMENT))->GetWindowText(m_ddxcomment);
	*pcomment = m_ddxcomment;

	// SAVE parameter file?
	if (((CButton*)GetDlgItem(IDC_SAVE))->GetCheck())
		((CdbWaveApp*) AfxGetApp())->ParmFile(dummy, FALSE);
		// b_read=FALSE		

	// LOAD parameter file?
	else
		if (!((CdbWaveApp*) AfxGetApp())->ParmFile(dummy, TRUE))
			// b_read=TRUE
			AfxMessageBox(_T("Parameter file not found!"), MB_ICONSTOP | MB_OK);		

	m_bcommentchanged=FALSE;
	m_bfilenamechanged=FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CLoadSaveOptionsDlg message handlers


BOOL CLoadSaveOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// get pointers to variables used in this dialog box
	pParmFiles=&(((CdbWaveApp*) AfxGetApp())->m_csParmFiles);
	pcomment=&(((CdbWaveApp*) AfxGetApp())->m_comment);
	
	// load data into combobox
	for (int i=0; i<pParmFiles->GetSize(); i++)	// browse through array
	{
		auto dummy = pParmFiles->GetAt(i);	// get string
		pFiles.Add(dummy);
		m_CBnamelist.AddString(dummy);		// add string to list box of combo box
	}
	m_currentFile = pFiles.GetAt(0);		// save current file name

	m_cursel=0;								// select first item
	m_CBnamelist.SetCurSel(m_cursel);		// of the combo box

	// load comment
	m_ddxcomment = *pcomment;

	// init parms to load 
	((CButton*)GetDlgItem(IDC_LOAD))->SetCheck(TRUE);
	OnLoad();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// ------------------------------------------------------------------

void CLoadSaveOptionsDlg::OnOK() 
{
	// save data from current selected parm file
	CString dummy;
	m_CBnamelist.GetWindowText(dummy);			// get content of edit control
	auto ifile = ParmFindString(dummy);			// find corresp file within parms
	if (ifile < 0)
	{
		UpdateFileList();
		ifile = ParmFindString(dummy);
	}

	if (ifile > 0)
	{
		pFiles.RemoveAt(ifile);	
		pFiles.InsertAt(0, dummy);
	}
	CDialog::OnOK();	
}


void CLoadSaveOptionsDlg::OnCancel() 
{
	((CdbWaveApp*) AfxGetApp())->ParmFile(m_currentFile, TRUE);	// b_read=TRUE
	CDialog::OnCancel();	
}

// -------------------------------------------------------------------------------

void CLoadSaveOptionsDlg::OnLoad() 
{
	((CEdit*)GetDlgItem(IDC_COMMENT))->SetReadOnly(TRUE);
}

void CLoadSaveOptionsDlg::OnSave() 
{
	((CEdit*)GetDlgItem(IDC_COMMENT))->SetReadOnly(FALSE);
}

///////////////////////////////////////////////////////////////////////
// comment related operations and messages

void CLoadSaveOptionsDlg::OnEnChangeComment() 
{
	m_bcommentchanged=TRUE;	
}

void CLoadSaveOptionsDlg::OnKillfocusComment() 
{
	UpdateData(TRUE);
}

///////////////////////////////////////////////////////////////////////
// combo box operations and messages

void CLoadSaveOptionsDlg::OnEditchangeNamelist() 
{
	m_bfilenamechanged=TRUE;	
}

void CLoadSaveOptionsDlg::OnSelchangeNamelist() 
{	
	
	// save current set of parameters if 
	if (m_bcommentchanged && ((CButton*)GetDlgItem(IDC_SAVE))->GetCheck())
	{		
		CString dummy;
		m_CBnamelist.GetLBText(m_cursel, dummy);		
		((CEdit*)GetDlgItem(IDC_COMMENT))->GetWindowText(m_ddxcomment);
		*pcomment = m_ddxcomment;
		((CdbWaveApp*) AfxGetApp())->ParmFile(dummy, FALSE); // b_read=FALSE
		m_bcommentchanged=FALSE;
	}

	UpdateFileList();

	// load new parameters
	CString dummy;
	m_cursel = m_CBnamelist.GetCurSel();
	m_CBnamelist.GetLBText(m_cursel, dummy);
	((CdbWaveApp*) AfxGetApp())->ParmFile(dummy, TRUE);	// b_read=TRUE
	m_ddxcomment = *pcomment;		// load comment into string
	UpdateData(FALSE);				// display changes
}

void CLoadSaveOptionsDlg::OnKillfocusNamelist() 
{	
	UpdateFileList();
}
