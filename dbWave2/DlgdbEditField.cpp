#include "StdAfx.h"

#include "dbWaveDoc.h"
#include "DlgdbEditField.h"
#include "afxdialogex.h"
#include "DlgEditList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgdbEditField, CDialogEx)

DlgdbEditField::DlgdbEditField(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD, pParent)
	
{
}

DlgdbEditField::~DlgdbEditField()
{
}

void DlgdbEditField::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_cs_field_value);
	DDX_Text(pDX, IDC_EDIT2, m_cs_text_search);
	DDX_Text(pDX, IDC_EDIT3, m_cs_text_replace_with);
	DDX_Control(pDX, IDC_COMBO1, m_co_dictionary);
	DDX_Control(pDX, IDC_COMBO3, m_co_source);
	DDX_Check(pDX, IDC_CHECKCASESENSITIV, m_b_case_sensitive);
}

BEGIN_MESSAGE_MAP(DlgdbEditField, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO1, &DlgdbEditField::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &DlgdbEditField::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &DlgdbEditField::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &DlgdbEditField::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, &DlgdbEditField::OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, &DlgdbEditField::OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_BUTTON1, &DlgdbEditField::OnBnClickedButton1)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDOK, &DlgdbEditField::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO3, &DlgdbEditField::OnCbnSelchangeCombo3)
END_MESSAGE_MAP()

BOOL DlgdbEditField::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// disable items not used here and enable only numeric input into edit boxes
	if (m_pIndexTable == nullptr)
		m_bIndexTable = FALSE;
	else
		m_bIndexTable = TRUE;
	if (!m_bIndexTable)
	{
		GetDlgItem(IDC_EDIT1)->ModifyStyle(NULL, ES_NUMBER);
		GetDlgItem(IDC_EDIT2)->ModifyStyle(NULL, ES_NUMBER);
		GetDlgItem(IDC_EDIT3)->ModifyStyle(NULL, ES_NUMBER);
		GetDlgItem(IDC_CHECKCASESENSITIV)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
		m_co_dictionary.ModifyStyle(LBS_SORT, NULL);
	}

	// Add extra initialization here
	static_cast<CButton*>(GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
	static_cast<CButton*>(GetDlgItem(IDC_RADIO4))->SetCheck(BST_CHECKED);
	display_elements();

	// fill source type
	m_co_source.SetCurSel(1); // only the current record

	// load source value from the main table
	COleVariant var_value;
	m_pMainTable->GetFieldValue(m_csColName, var_value);
	if (var_value.vt != VT_NULL)
	{
		m_initial_id_ = var_value.lVal;
		if (m_bIndexTable)
			m_cs_field_value = m_pIndexTable->get_string_from_id(var_value.lVal);
		else
			m_cs_field_value.Format(_T("%i"), var_value.lVal);
	}
	else
		m_cs_field_value.Empty(); // = _T("undefined");

	// linked field: fill CComboBox with content of linked table
	m_co_dictionary.ResetContent();
	if (m_bIndexTable)
	{
		if (m_pIndexTable->IsOpen() && !m_pIndexTable->IsBOF())
		{
			COleVariant var_value0, var_value1;
			m_pIndexTable->MoveFirst();
			while (!m_pIndexTable->IsEOF())
			{
				m_pIndexTable->GetFieldValue(0, var_value0);
				m_pIndexTable->GetFieldValue(1, var_value1);
				CString cs_dummy = var_value0.bstrVal;
				const auto i = m_co_dictionary.AddString(cs_dummy);
				m_co_dictionary.SetItemData(i, var_value1.lVal);
				m_pIndexTable->MoveNext();
			}
		}
	}
	// no linked field: fill CComboBox with uiArray stored into document file
	else
	{
		auto bookmark_current = m_pMainTable->GetBookmark();
		for (auto i = 0; i <= m_pliIDArray->GetUpperBound(); i++)
		{
			CString cs;
			const auto uc_id = m_pliIDArray->GetAt(i);
			cs.Format(_T("%i"), uc_id);
			auto j = m_co_dictionary.FindStringExact(0, cs);
			if (j == CB_ERR)
			{
				const auto k = m_co_dictionary.InsertString(i, cs);
				ASSERT(k != CB_ERR);
				m_co_dictionary.SetItemData(k, uc_id);
			}
		}
	}
	// select value in combobox
	int i_select = 0;
	if (!m_cs_field_value.IsEmpty())
		i_select = m_co_dictionary.FindStringExact(0, m_cs_field_value);
	m_co_dictionary.SetCurSel(i_select);

	UpdateData(FALSE);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgdbEditField::display_elements() const
{
	GetDlgItem(IDC_EDIT1)->EnableWindow(m_source_condition == COND_EQU);
	GetDlgItem(IDC_EDIT2)->EnableWindow(m_source_condition == COND_SEARCH);
	GetDlgItem(IDC_CHECKCASESENSITIV)->EnableWindow(m_source_condition == COND_SEARCH);

	GetDlgItem(IDC_COMBO1)->EnableWindow(m_dest_action == CHGE_ID);
	//if (m_bIndexTable)
	GetDlgItem(IDC_BUTTON1)->EnableWindow(m_dest_action == CHGE_ID);
	GetDlgItem(IDC_EDIT3)->EnableWindow(m_dest_action == CHGE_TXT);
}

void DlgdbEditField::OnCbnSelchangeCombo3()
{
	m_source_select = m_co_source.GetCurSel();
	display_elements();
}

void DlgdbEditField::OnBnClickedRadio1()
{
	m_source_condition = COND_EQU;
	display_elements();
}

void DlgdbEditField::OnBnClickedRadio2()
{
	m_source_condition = COND_SEARCH;
	display_elements();
}

void DlgdbEditField::OnBnClickedRadio3()
{
	m_source_condition = COND_NONE;
	display_elements();
}

void DlgdbEditField::OnBnClickedRadio4()
{
	m_dest_action = CHGE_ID;
	display_elements();
}

void DlgdbEditField::OnBnClickedRadio5()
{
	m_dest_action = CHGE_TXT;
	display_elements();
}

void DlgdbEditField::OnBnClickedRadio6()
{
	m_dest_action = CHGE_CLEAR;
	display_elements();
}

void DlgdbEditField::OnBnClickedButton1()
{
	DlgEditList dlg;
	dlg.pCo = &m_co_dictionary;
	const auto i_result = dlg.DoModal();
	if (IDOK == i_result)
	{
		m_co_dictionary.ResetContent();
		const auto n_items = dlg.m_csArray.GetCount();
		for (auto i = 0; i < n_items; i++)
		{
			auto cs = dlg.m_csArray.GetAt(i);
			m_co_dictionary.AddString(cs);
		}
		m_co_dictionary.SetCurSel(dlg.m_selected);
	}
	UpdateData(FALSE);
}

void DlgdbEditField::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}

void DlgdbEditField::OnBnClickedOk()
{
	UpdateData(TRUE);

	// (1) get ID of record selected in the combo, check if it exists (if not, add it) and select it as current
	if (m_bIndexTable)
	{
		m_dest_id_ = -1;
		if (m_co_dictionary.GetCount() > 0)
		{
			CString cs;
			m_co_dictionary.GetLBText(m_co_dictionary.GetCurSel(), cs);
			m_pIndexTable->add_strings_from_combo(&m_co_dictionary);
			ASSERT(m_pIndexTable->get_id_from_string(cs, m_dest_id_));
		}
		if (m_source_condition == COND_SEARCH && !m_b_case_sensitive)
			m_cs_text_search.MakeLower(); // change case of search string if case-sensitive is not checked
	}
	// numeric field only - value in the main table
	else
	{
		if (m_source_condition == COND_EQU)
			m_initial_id_ = GetDlgItemInt(IDC_EDIT1);

		if (m_dest_action == CHGE_ID)
		{
			// change ID
			CString cs;
			m_co_dictionary.GetLBText(m_co_dictionary.GetCurSel(), cs);
			m_dest_id_ = _tstoi(cs);
		}
		else if (m_dest_action == CHGE_TXT)
			m_dest_id_ = GetDlgItemInt(IDC_EDIT3);
	}

	// (2) edit the main table
	switch (m_source_select)
	{
	case REC_CURRENT:
		modify_current();
		break;
	case REC_ALL:
		modify_all();
		break;
	case REC_SELECTED:
		modify_selected();
		break;
	default:
		break;
	}

	// (3) check if we need/can "remove" records from the index table
	if (m_bIndexTable)
		m_pIndexTable->remove_strings_not_in_combo(&m_co_dictionary);

	// exit
	CDialogEx::OnOK();
}

void DlgdbEditField::modify_all()
{
	const auto i_edit = m_pMainTable->GetEditMode();
	if (i_edit != dbEditNone)
		m_pMainTable->Update();


	const auto bookmark_current = m_pMainTable->GetBookmark();
	m_pMainTable->MoveFirst();
	while (!m_pMainTable->IsEOF())
	{
		modify_current();
		m_pMainTable->MoveNext();
	}
	m_pMainTable->SetBookmark(bookmark_current);
}

void DlgdbEditField::modify_current()
{
	long id_current = 0; // ID of current record
	auto i_found = 0;
	CString cs_value;
	COleVariant var_value;
	m_pMainTable->GetFieldValue(m_csColName, var_value); // FALSE if field is null
	const BOOL b_valid = (var_value.vt != VT_NULL);
	if (b_valid)
		id_current = var_value.lVal;

	// reject record?  if condition "==" : reject if iID != IDscope
	switch (m_source_condition)
	{
	case COND_EQU:
		if (id_current != m_initial_id_) //&& bValid)
			return; // exit if current record is already correct or if record is not valid
		break;
	case COND_SEARCH:
		if (!m_bIndexTable)
			break;
		if (b_valid)
		{
			cs_value = m_pIndexTable->get_string_from_id(id_current);
			if (!m_b_case_sensitive)
				cs_value.MakeLower();
			i_found = cs_value.Find(m_cs_text_search, 0);
			if (i_found < 0)
				return; // exit if the search string is not found
		}
		break;
	case COND_NONE:
	default:
		break;
	}

	// change ID value, erase ID value or change text...
	switch (m_dest_action)
	{
	case CHGE_ID: // change ID
		m_pMainTable->set_long_value(m_dest_id_, m_csColName);
		break;
	case CHGE_CLEAR: // erase iID
		m_pMainTable->set_value_null(m_csColName);
		break;
	// replace text within current record with new text
	case CHGE_TXT:
		// indexed value
		if (m_bIndexTable)
		{
			const auto cs_new = cs_value.Left(i_found)
				+ m_cs_text_replace_with
				+ cs_value.Right(cs_value.GetLength() - (m_cs_text_search.GetLength() + i_found));
			const auto id_new = m_pIndexTable->get_string_in_linked_table(cs_new);
			if (id_new >= 0)
			{
				m_pMainTable->set_long_value(id_new, m_csColName);
				// make sure that the new string is stored in the combobox
				m_first = m_co_dictionary.FindStringExact(m_first, cs_new);
				if (CB_ERR == m_first)
					m_first = m_co_dictionary.AddString(cs_new);
			}
		}
		// raw value
		else
			m_pMainTable->set_long_value(m_dest_id_, m_csColName);
		break;
	default:
		break;
	}
}
