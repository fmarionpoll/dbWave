#include "StdAfx.h"

#include "dbWave.h"
#include "dbTableMain.h"
#include "dbWaveDoc.h"
#include "DlgdbEditField.h"


#include "DlgdbEditRecord.h"

#include "dbWave_constants.h"
#include "FilenameCleanupUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CdbEditRecordDlg dialog

IMPLEMENT_DYNAMIC(DlgdbEditRecord, CDialog)

DlgdbEditRecord::DlgdbEditRecord(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

DlgdbEditRecord::~DlgdbEditRecord()
{
	m_pdb_doc = nullptr;
}

void DlgdbEditRecord::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_COMBO_EXPT2, m_ctl_experiment);
	DDX_Control(p_dx, IDC_COMBO_INSECTID, m_ctl_insect_id);
	DDX_Control(p_dx, IDC_COMBO_SENSILLUMID, m_ctl_sensillum_id);
	DDX_Control(p_dx, IDC_COMBO_REPEATT, m_ctl_repeat);
	DDX_Control(p_dx, IDC_COMBO_REPEATT2, m_ctl_repeat2);
	DDX_Control(p_dx, IDC_COMBO_STIMULUS, m_ctl_stimulus);
	DDX_Control(p_dx, IDC_COMBO_CONCENTRATION, m_ctl_concentration);
	DDX_Control(p_dx, IDC_COMBO_STIMULUS2, m_ctl_stimulus2);
	DDX_Control(p_dx, IDC_COMBO_CONCENTRATION2, m_ctl_concentration2);
	DDX_Control(p_dx, IDC_COMBO_INSECTNAME, m_ctl_insect);
	DDX_Control(p_dx, IDC_COMBO_SENSILLUMNAME, m_ctl_sensillum);
	DDX_Control(p_dx, IDC_COMBO_LOCATION, m_ctl_location);
	DDX_Control(p_dx, IDC_COMBO_OPERATOR, m_ctl_operator);
	DDX_Control(p_dx, IDC_COMBO_STRAIN, m_ctl_strain);
	DDX_Control(p_dx, IDC_COMBO_SEX, m_ctl_sex);
	DDX_Control(p_dx, IDC_COMBO_PATHDAT, m_ctl_path_dat);
	DDX_Control(p_dx, IDC_COMBO_PATHSPK, m_ctl_path_spk);
	DDX_Control(p_dx, IDC_COMBO_FLAG, m_ctl_flag);

	DDX_Text(p_dx, IDC_EDIT_COMMENT, m_cs_more);
	DDX_Text(p_dx, IDC_EDIT_NAMEDAT, m_cs_name_dat);
	DDX_Text(p_dx, IDC_EDIT_NAMESPK, m_cs_name_spk);
}

BEGIN_MESSAGE_MAP(DlgdbEditRecord, CDialog)
	ON_BN_CLICKED(IDC_BUTTONINSECTID, &DlgdbEditRecord::on_bn_clicked_button_insect_id)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUMID, &DlgdbEditRecord::on_bn_clicked_button_sensillum_id)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS, &DlgdbEditRecord::on_bn_clicked_button_stimulus)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION, &DlgdbEditRecord::on_bn_clicked_button_concentration)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS2, &DlgdbEditRecord::on_bn_clicked_button_stimulus2)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION2, &DlgdbEditRecord::on_bn_clicked_button_concentration2)
	ON_BN_CLICKED(IDC_BUTTONINSECTNAME, &DlgdbEditRecord::on_bn_clicked_button_insect_name)
	ON_BN_CLICKED(IDC_BUTTONSTRAIN, &DlgdbEditRecord::on_bn_clicked_button_strain)
	ON_BN_CLICKED(IDC_BUTTONSEX, &DlgdbEditRecord::on_bn_clicked_button_sex)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUM, &DlgdbEditRecord::on_bn_clicked_button_sensillum)
	ON_BN_CLICKED(IDC_BUTTONLOCATION, &DlgdbEditRecord::on_bn_clicked_button_location)
	ON_BN_CLICKED(IDC_BUTTONOPERATOR, &DlgdbEditRecord::on_bn_clicked_button_operator)
	ON_BN_CLICKED(IDC_BUTTON5, &DlgdbEditRecord::on_bn_clicked_button5)
	ON_BN_CLICKED(IDC_BUTTON1, &DlgdbEditRecord::on_bn_clicked_button1)
	ON_BN_CLICKED(IDC_SYNCHROSINGLE, &DlgdbEditRecord::on_bn_clicked_synchro_single)
	ON_BN_CLICKED(IDC_SYNCHROALL, &DlgdbEditRecord::on_bn_clicked_synchro_all)
	ON_BN_CLICKED(IDC_PREVIOUS, &DlgdbEditRecord::on_bn_clicked_previous)
	ON_BN_CLICKED(IDC_NEXT, &DlgdbEditRecord::on_bn_clicked_next)
	ON_BN_CLICKED(IDC_BUTTONREPEAT, &DlgdbEditRecord::on_bn_clicked_button_repeat)
	ON_BN_CLICKED(IDC_BUTTONREPEAT2, &DlgdbEditRecord::on_bn_clicked_button_repeat2)
	ON_BN_CLICKED(IDC_BUTTONFLAG, &DlgdbEditRecord::on_bn_clicked_button_flag)
	ON_BN_CLICKED(IDC_BUTTONEXPT2, &DlgdbEditRecord::on_bn_clicked_button_expt2)
	ON_BN_CLICKED(IDC_BUTTON8, &DlgdbEditRecord::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &DlgdbEditRecord::OnBnClickedButton9)
END_MESSAGE_MAP()

// CdbEditRecordDlg message handlers

BOOL DlgdbEditRecord::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_p_set = &m_pdb_doc->db_table->m_main_table_set;
	populate_controls();
	// TODO? disable controls corresponding to a filtered field
	UpdateData(FALSE);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgdbEditRecord::populate_controls()
{
	auto p_db = m_pdb_doc->db_table;
	// fill combo boxes associated with a secondary table
	populate_combo_with_text(p_db->m_stimulus_set, m_ctl_stimulus, m_p_set->m_stimulus_id);
	populate_combo_with_text(p_db->m_concentration_set, m_ctl_concentration, m_p_set->m_concentration_id);
	populate_combo_with_text(p_db->m_stimulus_set, m_ctl_stimulus2, m_p_set->m_stimulus2_id);
	populate_combo_with_text(p_db->m_concentration_set, m_ctl_concentration2, m_p_set->m_concentration2_id);
	populate_combo_with_text(p_db->m_insect_set, m_ctl_insect, m_p_set->m_insect_id);
	populate_combo_with_text(p_db->m_strain_set, m_ctl_strain, m_p_set->m_strain_id);
	populate_combo_with_text(p_db->m_sex_set, m_ctl_sex, m_p_set->m_sex_id);
	populate_combo_with_text(p_db->m_sensillum_set, m_ctl_sensillum, m_p_set->m_sensillum_id);
	populate_combo_with_text(p_db->m_location_set, m_ctl_location, m_p_set->m_location_id);
	populate_combo_with_text(p_db->m_operator_set, m_ctl_operator, m_p_set->m_operator_id);
	populate_combo_with_text(p_db->m_path_set, m_ctl_path_dat, m_p_set->m_path_id);
	populate_combo_with_text(p_db->m_path_set, m_ctl_path_spk, m_p_set->m_path2_id);
	populate_combo_with_text(p_db->m_experiment_set, m_ctl_experiment, m_p_set->m_experiment_id);
	// ID combos
	populate_combo_with_numbers(m_ctl_insect_id, &m_p_set->m_desc[CH_IDINSECT].li_array, m_p_set->m_id_insect);
	populate_combo_with_numbers(m_ctl_sensillum_id, &m_p_set->m_desc[CH_IDSENSILLUM].li_array, m_p_set->m_id_sensillum);
	populate_combo_with_numbers(m_ctl_repeat, &m_p_set->m_desc[CH_REPEAT].li_array, m_p_set->m_repeat);
	populate_combo_with_numbers(m_ctl_repeat2, &m_p_set->m_desc[CH_REPEAT2].li_array, m_p_set->m_repeat2);
	populate_combo_with_numbers(m_ctl_flag, &m_p_set->m_desc[CH_FLAG].li_array, m_p_set->m_flag);

	// fixed parameters
	m_cs_more = m_p_set->m_more;
	m_cs_name_dat = m_p_set->m_file_dat;
	m_cs_name_spk = m_p_set->m_file_spk;

	// update IDC_NEXT IDC_PREVIOUS
	if (show_idc_next != GetDlgItem(IDC_NEXT)->IsWindowEnabled())
		GetDlgItem(IDC_NEXT)->EnableWindow(show_idc_next);
	if (show_idc_previous != GetDlgItem(IDC_PREVIOUS)->IsWindowEnabled())
		GetDlgItem(IDC_PREVIOUS)->EnableWindow(show_idc_previous);
}

void DlgdbEditRecord::populate_combo_with_numbers(CComboBox& combo, CArray<long, long>* p_id_array, const long& l_var)
{
	combo.ResetContent();
	CString cs;

	const auto array_size = p_id_array->GetSize();
	auto i_sel = 0;
	for (auto i = 0; i < array_size; i++)
	{
		const auto i_id = p_id_array->GetAt(i);
		cs.Format(_T("%i"), i_id);
		combo.AddString(cs);
		combo.SetItemData(i, i_id);
		if (i_id == l_var)
			i_sel = i;
	}
	combo.SetCurSel(i_sel);
}

void DlgdbEditRecord::populate_combo_with_text(CDaoRecordset& linked_table_set, CComboBox& combo, int i_id)
{
	combo.ResetContent();
	// fill combo box
	if (linked_table_set.IsOpen() && !linked_table_set.IsBOF())
	{
		COleVariant var_value0, var_value1;
		linked_table_set.MoveFirst();
		while (!linked_table_set.IsEOF())
		{
			linked_table_set.GetFieldValue(0, var_value0);
			linked_table_set.GetFieldValue(1, var_value1);
			const auto id = var_value1.lVal;
			CString cs = var_value0.bstrVal;
			if (!cs.IsEmpty())
			{
				const auto i = combo.AddString(cs);
				combo.SetItemData(i, id);
			}
			linked_table_set.MoveNext();
		}
	}

	// search item which has value iID
	auto i_sel = -1;
	for (auto i = 0; i < combo.GetCount(); i++)
	{
		if (i_id == static_cast<int>(combo.GetItemData(i)))
		{
			i_sel = i;
			break;
		}
	}
	combo.SetCurSel(i_sel);
}

void DlgdbEditRecord::update_database_from_dialog()
{
	UpdateData(TRUE); // transfer data from dlg to variables

	// update combo boxes associated with a secondary table
	m_p_set->Edit();

	CdbTable* p_database = m_pdb_doc->db_table;
	update_set_from_combo(p_database->m_stimulus_set, m_ctl_stimulus, m_p_set->m_stimulus_id);
	update_set_from_combo(p_database->m_concentration_set, m_ctl_concentration, m_p_set->m_concentration_id);
	update_set_from_combo(p_database->m_stimulus_set, m_ctl_stimulus2, m_p_set->m_stimulus2_id);
	update_set_from_combo(p_database->m_concentration_set, m_ctl_concentration2, m_p_set->m_concentration2_id);
	update_set_from_combo(p_database->m_insect_set, m_ctl_insect, m_p_set->m_insect_id);
	update_set_from_combo(p_database->m_strain_set, m_ctl_strain, m_p_set->m_strain_id);
	update_set_from_combo(p_database->m_sex_set, m_ctl_sex, m_p_set->m_sex_id);
	update_set_from_combo(p_database->m_sensillum_set, m_ctl_sensillum, m_p_set->m_sensillum_id);
	update_set_from_combo(p_database->m_location_set, m_ctl_location, m_p_set->m_location_id);
	update_set_from_combo(p_database->m_operator_set, m_ctl_operator, m_p_set->m_operator_id);
	update_set_from_combo(p_database->m_experiment_set, m_ctl_experiment, m_p_set->m_experiment_id);

	//// save fixed parameters
	CString cs;
	m_ctl_insect_id.GetWindowText(cs);
	m_p_set->m_id_insect = _ttoi(cs);
	m_ctl_sensillum_id.GetWindowText(cs);
	m_p_set->m_id_sensillum = _ttoi(cs);
	m_ctl_repeat.GetWindowText(cs);
	m_p_set->m_repeat = _ttoi(cs);
	m_ctl_repeat2.GetWindowText(cs);
	m_p_set->m_repeat2 = _ttoi(cs);
	m_ctl_flag.GetWindowText(cs);
	m_p_set->m_flag = _ttoi(cs);
	m_p_set->m_more = m_cs_more;
	m_p_set->m_file_dat = m_cs_name_dat;
	m_p_set->m_file_spk = m_cs_name_spk;
	m_p_set->Update();
}

void DlgdbEditRecord::OnOK()
{
	update_database_from_dialog();
	CDialog::OnOK();
}

void DlgdbEditRecord::update_set_from_combo(CDaoRecordset& linked_table_set, CComboBox& combo, long& id_set) const
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
			linked_table_set.AddNew();
			linked_table_set.SetFieldValue(0, COleVariant(cs_combo, VT_BSTRT));
			try { linked_table_set.Update(); }
			catch (CDaoException* e)
			{
				DisplayDaoException(e, 24);
				e->Delete();
			}

			// get value and set the ID number in the main table
			linked_table_set.MoveLast();
			COleVariant var_value0, var_value1;
			linked_table_set.GetFieldValue(0, var_value0);
			linked_table_set.GetFieldValue(1, var_value1);
			const CString cs = var_value0.bstrVal;
			ASSERT(cs_combo == cs);
			id_set = var_value1.lVal;
		}
		// if empty string, set field to null in the main table
		else
			m_p_set->SetFieldNull(&id_set, TRUE);
	}
	else
	{
		int i_id = combo.GetItemData(n_index);
		id_set = i_id;
	}
}

// edit and update combo boxes
// if new item, update the corresponding array in document...
// update

void DlgdbEditRecord::on_bn_clicked_button_insect_id()
{
	edit_change_item_main_field(IDC_COMBO_INSECTID);
}

void DlgdbEditRecord::on_bn_clicked_button_sensillum_id()
{
	edit_change_item_main_field(IDC_COMBO_SENSILLUMID);
}

void DlgdbEditRecord::on_bn_clicked_button_flag()
{
	edit_change_item_main_field(IDC_COMBO_FLAG);
}

void DlgdbEditRecord::on_bn_clicked_button_repeat()
{
	edit_change_item_main_field(IDC_COMBO_REPEATT);
}

void DlgdbEditRecord::on_bn_clicked_button_repeat2()
{
	edit_change_item_main_field(IDC_COMBO_REPEATT2);
}

// Edit item / linked lists
// IDC = ID descriptor for combo box

void DlgdbEditRecord::edit_change_item_indirect_field(const int idc)
{
	const DB_ITEMDESC* item_descriptor = get_item_descriptors(idc);
	if (item_descriptor->p_combo_box == nullptr)
		return;

	DlgdbEditField dlg;
	dlg.m_p_main_table = m_p_set;
	dlg.m_cs_col_name = item_descriptor->header_name; 
	dlg.m_p_index_table = item_descriptor->p_linked_set; 
	dlg.m_pli_id_array = nullptr; 
	dlg.m_pdb_doc = m_pdb_doc;
	if (dlg.DoModal() == IDOK)
		populate_combo_with_text(*item_descriptor->p_linked_set, *item_descriptor->p_combo_box, *item_descriptor->pdata_item);
}

void DlgdbEditRecord::edit_change_item_main_field(const int idc)
{
	const auto p_desc = get_item_descriptors(idc);
	if (p_desc->p_combo_box == nullptr)
		return;

	DlgdbEditField dlg;
	dlg.m_p_main_table = m_p_set; 
	dlg.m_cs_col_name = p_desc->header_name;
	dlg.m_pli_id_array = &p_desc->li_array; 
	dlg.m_p_index_table = nullptr;
	dlg.m_pdb_doc = m_pdb_doc;
	if (dlg.DoModal() == IDOK)
	{
		// update array
		m_p_set->build_and_sort_id_arrays();
		populate_combo_with_numbers(*p_desc->p_combo_box, &p_desc->li_array, *p_desc->pdata_item);

		// find current selection and set combo to this position
		const auto i_id = *p_desc->pdata_item;
		auto i_cur_sel = 0;
		for (auto i = p_desc->li_array.GetUpperBound(); i >= 0; i--)
		{
			if (i_id == p_desc->li_array.GetAt(i))
			{
				i_cur_sel = i;
				break;
			}
		}
		p_desc->p_combo_box->SetCurSel(i_cur_sel);
	}
}

// return pointers to parameters useful to change/edit item
// IDC = ID descriptor for check box

DB_ITEMDESC* DlgdbEditRecord::get_item_descriptors(const int idc)
{
	DB_ITEMDESC* p_desc = nullptr;
	const auto p_dbwave_doc = m_pdb_doc;
	auto p_db = p_dbwave_doc->db_table;
	int ich;
	switch (idc)
	{
	case IDC_COMBO_EXPT2:
		ich = CH_EXPT_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_experiment;
		break;
	case IDC_COMBO_INSECTNAME:
		ich = CH_INSECT_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_insect;
		break;
	case IDC_COMBO_LOCATION:
		ich = CH_LOCATION_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_location;
		break;
	case IDC_COMBO_SENSILLUMNAME:
		ich = CH_SENSILLUM_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_sensillum;
		break;
	case IDC_COMBO_STIMULUS:
		ich = CH_STIM_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_stimulus;
		break;
	case IDC_COMBO_CONCENTRATION:
		ich = CH_CONC_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_concentration;
		break;
	case IDC_COMBO_OPERATOR:
		ich = CH_OPERATOR_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_operator;
		break;
	case IDC_COMBO_STRAIN:
		ich = CH_STRAIN_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_strain;
		break;
	case IDC_COMBO_SEX:
		ich = CH_SEX_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_sex;
		break;
	case IDC_COMBO_STIMULUS2:
		ich = CH_STIM2_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_stimulus2;
		break;
	case IDC_COMBO_CONCENTRATION2:
		ich = CH_CONC2_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_concentration2;
		break;
	case IDC_COMBO_PATHDAT:
		ich = CH_PATH_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_path_dat;
		break;
	case IDC_COMBO_PATHSPK:
		ich = CH_PATH2_ID;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_path_spk;
		break;

	case IDC_COMBO_INSECTID:
		ich = CH_IDINSECT;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_insect_id;
		break;
	case IDC_COMBO_SENSILLUMID:
		ich = CH_IDSENSILLUM;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_sensillum_id;
		break;
	case IDC_COMBO_FLAG:
		ich = CH_FLAG;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_flag;
		break;
	case IDC_COMBO_REPEATT:
		ich = CH_REPEAT;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_repeat;
		break;
	case IDC_COMBO_REPEATT2:
		ich = CH_REPEAT2;
		m_p_set->m_desc[ich].p_combo_box = &m_ctl_repeat2;
		break;

	default:
		ich = -1;
		break;
	}
	if (ich >= 0) 
	{
		p_desc = &m_p_set->m_desc[ich];
		p_db->get_record_item_value(ich, p_desc);
	}

	return p_desc;
}

void DlgdbEditRecord::on_bn_clicked_button_stimulus()
{
	edit_change_item_indirect_field(IDC_COMBO_STIMULUS);
}

void DlgdbEditRecord::on_bn_clicked_button_concentration()
{
	edit_change_item_indirect_field(IDC_COMBO_CONCENTRATION);
}

void DlgdbEditRecord::on_bn_clicked_button_stimulus2()
{
	edit_change_item_indirect_field(IDC_COMBO_STIMULUS2);
}

void DlgdbEditRecord::on_bn_clicked_button_concentration2()
{
	edit_change_item_indirect_field(IDC_COMBO_CONCENTRATION2);
}

void DlgdbEditRecord::on_bn_clicked_button_insect_name()
{
	edit_change_item_indirect_field(IDC_COMBO_INSECTNAME);
}

void DlgdbEditRecord::on_bn_clicked_button_sensillum()
{
	edit_change_item_indirect_field(IDC_COMBO_SENSILLUMNAME);
}

void DlgdbEditRecord::on_bn_clicked_button_location()
{
	edit_change_item_indirect_field(IDC_COMBO_LOCATION);
}

void DlgdbEditRecord::on_bn_clicked_button_operator()
{
	edit_change_item_indirect_field(IDC_COMBO_OPERATOR);
}

void DlgdbEditRecord::on_bn_clicked_button5()
{
	edit_change_item_indirect_field(IDC_COMBO_PATHDAT);
}

void DlgdbEditRecord::on_bn_clicked_button1()
{
	if (AfxMessageBox(_T("Are spike files in the same directory as dat files?"), MB_YESNO, -1) != IDYES)
		edit_change_item_indirect_field(IDC_COMBO_PATHSPK);
	else
	{
		m_pdb_doc->db_transfer_dat_path_to_spk_path();
	}
}

void DlgdbEditRecord::on_bn_clicked_button_strain()
{
	edit_change_item_indirect_field(IDC_COMBO_STRAIN);
}

void DlgdbEditRecord::on_bn_clicked_button_sex()
{
	edit_change_item_indirect_field(IDC_COMBO_SEX);
}

void DlgdbEditRecord::on_bn_clicked_button_expt2()
{
	edit_change_item_indirect_field(IDC_COMBO_EXPT2);
}

void DlgdbEditRecord::on_bn_clicked_synchro_single()
{
	update_database_from_dialog();
	m_pdb_doc->synchronize_source_infos(FALSE);
}

void DlgdbEditRecord::on_bn_clicked_synchro_all()
{
	update_database_from_dialog();
	m_pdb_doc->synchronize_source_infos(TRUE);
}

void DlgdbEditRecord::on_bn_clicked_previous()
{
	update_database_from_dialog();
	m_pdb_doc->db_move_prev();

	m_pdb_doc->update_all_views_db_wave(nullptr, 0L, nullptr);
	populate_controls();
	UpdateData(FALSE);
}

void DlgdbEditRecord::on_bn_clicked_next()
{
	update_database_from_dialog();
	m_pdb_doc->db_move_next();

	m_pdb_doc->update_all_views_db_wave(nullptr, 0L, nullptr);

	populate_controls();
	UpdateData(FALSE);
}


void DlgdbEditRecord::OnBnClickedButton8()
{
	// Remove leading spaces from data file filenames
	if (AfxMessageBox(_T("This will remove leading spaces from all data file filenames in the database.\n\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	const auto* p_db_table = m_pdb_doc->db_table;
	if (!p_db_table || !p_db_table->m_main_table_set.IsOpen())
	{
		AfxMessageBox(_T("Database is not open."), MB_OK | MB_ICONERROR);
		return;
	}

	if ( CleanupDataFileFilenames(m_pdb_doc))
	{
		m_pdb_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	}
}


void DlgdbEditRecord::OnBnClickedButton9()
{
	// Remove leading spaces from spike file filenames
	if (AfxMessageBox(_T("This will remove leading spaces from all spike file filenames in the database.\n\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	const auto* p_db_table = m_pdb_doc->db_table;
	if (!p_db_table || !p_db_table->m_main_table_set.IsOpen())
	{
		AfxMessageBox(_T("Database is not open."), MB_OK | MB_ICONERROR);
		return;
	}

	if (CleanupSpikeFileFilenames(m_pdb_doc))
	{
		m_pdb_doc->UpdateAllViews(nullptr, HINT_REQUERY, nullptr);
	}
}
