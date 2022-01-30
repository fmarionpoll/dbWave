#include "StdAfx.h"
#include "resource.h"
#include "DlgGotoRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgGotoRecord::DlgGotoRecord(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	m_recordID = 0;
	m_recordPos = 0;
	m_bGotoRecordID = false;
}

void DlgGotoRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT2, m_recordID);
	DDX_Text(pDX, IDC_EDIT1, m_recordPos);
}

BEGIN_MESSAGE_MAP(DlgGotoRecord, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, OnClickedPosition)
	ON_BN_CLICKED(IDC_RADIO2, OnClickedID)
END_MESSAGE_MAP()

BOOL DlgGotoRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetOptions();
	return FALSE;
	//return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgGotoRecord::SetOptions()
{
	static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->SetCheck(!m_bGotoRecordID);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO2))->SetCheck(m_bGotoRecordID);
	int iselect = IDC_EDIT2;
	int idisable = IDC_EDIT1;
	if (!m_bGotoRecordID)
	{
		idisable = IDC_EDIT2;
		iselect = IDC_EDIT1;
	}
	auto p_wnd = static_cast<CEdit*>(GetDlgItem(iselect));
	p_wnd->EnableWindow(TRUE);
	p_wnd->SetFocus();
	p_wnd->SetSel(0, -1, FALSE);
	GetDlgItem(idisable)->EnableWindow(FALSE);
}

void DlgGotoRecord::OnClickedPosition()
{
	m_bGotoRecordID = FALSE;
	SetOptions();
}

void DlgGotoRecord::OnClickedID()
{
	m_bGotoRecordID = TRUE;
	SetOptions();
}
