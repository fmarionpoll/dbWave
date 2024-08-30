// ADExperi.cpp : implementation file
//
#include "StdAfx.h"
#include "DlgADExperiment.h"
#include "DlgEditList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DlgADExperiment::DlgADExperiment(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgADExperiment::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);

	DDX_Text(p_dx, IDC_EDIT_NAME, m_cs_basename);
	DDX_Text(p_dx, IDC_EDIT_COMMENT, m_cs_more_comment);
	DDX_Text(p_dx, IDC_EDIT_NUMNAME, m_experiment_number);
	DDX_Text(p_dx, IDC_INSECTID, m_insect_number);

	DDX_Control(p_dx, IDC_COMBO_STRAIN, m_co_strain);
	DDX_Control(p_dx, IDC_COMBO_SEX, m_co_sex);
	DDX_Control(p_dx, IDC_COMBO_SENSILLUMNAME, m_co_sensillum);
	DDX_Control(p_dx, IDC_COMBO_LOCATION, m_co_location);
	DDX_Control(p_dx, IDC_COMBO_OPERATOR, m_co_operator);
	DDX_Control(p_dx, IDC_COMBO_INSECTNAME, m_co_insect);
	DDX_Control(p_dx, IDC_COMBO_STIMULUS, m_co_stimulus);
	DDX_Control(p_dx, IDC_COMBO_CONCENTRATION, m_co_concentration);
	DDX_Control(p_dx, IDC_COMBO_STIMULUS2, m_co_stimulus2);
	DDX_Control(p_dx, IDC_COMBO_CONCENTRATION2, m_co_concentration2);
	DDX_Control(p_dx, IDC_COMBO1_REPEAT, m_co_repeat);
	DDX_Control(p_dx, IDC_COMBO1_REPEAT2, m_co_repeat2);
	DDX_Control(p_dx, IDC_COMBO_EXPT3, m_co_experiment);

	DDX_Check(p_dx, IDC_CHECK2, m_b_hide_subsequent);
	DDX_Control(p_dx, IDC_MFCEDITBROWSE1, m_mfc_browse_path);
}

BEGIN_MESSAGE_MAP(DlgADExperiment, CDialog)

	ON_BN_CLICKED(IDC_BUTTONINSECTNAME, &DlgADExperiment::on_bn_clicked_button_insect_name)
	ON_BN_CLICKED(IDC_BUTTONSTRAIN, &DlgADExperiment::on_bn_clicked_button_strain)
	ON_BN_CLICKED(IDC_BUTTONSEX, &DlgADExperiment::on_bn_clicked_button_sex)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUM, &DlgADExperiment::on_bn_clicked_button_sensillum)
	ON_BN_CLICKED(IDC_BUTTONLOCATION, &DlgADExperiment::on_bn_clicked_button_location)
	ON_BN_CLICKED(IDC_BUTTONOPERATOR, &DlgADExperiment::on_bn_clicked_button_operator)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS, &DlgADExperiment::on_bn_clicked_button_stimulus)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION, &DlgADExperiment::on_bn_clicked_button_concentration)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS2, &DlgADExperiment::on_bn_clicked_button_stimulus2)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION2, &DlgADExperiment::on_bn_clicked_button_concentration2)
	ON_BN_CLICKED(IDC_BUTTONREPEAT, &DlgADExperiment::on_bn_clicked_button_repeat)
	ON_BN_CLICKED(IDC_BUTTONREPEAT2, &DlgADExperiment::on_bn_clicked_button_repeat2)
	ON_BN_CLICKED(IDC_BUTTONEXPT3, &DlgADExperiment::on_bn_clicked_button_experiment)

	ON_EN_KILLFOCUS(IDC_MFCEDITBROWSE1, &DlgADExperiment::on_en_kill_focus_mfc_edit_browse1)
	ON_BN_CLICKED(IDC_BUTTON_NEXTID, &DlgADExperiment::on_bn_clicked_button_next_id)
END_MESSAGE_MAP()

void DlgADExperiment::OnOK()
{
	UpdateData(TRUE);

	m_mfc_browse_path.GetWindowTextW(m_cs_pathname);
	if (m_cs_pathname.GetLength() > 2
		&& m_cs_pathname.Right(1) != _T("\\"))
		m_cs_pathname += _T("\\");

	// check that directory is present - otherwise create...
	const auto cs_path = m_cs_pathname.Left(m_cs_pathname.GetLength() - 1);

	// create directory if necessary
	CFileFind cf;
	if (!cs_path.IsEmpty() && !cf.FindFile(cs_path))
	{
		if (!CreateDirectory(cs_path, nullptr))
			AfxMessageBox(IDS_DIRECTORYFAILED);
	}

	// build file name
	int i_experiment_number = static_cast<int>(m_experiment_number);
	CString cs_buf_temp;
	cs_buf_temp.Format(_T("%06.6lu"), i_experiment_number);
	m_sz_file_name = m_cs_pathname + m_cs_basename + cs_buf_temp + _T(".dat");

	// check if this file is already present, exit if not...
	CFileStatus status;
	auto i_id_response = IDYES; // default: go on if file not found
	if (CFile::GetStatus(m_sz_file_name, status))
		i_id_response = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
	// no? find first available number
	if (IDNO == i_id_response)
	{
		BOOL flag = TRUE;
		while (flag)
		{
			i_experiment_number++;
			cs_buf_temp.Format(_T("%06.6lu"), i_experiment_number);
			m_sz_file_name = m_cs_pathname + m_cs_basename + cs_buf_temp + _T(".dat");
			flag = CFile::GetStatus(m_sz_file_name, status);
		}
		const auto cs = _T("Next available file name is: ") + m_sz_file_name;
		i_id_response = AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
	}

	// OK  pass parameters
	if (IDYES == i_id_response)
	{
		// update experiment number
		m_experiment_number = i_experiment_number;

		// update file descriptors
		options_input->cs_pathname = m_cs_pathname;
		options_input->cs_basename = m_cs_basename;
		options_input->experiment_number = static_cast<int>(m_experiment_number) + 1;
		options_input->insect_number = static_cast<int>(m_insect_number);

		p_wave_format->insect_id = static_cast<long>(m_insect_number);
		p_wave_format->repeat = options_input->ics_a_repeat;
		p_wave_format->repeat2 = options_input->ics_a_repeat2;
		p_wave_format->cs_more_comment = m_cs_more_comment;

		// save descriptors into waveFormat (data acq file descriptor) and update database
		m_co_stimulus.GetWindowText(p_wave_format->cs_stimulus);
		m_co_concentration.GetWindowText(p_wave_format->cs_concentration);
		m_co_stimulus2.GetWindowText(p_wave_format->cs_stimulus2);
		m_co_concentration2.GetWindowText(p_wave_format->cs_concentration2);
		m_co_location.GetWindowText(p_wave_format->cs_location);
		m_co_sensillum.GetWindowText(p_wave_format->cs_sensillum);
		m_co_strain.GetWindowText(p_wave_format->cs_strain);
		m_co_sex.GetWindowText(p_wave_format->cs_sex);
		m_co_operator.GetWindowText(p_wave_format->cs_operator);
		m_co_insect.GetWindowText(p_wave_format->cs_insect_name);
		m_co_experiment.GetWindowText(p_wave_format->cs_comment);

		// save data into commons
		options_input->ics_a_concentration = save_list(&m_co_concentration, &(options_input->cs_a_concentration));
		options_input->ics_a_stimulus = save_list(&m_co_stimulus, &(options_input->cs_a_stimulus));
		options_input->ics_a_insect = save_list(&m_co_insect, &(options_input->cs_a_insect));
		options_input->ics_a_location = save_list(&m_co_location, &(options_input->cs_a_location));
		options_input->ics_a_sensillum = save_list(&m_co_sensillum, &(options_input->cs_a_sensillum));
		options_input->ics_a_strain = save_list(&m_co_strain, &(options_input->cs_a_strain));
		options_input->ics_a_sex = save_list(&m_co_sex, &(options_input->cs_a_sex));
		options_input->ics_a_operator_name = save_list(&m_co_operator, &(options_input->cs_a_operator_name));
		options_input->ics_a_concentration2 = save_list(&m_co_concentration2, &(options_input->cs_a_concentration2));
		options_input->ics_a_stimulus2 = save_list(&m_co_stimulus2, &(options_input->cs_a_stimulus2));
		options_input->ics_a_repeat = save_list(&m_co_repeat, nullptr);
		options_input->ics_a_repeat2 = save_list(&m_co_repeat2, nullptr);
		options_input->ics_a_experiment = save_list(&m_co_experiment, &(options_input->cs_a_experiment));

		CDialog::OnOK();
	}
}

void DlgADExperiment::OnCancel()
{
	CDialog::OnCancel();
}

BOOL DlgADExperiment::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (!m_b_filename)
	{
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NUMNAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SPIN1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK2)->ShowWindow(SW_HIDE);
		m_mfc_browse_path.ShowWindow(SW_HIDE);
		m_b_hide_subsequent = FALSE;
	}

	// load address of items defined for this dialog and load corresponding data
	if (b_ad_experiment)
		p_wave_format = &(options_input->wave_format);

	m_experiment_number = options_input->experiment_number;
	m_insect_number = options_input->insect_number;
	m_cs_basename = options_input->cs_basename;
	if (m_cs_basename.IsEmpty())
		m_cs_basename = _T("data");
	m_cs_pathname = p_db_doc->proposed_data_path_name;
	m_mfc_browse_path.SetWindowTextW(m_cs_pathname);

	m_cs_more_comment = p_wave_format->cs_more_comment;
	load_list(&m_co_concentration, &(options_input->cs_a_concentration), options_input->ics_a_concentration,
		&(p_db_doc->db_table->m_concentration_set));
	load_list(&m_co_stimulus, &(options_input->cs_a_stimulus), options_input->ics_a_stimulus,
		&(p_db_doc->db_table->m_stimulus_set));
	load_list(&m_co_concentration2, &(options_input->cs_a_concentration2), options_input->ics_a_concentration2,
		&(p_db_doc->db_table->m_concentration_set));
	load_list(&m_co_stimulus2, &(options_input->cs_a_stimulus2), options_input->ics_a_stimulus2,
		&(p_db_doc->db_table->m_stimulus_set));
	load_list(&m_co_insect, &(options_input->cs_a_insect), options_input->ics_a_insect, &(p_db_doc->db_table->m_insect_set));
	load_list(&m_co_location, &(options_input->cs_a_location), options_input->ics_a_location,
		&(p_db_doc->db_table->m_location_set));
	load_list(&m_co_sensillum, &(options_input->cs_a_sensillum), options_input->ics_a_sensillum,
		&(p_db_doc->db_table->m_sensillum_set));
	load_list(&m_co_strain, &(options_input->cs_a_strain), options_input->ics_a_strain, &(p_db_doc->db_table->m_strain_set));
	load_list(&m_co_sex, &(options_input->cs_a_sex), options_input->ics_a_sex, &(p_db_doc->db_table->m_sex_set));
	load_list(&m_co_operator, &(options_input->cs_a_operator_name), options_input->ics_a_operator_name,
		&(p_db_doc->db_table->m_operator_set));
	load_list(&m_co_experiment, &(options_input->cs_a_experiment), options_input->ics_a_experiment, &(p_db_doc->db_table->m_experiment_set));
	load_list(&m_co_repeat, nullptr, options_input->ics_a_repeat, nullptr);
	load_list(&m_co_repeat2, nullptr, options_input->ics_a_repeat2, nullptr);

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange32(0, 99999);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN2))->SetRange32(0, 99999);

	if (!b_ad_experiment)
	{
		m_co_stimulus.SetWindowText(p_wave_format->cs_stimulus);
		m_co_concentration.SetWindowText(p_wave_format->cs_concentration);
		m_co_stimulus2.SetWindowText(p_wave_format->cs_stimulus2);
		m_co_concentration2.SetWindowText(p_wave_format->cs_concentration2);
		m_co_location.SetWindowText(p_wave_format->cs_location);
		m_co_sensillum.SetWindowText(p_wave_format->cs_sensillum);
		m_co_strain.SetWindowText(p_wave_format->cs_strain);
		m_co_sex.SetWindowText(p_wave_format->cs_sex);
		m_co_operator.SetWindowText(p_wave_format->cs_operator);
		m_co_insect.SetWindowText(p_wave_format->cs_insect_name);
		m_co_experiment.SetWindowText(p_wave_format->cs_comment);

		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NUMNAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SPIN1)->ShowWindow(SW_HIDE);
		m_mfc_browse_path.ShowWindow(SW_HIDE);
	}
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// save content of the list
int DlgADExperiment::save_list(CComboBox* p_combo, CStringArray* p_s)
{
	int exists = -1;
	if (p_s != nullptr)
	{
		CString csEdit;
		p_combo->GetWindowText(csEdit);
		if (!csEdit.IsEmpty())
		{
			exists = p_combo->FindStringExact(-1, csEdit);
			if (exists == CB_ERR)
				exists = p_combo->AddString(csEdit);
		}

		// loop over each entry of the combo box and save it in the string array
		CString cs_dummy;
		const int i_s_count = p_combo->GetCount();
		p_s->RemoveAll();
		p_s->SetSize(i_s_count);
		for (auto i = 0; i < i_s_count; i++)
		{
			p_combo->GetLBText(i, cs_dummy); // load string from combo box
			p_s->Add(cs_dummy);			// save string into string list
		}
	}
	return exists;
}

// load content of the list and of the corresponding DAO_recordset
void DlgADExperiment::load_list(CComboBox* p_co, const CStringArray* p_spike_element, int i_sel, CdbTableAssociated* pm_set)
{
	// add string from the string array
	p_co->ResetContent();

	// associated list available? yes
	if (p_spike_element != nullptr)
	{
		for (auto i = 0; i < p_spike_element->GetSize(); i++) {
			const CString& descriptor = p_spike_element->GetAt(i);
			if (!descriptor.IsEmpty())
				p_co->AddString(descriptor);
		}
	}
	// no associated list -  insert provisional values in the table
	else
	{
		const auto imax = i_sel + 10;
		CString cs;
		for (auto i = 0; i < imax; i++)
		{
			cs.Format(_T("%i"), i);
			const auto j = p_co->AddString(cs);
			p_co->SetItemData(j, i);
		}
	}

	// scan table and add missing strings
	p_co->SetCurSel(i_sel);
	if (pm_set == nullptr)
		return;

	if (pm_set->IsOpen() && !pm_set->IsBOF())
	{
		COleVariant var_value1;
		pm_set->MoveFirst();
		while (!pm_set->IsEOF())
		{
			pm_set->GetFieldValue(0, var_value1);
			CString cs_field = var_value1.bstrVal;
			if (!cs_field.IsEmpty() && p_co->FindStringExact(0, cs_field) == CB_ERR)
			{
				p_co->AddString(cs_field);
				i_sel = 0; // reset selection if a chain is added
			}
			pm_set->MoveNext();
		}
	}
	p_co->SetCurSel(i_sel);
}

void DlgADExperiment::edit_combo_box(CComboBox* p_co)
{
	DlgEditList dlg;
	dlg.p_co = p_co;
	if (IDOK == dlg.DoModal())
	{
		p_co->ResetContent();
		const auto n_items = dlg.m_cs_array.GetCount();
		for (auto i = 0; i < n_items; i++)
		{
			p_co->AddString(dlg.m_cs_array.GetAt(i));
		}
		p_co->SetCurSel(dlg.m_selected);
	}
	UpdateData(FALSE);
}

void DlgADExperiment::on_bn_clicked_button_insect_name()
{
	edit_combo_box(&m_co_insect);
}

void DlgADExperiment::on_bn_clicked_button_strain()
{
	edit_combo_box(&m_co_strain);
}

void DlgADExperiment::on_bn_clicked_button_sex()
{
	edit_combo_box(&m_co_sex);
}

void DlgADExperiment::on_bn_clicked_button_sensillum()
{
	edit_combo_box(&m_co_sensillum);
}

void DlgADExperiment::on_bn_clicked_button_location()
{
	edit_combo_box(&m_co_location);
}

void DlgADExperiment::on_bn_clicked_button_operator()
{
	edit_combo_box(&m_co_operator);
}

void DlgADExperiment::on_bn_clicked_button_stimulus()
{
	edit_combo_box(&m_co_stimulus);
}

void DlgADExperiment::on_bn_clicked_button_concentration()
{
	edit_combo_box(&m_co_concentration);
}

void DlgADExperiment::on_bn_clicked_button_stimulus2()
{
	edit_combo_box(&m_co_stimulus2);
}

void DlgADExperiment::on_bn_clicked_button_concentration2()
{
	edit_combo_box(&m_co_concentration2);
}

void DlgADExperiment::on_bn_clicked_button_repeat()
{
	edit_combo_box(&m_co_repeat);
}

void DlgADExperiment::on_bn_clicked_button_repeat2()
{
	edit_combo_box(&m_co_repeat2);
}

void DlgADExperiment::on_bn_clicked_button_experiment()
{
	edit_combo_box(&m_co_experiment);
}

void DlgADExperiment::on_en_kill_focus_mfc_edit_browse1()
{
	UpdateData(TRUE);
	m_mfc_browse_path.GetWindowTextW(m_cs_pathname);
	if (m_cs_pathname.GetLength() > 2
		&& m_cs_pathname.Right(1) != _T("\\"))
		m_cs_pathname += _T("\\");

	// check that directory is present - otherwise create...
	auto cs_path = m_cs_pathname.Left(m_cs_pathname.GetLength() - 1);

	// create directory if necessary
	CFileFind cf;
	if (!cs_path.IsEmpty() && !cf.FindFile(cs_path))
	{
		if (!CreateDirectory(cs_path, nullptr))
			AfxMessageBox(IDS_DIRECTORYFAILED);
	}
}

void DlgADExperiment::on_bn_clicked_button_next_id()
{
	const auto p_database = p_db_doc->db_table;
	p_database->m_main_table_set.get_max_id();
	m_insect_number = p_database->m_main_table_set.max_insect_id + 1;
	UpdateData(FALSE);
}
