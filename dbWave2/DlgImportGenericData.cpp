#include "StdAfx.h"
#include "resource.h"
#include "DlgImportGenericData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgImportGenericData::DlgImportGenericData(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgImportGenericData::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_FILELIST, m_file_drop_list);
	DDX_Control(p_dx, IDC_PRECISION, m_combo_precision);
	DDX_Text(p_dx, IDC_CHANNELCOMMENT, m_ad_channel_comment);
	DDX_Text(p_dx, IDC_CHANNELGAIN, m_ad_channel_gain);
	DDX_Text(p_dx, IDC_CHANNELNO, m_ad_channel_chan);
	DDX_Text(p_dx, IDC_NUMBEROFCHANNELS, m_nb_ad_channels);
	DDX_Text(p_dx, IDC_NUMBEROFRUNS, m_nb_runs);
	DDX_Text(p_dx, IDC_SAMPLINGRATE, m_sampling_rate);
	DDX_Text(p_dx, IDC_SKIPNBYTES, m_skip_n_bytes);
	DDX_Text(p_dx, IDC_VMAX, m_voltage_max);
	DDX_Text(p_dx, IDC_VMIN, m_voltage_min);
	DDX_Text(p_dx, IDC_FILETITLE, m_cs_file_title);
	DDX_Check(p_dx, IDC_CHECK1, m_b_preview_on);
}

BEGIN_MESSAGE_MAP(DlgImportGenericData, CDialog)
	ON_CBN_SELCHANGE(IDC_PRECISION, on_sel_change_precision)
	ON_EN_CHANGE(IDC_NUMBEROFCHANNELS, on_en_change_number_of_channels)
	ON_BN_CLICKED(IDC_MULTIPLERUNS, on_multiple_runs)
	ON_BN_CLICKED(IDC_SINGLERUN, on_single_run)
	ON_EN_CHANGE(IDC_CHANNELNO, on_en_change_channel_no)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, on_delta_pos_spin_nb_channels)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, on_delta_pos_spin_no_chan)
	ON_BN_CLICKED(IDC_IMPORT, on_import)
	ON_BN_CLICKED(IDC_IMPORTALL, on_import_all)
	ON_BN_CLICKED(IDC_CHECK1, on_set_preview)
	ON_CBN_SELCHANGE(IDC_FILELIST, on_sel_change_file_list)
	ON_BN_CLICKED(IDC_OFFSETBINARY, on_offset_binary)
	ON_BN_CLICKED(IDC_TWOSCOMPLEMENT, on_twos_complement)
	ON_EN_CHANGE(IDC_SKIPNBYTES, on_en_change_skip_n_bytes)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, on_delta_pos_skip_n_bytes)
	ON_BN_CLICKED(IDC_CHECKSAPID, on_sapid3_5)
	ON_EN_CHANGE(IDC_CHANNELCOMMENT, on_en_change_channel_comment)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportGenericDataDlg message handlers

// init dialog box. Prepare controls and copy relevant parameters into them
// hide controls when the dialog box is called only to adjust parameters
// otherwise, copy list of files into comboBox

BOOL DlgImportGenericData::OnInitDialog()
{
	CDialog::OnInitDialog(); // normal initialization CSpinButtonCtrl

	// subclass edit control
	VERIFY(mm_nb_ad_channels.SubclassDlgItem(IDC_NUMBEROFCHANNELS, this));
	VERIFY(mm_ad_channel_chan.SubclassDlgItem(IDC_CHANNELNO, this));
	VERIFY(mm_skip_n_bytes.SubclassDlgItem(IDC_SKIPNBYTES, this));
	VERIFY(m_chart_data_wnd.SubclassDlgItem(IDC_DISPLAYSOURCE, this));

	// inhibit buttons linked to conversion if requested
	int nIDFocus;
	if (!b_convert)
	{
		GetDlgItem(IDC_FILELIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IMPORT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IMPORTALL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_FILESOURCE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DISPLAYSOURCE)->ShowWindow(SW_HIDE);
		nIDFocus = IDOK;
	}
	else
	{
		// transfer file names into CComboBox
		const int last_index = m_p_file_name_array->GetSize();
		for (int i = 0; i < last_index; i++)
		{
			CString filename = m_p_file_name_array->GetAt(i);
			const int i_count = filename.GetLength() - filename.ReverseFind('\\') - 1;
			m_file_drop_list.AddString(filename.Right(i_count));
			m_file_drop_list.SetItemData(i, i);
		}
		m_file_drop_list.SetCurSel(0); // select first item in the list
		nIDFocus = IDC_IMPORT;
		static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->SetCheck(options_import->is_sapid_3_5);
		if (!options_import->preview_requested)
			GetDlgItem(IDC_DISPLAYSOURCE)->ShowWindow(SW_HIDE);
	}

	// copy relevant data into controls
	m_ad_channel_chan = 1;
	update_controls_from_struct();
	// convert selected file if options say so
	if (b_convert && options_import->preview_requested)
	{
		m_b_preview_on = options_import->preview_requested;
		// get options as set by caller
		static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(options_import->preview_requested);
		set_file_names(0);
		update_preview();
	}

	GetDlgItem(nIDFocus)->SetFocus();
	return FALSE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// when user check "multiple runs", enable dependent controls: "nbruns" and
// corresponding edit control
// otherwise disable them

void DlgImportGenericData::enable_run_parameters()
{
	int idc_button = IDC_SINGLERUN; // check button concerning structure of
	if (!options_import->is_single_run) // file; either a single acquisition bout
		idc_button = IDC_MULTIPLERUNS; // or more than one
	CheckRadioButton(IDC_SINGLERUN, IDC_MULTIPLERUNS, idc_button);
	GetDlgItem(IDC_NUMBEROFRUNS)->EnableWindow(!options_import->is_single_run); // edit window
	GetDlgItem(IDC_STATIC1)->EnableWindow(!options_import->is_single_run); // static
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(options_import->preview_requested);
	m_b_changed = TRUE;
}

// copy values from structure iivO into controls
// this routine is called when the dialog box is opened

void DlgImportGenericData::update_controls_from_struct()
{
	// copy structure content into controls
	m_nb_runs = options_import->nb_runs; // load number of repetitions & enable corresponding
	GetDlgItem(IDC_SAMPLINGRATE)->EnableWindow(!options_import->is_sapid_3_5);
	enable_run_parameters(); // enable dependent dlg items
	m_sampling_rate = options_import->sampling_rate; // sampling rate per channel (in Hertz)
	m_nb_ad_channels = options_import->nb_channels; // number of data acquisition channels
	options_import->p_wave_chan_array->chan_array_set_size(options_import->nb_channels);

	int IDC_button = IDC_OFFSETBINARY; // check button concerning data encoding mode
	if (options_import->encoding_mode > 0)
		IDC_button = IDC_TWOSCOMPLEMENT;
	CheckRadioButton(IDC_OFFSETBINARY, IDC_TWOSCOMPLEMENT, IDC_button);

	// set precision combo box
	int precision_index = 1; // default = 1 (12 bits)
	if (options_import->bits_precision == 16) // 16 bits = 2nd position
		precision_index = 2;
	else if (options_import->bits_precision == 8) // 8 bits = initial position
		precision_index = 0;
	else if (options_import->bits_precision == 24) // 24 bits
		precision_index = 3;
	else if (options_import->bits_precision == 32) // 32 bits
		precision_index = 4;
	m_combo_precision.SetCurSel(precision_index);

	m_voltage_max = options_import->voltage_max; // set voltage max
	m_voltage_min = options_import->voltage_min; // set voltage min
	m_skip_n_bytes = options_import->skip_n_bytes; // set "skip n bytes"
	m_cs_file_title = options_import->title; // file global comment
	const CWaveChan* p_channel = options_import->p_wave_chan_array->get_p_channel(m_ad_channel_chan - 1);
	m_ad_channel_gain = static_cast<float>(p_channel->am_gaintotal); // set gain
	m_ad_channel_comment = p_channel->am_csComment; // and comment

	// set limits associated spin controls
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN2))->SetRange(1, 1024);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange(1, m_nb_ad_channels);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN3))->SetRange(1, 32767);

	UpdateData(FALSE); // refresh controls
	m_b_changed = FALSE;
}

// copy values from controls into structure iivO
// this routine is called on exit

void DlgImportGenericData::update_struct_from_controls()
{
	UpdateData(TRUE);
	// copy controls into structure
	if (IsDlgButtonChecked(IDC_SINGLERUN))
		options_import->is_single_run = TRUE;
	else
	{
		options_import->is_single_run = FALSE;
		options_import->nb_runs = m_nb_runs;
	}
	options_import->sampling_rate = m_sampling_rate; // sampling rate per channel (in Herz)
	options_import->nb_channels = m_nb_ad_channels; // number of data acquisition channels
	// encoding
	if (IsDlgButtonChecked(IDC_OFFSETBINARY))
		options_import->encoding_mode = OLx_ENC_BINARY;
	else
		options_import->encoding_mode = OLx_ENC_2SCOMP;

	// precision is updated when a change is made

	options_import->voltage_max = m_voltage_max; // set voltage max
	options_import->voltage_min = m_voltage_min; // set voltage min
	options_import->skip_n_bytes = m_skip_n_bytes; // set "skip n bytes"
	options_import->title = m_cs_file_title; // file global comment

	CWaveChan* p_channel = options_import->p_wave_chan_array->get_p_channel(m_ad_channel_chan - 1);
	p_channel->am_gaintotal = m_ad_channel_gain; // set gain
	p_channel->am_amplifiergain = p_channel->am_gaintotal;
	p_channel->am_gainAD = 1;
	p_channel->am_gainpre = 1;
	p_channel->am_gainpost = 1;
	p_channel->am_gainheadstage = 1;
	p_channel->am_csComment = m_ad_channel_comment; // and comment

	// adjust size of chan descriptors array
	options_import->p_wave_chan_array->chan_array_set_size(m_nb_ad_channels);

	m_b_changed = FALSE;
}

// exit dialog box
// save iivO parameters
// copy names of the files converted back into the name array

void DlgImportGenericData::OnOK()
{
	if (m_b_changed)
		update_struct_from_controls(); // update structures with options

	// remove names that were not converted
	if (b_convert)
	{
		int last_index = m_p_file_name_array->GetUpperBound();
		for (int i = last_index; i >= 0; i--) // loop backwards
		{
			CString filename = m_p_file_name_array->GetAt(i);
			int i_count = filename.Find(_T("AW_"));
			if (i_count < 0) // delete unconverted file name
				m_p_file_name_array->RemoveAt(i);
		}
	}
	// base class routine...
	CDialog::OnOK();
}

// change parameter describing the precision of the data
// convert CCombobox selection into a number of bits, store result into iivO

void DlgImportGenericData::on_sel_change_precision()
{
	UpdateData(TRUE); // load data from controls
	// set precision combo box
	const int precision_index = m_combo_precision.GetCurSel();
	int value; // convert corresponding precision  value
	switch (precision_index)
	{
	case 0: value = 8;
		break;
	case 1: value = 12;
		break;
	case 2: value = 16;
		break;
	case 3: value = 24;
		break;
	case 4: value = 32;
		break;
	default: value = 12;
		break;
	}
	// action only if different
	if (value != options_import->bits_precision)
	{
		options_import->bits_precision = value;
		update_preview();
	}
	m_b_changed = TRUE;
}

// On check : set flag single / multiple run

void DlgImportGenericData::on_multiple_runs()
{
	if (IsDlgButtonChecked(IDC_MULTIPLERUNS)
		&& options_import->is_single_run != FALSE)
	{
		options_import->is_single_run = FALSE;
		enable_run_parameters();
		// UpdatePreview();
	}
	m_b_changed = TRUE;
}

void DlgImportGenericData::on_single_run()
{
	if (IsDlgButtonChecked(IDC_SINGLERUN)
		&& options_import->is_single_run != TRUE)
	{
		options_import->is_single_run = TRUE;
		enable_run_parameters();
		// UpdatePreview();
	}
	m_b_changed = TRUE;
}

// change the number of data acquisition channels
//
// this value has an impact on the channel descriptors, an array, which
// size should be modified accordingly
// the edit control that maintain this value is subclassed by a CEditCtrl
// which traps and filter keyboard hits. The associated spin control does not
// communicate directly to this control but to the dialog box.
// this routine get the data from the edit control, evaluate the result and
// restore flags to enable CEditCtrl to receive new data

void DlgImportGenericData::on_en_change_number_of_channels()
{
	if (!mm_nb_ad_channels.m_b_entry_done)
		return;

	const UINT nb_AD_channels = m_nb_ad_channels;
	mm_nb_ad_channels.on_en_change(this, m_nb_ad_channels, 1, -1);

	//  limit max of chan parameters
	if (m_nb_ad_channels < 1) // check that there is at least one chan
		m_nb_ad_channels = 1; // then change the limit of the spin

	// action if value has changed
	if (m_nb_ad_channels != nb_AD_channels)
	{
		options_import->nb_channels = m_nb_ad_channels;
		options_import->p_wave_chan_array->chan_array_set_size(m_nb_ad_channels);
		static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange(1, m_nb_ad_channels);
		if (m_ad_channel_chan > m_nb_ad_channels) // and update dependent chan no
		{
			m_ad_channel_chan = m_nb_ad_channels;
			UpdateData(FALSE);
		}
		update_preview();
	}
	m_b_changed = TRUE;
}

// change channel number
//
// each data acquisition channel is associated with descriptors
// which can be modified by the user. This control allow the selection
// of one descriptor
// same comments as with OnEnChange_Number_of_channels()
// operations:
// - evaluate and clip irrelevant values
// - save current parameters
// - load parameters for new channel

void DlgImportGenericData::on_en_change_channel_no()
{
	if (!mm_ad_channel_chan.m_b_entry_done)
		return;

	UINT previous_channel = m_ad_channel_chan;
	mm_ad_channel_chan.on_en_change(this, m_ad_channel_chan, 1, -1);

	// check limits of m_adChannelChan
	if (m_ad_channel_chan < 1)
		m_ad_channel_chan = 1;
	if (m_ad_channel_chan > m_nb_ad_channels)
		m_ad_channel_chan = m_nb_ad_channels;
	if (m_ad_channel_chan != previous_channel)
	{
		// save previous data
		previous_channel = m_ad_channel_chan;
		UpdateData(TRUE); // load data from controls
		m_ad_channel_chan = previous_channel;
		CWaveChan* p_channel = options_import->p_wave_chan_array->get_p_channel(previous_channel - 1);
		p_channel->am_gaintotal = m_ad_channel_gain; // set gain
		p_channel->am_amplifiergain = p_channel->am_gaintotal;
		p_channel->am_csComment = m_ad_channel_comment; // and comment
		// point to new channel: add new descriptors if necessary
		options_import->p_wave_chan_array->chan_array_set_size(m_nb_ad_channels);

		// load data from new current channel
		p_channel = options_import->p_wave_chan_array->get_p_channel(m_ad_channel_chan - 1);
		m_ad_channel_gain = static_cast<float>(p_channel->am_gaintotal); // set gain
		m_ad_channel_comment = p_channel->am_csComment; // and comment
		UpdateData(FALSE);
	}

	m_b_changed = TRUE;
}

// change size of data header... ie offset from start of file to data

void DlgImportGenericData::on_en_change_skip_n_bytes()
{
	if (!mm_skip_n_bytes.m_b_entry_done)
		return;

	const UINT skip_n_bytes = m_skip_n_bytes;
	mm_skip_n_bytes.on_en_change(this, m_skip_n_bytes, 1, -1);

	// check limits of m_adChannelChan
	if (m_skip_n_bytes < 0)
		m_skip_n_bytes = 0;
	UpdateData(FALSE);

	if (m_skip_n_bytes != skip_n_bytes)
	{
		options_import->skip_n_bytes = static_cast<int>(m_skip_n_bytes);
		update_preview();
	}
	m_b_changed = TRUE;
}

// spin control associated with Edit control "nb of data channels"
// convert notification message into flags adapted to the CEditCtrl

void DlgImportGenericData::on_delta_pos_spin_nb_channels(NMHDR* p_nmhdr, LRESULT* p_result)
{
	NM_UPDOWN* p_nm_up_down = reinterpret_cast<NMUPDOWN*>(p_nmhdr);
	if (p_nm_up_down->iDelta > 0)
		mm_nb_ad_channels.m_n_char = VK_UP;
	else
		mm_nb_ad_channels.m_n_char = VK_DOWN;
	mm_nb_ad_channels.m_b_entry_done = TRUE;
	*p_result = 0;
}

// spin control associated with channel no
// same remarks as above

void DlgImportGenericData::on_delta_pos_spin_no_chan(NMHDR* p_nmhdr, LRESULT* p_result)
{
	NM_UPDOWN* p_nm_up_down = reinterpret_cast<NMUPDOWN*>(p_nmhdr);
	if (p_nm_up_down->iDelta > 0)
		mm_ad_channel_chan.m_n_char = VK_UP;
	else
		mm_ad_channel_chan.m_n_char = VK_DOWN;
	mm_ad_channel_chan.m_b_entry_done = TRUE;
	*p_result = 0;
}

void DlgImportGenericData::on_delta_pos_skip_n_bytes(NMHDR* p_nmhdr, LRESULT* p_result)
{
	NM_UPDOWN* p_nm_up_down = reinterpret_cast<NMUPDOWN*>(p_nmhdr);
	if (p_nm_up_down->iDelta > 0)
		mm_skip_n_bytes.m_n_char = VK_UP;
	else
		mm_skip_n_bytes.m_n_char = VK_DOWN;
	mm_skip_n_bytes.m_b_entry_done = TRUE;
	*p_result = 0;
}

//////////////////////////////////////////////////////////////////////////
// functions  related to file browse and management

// import a single file (the one that is currently selected)
// - read filename array index from control
// - get name from filename array
// - convert file
// - delete current item from combobox
// - select next item
// if combobox is empty, exit dialog.

void DlgImportGenericData::on_import()
{
	if (!m_b_import_all)
		update_struct_from_controls();

	const int cur_sel = m_file_drop_list.GetCurSel();
	const int index = m_file_drop_list.GetItemData(cur_sel);
	set_file_names(index);

	// convert file
	update_wave_descriptors(&m_acq_data_file);
	m_acq_data_file.save_as(m_file_dest);
	m_p_file_name_array->SetAt(index, m_file_dest);

	// delete drop list item CComboBox
	m_file_drop_list.DeleteString(cur_sel);
	if (m_file_drop_list.GetCount() > 0)
	{
		m_file_drop_list.SetCurSel(0);
		on_sel_change_file_list();
	}
	else
		OnOK();
}

// import all files at once: loop through all members of the combo box

void DlgImportGenericData::on_import_all()
{
	m_b_import_all = TRUE;
	update_struct_from_controls();
	const int n_items = m_file_drop_list.GetCount();
	for (int i = 0; i < n_items; i++)
		on_import();
	m_b_import_all = FALSE;
}

// update preview file

void DlgImportGenericData::update_preview()
{
	// convert data
	if (m_file_source != m_file_old)
	{
		CFileStatus status;
		const auto b_open = CFile::GetStatus(m_file_source, status);
		m_acq_data_file.open_acq_file(m_file_source, status);
		m_file_old = m_file_source;
	}
	m_acq_data_file.set_reading_buffer_dirty(); // invalidate data buffer
	update_wave_descriptors(&m_acq_data_file);

	// display data
	if (!m_b_preview_on)
		return;

	const int n_doc_channels = m_acq_data_file.get_wave_format()->scan_count;
	m_chart_data_wnd.attach_data_file(&m_acq_data_file);
	const CSize l_size = m_chart_data_wnd.get_rect_size();
	m_chart_data_wnd.resize_channels(l_size.cx, m_acq_data_file.get_doc_channel_length());
	if (m_chart_data_wnd.get_channel_list_size() < n_doc_channels) // add envelopes if necessary
	{
		for (int j_doc_channel = 1; j_doc_channel < n_doc_channels; j_doc_channel++)
			m_chart_data_wnd.add_channel_list_item(j_doc_channel, 0);
	}
	// load data from document and adjust display
	m_chart_data_wnd.get_data_from_doc(0, m_acq_data_file.get_doc_channel_length() - 1);
	for (int i = 0; i < n_doc_channels; i++)
	{
		int max, min;
		CChanlistItem* chan = m_chart_data_wnd.get_channel_list_item(i);
		chan->get_max_min(&max, &min);
		// split curves if requested by options
		const int i_extent = MulDiv(max - min + 1, 11, 10);
		const int i_zero = (max + min) / 2;
		chan->set_y_extent(i_extent);
		chan->set_y_zero(i_zero);
	}
	m_chart_data_wnd.Invalidate();
}

void DlgImportGenericData::on_set_preview()
{
	// get latest from controls
	options_import->preview_requested = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	int n_cmd_show = SW_SHOW;
	if (!options_import->preview_requested)
		n_cmd_show = SW_HIDE;
	GetDlgItem(IDC_DISPLAYSOURCE)->ShowWindow(n_cmd_show);

	// if preview OFF: hide display source
	if (!options_import->preview_requested && m_b_preview_on)
	{
		m_b_preview_on = FALSE;
	}
	else if (options_import->preview_requested && !m_b_preview_on)
	{
		// build current file
		set_file_names(m_file_drop_list.GetItemData(m_file_drop_list.GetCurSel()));
		m_b_preview_on = TRUE;
		update_preview();
	}
}

void DlgImportGenericData::on_sel_change_file_list()
{
	const int cur_sel = m_file_drop_list.GetCurSel();
	const int index = m_file_drop_list.GetItemData(cur_sel);
	const CString file_source = m_p_file_name_array->GetAt(index);
	if (file_source.CompareNoCase(m_file_source) != 0)
	{
		set_file_names(index);
		update_preview();
	}
}

void DlgImportGenericData::on_offset_binary()
{
	const UINT state = IsDlgButtonChecked(IDC_OFFSETBINARY);
	if (state == 1
		&& options_import->encoding_mode != OLx_ENC_BINARY)
	{
		options_import->encoding_mode = OLx_ENC_BINARY;
		update_preview();
	}
	m_b_changed = TRUE;
}

void DlgImportGenericData::on_twos_complement()
{
	const UINT state = IsDlgButtonChecked(IDC_TWOSCOMPLEMENT);
	if (state == 1
		&& options_import->encoding_mode != OLx_ENC_2SCOMP)
	{
		options_import->encoding_mode = OLx_ENC_2SCOMP;
		update_preview();
	}
	m_b_changed = TRUE;
}

void DlgImportGenericData::set_file_names(int index)
{
	m_file_source = m_p_file_name_array->GetAt(index);
	const int i_count = m_file_source.ReverseFind('\\') + 1;
	const CString cs_path = m_file_source.Left(i_count);
	const int i_count2 = m_file_source.Find('.') - i_count + 1;
	m_file_dest = _T("AW_") + m_file_source.Mid(i_count, i_count2);
	m_file_dest = cs_path + m_file_dest;
	m_file_dest += _T("dat");
}

void DlgImportGenericData::on_sapid3_5()
{
	// get latest from controls
	if (options_import->is_sapid_3_5 != static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->GetCheck())
	{
		options_import->is_sapid_3_5 = static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->GetCheck();
		if (options_import->is_sapid_3_5)
		{
			options_import->is_single_run = TRUE;
			options_import->bits_precision = 12;
			options_import->skip_n_bytes = 7;
			options_import->nb_channels = 1;
			update_controls_from_struct();
		}
		GetDlgItem(IDC_SAMPLINGRATE)->EnableWindow(!options_import->is_sapid_3_5);
		update_preview();
	}
	m_b_changed = TRUE;
}

// UpdateWaveDescriptors()
//
// Update parameters that describe the structure of the data within data file
// load parameters from user selected options

void DlgImportGenericData::update_wave_descriptors(const AcqDataDoc* p_df)
{
	CFileStatus status;
	p_df->x_file->GetStatus(status);
	p_df->set_offset_to_data(m_skip_n_bytes);

	p_df->p_w_buf->create_buffer_with_n_channels(options_import->nb_channels);
	CWaveFormat* pwF = p_df->get_wave_format();

	// define parameters within CWaveFormat
	pwF->cs_comment = options_import->title;
	pwF->acquisition_time = status.m_ctime; // use CFile creation time
	pwF->full_scale_volts = options_import->voltage_max - options_import->voltage_min;
	pwF->mode_encoding = options_import->encoding_mode;
	long binspan = 4096;
	switch (options_import->bits_precision)
	{
	case 8: binspan = 256;
		break;
	case 12: binspan = 4096;
		break;
	case 16:
	case 24:
	case 32: binspan = 65536;
		break;
	default: break;
	}
	pwF->bin_span = binspan;
	pwF->bin_zero = 0;
	if (options_import->encoding_mode == 0) // OLx_ENC_BINARY
		pwF->bin_zero = binspan / 2;

	// copy ACQCHAN directly from iivO
	p_df->get_wave_channels_array()->Copy(options_import->p_wave_chan_array);

	// UNUSED PARAMETERS FROM iivO :
	//			BOOL	bSingleRun;
	//			short	nbRuns;

	// read data and copy into CDataFileAWAVE
	ULONGLONG l_count = status.m_size - options_import->skip_n_bytes; // get size of data
	if (options_import->is_sapid_3_5) // special case SAPID
	{
		l_count -= 2; // last word = sampling rate
		WORD w_rate; // get rate from source file
		p_df->x_file->Seek(-2, CFile::end); // position and read
		p_df->x_file->Read(&w_rate, sizeof(WORD));
		options_import->sampling_rate = w_rate; // transfer rate
	}
	pwF->sampling_rate_per_channel = options_import->sampling_rate;
	pwF->sample_count = static_cast<long>(l_count / 2);
	m_acq_data_file.read_data_infos();
}

//////////////////////////////////////////////////////////////////////////
// convert generic data file
// - open data file
// - create new CDataFileAWAVE object
// - set header parameters from iivO parameters
// - transfer data from source file into destination
// - close file and delete CDataFileAWAVE object

void DlgImportGenericData::on_en_change_channel_comment()
{
	m_b_changed = TRUE;
}
