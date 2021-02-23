// dbEditRecordDlg.cpp : implementation file
//

#include "StdAfx.h"
//#include <afxconv.h>           // For LPTSTR -> LPSTR macros

#include "dbWave.h"
#include "dbMainTable.h"
#include "dbWaveDoc.h"
#include "DlgdbEditField.h"
//#include "DlgFindFiles.h"
//#include "afxdialogex.h"

#include "DlgdbEditRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdbEditRecordDlg dialog

IMPLEMENT_DYNAMIC(CDlgdbEditRecord, CDialog)
CDlgdbEditRecord::CDlgdbEditRecord(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgdbEditRecord::IDD, pParent)
	, m_pSet(nullptr), m_cs_more(_T(""))
	, m_csnameDat(_T(""))
	, m_csnameSpk(_T(""))
{
	//m_pDaoView = NULL;
	m_bshowIDC_NEXT = TRUE;
	m_bshowIDC_PREVIOUS = TRUE;
	m_pdbDoc = nullptr;
}

CDlgdbEditRecord::~CDlgdbEditRecord()
{
	m_pdbDoc = nullptr;
}

void CDlgdbEditRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_EXPT2, m_ctlexpt);
	DDX_Control(pDX, IDC_COMBO_INSECTID, m_ctlinsectID);
	DDX_Control(pDX, IDC_COMBO_SENSILLUMID, m_ctlSensillumID);
	DDX_Control(pDX, IDC_COMBO_REPEATT, m_ctlrepeat);
	DDX_Control(pDX, IDC_COMBO_REPEATT2, m_ctlrepeat2);
	DDX_Control(pDX, IDC_COMBO_STIMULUS, m_ctlstim);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION, m_ctlconc);
	DDX_Control(pDX, IDC_COMBO_STIMULUS2, m_ctlstim2);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION2, m_ctlconc2);
	DDX_Control(pDX, IDC_COMBO_INSECTNAME, m_ctlinsect);
	DDX_Control(pDX, IDC_COMBO_SENSILLUMNAME, m_ctlsensillum);
	DDX_Control(pDX, IDC_COMBO_LOCATION, m_ctllocation);
	DDX_Control(pDX, IDC_COMBO_OPERATOR, m_ctlOperator);
	DDX_Control(pDX, IDC_COMBO_STRAIN, m_ctlstrain);
	DDX_Control(pDX, IDC_COMBO_SEX, m_ctlsex);
	DDX_Control(pDX, IDC_COMBO_PATHDAT, m_ctlpathdat);
	DDX_Control(pDX, IDC_COMBO_PATHSPK, m_ctlpathspk);
	DDX_Control(pDX, IDC_COMBO_FLAG, m_ctlflag);

	DDX_Text(pDX, IDC_EDIT_COMMENT, m_cs_more);
	DDX_Text(pDX, IDC_EDIT_NAMEDAT, m_csnameDat);
	DDX_Text(pDX, IDC_EDIT_NAMESPK, m_csnameSpk);
}

BEGIN_MESSAGE_MAP(CDlgdbEditRecord, CDialog)
	ON_BN_CLICKED(IDC_BUTTONINSECTID, &CDlgdbEditRecord::OnBnClickedButtoninsectid)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUMID, &CDlgdbEditRecord::OnBnClickedButtonsensillumid)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS, &CDlgdbEditRecord::OnBnClickedButtonstimulus)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION, &CDlgdbEditRecord::OnBnClickedButtonconcentration)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS2, &CDlgdbEditRecord::OnBnClickedButtonstimulus2)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION2, &CDlgdbEditRecord::OnBnClickedButtonconcentration2)
	ON_BN_CLICKED(IDC_BUTTONINSECTNAME, &CDlgdbEditRecord::OnBnClickedButtoninsectname)
	ON_BN_CLICKED(IDC_BUTTONSTRAIN, &CDlgdbEditRecord::OnBnClickedButtonstrain)
	ON_BN_CLICKED(IDC_BUTTONSEX, &CDlgdbEditRecord::OnBnClickedButtonsex)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUM, &CDlgdbEditRecord::OnBnClickedButtonsensillum)
	ON_BN_CLICKED(IDC_BUTTONLOCATION, &CDlgdbEditRecord::OnBnClickedButtonlocation)
	ON_BN_CLICKED(IDC_BUTTONOPERATOR, &CDlgdbEditRecord::OnBnClickedButtonoperator)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgdbEditRecord::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgdbEditRecord::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_SYNCHROSINGLE, &CDlgdbEditRecord::OnBnClickedSynchrosingle)
	ON_BN_CLICKED(IDC_SYNCHROALL, &CDlgdbEditRecord::OnBnClickedSynchroall)
	ON_BN_CLICKED(IDC_PREVIOUS, &CDlgdbEditRecord::OnBnClickedPrevious)
	ON_BN_CLICKED(IDC_NEXT, &CDlgdbEditRecord::OnBnClickedNext)
	ON_BN_CLICKED(IDC_BUTTONREPEAT, &CDlgdbEditRecord::OnBnClickedButtonrepeat)
	ON_BN_CLICKED(IDC_BUTTONREPEAT2, &CDlgdbEditRecord::OnBnClickedButtonrepeat2)
	ON_BN_CLICKED(IDC_BUTTONFLAG, &CDlgdbEditRecord::OnBnClickedButtonflag)
	ON_BN_CLICKED(IDC_BUTTONEXPT2, &CDlgdbEditRecord::OnBnClickedButtonexpt2)
END_MESSAGE_MAP()

// CdbEditRecordDlg message handlers

BOOL CDlgdbEditRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_pSet = &m_pdbDoc->m_pDB->m_mainTableSet;
	PopulateControls();
	// TODO? disable controls corresponding to a filtered field
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgdbEditRecord::PopulateControls()
{
	auto p_db = m_pdbDoc->m_pDB;
	// fill combo boxes associated with a secondary table
	PopulateCombo_WithText(p_db->m_stimSet, m_ctlstim, m_pSet->m_stim_ID);
	PopulateCombo_WithText(p_db->m_concSet, m_ctlconc, m_pSet->m_conc_ID);
	PopulateCombo_WithText(p_db->m_stimSet, m_ctlstim2, m_pSet->m_stim2_ID);
	PopulateCombo_WithText(p_db->m_concSet, m_ctlconc2, m_pSet->m_conc2_ID);
	PopulateCombo_WithText(p_db->m_insectSet, m_ctlinsect, m_pSet->m_insect_ID);
	PopulateCombo_WithText(p_db->m_strainSet, m_ctlstrain, m_pSet->m_strain_ID);
	PopulateCombo_WithText(p_db->m_sexSet, m_ctlsex, m_pSet->m_sex_ID);
	PopulateCombo_WithText(p_db->m_sensillumSet, m_ctlsensillum, m_pSet->m_sensillum_ID);
	PopulateCombo_WithText(p_db->m_locationSet, m_ctllocation, m_pSet->m_location_ID);
	PopulateCombo_WithText(p_db->m_operatorSet, m_ctlOperator, m_pSet->m_operator_ID);
	PopulateCombo_WithText(p_db->m_pathSet, m_ctlpathdat, m_pSet->m_path_ID);
	PopulateCombo_WithText(p_db->m_pathSet, m_ctlpathspk, m_pSet->m_path2_ID);
	PopulateCombo_WithText(p_db->m_exptSet, m_ctlexpt, m_pSet->m_expt_ID);
	// ID combos
	PopulateCombo_WithNumbers(m_ctlinsectID, &m_pSet->m_desc[CH_IDINSECT].liArray, m_pSet->m_IDinsect);
	PopulateCombo_WithNumbers(m_ctlSensillumID, &m_pSet->m_desc[CH_IDSENSILLUM].liArray, m_pSet->m_IDsensillum);
	PopulateCombo_WithNumbers(m_ctlrepeat, &m_pSet->m_desc[CH_REPEAT].liArray, m_pSet->m_repeat);
	PopulateCombo_WithNumbers(m_ctlrepeat2, &m_pSet->m_desc[CH_REPEAT2].liArray, m_pSet->m_repeat2);
	PopulateCombo_WithNumbers(m_ctlflag, &m_pSet->m_desc[CH_FLAG].liArray, m_pSet->m_flag);

	// fixed parameters
	m_cs_more = m_pSet->m_more;
	m_csnameDat = m_pSet->m_Filedat;
	m_csnameSpk = m_pSet->m_Filespk;

	// update IDC_NEXT IDC_PREVIOUS
	if (m_bshowIDC_NEXT != GetDlgItem(IDC_NEXT)->IsWindowEnabled())
		GetDlgItem(IDC_NEXT)->EnableWindow(m_bshowIDC_NEXT);
	if (m_bshowIDC_PREVIOUS != GetDlgItem(IDC_PREVIOUS)->IsWindowEnabled())
		GetDlgItem(IDC_PREVIOUS)->EnableWindow(m_bshowIDC_PREVIOUS);
}

void CDlgdbEditRecord::PopulateCombo_WithNumbers(CComboBox& combo, CArray<long, long>* pIDarray, long& lvar)
{
	combo.ResetContent();
	CString cs;

	const auto array_size = pIDarray->GetSize();
	auto isel = 0;
	for (auto i = 0; i < array_size; i++)
	{
		const auto i_id = pIDarray->GetAt(i);
		cs.Format(_T("%i"), i_id);
		combo.AddString(cs);
		combo.SetItemData(i, i_id);
		if (i_id == lvar)
			isel = i;
	}
	combo.SetCurSel(isel);
}

void CDlgdbEditRecord::PopulateCombo_WithText(CDaoRecordset& linkedtableSet, CComboBox& combo, int iID)
{
	combo.ResetContent();
	// fill combo box
	if (linkedtableSet.IsOpen() && !linkedtableSet.IsBOF())
	{
		COleVariant var_value0, var_value1;
		linkedtableSet.MoveFirst();
		while (!linkedtableSet.IsEOF())
		{
			linkedtableSet.GetFieldValue(0, var_value0);
			linkedtableSet.GetFieldValue(1, var_value1);
			const auto id = var_value1.lVal;
			CString cs = var_value0.bstrVal;
			if (!cs.IsEmpty())
			{
				const auto i = combo.AddString(cs);
				combo.SetItemData(i, id);
			}
			linkedtableSet.MoveNext();
		}
	}

	// search item which has value iID
	auto isel = -1;
	for (auto i = 0; i < combo.GetCount(); i++)
	{
		if (iID == static_cast<int>(combo.GetItemData(i)))
		{
			isel = i;
			break;
		}
	}
	combo.SetCurSel(isel);
}

void CDlgdbEditRecord::UpdateDatabaseFromDialog()
{
	UpdateData(TRUE); // transfer data from dlg to variables

	// update combo boxes associated with a secondary table
	m_pSet->Edit();

	CdbWdatabase* p_database = m_pdbDoc->m_pDB;
	UpdateSetFromCombo(p_database->m_stimSet, m_ctlstim, m_pSet->m_stim_ID);
	UpdateSetFromCombo(p_database->m_concSet, m_ctlconc, m_pSet->m_conc_ID);
	UpdateSetFromCombo(p_database->m_stimSet, m_ctlstim2, m_pSet->m_stim2_ID);
	UpdateSetFromCombo(p_database->m_concSet, m_ctlconc2, m_pSet->m_conc2_ID);
	UpdateSetFromCombo(p_database->m_insectSet, m_ctlinsect, m_pSet->m_insect_ID);
	UpdateSetFromCombo(p_database->m_strainSet, m_ctlstrain, m_pSet->m_strain_ID);
	UpdateSetFromCombo(p_database->m_sexSet, m_ctlsex, m_pSet->m_sex_ID);
	UpdateSetFromCombo(p_database->m_sensillumSet, m_ctlsensillum, m_pSet->m_sensillum_ID);
	UpdateSetFromCombo(p_database->m_locationSet, m_ctllocation, m_pSet->m_location_ID);
	UpdateSetFromCombo(p_database->m_operatorSet, m_ctlOperator, m_pSet->m_operator_ID);
	UpdateSetFromCombo(p_database->m_exptSet, m_ctlexpt, m_pSet->m_expt_ID);

	//// save fixed parameters

	CString cs;
	m_ctlinsectID.GetWindowText(cs);
	m_pSet->m_IDinsect = _ttoi(cs);
	m_ctlSensillumID.GetWindowText(cs);
	m_pSet->m_IDsensillum = _ttoi(cs);
	m_ctlrepeat.GetWindowText(cs);
	m_pSet->m_repeat = _ttoi(cs);
	m_ctlrepeat2.GetWindowText(cs);
	m_pSet->m_repeat2 = _ttoi(cs);
	m_ctlflag.GetWindowText(cs);
	m_pSet->m_flag = _ttoi(cs);
	m_pSet->m_more = m_cs_more;
	m_pSet->m_Filedat = m_csnameDat;
	m_pSet->m_Filespk = m_csnameSpk;
	m_pSet->Update();
}

void CDlgdbEditRecord::OnOK()
{
	UpdateDatabaseFromDialog();
	CDialog::OnOK();
}

void CDlgdbEditRecord::UpdateSetFromCombo(CDaoRecordset& linkedtableSet, CComboBox& combo, long& iIDset)
{
	// search if content of edit window is listed in the combo
	CString cs_combo;
	combo.GetWindowText(cs_combo);
	const auto n_index = combo.FindStringExact(0, cs_combo);
	if (n_index == CB_ERR)
	{
		// if new value, add a record in the linked table
		if (!cs_combo.IsEmpty())
		{
			linkedtableSet.AddNew();
			linkedtableSet.SetFieldValue(0, COleVariant(cs_combo, VT_BSTRT));
			try { linkedtableSet.Update(); }
			catch (CDaoException* e) { DisplayDaoException(e, 24); e->Delete(); }

			// get value and set the ID number in the main table
			linkedtableSet.MoveLast();
			COleVariant var_value0, var_value1;
			linkedtableSet.GetFieldValue(0, var_value0);
			linkedtableSet.GetFieldValue(1, var_value1);
			const CString cs = var_value0.bstrVal;
			ASSERT(cs_combo == cs);
			iIDset = var_value1.lVal;
		}
		// if empty string, set field to null in the main table
		else
			m_pSet->SetFieldNull(&iIDset, TRUE);
	}
	else
	{
		int iID = combo.GetItemData(n_index);
		iIDset = iID;
	}
}

// edit and update combo boxes
// if new item, update the corresponding array in document...
// update

void CDlgdbEditRecord::OnBnClickedButtoninsectid()
{
	EditChangeItem_MainField(IDC_COMBO_INSECTID);
}

void CDlgdbEditRecord::OnBnClickedButtonsensillumid()
{
	EditChangeItem_MainField(IDC_COMBO_SENSILLUMID);
}

void CDlgdbEditRecord::OnBnClickedButtonflag()
{
	EditChangeItem_MainField(IDC_COMBO_FLAG);
}

void CDlgdbEditRecord::OnBnClickedButtonrepeat()
{
	EditChangeItem_MainField(IDC_COMBO_REPEATT);
}

void CDlgdbEditRecord::OnBnClickedButtonrepeat2()
{
	EditChangeItem_MainField(IDC_COMBO_REPEATT2);
}

// Edit item / linked lists
// IDC = ID descriptor for combo box

void CDlgdbEditRecord::EditChangeItem_IndirectField(int IDC)
{
	DB_ITEMDESC* pdesc = GetItemDescriptors(IDC);
	if (pdesc->pComboBox == nullptr)
		return;

	CDlgdbEditField dlg;
	dlg.m_pMainTable = m_pSet;				// address main table
	dlg.m_csColName = pdesc->csColName;		// name of the column
	dlg.m_pIndexTable = pdesc->plinkedSet;	// address secondary table
	dlg.m_pliIDArray = nullptr;					// not a primary field
	dlg.m_pdbDoc = m_pdbDoc;
	if (dlg.DoModal() == IDOK)
		PopulateCombo_WithText(*pdesc->plinkedSet, *pdesc->pComboBox, *pdesc->pdataItem);
}

void CDlgdbEditRecord::EditChangeItem_MainField(int IDC)
{
	auto pdesc = GetItemDescriptors(IDC);
	if (pdesc->pComboBox == nullptr)
		return;

	CDlgdbEditField dlg;
	dlg.m_pMainTable = m_pSet;				// address main table
	dlg.m_csColName = pdesc->csColName;		// name of the column
	dlg.m_pliIDArray = &pdesc->liArray;		// address of table of ids
	dlg.m_pIndexTable = nullptr;
	dlg.m_pdbDoc = m_pdbDoc;
	if (dlg.DoModal() == IDOK)
	{
		// update array
		m_pSet->BuildAndSortIDArrays();
		PopulateCombo_WithNumbers(*pdesc->pComboBox, &pdesc->liArray, *pdesc->pdataItem);

		// find current selection and set combo to this position
		const auto i_id = *pdesc->pdataItem;
		auto icursel = 0;
		for (auto i = pdesc->liArray.GetUpperBound(); i >= 0; i--)
		{
			if (i_id == pdesc->liArray.GetAt(i))
			{
				icursel = i;
				break;
			}
		}
		pdesc->pComboBox->SetCurSel(icursel);
	}
}

// return pointers to parameters useful to change/edit item
// IDC = ID descriptor for check box

DB_ITEMDESC* CDlgdbEditRecord::GetItemDescriptors(int IDC)
{
	DB_ITEMDESC* pdesc = nullptr;
	const auto p_dbwave_doc = m_pdbDoc;
	auto p_db = p_dbwave_doc->m_pDB;
	int ich;
	switch (IDC)
	{
	case IDC_COMBO_EXPT2:
		ich = CH_EXPT_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlexpt;
		break;
	case IDC_COMBO_INSECTNAME:
		ich = CH_INSECT_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlinsect;
		break;
	case IDC_COMBO_LOCATION:
		ich = CH_LOCATION_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctllocation;
		break;
	case IDC_COMBO_SENSILLUMNAME:
		ich = CH_SENSILLUM_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlsensillum;
		break;
	case IDC_COMBO_STIMULUS:
		ich = CH_STIM_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlstim;
		break;
	case IDC_COMBO_CONCENTRATION:
		ich = CH_CONC_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlconc;
		break;
	case IDC_COMBO_OPERATOR:
		ich = CH_OPERATOR_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlOperator;
		break;
	case IDC_COMBO_STRAIN:
		ich = CH_STRAIN_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlstrain;
		break;
	case IDC_COMBO_SEX:
		ich = CH_SEX_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlsex;
		break;
	case IDC_COMBO_STIMULUS2:
		ich = CH_STIM2_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlstim2;
		break;
	case IDC_COMBO_CONCENTRATION2:
		ich = CH_CONC2_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlconc2;
		break;
	case IDC_COMBO_PATHDAT:
		ich = CH_PATH_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlpathdat;
		break;
	case IDC_COMBO_PATHSPK:
		ich = CH_PATH2_ID;
		m_pSet->m_desc[ich].pComboBox = &m_ctlpathspk;
		break;

	case IDC_COMBO_INSECTID:
		ich = CH_IDINSECT;
		m_pSet->m_desc[ich].pComboBox = &m_ctlinsectID;
		break;
	case IDC_COMBO_SENSILLUMID:
		ich = CH_IDSENSILLUM;
		m_pSet->m_desc[ich].pComboBox = &m_ctlSensillumID;
		break;
	case IDC_COMBO_FLAG:
		ich = CH_FLAG;
		m_pSet->m_desc[ich].pComboBox = &m_ctlflag;
		break;
	case IDC_COMBO_REPEATT:
		ich = CH_REPEAT;
		m_pSet->m_desc[ich].pComboBox = &m_ctlrepeat;
		break;
	case IDC_COMBO_REPEATT2:
		ich = CH_REPEAT2;
		m_pSet->m_desc[ich].pComboBox = &m_ctlrepeat2;
		break;

	default:
		ich = -1;
		break;
	}
	if (ich >= 0)
		pdesc = p_db->GetRecordItemDescriptor(ich);

	return pdesc;
}

void CDlgdbEditRecord::OnBnClickedButtonstimulus()
{
	EditChangeItem_IndirectField(IDC_COMBO_STIMULUS);
}

void CDlgdbEditRecord::OnBnClickedButtonconcentration()
{
	EditChangeItem_IndirectField(IDC_COMBO_CONCENTRATION);
}

void CDlgdbEditRecord::OnBnClickedButtonstimulus2()
{
	EditChangeItem_IndirectField(IDC_COMBO_STIMULUS2);
}

void CDlgdbEditRecord::OnBnClickedButtonconcentration2()
{
	EditChangeItem_IndirectField(IDC_COMBO_CONCENTRATION2);
}

void CDlgdbEditRecord::OnBnClickedButtoninsectname()
{
	EditChangeItem_IndirectField(IDC_COMBO_INSECTNAME);
}

void CDlgdbEditRecord::OnBnClickedButtonsensillum()
{
	EditChangeItem_IndirectField(IDC_COMBO_SENSILLUMNAME);
}

void CDlgdbEditRecord::OnBnClickedButtonlocation()
{
	EditChangeItem_IndirectField(IDC_COMBO_LOCATION);
}

void CDlgdbEditRecord::OnBnClickedButtonoperator()
{
	EditChangeItem_IndirectField(IDC_COMBO_OPERATOR);
}

void CDlgdbEditRecord::OnBnClickedButton5()
{
	EditChangeItem_IndirectField(IDC_COMBO_PATHDAT);
}

void CDlgdbEditRecord::OnBnClickedButton1()
{
	if (AfxMessageBox(_T("Are spike files in the same directory as dat files?"), MB_YESNO, -1) != IDYES)
		EditChangeItem_IndirectField(IDC_COMBO_PATHSPK);
	else
	{
		m_pdbDoc->DBTransferDatPathToSpkPath();
	}
}

void CDlgdbEditRecord::OnBnClickedButtonstrain()
{
	EditChangeItem_IndirectField(IDC_COMBO_STRAIN);
}

void CDlgdbEditRecord::OnBnClickedButtonsex()
{
	EditChangeItem_IndirectField(IDC_COMBO_SEX);
}

void CDlgdbEditRecord::OnBnClickedButtonexpt2()
{
	EditChangeItem_IndirectField(IDC_COMBO_EXPT2);
}

void CDlgdbEditRecord::OnBnClickedSynchrosingle()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->SynchronizeSourceInfos(FALSE);
}

void CDlgdbEditRecord::OnBnClickedSynchroall()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->SynchronizeSourceInfos(TRUE);
}

void CDlgdbEditRecord::OnBnClickedPrevious()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->DBMovePrev();

	m_pdbDoc->UpdateAllViews(nullptr, 0L, nullptr);
	PopulateControls();
	UpdateData(FALSE);
}

void CDlgdbEditRecord::OnBnClickedNext()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->DBMoveNext();

	m_pdbDoc->UpdateAllViews(nullptr, 0L, nullptr);

	PopulateControls();
	UpdateData(FALSE);
}