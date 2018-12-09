// CdbEditFieldDlg.cpp : implementation file
//

#include "StdAfx.h"
#include <afxconv.h>           // For LPTSTR -> LPSTR macros
#include "dbWave.h"
#include "dbWaveDoc.h"
#include "dbEditFieldDlg.h"
#include "afxdialogex.h"
#include "EditListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdbEditFieldDlg dialog

IMPLEMENT_DYNAMIC(CdbEditFieldDlg, CDialogEx)

CdbEditFieldDlg::CdbEditFieldDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CdbEditFieldDlg::IDD, pParent)
	, m_csfieldvalue(_T(""))
	, m_cstextsearch(_T(""))
	, m_cstextreplacewith(_T(""))
{
	m_sourceselect = REC_CURRENT;
	m_sourcecondition = COND_EQU;
	m_destaction = CHGE_ID;
	m_bCaseSensitive = FALSE;
	m_bcodictchanged = FALSE;
	m_initialID = -1;
	m_bIndexTable = TRUE;
	m_first=-1;
	m_sourceID = 0;
}

CdbEditFieldDlg::~CdbEditFieldDlg()
{
}

void CdbEditFieldDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_csfieldvalue);
	DDX_Text(pDX, IDC_EDIT2, m_cstextsearch);
	DDX_Text(pDX, IDC_EDIT3, m_cstextreplacewith);
	DDX_Control(pDX, IDC_COMBO1, m_codictionary);
	DDX_Control(pDX, IDC_COMBO3, m_cosource);
	DDX_Check(pDX, IDC_CHECKCASESENSITIV, m_bCaseSensitive);
}

BEGIN_MESSAGE_MAP(CdbEditFieldDlg, CDialogEx)
	ON_BN_CLICKED(IDC_RADIO1, &CdbEditFieldDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CdbEditFieldDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CdbEditFieldDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CdbEditFieldDlg::OnBnClickedRadio4)
	ON_BN_CLICKED(IDC_RADIO5, &CdbEditFieldDlg::OnBnClickedRadio5)
	ON_BN_CLICKED(IDC_RADIO6, &CdbEditFieldDlg::OnBnClickedRadio6)
	ON_BN_CLICKED(IDC_BUTTON1,&CdbEditFieldDlg::OnBnClickedButton1)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDOK, &CdbEditFieldDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CdbEditFieldDlg::OnCbnSelchangeCombo3)
END_MESSAGE_MAP()

// CdbEditFieldDlg message handlers

BOOL CdbEditFieldDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// disable items not used here and enable only numeric input into edit boxes
	if (m_pIndexTable == nullptr)
		m_bIndexTable = FALSE;
	else 
		m_bIndexTable = TRUE;
	if (!m_bIndexTable)
	{
		((CEdit*) GetDlgItem(IDC_EDIT1))->ModifyStyle(NULL, ES_NUMBER);
		((CEdit*) GetDlgItem(IDC_EDIT2))->ModifyStyle(NULL, ES_NUMBER);
		((CEdit*) GetDlgItem(IDC_EDIT3))->ModifyStyle(NULL, ES_NUMBER);
		GetDlgItem(IDC_CHECKCASESENSITIV)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
		m_codictionary.ModifyStyle(LBS_SORT, NULL);
	}

	// Add extra initialization here
	((CButton*) GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
	((CButton*) GetDlgItem(IDC_RADIO4))->SetCheck(BST_CHECKED);
	DisplayElements();

	// fill source type
	m_cosource.SetCurSel(1);		// only the current record

	// load source value from the main table
	COleVariant varValue;
	m_pMainTable->GetFieldValue(m_csColName, varValue);
	if (varValue.vt != VT_NULL)
	{
		m_initialID = varValue.lVal;
		if (m_bIndexTable)
			m_csfieldvalue = m_pIndexTable->GetStringFromID(varValue.lVal);
		else
			m_csfieldvalue.Format( _T("%i"), varValue.lVal);
	}
	else
		m_csfieldvalue.Empty(); // = _T("undefined");

	// linked field: fill CComboBox with content of linked table
	m_codictionary.ResetContent();
	if (m_bIndexTable)
	{
		if (m_pIndexTable->IsOpen() && !m_pIndexTable->IsBOF()) 
		{
			COleVariant varValue0, varValue1;
			m_pIndexTable->MoveFirst();
			while(!m_pIndexTable->IsEOF()) 
			{
				m_pIndexTable->GetFieldValue(0, varValue0);
				m_pIndexTable->GetFieldValue(1, varValue1);
				CString csDummy = varValue0.bstrVal;
				int i = m_codictionary.AddString(csDummy);
				m_codictionary.SetItemData(i, varValue1.lVal);
				m_pIndexTable->MoveNext();
			}
		}
	}
	// no linked field: fill CComboBox with uiArray stored into document file
	else
	{
		COleVariant bookmarkCurrent;
		bookmarkCurrent = m_pMainTable->GetBookmark();
		for (int i = 0; i <= m_pliIDArray->GetUpperBound(); i++)
		{
			CString cs;
			long ucID = m_pliIDArray->GetAt(i);
			cs.Format(_T("%i"), ucID);
			int j = m_codictionary.FindStringExact(0, cs);
			if (j == CB_ERR)
			{
				int k = m_codictionary.InsertString(i, cs);
				ASSERT( k != CB_ERR);;
				m_codictionary.SetItemData(k, ucID);
			}
		}
	}
	// select value in combobox
	int iselect = 0;
	if (!m_csfieldvalue.IsEmpty())
		iselect = m_codictionary.FindStringExact(0, m_csfieldvalue);
	m_codictionary.SetCurSel(iselect);

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CdbEditFieldDlg::DisplayElements()
{
	GetDlgItem(IDC_EDIT1)->EnableWindow(m_sourcecondition==COND_EQU);
	GetDlgItem(IDC_EDIT2)->EnableWindow(m_sourcecondition==COND_SEARCH);
	GetDlgItem(IDC_CHECKCASESENSITIV)->EnableWindow(m_sourcecondition==COND_SEARCH);

	GetDlgItem(IDC_COMBO1)->EnableWindow(m_destaction==CHGE_ID);
	//if (m_bIndexTable)
		GetDlgItem(IDC_BUTTON1)->EnableWindow(m_destaction==CHGE_ID);
	GetDlgItem(IDC_EDIT3)->EnableWindow(m_destaction==CHGE_TXT);
}

void CdbEditFieldDlg::OnCbnSelchangeCombo3()
{
	m_sourceselect = m_cosource.GetCurSel();
	DisplayElements();
}
void CdbEditFieldDlg::OnBnClickedRadio1()
{
	m_sourcecondition=COND_EQU;
	DisplayElements();
}
void CdbEditFieldDlg::OnBnClickedRadio2()
{
	m_sourcecondition=COND_SEARCH;
	DisplayElements();
}
void CdbEditFieldDlg::OnBnClickedRadio3()
{
	m_sourcecondition=COND_NONE;
	DisplayElements();
}
void CdbEditFieldDlg::OnBnClickedRadio4()
{
	m_destaction=CHGE_ID;
	DisplayElements();
}
void CdbEditFieldDlg::OnBnClickedRadio5()
{
	m_destaction=CHGE_TXT;
	DisplayElements();
}
void CdbEditFieldDlg::OnBnClickedRadio6()
{
	m_destaction=CHGE_CLEAR;
	DisplayElements();
}

void CdbEditFieldDlg::OnBnClickedButton1()
{
	CEditListDlg dlg;
	dlg.pCo = &m_codictionary;
	int iresult = dlg.DoModal();
	if (IDOK == iresult)
	{
		m_codictionary.ResetContent();
		int nitems = dlg.m_csArray.GetCount();
		for (int i=0; i< nitems; i++)
		{
			CString cs = dlg.m_csArray.GetAt(i);
			m_codictionary.AddString(cs);
		}
		m_codictionary.SetCurSel(dlg.m_selected);
	}
	UpdateData(FALSE);
}

void CdbEditFieldDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}

void CdbEditFieldDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	// (1) get ID of record selected in the combo, check if it exists (if not, add it) and select it as current
	if (m_bIndexTable)
	{
		CString cs;
		m_destID = -1;
		if (m_codictionary.GetCount() > 0)
		{
			m_codictionary.GetLBText(m_codictionary.GetCurSel(), cs);
			m_pIndexTable->AddStringsFromCombo(&m_codictionary);
			ASSERT(m_pIndexTable->GetIDFromString(cs, m_destID));
		}
		if (m_sourcecondition == COND_SEARCH && !m_bCaseSensitive)
			m_cstextsearch.MakeLower();			// change case of search string if case-sensitive is not checked		
	}
	// numeric field only - value in the main table
	else
	{
		if (m_sourcecondition == COND_EQU)
			m_initialID = GetDlgItemInt(IDC_EDIT1);

		if (m_destaction == CHGE_ID)
		{// change ID
			CString cs;
			m_codictionary.GetLBText(m_codictionary.GetCurSel(), cs);
			m_destID = _tstoi(cs);
		}
		else if (m_destaction == CHGE_TXT)
			m_destID = GetDlgItemInt(IDC_EDIT3);
	}

	// (2) edit the main table
	switch (m_sourceselect)
	{
	case REC_CURRENT:
		ModifyCurrent();
		break;
	case REC_ALL:
		ModifyAll();
		break;
	case REC_SELECTED:
		ModifySelected();
		break;
	default:
		break;
	}

	// (3) check if we need/can "remove" records from the index table
	if (m_bIndexTable)
		m_pIndexTable->RemoveStringsNotInCombo(&m_codictionary);

	// exit
	CDialogEx::OnOK();
}

void CdbEditFieldDlg::ModifySelected()
{
	short iedit = m_pMainTable->GetEditMode();
	if (iedit != dbEditNone)
		m_pMainTable->Update();

	COleVariant bookmarkCurrent;
	bookmarkCurrent = m_pMainTable->GetBookmark();

	int uSelectedCount = m_pdbDoc->m_selectedRecords.GetSize();
	ASSERT(uSelectedCount > 0);
	
	for (int i =0; i< uSelectedCount; i++)
	{
		long nItem = m_pdbDoc->m_selectedRecords.GetAt(i);
		m_pMainTable->SetAbsolutePosition(nItem);
		ModifyCurrent();
	}
	m_pMainTable->SetBookmark(bookmarkCurrent);
}

void CdbEditFieldDlg::ModifyAll()
{
	short iedit = m_pMainTable->GetEditMode();
	if (iedit != dbEditNone)
		m_pMainTable->Update();

	COleVariant bookmarkCurrent;
	bookmarkCurrent = m_pMainTable->GetBookmark();

	m_pMainTable->MoveFirst();
	while(!m_pMainTable->IsEOF()) 
	{
		ModifyCurrent();
		m_pMainTable->MoveNext();
	}
	m_pMainTable->SetBookmark(bookmarkCurrent);
}

void CdbEditFieldDlg::ModifyCurrent()
{
	long iIDcurrent = 0;								// ID of current record
	int ifound = 0;
	CString csvalue;
	COleVariant varValue;
	m_pMainTable->GetFieldValue(m_csColName, varValue);	// FALSE if field is null
	BOOL bValid = (varValue.vt != VT_NULL);
	if (bValid)
		iIDcurrent = varValue.lVal;

	// reject record?  if condition "==" : reject if iID != IDscope
	switch (m_sourcecondition)
	{
	case COND_EQU: 
		if (iIDcurrent != m_initialID)	//&& bValid)
			return;			// exit if current record is already correct or if record is not valid 
		break;
	case COND_SEARCH:
		if (!m_bIndexTable)
			break;
		if (bValid)
		{
			csvalue = m_pIndexTable->GetStringFromID(iIDcurrent);
			if (!m_bCaseSensitive)
				csvalue.MakeLower();
			ifound = csvalue.Find(m_cstextsearch, 0);
			if (ifound < 0)
				return;			// exit if the search string is not found
		}
		break;
	case COND_NONE:
	default:
		break;
	}

	// change ID value, erase ID value or change text...
	switch (m_destaction)
	{
	case CHGE_ID:				// change ID
		m_pMainTable->SetLongValue(m_destID, m_csColName);
		break;
	case CHGE_CLEAR:			// erase iID 
		m_pMainTable->SetValueNull(m_csColName);
		break;
	// replace text within current record with new text
	case CHGE_TXT:
		// indexed value
		if (m_bIndexTable)
		{
			CString csnew;
			csnew = csvalue.Left(ifound) 
				+ m_cstextreplacewith
				+ csvalue.Right(csvalue.GetLength() - (m_cstextsearch.GetLength()+ ifound));
			long iIDNew = m_pIndexTable->GetIDorCreateIDforString(csnew);
			if (iIDNew >= 0)
			{
				m_pMainTable->SetLongValue(iIDNew, m_csColName);
				// make sure that the new string is stored in the combobox
				m_first = m_codictionary.FindStringExact(m_first, csnew);
				if (CB_ERR == m_first)
					m_first = m_codictionary.AddString(csnew);
			}
		}
		// raw value
		else
			m_pMainTable->SetLongValue(m_destID, m_csColName);
		break;
	default:
		break;
	}
}

