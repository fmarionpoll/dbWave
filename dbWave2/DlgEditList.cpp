#include "StdAfx.h"
#include "dbWave.h"
#include "DlgEditList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgEditList, CDialog)

CDlgEditList::CDlgEditList(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
	  , m_csNewString(_T("")), m_selected(0)
{
	pCo = nullptr;
}

CDlgEditList::~CDlgEditList()
{
}

void CDlgEditList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_csNewString);
	DDX_Control(pDX, IDC_LIST1, m_clStrings);
}

BEGIN_MESSAGE_MAP(CDlgEditList, CDialog)
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_DELETE, &CDlgEditList::OnBnClickedDelete)
	ON_BN_CLICKED(IDC_ADDITEM, &CDlgEditList::OnBnClickedAdditem)
	ON_WM_SIZE()
	//	ON_BN_CLICKED(IDC_BUTTON1, &CEditListDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

BOOL CDlgEditList::OnInitDialog()
{
	CDialog::OnInitDialog();

	const auto nitems = pCo->GetCount();
	CString csdummy;
	for (auto i = 0; i < nitems; i++)
	{
		pCo->GetLBText(i, csdummy);
		m_clStrings.AddString(csdummy);
	}
	return TRUE;
}

void CDlgEditList::OnOK()
{
	m_selected = m_clStrings.GetCurSel();
	const auto nitems = m_clStrings.GetCount();
	CString csdummy;
	for (auto i = 0; i < nitems; i++)
	{
		m_clStrings.GetText(i, csdummy);
		if (!csdummy.IsEmpty())
			m_csArray.Add(csdummy);
	}
	CDialog::OnOK();
}

void CDlgEditList::OnBnClickedDelete()
{
	const auto ncount = m_clStrings.GetSelCount();
	if (0 == ncount)
		return;
	const auto sel_index = new int[ncount];
	m_clStrings.GetSelItems(ncount, sel_index);

	for (auto i = ncount; i > 0; i--)
		m_clStrings.DeleteString(sel_index[i - 1]);
	delete[] sel_index;

	const auto nitems = m_clStrings.GetCount();
	CString csdummy;
	for (auto i = nitems - 1; i >= 0; i--)
	{
		m_clStrings.GetText(i, csdummy);
		if (csdummy.IsEmpty())
			m_clStrings.DeleteString(i);
	}

	UpdateData(FALSE);
}

void CDlgEditList::OnBnClickedAdditem()
{
	UpdateData(TRUE); // get data from dlg
	if (!m_csNewString.IsEmpty()) // add string if not empty
	{
		m_clStrings.AddString(m_csNewString);
		m_csNewString.Empty();
		UpdateData(FALSE); // update data
	}
	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void CDlgEditList::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
}
