#include "StdAfx.h"
#include "dbWave.h"
#include "DlgImportFiles.h"

#include "ADAcqDataDoc.h"
#include "resource.h"

#include "dataheader_Atlab.H"
#include "dbWaveDoc.h"
#include "DlgADExperiment.h"
#include "DlgADInputParms.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(DlgImportFiles, CDialog)

DlgImportFiles::DlgImportFiles(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

DlgImportFiles::~DlgImportFiles()
= default;

void DlgImportFiles::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Text(p_dx, IDC_EDIT1, m_n_current_);
	DDX_Text(p_dx, IDC_EDIT12, m_n_files_);
	DDX_Text(p_dx, IDC_EDIT3, m_file_from_);
	DDX_Text(p_dx, IDC_EDIT4, m_file_to_);
	DDX_Check(p_dx, IDC_CHECK1, m_b_read_header);
}

BEGIN_MESSAGE_MAP(DlgImportFiles, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, &DlgImportFiles::on_bn_clicked_cancel)
	ON_BN_CLICKED(ID_START_STOP, &DlgImportFiles::adc_on_bn_clicked_start_stop)
END_MESSAGE_MAP()

// CImportFilesDlg message handlers
BOOL DlgImportFiles::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_n_files_ = m_p_file_name_array->GetSize(); // get nb of files
	m_n_current_ = 1;
	// output file extension
	switch (m_option)
	{
	case GERT: // option noi implemented
		m_ext_ = ".spk";
		break;
	case ATFFILE:
	case ASCIISYNTECH: // option not implemented
	default:
		m_ext_ = ".dat";
		break;
	}

	update_dlg_items();
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgImportFiles::update_dlg_items()
{
	int j = m_n_current_ - 1;
	m_file_from_ = m_p_file_name_array->GetAt(j);
	m_file_to_ = m_file_from_ + m_ext_;
	UpdateData(FALSE);
}

void DlgImportFiles::OnDestroy()
{
	CDialog::OnDestroy();

	const int i_last = m_n_current_ + 1; // index last data file to open
	// adjust size of the file array
	if (m_p_file_name_array->GetSize() > i_last)
		m_p_file_name_array->SetSize(i_last);
}

void DlgImportFiles::on_bn_clicked_cancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

void DlgImportFiles::adc_on_bn_clicked_start_stop()
{
	m_p_converted_files->RemoveAll();

	// loop over the array of files
	for (int i = 0; i < m_n_files_; i++)
	{
		m_file_from_ = m_p_file_name_array->GetAt(i);
		m_file_to_ = m_file_from_ + m_ext_;
		BOOL flag = TRUE;
		UpdateData(FALSE);

		switch (m_option)
		{
		case ATFFILE:
			flag = import_atf_file();
			break;

		case GERT: // option not implemented
		case ASCIISYNTECH: // option not implemented
		default:
			flag = FALSE;
			break;
		}
		// if the conversion went well, add file to database and ask for further info from user
		if (flag)
			m_p_converted_files->Add(m_file_to_);
		m_n_current_++;
	}
	CDialog::OnOK();
}

BOOL DlgImportFiles::import_atf_file()
{
	// load parameters
	const BOOL flag = TRUE;

	// get infos from the source file
	CFileStatus status;
	BOOL b_flag_exists = CFile::GetStatus(m_file_from_, status);
	if (!b_flag_exists || (status.m_attribute & CFile::readOnly))
	{
		update_dlg_items();
		return FALSE;
	}

	// open source file; exit if a problem arises
	const auto p_from = new CStdioFile;
	ASSERT(p_from != NULL);
	CFileException fe; // exception if something get wrong
	if (!p_from->Open(m_file_from_, CFile::modeRead | CFile::shareDenyNone | CFile::typeText, &fe))
	{
		p_from->Abort(); // abort process
		delete p_from; // delete object and exit
		update_dlg_items();
		return FALSE;
	}

	// read infos; exit if file does not have the requested buzzword
	CString cs_dummy;
	CString cs_dummy2;
	CString cs_line;

	// read first line
	p_from->ReadString(cs_line);
	int i = 0;
	AfxExtractSubString(cs_dummy, cs_line, i, '\t');
	i++;
	AfxExtractSubString(cs_dummy2, cs_line, i, '\t');
	if (cs_dummy.Compare(_T("ATF")) != 0 || cs_dummy2.Compare(_T("1.0")))
	{
		p_from->Abort(); 
		delete p_from;
		update_dlg_items();
		return FALSE;
	}

	// create data file object
	// save data
	b_flag_exists = CFile::GetStatus(m_file_to_, status);
	if (b_flag_exists || (status.m_attribute & CFile::readOnly))
	{
		update_dlg_items();
		delete p_from;
		return FALSE;
	}

	auto pTo = new ADAcqDataDoc;
	ASSERT(pTo != NULL);
	if (!pTo->acq_create_file(m_file_to_))
	{
		update_dlg_items();
		delete pTo;
		delete p_from;
		return FALSE;
	}

	// read first line i.e. length of the data header
	p_from->ReadString(cs_line); // 1
	i = 0;
	AfxExtractSubString(cs_dummy, cs_line, i, '\t');
	i++;
	AfxExtractSubString(cs_dummy2, cs_line, i, '\t');
	i++;
	const int n_lines_in_header = _ttoi(cs_dummy);
	int n_columns = _ttoi(cs_dummy2);

	m_scan_count_ = n_columns - 1;
	ASSERT(m_scan_count_ > 0);
	ASSERT(m_scan_count_ <= 16);

	// describe data
	m_x_inst_gain_ = 100.; // tentative value (top to min=2000 mV)
	m_x_rate_ = 10000.0f; // tentative value
	CWaveFormat* pw_f = pTo->get_wave_format();
	pw_f->full_scale_volts = 20.0f; // 20 V full scale
	pw_f->bin_span = 65536; // 16 bits resolution
	pw_f->bin_zero = 0; // ?

	pw_f->mode_encoding = OLx_ENC_2SCOMP;
	pw_f->mode_clock = INTERNAL_CLOCK;
	pw_f->mode_trigger = INTERNAL_TRIGGER;
	pw_f->scan_count = static_cast<short>(m_scan_count_); 
	pw_f->sampling_rate_per_channel = static_cast<float>(m_x_rate_); 
	pw_f->cs_ad_card_name = "Digidata Axon";

	for (int i1 = 0; i1 < m_scan_count_; i1++)
	{
		const int i_chan = (pTo->get_wave_channels_array())->chan_array_add();
		CWaveChan* p_channel = (pTo->get_wave_channels_array())->get_p_channel(i_chan);
		p_channel->am_gaintotal = static_cast<float>(m_x_inst_gain_);
		m_d_span_[i1] = 20000. / m_x_inst_gain_; // span= 20 V max to min
		m_d_bin_val_[i1] = m_d_span_[i1] / 65536.; // divide voltage span into 2exp16 bins
		p_channel->am_amplifiergain = p_channel->am_gaintotal;
		p_channel->am_gainAD = 1;
		p_channel->am_gainpre = 1;
		p_channel->am_gainpost = 1;
		p_channel->am_gainheadstage = 1;
		p_channel->am_adchannel = i1; // channel A/D
	}

	/*
line 1-	ATF	1.0
line 2-	7	3
line 3-	"AcquisitionMode=Gap Free"
line 4-	"Comment="
line 5-	"YTop=200,10"
line 6-	"YBottom=-200,-10"
line 7-	"SweepStartTimesMS=0.000"
line 8-	"SignalsExported=Scaled V,Odour Cmd"
line 9-	"Signals="	"Scaled V"	"Odour Cmd"
line 10- "Time (s)"	"Trace #1 (mV)"	"Trace #1 (Au)"
line 11-	0	141.144	0.0317383
	*/
	// read dummy lines if user requests it or if number of lines != 7
	if (m_b_read_header == FALSE || n_lines_in_header != 7)
	{
		for (i = 0; i <= n_lines_in_header; i++)
			p_from->ReadString(cs_line); // 1
	}
	else
	{
		// 1 acquisition mode "AcquisitionMode=Gap Free"
		p_from->ReadString(cs_line);

		// 2 dummy "comments"
		p_from->ReadString(cs_line);

		// 3 & 4 max & min values "YTop=200,10"
		p_from->ReadString(cs_line);
		cs_line.Replace('=', ',');
		int i = 0;
		CString cs_line2;
		p_from->ReadString(cs_line2);
		cs_line2.Replace('=', ',');

		for (int i_chan = 0; i_chan < m_scan_count_; i_chan++)
		{
			i++;
			AfxExtractSubString(cs_dummy, cs_line, i, ',');
			float x_max = static_cast<float>(_ttof(cs_dummy));
			AfxExtractSubString(cs_dummy2, cs_line2, i, ',');
			float x_min = static_cast<float>(_ttof(cs_dummy2));
			// take the max absolute value
			x_min = abs(x_min);
			x_max = abs(x_max);
			if (x_min > x_max)
				x_max = x_min;
			float x_total = static_cast<float>(x_max * 2.);
			CWaveChan* p_channel = (pTo->get_wave_channels_array())->get_p_channel(i_chan);
			p_channel->am_gaintotal = 20000. / x_total;
		}

		// 5 start time
		p_from->ReadString(cs_line);
		// 6 columns names "SignalsExported=Scaled V,Odour Cmd"
		p_from->ReadString(cs_line);
		cs_line.Replace('=', ',');
		i = 0;
		for (int i_chan = 0; i_chan < m_scan_count_; i_chan++)
		{
			i++;
			AfxExtractSubString(cs_dummy2, cs_line, i, ',');
			cs_dummy2.Replace('"', ' ');
			CWaveChan* p_channel = (pTo->get_wave_channels_array())->get_p_channel(i_chan);
			p_channel->am_csComment = cs_dummy2;
		}

		p_from->ReadString(cs_line);
		// 7 columns scale
		p_from->ReadString(cs_line);
		// (8) title of the line
		p_from->ReadString(cs_line);
	}

	// change acq params & ask for proper descriptors
	if (!get_acquisition_parameters(pTo)
		|| !get_experiment_parameters(pTo))
	{
		pTo->AcqDoc_DataAppendStop();
		pTo->acq_delete_file();
		goto Emergency_exit;
	}

	pw_f->bin_span = 65536; // 16 bits resolution
	pw_f->bin_zero = 0;
	pw_f->mode_encoding = OLx_ENC_BINARY;

	// start conversion
	constexpr int buf_len = 32768;
	pTo->adjust_buffer(buf_len);
	ULONGLONG counter1 = pTo->get_buffer_channel_length();
	int counter2 = 0;
	pTo->AcqDoc_DataAppendStart();
	short* pdata_buf0 = pTo->get_raw_data_buffer();

	TRY
		{
			double d_time_end = 0;
			double d_time_start = 0;
			ULONGLONG global_counter = 0;
			while (counter1 > 0)
			{
				short* pdata_buf = pdata_buf0;
				CString cs_line_tab;
				counter2 = 0;

				for (int ii = 0; ii < buf_len; ii++)
				{
					p_from->ReadString(cs_line_tab); // ; Wave data Signal
					// trap end-of-file condition - this is the only way we get out of this while loop
					if (cs_line_tab.IsEmpty())
					{
						d_time_end = _ttof(cs_dummy2);
						counter1 = 0;
						break;
					}

					// extract time
					int i = 0; // substring index to extract
					AfxExtractSubString(cs_dummy2, cs_line_tab, i, '\t');
					if (global_counter == 0)
						d_time_start = _ttof(cs_dummy2);

					global_counter++;

					// extract channels
					for (int i_chan = 0; i_chan < m_scan_count_; i_chan++)
					{
						i++;
						AfxExtractSubString(cs_dummy, cs_line_tab, i, '\t');
						const double d_value1 = _ttof(cs_dummy);
						const double d_value2 = d_value1 / m_d_bin_val_[i_chan];
						if ((d_value2 > 32768.) || (d_value2 < -32767.))
						{
							MessageBox(_T("Overflow error : decrease the amplifier gain"));
							pTo->AcqDoc_DataAppendStop();
							pTo->acq_delete_file();
							goto Emergency_exit;
						}
						*pdata_buf = static_cast<short>(d_value2);
						pdata_buf++;
						counter2++;
					}
				}

				// save data and update display
				CWnd* p_wnd = GetDlgItem(IDC_STATIC6);
				p_wnd->SetWindowText(cs_dummy2);
				pTo->AcqDoc_DataAppend(pdata_buf0, counter2 * sizeof(short));
			}

			// update rate
			CWaveFormat* pw_f = pTo->get_wave_format();
			const float xx_rate = static_cast<float>(global_counter) / static_cast<float>(d_time_end - d_time_start);
			pw_f->sampling_rate_per_channel = xx_rate;
		}

		// useless but ...
	CATCH(CFileException, fe)
		{
			pTo->AcqDoc_DataAppend(pdata_buf0, counter2 * sizeof(short));
		}
	END_CATCH

	// TODO: update file length?
	pTo->AcqDoc_DataAppendStop();
	pTo->acq_close_file();

	// exit: delete objects
Emergency_exit:
	delete pTo;
	delete p_from;
	return flag;
}

BOOL DlgImportFiles::get_experiment_parameters(const AcqDataDoc* p_to) const
{
	DlgADExperiment dlg; 
	dlg.m_b_filename = FALSE; 
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());
	options_input* pacqD = &(p_app->options_acq_data);
	dlg.options_input = pacqD;
	dlg.p_db_doc = m_pdb_doc;
	const BOOL flag = dlg.DoModal();
	if (IDOK == flag)
	{
		CWaveFormat* pwFTo = p_to->get_wave_format();
		const CWaveFormat* pwFDlg = &pacqD->wave_format;

		pwFTo->cs_ad_card_name = pwFDlg->cs_ad_card_name;
		pwFTo->cs_comment = pwFDlg->cs_comment;
		pwFTo->cs_stimulus = pwFDlg->cs_stimulus;
		pwFTo->cs_concentration = pwFDlg->cs_concentration;
		pwFTo->cs_insect_name = pwFDlg->cs_insect_name;
		pwFTo->cs_location = pwFDlg->cs_location;
		pwFTo->cs_sensillum = pwFDlg->cs_sensillum;
		pwFTo->cs_strain = pwFDlg->cs_strain;
		pwFTo->cs_operator = pwFDlg->cs_operator;
		pwFTo->cs_more_comment = pwFDlg->cs_more_comment;
		pwFTo->cs_stimulus2 = pwFDlg->cs_stimulus2;
		pwFTo->cs_concentration2 = pwFDlg->cs_concentration2;
		pwFTo->cs_sex = pwFDlg->cs_sex;
		pwFTo->insect_id = pwFDlg->insect_id;
		pwFTo->sensillum_id = pwFDlg->sensillum_id;
		return TRUE;
	}
	return FALSE;
}

BOOL DlgImportFiles::get_acquisition_parameters(const AcqDataDoc* p_to)
{
	DlgADInputs dlg2;
	dlg2.m_pw_format = p_to->get_wave_format();
	dlg2.m_pch_array = p_to->get_wave_channels_array();

	// invoke dialog box
	const BOOL flag = dlg2.DoModal();
	if (IDOK == flag)
	{
		for (int i = 0; i < m_scan_count_; i++)
		{
			const CWaveChan* p_channel = (p_to->get_wave_channels_array())->get_p_channel(i);
			m_x_inst_gain_ = p_channel->am_gaintotal;
			m_d_span_[i] = 20000. / m_x_inst_gain_; // span= 20 V max to min
			m_d_bin_val_[i] = m_d_span_[i] / 65536.; // divide voltage span into 2exp16 bins
		}
		return TRUE;
	}
	return FALSE;
}
