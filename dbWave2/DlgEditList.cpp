#include "StdAfx.h"
#include "DlgEditList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(DlgEditList, CDialog)

DlgEditList::DlgEditList(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

DlgEditList::~DlgEditList()
= default;

void DlgEditList::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_EDIT1, m_cs_new_string);
	DDX_Control(p_dx, IDC_LIST1, m_cl_strings);
}

BEGIN_MESSAGE_MAP(DlgEditList, CDialog)
	ON_WM_INITMENU()
	ON_BN_CLICKED(IDC_DELETE, &DlgEditList::on_bn_clicked_delete)
	ON_BN_CLICKED(IDC_ADDITEM, &DlgEditList::on_bn_clicked_add_item)
	ON_WM_SIZE()
	//	ON_BN_CLICKED(IDC_BUTTON1, &CEditListDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

BOOL DlgEditList::OnInitDialog()
{
	CDialog::OnInitDialog();

	const auto nitems = p_co->GetCount();
	CString cs_dummy;
	for (auto i = 0; i < nitems; i++)
	{
		p_co->GetLBText(i, cs_dummy);
		m_cl_strings.AddString(cs_dummy);
	}
	return TRUE;
}

void DlgEditList::OnOK()
{
	m_selected = m_cl_strings.GetCurSel();
	const auto n_items = m_cl_strings.GetCount();
	CString cs_dummy;
	for (auto i = 0; i < n_items; i++)
	{
		m_cl_strings.GetText(i, cs_dummy);
		if (!cs_dummy.IsEmpty())
			m_cs_array.Add(cs_dummy);
	}
	CDialog::OnOK();
}

void DlgEditList::on_bn_clicked_delete()
{
	const auto n_count = m_cl_strings.GetSelCount();
	if (0 == n_count)
		return;
	const auto sel_index = new int[n_count];
	m_cl_strings.GetSelItems(n_count, sel_index);

	for (auto i = n_count; i > 0; i--)
		m_cl_strings.DeleteString(sel_index[i - 1]);
	delete[] sel_index;

	const auto n_items = m_cl_strings.GetCount();
	CString cs_dummy;
	for (auto i = n_items - 1; i >= 0; i--)
	{
		m_cl_strings.GetText(i, cs_dummy);
		if (cs_dummy.IsEmpty())
			m_cl_strings.DeleteString(i);
	}

	UpdateData(FALSE);
}

void DlgEditList::on_bn_clicked_add_item()
{
	UpdateData(TRUE); 
	if (!m_cs_new_string.IsEmpty()) 
	{
		m_cl_strings.AddString(m_cs_new_string);
		m_cs_new_string.Empty();
		UpdateData(FALSE);
	}
	GetDlgItem(IDC_EDIT1)->SetFocus();
}

void DlgEditList::OnSize(const UINT n_type, const int cx, const int cy)
{
	CDialog::OnSize(n_type, cx, cy);
}
