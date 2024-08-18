// ADExperi.cpp : implementation file
//
#include "StdAfx.h"
#include "DlgADExperiment.h"
#include "DlgEditList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DlgADExperiment::DlgADExperiment(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgADExperiment::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT_NAME, m_csBasename);
	DDX_Text(pDX, IDC_EDIT_COMMENT, m_csMoreComment);
	DDX_Text(pDX, IDC_EDIT_NUMNAME, m_exptnumber);
	DDX_Text(pDX, IDC_INSECTID, m_insectnumber);

	DDX_Control(pDX, IDC_COMBO_STRAIN, m_coStrain);
	DDX_Control(pDX, IDC_COMBO_SEX, m_coSex);
	DDX_Control(pDX, IDC_COMBO_SENSILLUMNAME, m_coSensillum);
	DDX_Control(pDX, IDC_COMBO_LOCATION, m_coLocation);
	DDX_Control(pDX, IDC_COMBO_OPERATOR, m_coOperator);
	DDX_Control(pDX, IDC_COMBO_INSECTNAME, m_coInsect);
	DDX_Control(pDX, IDC_COMBO_STIMULUS, m_coStimulus);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION, m_coConcentration);
	DDX_Control(pDX, IDC_COMBO_STIMULUS2, m_coStimulus2);
	DDX_Control(pDX, IDC_COMBO_CONCENTRATION2, m_coConcentration2);
	DDX_Control(pDX, IDC_COMBO1_REPEAT, m_coRepeat);
	DDX_Control(pDX, IDC_COMBO1_REPEAT2, m_coRepeat2);
	DDX_Control(pDX, IDC_COMBO_EXPT3, m_coExpt);

	DDX_Check(pDX, IDC_CHECK2, m_bhidesubsequent);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_mfcBrowsePath);
}

BEGIN_MESSAGE_MAP(DlgADExperiment, CDialog)

	ON_BN_CLICKED(IDC_BUTTONINSECTNAME, &DlgADExperiment::OnBnClickedButtoninsectname)
	ON_BN_CLICKED(IDC_BUTTONSTRAIN, &DlgADExperiment::OnBnClickedButtonstrain)
	ON_BN_CLICKED(IDC_BUTTONSEX, &DlgADExperiment::OnBnClickedButtonsex)
	ON_BN_CLICKED(IDC_BUTTONSENSILLUM, &DlgADExperiment::OnBnClickedButtonsensillum)
	ON_BN_CLICKED(IDC_BUTTONLOCATION, &DlgADExperiment::OnBnClickedButtonlocation)
	ON_BN_CLICKED(IDC_BUTTONOPERATOR, &DlgADExperiment::OnBnClickedButtonoperator)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS, &DlgADExperiment::OnBnClickedButtonstimulus)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION, &DlgADExperiment::OnBnClickedButtonconcentration)
	ON_BN_CLICKED(IDC_BUTTONSTIMULUS2, &DlgADExperiment::OnBnClickedButtonstimulus2)
	ON_BN_CLICKED(IDC_BUTTONCONCENTRATION2, &DlgADExperiment::OnBnClickedButtonconcentration2)
	ON_BN_CLICKED(IDC_BUTTONREPEAT, &DlgADExperiment::OnBnClickedButtonrepeat)
	ON_BN_CLICKED(IDC_BUTTONREPEAT2, &DlgADExperiment::OnBnClickedButtonrepeat2)
	ON_BN_CLICKED(IDC_BUTTONEXPT3, &DlgADExperiment::OnBnClickedButtonexpt)

	ON_EN_KILLFOCUS(IDC_MFCEDITBROWSE1, &DlgADExperiment::OnEnKillfocusMfceditbrowse1)
	ON_BN_CLICKED(IDC_BUTTON_NEXTID, &DlgADExperiment::OnBnClickedButtonNextid)
END_MESSAGE_MAP()

void DlgADExperiment::OnOK()
{
	UpdateData(TRUE);

	m_mfcBrowsePath.GetWindowTextW(m_csPathname);
	if (m_csPathname.GetLength() > 2
		&& m_csPathname.Right(1) != _T("\\"))
		m_csPathname += _T("\\");

	// check that directory is present - otherwise create...
	auto cs_path = m_csPathname.Left(m_csPathname.GetLength() - 1);

	// create directory if necessary
	CFileFind cf;
	if (!cs_path.IsEmpty() && !cf.FindFile(cs_path))
	{
		if (!CreateDirectory(cs_path, nullptr))
			AfxMessageBox(IDS_DIRECTORYFAILED);
	}

	// build file name
	int i_experiment_number = int(m_exptnumber);
	CString cs_buf_temp;
	cs_buf_temp.Format(_T("%06.6lu"), i_experiment_number);
	m_szFileName = m_csPathname + m_csBasename + cs_buf_temp + _T(".dat");

	// check if this file is already present, exit if not...
	CFileStatus status;
	auto i_id_response = IDYES; // default: go on if file not found
	if (CFile::GetStatus(m_szFileName, status))
		i_id_response = AfxMessageBox(IDS_FILEOVERWRITE, MB_YESNO | MB_ICONWARNING);
	// no .. find first available number
	if (IDNO == i_id_response)
	{
		BOOL flag = TRUE;
		while (flag)
		{
			i_experiment_number++;
			cs_buf_temp.Format(_T("%06.6lu"), i_experiment_number);
			m_szFileName = m_csPathname + m_csBasename + cs_buf_temp + _T(".dat");
			flag = CFile::GetStatus(m_szFileName, status);
		}
		const auto cs = _T("Next available file name is: ") + m_szFileName;
		i_id_response = AfxMessageBox(cs, MB_YESNO | MB_ICONWARNING);
	}

	// OK .. pass parameters
	if (IDYES == i_id_response)
	{
		// update expt number
		m_exptnumber = i_experiment_number;

		// update file descriptors
		options_inputdata_->cs_pathname = m_csPathname;
		options_inputdata_->cs_basename = m_csBasename;
		options_inputdata_->experiment_number = int(m_exptnumber) + 1;
		options_inputdata_->insect_number = int(m_insectnumber);

		m_pwaveFormat->insect_id = long(m_insectnumber);
		m_pwaveFormat->repeat = options_inputdata_->ics_a_repeat;
		m_pwaveFormat->repeat2 = options_inputdata_->ics_a_repeat2;
		m_pwaveFormat->cs_more_comment = m_csMoreComment;

		// save descriptors into waveFormat (data acq file descriptor) and update database
		m_coStimulus.GetWindowText(m_pwaveFormat->cs_stimulus);
		m_coConcentration.GetWindowText(m_pwaveFormat->cs_concentration);
		m_coStimulus2.GetWindowText(m_pwaveFormat->cs_stimulus2);
		m_coConcentration2.GetWindowText(m_pwaveFormat->cs_concentration2);
		m_coLocation.GetWindowText(m_pwaveFormat->cs_location);
		m_coSensillum.GetWindowText(m_pwaveFormat->cs_sensillum);
		m_coStrain.GetWindowText(m_pwaveFormat->cs_strain);
		m_coSex.GetWindowText(m_pwaveFormat->cs_sex);
		m_coOperator.GetWindowText(m_pwaveFormat->cs_operator);
		m_coInsect.GetWindowText(m_pwaveFormat->cs_insect_name);
		m_coExpt.GetWindowText(m_pwaveFormat->cs_comment);

		// save data into commons
		options_inputdata_->ics_a_concentration = SaveList(&m_coConcentration, &(options_inputdata_->cs_a_concentration));
		options_inputdata_->ics_a_stimulus = SaveList(&m_coStimulus, &(options_inputdata_->cs_a_stimulus));
		options_inputdata_->ics_a_insect = SaveList(&m_coInsect, &(options_inputdata_->cs_a_insect));
		options_inputdata_->ics_a_location = SaveList(&m_coLocation, &(options_inputdata_->cs_a_location));
		options_inputdata_->ics_a_sensillum = SaveList(&m_coSensillum, &(options_inputdata_->cs_a_sensillum));
		options_inputdata_->ics_a_strain = SaveList(&m_coStrain, &(options_inputdata_->cs_a_strain));
		options_inputdata_->ics_a_sex = SaveList(&m_coSex, &(options_inputdata_->cs_a_sex));
		options_inputdata_->ics_a_operator_name = SaveList(&m_coOperator, &(options_inputdata_->cs_a_operator_name));
		options_inputdata_->ics_a_concentration2 = SaveList(&m_coConcentration2, &(options_inputdata_->cs_a_concentration2));
		options_inputdata_->ics_a_stimulus2 = SaveList(&m_coStimulus2, &(options_inputdata_->cs_a_stimulus2));
		options_inputdata_->ics_a_repeat = SaveList(&m_coRepeat, nullptr);
		options_inputdata_->ics_a_repeat2 = SaveList(&m_coRepeat2, nullptr);
		options_inputdata_->ics_a_experiment = SaveList(&m_coExpt, &(options_inputdata_->cs_a_experiment));

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

	if (!m_bFilename)
	{
		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NUMNAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SPIN1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK2)->ShowWindow(SW_HIDE);
		m_mfcBrowsePath.ShowWindow(SW_HIDE);
		m_bhidesubsequent = FALSE;
	}

	// load address of items defined for this dialog and load corresp data
	if (m_bADexpt)
		m_pwaveFormat = &(options_inputdata_->wave_format);

	m_exptnumber = options_inputdata_->experiment_number;
	m_insectnumber = options_inputdata_->insect_number;
	m_csBasename = options_inputdata_->cs_basename;
	if (m_csBasename.IsEmpty())
		m_csBasename = _T("data");
	m_csPathname = m_pdbDoc->proposed_data_path_name;
	m_mfcBrowsePath.SetWindowTextW(m_csPathname);

	m_csMoreComment = m_pwaveFormat->cs_more_comment;
	LoadList(&m_coConcentration, &(options_inputdata_->cs_a_concentration), options_inputdata_->ics_a_concentration,
		&(m_pdbDoc->db_table->m_concentration_set));
	LoadList(&m_coStimulus, &(options_inputdata_->cs_a_stimulus), options_inputdata_->ics_a_stimulus,
		&(m_pdbDoc->db_table->m_stimulus_set));
	LoadList(&m_coConcentration2, &(options_inputdata_->cs_a_concentration2), options_inputdata_->ics_a_concentration2,
		&(m_pdbDoc->db_table->m_concentration_set));
	LoadList(&m_coStimulus2, &(options_inputdata_->cs_a_stimulus2), options_inputdata_->ics_a_stimulus2,
		&(m_pdbDoc->db_table->m_stimulus_set));
	LoadList(&m_coInsect, &(options_inputdata_->cs_a_insect), options_inputdata_->ics_a_insect, &(m_pdbDoc->db_table->m_insect_set));
	LoadList(&m_coLocation, &(options_inputdata_->cs_a_location), options_inputdata_->ics_a_location,
		&(m_pdbDoc->db_table->m_location_set));
	LoadList(&m_coSensillum, &(options_inputdata_->cs_a_sensillum), options_inputdata_->ics_a_sensillum,
		&(m_pdbDoc->db_table->m_sensillum_set));
	LoadList(&m_coStrain, &(options_inputdata_->cs_a_strain), options_inputdata_->ics_a_strain, &(m_pdbDoc->db_table->m_strain_set));
	LoadList(&m_coSex, &(options_inputdata_->cs_a_sex), options_inputdata_->ics_a_sex, &(m_pdbDoc->db_table->m_sex_set));
	LoadList(&m_coOperator, &(options_inputdata_->cs_a_operator_name), options_inputdata_->ics_a_operator_name,
		&(m_pdbDoc->db_table->m_operator_set));
	LoadList(&m_coExpt, &(options_inputdata_->cs_a_experiment), options_inputdata_->ics_a_experiment, &(m_pdbDoc->db_table->m_experiment_set));
	LoadList(&m_coRepeat, nullptr, options_inputdata_->ics_a_repeat, nullptr);
	LoadList(&m_coRepeat2, nullptr, options_inputdata_->ics_a_repeat2, nullptr);

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange32(0, 99999);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN2))->SetRange32(0, 99999);

	if (!m_bADexpt)
	{
		m_coStimulus.SetWindowText(m_pwaveFormat->cs_stimulus);
		m_coConcentration.SetWindowText(m_pwaveFormat->cs_concentration);
		m_coStimulus2.SetWindowText(m_pwaveFormat->cs_stimulus2);
		m_coConcentration2.SetWindowText(m_pwaveFormat->cs_concentration2);
		m_coLocation.SetWindowText(m_pwaveFormat->cs_location);
		m_coSensillum.SetWindowText(m_pwaveFormat->cs_sensillum);
		m_coStrain.SetWindowText(m_pwaveFormat->cs_strain);
		m_coSex.SetWindowText(m_pwaveFormat->cs_sex);
		m_coOperator.SetWindowText(m_pwaveFormat->cs_operator);
		m_coInsect.SetWindowText(m_pwaveFormat->cs_insect_name);
		m_coExpt.SetWindowText(m_pwaveFormat->cs_comment);

		GetDlgItem(IDC_STATIC1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC2)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC3)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_NUMNAME)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SPIN1)->ShowWindow(SW_HIDE);
		m_mfcBrowsePath.ShowWindow(SW_HIDE);
	}
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// save content of the list
int DlgADExperiment::SaveList(CComboBox* p_combo, CStringArray* p_s)
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

// load content of the list and of the corresponding DAOrecordset
void DlgADExperiment::LoadList(CComboBox* p_combo, CStringArray* p_s, int i_sel, CdbTableAssociated* pm_set)
{
	// add string from the string array
	p_combo->ResetContent();

	// associated list available? yes
	if (p_s != nullptr)
	{
		for (auto i = 0; i < p_s->GetSize(); i++) {
			const CString& descriptor = p_s->GetAt(i);
			if (!descriptor.IsEmpty())
				p_combo->AddString(descriptor);
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
			const auto j = p_combo->AddString(cs);
			p_combo->SetItemData(j, i);
		}
	}

	// scan table and add missing strings
	p_combo->SetCurSel(i_sel);
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
			if (!cs_field.IsEmpty() && p_combo->FindStringExact(0, cs_field) == CB_ERR)
			{
				p_combo->AddString(cs_field);
				i_sel = 0; // reset selection if a chain is added
			}
			pm_set->MoveNext();
		}
	}
	p_combo->SetCurSel(i_sel);
}

void DlgADExperiment::EditComboBox(CComboBox* pCo)
{
	DlgEditList dlg;
	dlg.pCo = pCo;
	if (IDOK == dlg.DoModal())
	{
		pCo->ResetContent();
		const auto n_items = dlg.m_csArray.GetCount();
		for (auto i = 0; i < n_items; i++)
		{
			pCo->AddString(dlg.m_csArray.GetAt(i));
		}
		pCo->SetCurSel(dlg.m_selected);
	}
	UpdateData(FALSE);
}

void DlgADExperiment::OnBnClickedButtoninsectname()
{
	EditComboBox(&m_coInsect);
}

void DlgADExperiment::OnBnClickedButtonstrain()
{
	EditComboBox(&m_coStrain);
}

void DlgADExperiment::OnBnClickedButtonsex()
{
	EditComboBox(&m_coSex);
}

void DlgADExperiment::OnBnClickedButtonsensillum()
{
	EditComboBox(&m_coSensillum);
}

void DlgADExperiment::OnBnClickedButtonlocation()
{
	EditComboBox(&m_coLocation);
}

void DlgADExperiment::OnBnClickedButtonoperator()
{
	EditComboBox(&m_coOperator);
}

void DlgADExperiment::OnBnClickedButtonstimulus()
{
	EditComboBox(&m_coStimulus);
}

void DlgADExperiment::OnBnClickedButtonconcentration()
{
	EditComboBox(&m_coConcentration);
}

void DlgADExperiment::OnBnClickedButtonstimulus2()
{
	EditComboBox(&m_coStimulus2);
}

void DlgADExperiment::OnBnClickedButtonconcentration2()
{
	EditComboBox(&m_coConcentration2);
}

void DlgADExperiment::OnBnClickedButtonrepeat()
{
	EditComboBox(&m_coRepeat);
}

void DlgADExperiment::OnBnClickedButtonrepeat2()
{
	EditComboBox(&m_coRepeat2);
}

void DlgADExperiment::OnBnClickedButtonexpt()
{
	EditComboBox(&m_coExpt);
}

void DlgADExperiment::OnEnKillfocusMfceditbrowse1()
{
	UpdateData(TRUE);
	m_mfcBrowsePath.GetWindowTextW(m_csPathname);
	if (m_csPathname.GetLength() > 2
		&& m_csPathname.Right(1) != _T("\\"))
		m_csPathname += _T("\\");

	// check that directory is present - otherwise create...
	auto cs_path = m_csPathname.Left(m_csPathname.GetLength() - 1);

	// create directory if necessary
	CFileFind cf;
	if (!cs_path.IsEmpty() && !cf.FindFile(cs_path))
	{
		if (!CreateDirectory(cs_path, nullptr))
			AfxMessageBox(IDS_DIRECTORYFAILED);
	}
}

void DlgADExperiment::OnBnClickedButtonNextid()
{
	const auto p_database = m_pdbDoc->db_table;
	p_database->m_main_table_set.get_max_id();
	m_insectnumber = p_database->m_main_table_set.max_insectID + 1;
	UpdateData(FALSE);
}
