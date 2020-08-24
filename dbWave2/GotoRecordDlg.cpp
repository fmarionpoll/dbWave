// GotoRecordDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "GotoRecordDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CGotoRecordDlg dialog

CGotoRecordDlg::CGotoRecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoRecordDlg::IDD, pParent)
{
	m_recordID = 0;
	m_recordPos = 0;
	m_bGotoRecordID = false;
}

void CGotoRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_recordID);
	DDX_Text(pDX, IDC_EDIT1, m_recordPos);
}

BEGIN_MESSAGE_MAP(CGotoRecordDlg, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, OnClickedPosition)
	ON_BN_CLICKED(IDC_RADIO2, OnClickedID)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGotoRecordDlg message handlers

BOOL CGotoRecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetOptions();
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CGotoRecordDlg::SetOptions()
{
	((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(!m_bGotoRecordID);
	((CButton*)GetDlgItem(IDC_RADIO2))->SetCheck(m_bGotoRecordID);
	int iselect = IDC_EDIT2;
	int idisable = IDC_EDIT1;
	if (!m_bGotoRecordID)
	{
		idisable = IDC_EDIT2;
		iselect = IDC_EDIT1;
	}
	CEdit* p_wnd = (CEdit*)GetDlgItem(iselect);
	p_wnd->EnableWindow(TRUE);
	p_wnd->SetFocus();
	p_wnd->SetSel(0, -1, FALSE);
	GetDlgItem(idisable)->EnableWindow(FALSE);
}

void CGotoRecordDlg::OnClickedPosition()
{
	m_bGotoRecordID = FALSE;
	SetOptions();
}

void CGotoRecordDlg::OnClickedID()
{
	m_bGotoRecordID = TRUE;
	SetOptions();
}