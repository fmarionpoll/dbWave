// EditListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include "EditListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CEditListDlg dialog

IMPLEMENT_DYNAMIC(CEditListDlg, CDialog)

CEditListDlg::CEditListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEditListDlg::IDD, pParent)
	, m_csNewString(_T(""))
{

}

CEditListDlg::~CEditListDlg()
{
}

void CEditListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_csNewString);
	DDX_Control(pDX, IDC_LIST1, m_clStrings);
}

BEGIN_MESSAGE_MAP(CEditListDlg, CDialog)
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_DELETE, &CEditListDlg::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_ADDITEM, &CEditListDlg::OnBnClickedAdditem)
	ON_WM_SIZE()
//	ON_BN_CLICKED(IDC_BUTTON1, &CEditListDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

// CEditListDlg message handlers
BOOL CEditListDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int nitems = pCo->GetCount();
	CString csdummy;
	for (int i = 0; i< nitems; i++)
	{
		pCo->GetLBText(i, csdummy);		
		m_clStrings.AddString(csdummy);
	}
	return TRUE;
}

void CEditListDlg::OnOK() 
{
	m_selected = m_clStrings.GetCurSel();
	int nitems = m_clStrings.GetCount();
	CString csdummy;
	for (int i=0; i< nitems; i++)
	{
		m_clStrings.GetText(i, csdummy);
		if (!csdummy.IsEmpty())
			m_csArray.Add(csdummy);
	}
	CDialog::OnOK();
}

void CEditListDlg::OnBnClickedDelete()
{
	int ncount = m_clStrings.GetSelCount();
	if (0 == ncount)
		return;
	int *selIndex = new int [ncount];
	m_clStrings.GetSelItems(ncount, selIndex); 
	for (int i= ncount; i>0; i--)
		m_clStrings.DeleteString(selIndex[i-1]);
	delete [] selIndex;

	int nitems = m_clStrings.GetCount();
	CString csdummy;
	for (int i = nitems-1; i>= 0; i--)
	{
		m_clStrings.GetText(i, csdummy);
		if (csdummy.IsEmpty())
			m_clStrings.DeleteString(i);
	}

	UpdateData(FALSE);
}

void CEditListDlg::OnBnClickedAdditem()
{
	UpdateData(TRUE);					// get data from dlg
	if (!m_csNewString.IsEmpty())		// add string if not empty
	{
		m_clStrings.AddString(m_csNewString);
		m_csNewString.Empty();
		UpdateData(FALSE);				// update data
	}
	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void CEditListDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
}
