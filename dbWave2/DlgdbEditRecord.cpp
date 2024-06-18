#include "StdAfx.h"

#include "dbWave.h"
#include "dbTableMain.h"
#include "dbWaveDoc.h"
#include "DlgdbEditField.h"


#include "DlgdbEditRecord.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdbEditRecordDlg dialog

IMPLEMENT_DYNAMIC(DlgdbEditRecord, CDialog)

DlgdbEditRecord::DlgdbEditRecord(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

DlgdbEditRecord::~DlgdbEditRecord()
{
	m_pdbDoc = nullptr;
}

void DlgdbEditRecord::DoDataExchange(CDataExchange* pDX)
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

BEGIN_MESSAGE_MAP(DlgdbEditRecord, CDialog)
	ON_BN_CLICKED(IDC_BUTTONINSECTID, &DlgdbEditRecord::OnBnClickedButtoninsectid)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUMID, &DlgdbEditRecord::OnBnClickedButtonsensillumid)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS, &DlgdbEditRecord::OnBnClickedButtonstimulus)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION, &DlgdbEditRecord::OnBnClickedButtonconcentration)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS2, &DlgdbEditRecord::OnBnClickedButtonstimulus2)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION2, &DlgdbEditRecord::OnBnClickedButtonconcentration2)
	ON_BN_CLICKED(IDC_BUTTONINSECTNAME, &DlgdbEditRecord::OnBnClickedButtoninsectname)
	ON_BN_CLICKED(IDC_BUTTONSTRAIN, &DlgdbEditRecord::OnBnClickedButtonstrain)
	ON_BN_CLICKED(IDC_BUTTONSEX, &DlgdbEditRecord::OnBnClickedButtonsex)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUM, &DlgdbEditRecord::OnBnClickedButtonsensillum)
	ON_BN_CLICKED(IDC_BUTTONLOCATION, &DlgdbEditRecord::OnBnClickedButtonlocation)
	ON_BN_CLICKED(IDC_BUTTONOPERATOR, &DlgdbEditRecord::OnBnClickedButtonoperator)
	ON_BN_CLICKED(IDC_BUTTON5, &DlgdbEditRecord::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON1, &DlgdbEditRecord::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_SYNCHROSINGLE, &DlgdbEditRecord::OnBnClickedSynchrosingle)
	ON_BN_CLICKED(IDC_SYNCHROALL, &DlgdbEditRecord::OnBnClickedSynchroall)
	ON_BN_CLICKED(IDC_PREVIOUS, &DlgdbEditRecord::OnBnClickedPrevious)
	ON_BN_CLICKED(IDC_NEXT, &DlgdbEditRecord::OnBnClickedNext)
	ON_BN_CLICKED(IDC_BUTTONREPEAT, &DlgdbEditRecord::OnBnClickedButtonrepeat)
	ON_BN_CLICKED(IDC_BUTTONREPEAT2, &DlgdbEditRecord::OnBnClickedButtonrepeat2)
	ON_BN_CLICKED(IDC_BUTTONFLAG, &DlgdbEditRecord::OnBnClickedButtonflag)
	ON_BN_CLICKED(IDC_BUTTONEXPT2, &DlgdbEditRecord::OnBnClickedButtonexpt2)
END_MESSAGE_MAP()

// CdbEditRecordDlg message handlers

BOOL DlgdbEditRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_pSet = &m_pdbDoc->m_pDB->m_mainTableSet;
	PopulateControls();
	// TODO? disable controls corresponding to a filtered field
	UpdateData(FALSE);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgdbEditRecord::PopulateControls()
{
	auto p_db = m_pdbDoc->m_pDB;
	// fill combo boxes associated with a secondary table
	PopulateCombo_WithText(p_db->m_stim_set, m_ctlstim, m_pSet->m_stim_ID);
	PopulateCombo_WithText(p_db->m_conc_set, m_ctlconc, m_pSet->m_conc_ID);
	PopulateCombo_WithText(p_db->m_stim_set, m_ctlstim2, m_pSet->m_stim2_ID);
	PopulateCombo_WithText(p_db->m_conc_set, m_ctlconc2, m_pSet->m_conc2_ID);
	PopulateCombo_WithText(p_db->m_insect_set, m_ctlinsect, m_pSet->m_insect_ID);
	PopulateCombo_WithText(p_db->m_strain_set, m_ctlstrain, m_pSet->m_strain_ID);
	PopulateCombo_WithText(p_db->m_sex_set, m_ctlsex, m_pSet->m_sex_ID);
	PopulateCombo_WithText(p_db->m_sensillum_set, m_ctlsensillum, m_pSet->m_sensillum_ID);
	PopulateCombo_WithText(p_db->m_locationSet, m_ctllocation, m_pSet->m_location_ID);
	PopulateCombo_WithText(p_db->m_operator_set, m_ctlOperator, m_pSet->m_operator_ID);
	PopulateCombo_WithText(p_db->m_path_set, m_ctlpathdat, m_pSet->m_path_ID);
	PopulateCombo_WithText(p_db->m_path_set, m_ctlpathspk, m_pSet->m_path2_ID);
	PopulateCombo_WithText(p_db->m_expt_set, m_ctlexpt, m_pSet->m_expt_ID);
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

void DlgdbEditRecord::PopulateCombo_WithNumbers(CComboBox& combo, CArray<long, long>* pIDarray, long& lvar)
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

void DlgdbEditRecord::PopulateCombo_WithText(CDaoRecordset& linkedtableSet, CComboBox& combo, int iID)
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

void DlgdbEditRecord::UpdateDatabaseFromDialog()
{
	UpdateData(TRUE); // transfer data from dlg to variables

	// update combo boxes associated with a secondary table
	m_pSet->Edit();

	CdbTable* p_database = m_pdbDoc->m_pDB;
	UpdateSetFromCombo(p_database->m_stim_set, m_ctlstim, m_pSet->m_stim_ID);
	UpdateSetFromCombo(p_database->m_conc_set, m_ctlconc, m_pSet->m_conc_ID);
	UpdateSetFromCombo(p_database->m_stim_set, m_ctlstim2, m_pSet->m_stim2_ID);
	UpdateSetFromCombo(p_database->m_conc_set, m_ctlconc2, m_pSet->m_conc2_ID);
	UpdateSetFromCombo(p_database->m_insect_set, m_ctlinsect, m_pSet->m_insect_ID);
	UpdateSetFromCombo(p_database->m_strain_set, m_ctlstrain, m_pSet->m_strain_ID);
	UpdateSetFromCombo(p_database->m_sex_set, m_ctlsex, m_pSet->m_sex_ID);
	UpdateSetFromCombo(p_database->m_sensillum_set, m_ctlsensillum, m_pSet->m_sensillum_ID);
	UpdateSetFromCombo(p_database->m_locationSet, m_ctllocation, m_pSet->m_location_ID);
	UpdateSetFromCombo(p_database->m_operator_set, m_ctlOperator, m_pSet->m_operator_ID);
	UpdateSetFromCombo(p_database->m_expt_set, m_ctlexpt, m_pSet->m_expt_ID);

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

void DlgdbEditRecord::OnOK()
{
	UpdateDatabaseFromDialog();
	CDialog::OnOK();
}

void DlgdbEditRecord::UpdateSetFromCombo(CDaoRecordset& linkedtableSet, CComboBox& combo, long& iIDset)
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
			catch (CDaoException* e)
			{
				DisplayDaoException(e, 24);
				e->Delete();
			}

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

void DlgdbEditRecord::OnBnClickedButtoninsectid()
{
	EditChangeItem_MainField(IDC_COMBO_INSECTID);
}

void DlgdbEditRecord::OnBnClickedButtonsensillumid()
{
	EditChangeItem_MainField(IDC_COMBO_SENSILLUMID);
}

void DlgdbEditRecord::OnBnClickedButtonflag()
{
	EditChangeItem_MainField(IDC_COMBO_FLAG);
}

void DlgdbEditRecord::OnBnClickedButtonrepeat()
{
	EditChangeItem_MainField(IDC_COMBO_REPEATT);
}

void DlgdbEditRecord::OnBnClickedButtonrepeat2()
{
	EditChangeItem_MainField(IDC_COMBO_REPEATT2);
}

// Edit item / linked lists
// IDC = ID descriptor for combo box

void DlgdbEditRecord::EditChangeItem_IndirectField(int IDC)
{
	const DB_ITEMDESC* item_descriptor = GetItemDescriptors(IDC);
	if (item_descriptor->pComboBox == nullptr)
		return;

	DlgdbEditField dlg;
	dlg.m_pMainTable = m_pSet;
	dlg.m_csColName = item_descriptor->header_name; 
	dlg.m_pIndexTable = item_descriptor->plinkedSet; 
	dlg.m_pliIDArray = nullptr; 
	dlg.m_pdbDoc = m_pdbDoc;
	if (dlg.DoModal() == IDOK)
		PopulateCombo_WithText(*item_descriptor->plinkedSet, *item_descriptor->pComboBox, *item_descriptor->pdataItem);
}

void DlgdbEditRecord::EditChangeItem_MainField(int IDC)
{
	auto pdesc = GetItemDescriptors(IDC);
	if (pdesc->pComboBox == nullptr)
		return;

	DlgdbEditField dlg;
	dlg.m_pMainTable = m_pSet; 
	dlg.m_csColName = pdesc->header_name;
	dlg.m_pliIDArray = &pdesc->liArray; 
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

DB_ITEMDESC* DlgdbEditRecord::GetItemDescriptors(int IDC)
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
	{
		pdesc = &m_pSet->m_desc[ich];
		p_db->get_record_item_value(ich, pdesc);
	}

	return pdesc;
}

void DlgdbEditRecord::OnBnClickedButtonstimulus()
{
	EditChangeItem_IndirectField(IDC_COMBO_STIMULUS);
}

void DlgdbEditRecord::OnBnClickedButtonconcentration()
{
	EditChangeItem_IndirectField(IDC_COMBO_CONCENTRATION);
}

void DlgdbEditRecord::OnBnClickedButtonstimulus2()
{
	EditChangeItem_IndirectField(IDC_COMBO_STIMULUS2);
}

void DlgdbEditRecord::OnBnClickedButtonconcentration2()
{
	EditChangeItem_IndirectField(IDC_COMBO_CONCENTRATION2);
}

void DlgdbEditRecord::OnBnClickedButtoninsectname()
{
	EditChangeItem_IndirectField(IDC_COMBO_INSECTNAME);
}

void DlgdbEditRecord::OnBnClickedButtonsensillum()
{
	EditChangeItem_IndirectField(IDC_COMBO_SENSILLUMNAME);
}

void DlgdbEditRecord::OnBnClickedButtonlocation()
{
	EditChangeItem_IndirectField(IDC_COMBO_LOCATION);
}

void DlgdbEditRecord::OnBnClickedButtonoperator()
{
	EditChangeItem_IndirectField(IDC_COMBO_OPERATOR);
}

void DlgdbEditRecord::OnBnClickedButton5()
{
	EditChangeItem_IndirectField(IDC_COMBO_PATHDAT);
}

void DlgdbEditRecord::OnBnClickedButton1()
{
	if (AfxMessageBox(_T("Are spike files in the same directory as dat files?"), MB_YESNO, -1) != IDYES)
		EditChangeItem_IndirectField(IDC_COMBO_PATHSPK);
	else
	{
		m_pdbDoc->db_transfer_dat_path_to_spk_path();
	}
}

void DlgdbEditRecord::OnBnClickedButtonstrain()
{
	EditChangeItem_IndirectField(IDC_COMBO_STRAIN);
}

void DlgdbEditRecord::OnBnClickedButtonsex()
{
	EditChangeItem_IndirectField(IDC_COMBO_SEX);
}

void DlgdbEditRecord::OnBnClickedButtonexpt2()
{
	EditChangeItem_IndirectField(IDC_COMBO_EXPT2);
}

void DlgdbEditRecord::OnBnClickedSynchrosingle()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->synchronize_source_infos(FALSE);
}

void DlgdbEditRecord::OnBnClickedSynchroall()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->synchronize_source_infos(TRUE);
}

void DlgdbEditRecord::OnBnClickedPrevious()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->db_move_prev();

	m_pdbDoc->update_all_views_db_wave(nullptr, 0L, nullptr);
	PopulateControls();
	UpdateData(FALSE);
}

void DlgdbEditRecord::OnBnClickedNext()
{
	UpdateDatabaseFromDialog();
	m_pdbDoc->db_move_next();

	m_pdbDoc->update_all_views_db_wave(nullptr, 0L, nullptr);

	PopulateControls();
	UpdateData(FALSE);
}
