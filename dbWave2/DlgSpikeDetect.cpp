
//
// caller must set:
// m_i_Detect_Parms_Dlg	= index spk detection parameters set to edit
// m_spk_DA = address of structure containing array of spk detection parameters structures

#include "StdAfx.h"
#include "resource.h"
#include "DlgSpikeDetect.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgSpikeDetect::DlgSpikeDetect(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgSpikeDetect::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_PARAMETERSTAB, m_c_parameter_tab_ctrl);
}

BEGIN_MESSAGE_MAP(DlgSpikeDetect, CDialog)
	ON_BN_CLICKED(IDC_DETECTFROMTAG, on_detect_from_tag)
	ON_BN_CLICKED(IDC_DETECTFROMCHAN, on_detect_from_chan)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PARAMETERSTAB, on_tcn_sel_change_parameters_tab)
	ON_BN_CLICKED(IDC_SPIKESRADIO, on_bn_clicked_spikes_radio)
	ON_BN_CLICKED(IDC_STIMRADIO, on_bn_clicked_stimulus_radio)
	ON_BN_CLICKED(IDC_ADDPARAMBTTN, on_bn_clicked_add_parameters_button)
	ON_BN_CLICKED(IDC_DELPARAMBTTN, on_bn_clicked_del_parameters_button)
	ON_EN_CHANGE(IDC_DETECTTHRESHOLD, on_en_change_detect_threshold)
	ON_CBN_SELCHANGE(IDC_DETECTCHAN, on_cbn_sel_change_detect_channel)
	ON_CBN_SELCHANGE(IDC_DETECTTRANSFORM, on_cbn_sel_change_detect_transform)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, on_delta_pos_spin1)
	ON_CBN_SELCHANGE(IDC_EXTRACTCHAN, on_cbn_sel_change_extract_channel)
	ON_EN_CHANGE(IDC_COMMENT, on_en_change_comment)
	ON_BN_CLICKED(IDC_RIGHTSHIFT, on_bn_clicked_shift_right)
	ON_BN_CLICKED(IDC_LEFTSHIFT, on_bn_clicked_shift_left)
END_MESSAGE_MAP()

BOOL DlgSpikeDetect::OnInitDialog()
{
	CDialog::OnInitDialog();
	// init chan list, select first detection channel
	CString comment;
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->ResetContent();
	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTCHAN))->ResetContent();

	const auto p_wave_format = m_db_doc->get_wave_format();
	const auto p_chan_array = m_db_doc->get_wave_channels_array();
	int chan_max = p_wave_format->scan_count;
	m_scan_count_ = chan_max;

	// load list of channels into combo boxes
	for (auto i = 0; i < chan_max; i++)
	{
		comment.Format(_T("%i"), i); // channel index CString
		const auto p_chan = p_chan_array->get_p_channel(i);
		comment += _T(" - ") + p_chan->am_csComment;
		VERIFY(((CComboBox*)GetDlgItem(IDC_DETECTCHAN))->AddString(comment) != CB_ERR);
		VERIFY(((CComboBox*)GetDlgItem(IDC_EXTRACTCHAN))->AddString(comment) != CB_ERR);
	}

	// load list of transform methods
	chan_max = AcqDataDoc::get_transforms_count();
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->ResetContent();
	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTTRANSFORM))->ResetContent();
	for (int i = 0; i < chan_max; i++)
	{
		VERIFY(((CComboBox*)GetDlgItem(IDC_DETECTTRANSFORM))->AddString(m_db_doc->get_transform_name(i)) != CB_ERR);
		VERIFY(((CComboBox*)GetDlgItem(IDC_EXTRACTTRANSFORM))->AddString(m_db_doc->get_transform_name(i)) != CB_ERR);
	}

	// load list of detection parameters
	for (int i = 0; i < m_p_detect_settings_array->get_size(); i++)
	{
		CString cs;
		cs.Format(_T("#%i "), i);
		cs += (m_p_detect_settings_array->get_item(i))->comment;
		m_c_parameter_tab_ctrl.InsertItem(i, cs);
	}

	// fill dialog with values from array
	load_chan_parameters(m_i_detect_parameters_dlg);
	update_tab_shift_buttons();

	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgSpikeDetect::save_chan_parameters(const int chan)
{
	options_detect_spikes_ = m_p_detect_settings_array->get_item(chan);
	GetDlgItem(IDC_COMMENT)->GetWindowText(options_detect_spikes_->comment);
	md_pm->b_detect_while_browse = static_cast<CButton*>(GetDlgItem(IDC_DETECTBROWSE))->GetCheck();

	// spikes detection parameters
	const auto flag2 = static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMCHAN))->GetCheck();
	options_detect_spikes_->detect_from = flag2 ? 0 : 1;
	options_detect_spikes_->detect_channel = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->GetCurSel();
	options_detect_spikes_->detect_transform = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->GetCurSel();
	options_detect_spikes_->detect_threshold_bin = GetDlgItemInt(IDC_DETECTTHRESHOLD);

	// detect spikes
	if (static_cast<CButton*>(GetDlgItem(IDC_SPIKESRADIO))->GetCheck() == BST_CHECKED)
	{
		options_detect_spikes_->detect_what = DETECT_SPIKES;
		options_detect_spikes_->extract_channel = static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTCHAN))->GetCurSel();
		options_detect_spikes_->extract_transform = static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTTRANSFORM))->GetCurSel();
		options_detect_spikes_->extract_n_points = static_cast<int>(GetDlgItemInt(IDC_SPIKENPOINTS));
		options_detect_spikes_->detect_pre_threshold = static_cast<int>(GetDlgItemInt(IDC_PRETHRESHOLD));
		options_detect_spikes_->detect_refractory_period = static_cast<int>(GetDlgItemInt(IDC_REFRACTORY));
	}
	// detect stimulus
	else
	{
		options_detect_spikes_->detect_what = DETECT_STIMULUS;
		options_detect_spikes_->detect_mode = static_cast<CComboBox*>(GetDlgItem(IDC_STIMDETECTMODE))->GetCurSel();
		options_detect_spikes_->extract_channel = options_detect_spikes_->detect_channel;
	}
}

void DlgSpikeDetect::load_chan_parameters(const int chan)
{
	m_i_detect_parameters_dlg = chan;
	options_detect_spikes_ = m_p_detect_settings_array->get_item(chan);
	GetDlgItem(IDC_COMMENT)->SetWindowText(options_detect_spikes_->comment);

	if (options_detect_spikes_->detect_what == DETECT_SPIKES)
	{
		static_cast<CButton*>(GetDlgItem(IDC_SPIKESRADIO))->SetCheck(BST_CHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_STIMRADIO))->SetCheck(BST_UNCHECKED);
	}
	else
	{
		static_cast<CButton*>(GetDlgItem(IDC_SPIKESRADIO))->SetCheck(BST_UNCHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_STIMRADIO))->SetCheck(BST_CHECKED);
	}
	set_dlg_interface_state(options_detect_spikes_->detect_what);

	// spikes detection parameters
	const BOOL flag = (options_detect_spikes_->detect_from == 0);
	static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMCHAN))->SetCheck(flag);
	static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMTAG))->SetCheck(!flag);
	display_detect_from_chan();
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->SetCurSel(options_detect_spikes_->detect_channel);
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->SetCurSel(options_detect_spikes_->detect_transform);
	SetDlgItemInt(IDC_DETECTTHRESHOLD, options_detect_spikes_->detect_threshold_bin);

	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTCHAN))->SetCurSel(options_detect_spikes_->extract_channel);
	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTTRANSFORM))->SetCurSel(options_detect_spikes_->extract_transform);
	SetDlgItemInt(IDC_SPIKENPOINTS, options_detect_spikes_->extract_n_points);
	SetDlgItemInt(IDC_PRETHRESHOLD, options_detect_spikes_->detect_pre_threshold);
	SetDlgItemInt(IDC_REFRACTORY, options_detect_spikes_->detect_refractory_period);
	static_cast<CButton*>(GetDlgItem(IDC_DETECTBROWSE))->SetCheck(md_pm->b_detect_while_browse);

	// stimulus detection parameters
	static_cast<CComboBox*>(GetDlgItem(IDC_STIMDETECTMODE))->SetCurSel(options_detect_spikes_->detect_mode);

	// select proper tab
	m_c_parameter_tab_ctrl.SetCurSel(chan);
}

void DlgSpikeDetect::OnOK()
{
	// save spike detection parameters
	save_chan_parameters(m_i_detect_parameters_dlg);
	CDialog::OnOK();
}

void DlgSpikeDetect::display_detect_from_chan()
{
	BOOL flag = FALSE;
	if (static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMCHAN))->GetCheck())
		flag = TRUE;
	GetDlgItem(IDC_DETECTCHAN)->EnableWindow(flag);
	GetDlgItem(IDC_DETECTTRANSFORM)->EnableWindow(flag);
	GetDlgItem(IDC_DETECTTHRESHOLD)->EnableWindow(flag);
}

void DlgSpikeDetect::on_detect_from_tag()
{
	display_detect_from_chan();
}

void DlgSpikeDetect::on_detect_from_chan()
{
	display_detect_from_chan();
}

void DlgSpikeDetect::on_tcn_sel_change_parameters_tab(NMHDR* p_nmhdr, LRESULT* p_result)
{
	save_chan_parameters(m_i_detect_parameters_dlg);
	*p_result = 0;
	const auto i_chan = m_c_parameter_tab_ctrl.GetCurSel();
	load_chan_parameters(i_chan);
	Invalidate();
	update_source_view();
	update_tab_shift_buttons();
}

void DlgSpikeDetect::set_dlg_interface_state(const int detect_what) const
{
	auto b_spike_detect_items = TRUE;
	auto b_stimulus_detect_items = FALSE;
	if (detect_what == 1)
	{
		b_spike_detect_items = FALSE;
		b_stimulus_detect_items = TRUE;
	}
	GetDlgItem(IDC_DETECTFROMTAG)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_DETECTFROMCHAN)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_EXTRACTCHAN)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_EXTRACTTRANSFORM)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_SPIKENPOINTS)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_PRETHRESHOLD)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_REFRACTORY)->EnableWindow(b_spike_detect_items);

	GetDlgItem(IDC_STIMDETECTMODE)->EnableWindow(b_stimulus_detect_items);
}

void DlgSpikeDetect::on_bn_clicked_spikes_radio() 
{
	options_detect_spikes_->detect_what = DETECT_SPIKES;
	set_dlg_interface_state(options_detect_spikes_->detect_what);
}

void DlgSpikeDetect::on_bn_clicked_stimulus_radio()
{
	options_detect_spikes_->detect_what = DETECT_STIMULUS;
	set_dlg_interface_state(options_detect_spikes_->detect_what);
}

void DlgSpikeDetect::on_bn_clicked_add_parameters_button()
{
	// save current parameters set
	save_chan_parameters(m_i_detect_parameters_dlg);

	// Add parameter set
	const auto i_last = m_p_detect_settings_array->add_item() - 1;
	CString cs;
	cs.Format(_T("set #%i"), i_last);
	m_c_parameter_tab_ctrl.InsertItem(i_last, cs);
	load_chan_parameters(i_last);
}

void DlgSpikeDetect::on_bn_clicked_del_parameters_button()
{
	const auto i_last = m_p_detect_settings_array->remove_item(m_i_detect_parameters_dlg) - 1;
	m_c_parameter_tab_ctrl.DeleteItem(m_i_detect_parameters_dlg);
	load_chan_parameters(i_last);
	Invalidate();
}

void DlgSpikeDetect::on_en_change_detect_threshold() 
{
	const int i_threshold = static_cast<int>(GetDlgItemInt(IDC_DETECTTHRESHOLD));
	m_p_chart_data_detect_wnd->move_hz_tag_to_val(0, i_threshold);
}

void DlgSpikeDetect::on_cbn_sel_change_detect_channel() 
{
	update_source_view();
}

void DlgSpikeDetect::update_source_view() const
{
	const auto i_cursel = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->GetCurSel();
	const auto i_cursel_2 = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->GetCurSel();
	// TODO not used??
	//int icursel3 = ((CComboBox*) GetDlgItem(IDC_EXTRACT CHAN))->GetCurSel();
	m_p_chart_data_detect_wnd->set_channel_list_y(0, i_cursel, i_cursel_2);
	m_p_chart_data_detect_wnd->get_data_from_doc();
	m_p_chart_data_detect_wnd->auto_zoom_chan(0);
	m_p_chart_data_detect_wnd->Invalidate();
}

void DlgSpikeDetect::on_cbn_sel_change_detect_transform() 
{
	update_source_view();
}

void DlgSpikeDetect::on_delta_pos_spin1(NMHDR* p_nmhdr, LRESULT* p_result)
{
	const auto p_nm_up_down = reinterpret_cast<LPNMUPDOWN>(p_nmhdr);
	int i_threshold = GetDlgItemInt(IDC_DETECTTHRESHOLD);
	i_threshold -= MulDiv(m_p_chart_data_detect_wnd->get_channel_list_item(0)->get_y_extent(), p_nm_up_down->iDelta, 10);
	SetDlgItemInt(IDC_DETECTTHRESHOLD, i_threshold);

	*p_result = 0;
}

void DlgSpikeDetect::on_cbn_sel_change_extract_channel() 
{
	update_source_view();
}

void DlgSpikeDetect::on_en_change_comment()
{
	CString cs;
	GetDlgItem(IDC_COMMENT)->GetWindowText(cs);
	set_tab_comment(m_i_detect_parameters_dlg, cs);
}

void DlgSpikeDetect::set_tab_comment(const int i, const CString& cs)
{
	TCITEM tc_item;
	tc_item.mask = TCIF_TEXT;
	CString cs1;
	cs1.Format(_T("#%i %s"), m_i_detect_parameters_dlg, (LPCTSTR)cs);
	//  Set the new text for the item.
	tc_item.pszText = cs1.GetBuffer(2);
	m_c_parameter_tab_ctrl.SetItem(i, &tc_item);
}

// move current parameter settings one slot more

void DlgSpikeDetect::on_bn_clicked_shift_right()
{
	const auto i_source = m_i_detect_parameters_dlg;
	const auto i_destination = i_source + 1;
	if (i_destination < m_p_detect_settings_array->get_size())
		exchange_parameters(i_source, i_destination);
}

void DlgSpikeDetect::on_bn_clicked_shift_left()
{
	const auto i_source = m_i_detect_parameters_dlg;
	const auto i_destination = i_source - 1;
	if (i_destination <= 0)
		exchange_parameters(i_source, i_destination);
}

void DlgSpikeDetect::exchange_parameters(const int i_source, const int i_destination)
{
	save_chan_parameters(i_source); // save current data
	const auto p_sp = new options_detect_spikes(); // create temporary data to exchange
	const auto p_source = m_p_detect_settings_array->get_item(i_source);
	const auto p_destination = m_p_detect_settings_array->get_item(i_destination);
	*p_sp = *p_source;
	*p_source = *p_destination;
	*p_destination = *p_sp;
	m_i_detect_parameters_dlg = i_destination;
	load_chan_parameters(i_destination);
	set_tab_comment(i_source, (m_p_detect_settings_array->get_item(i_source))->comment);
	set_tab_comment(i_destination, (m_p_detect_settings_array->get_item(i_destination))->comment);

	// update interface
	m_c_parameter_tab_ctrl.SetCurSel(i_destination);
	Invalidate();
	update_source_view();
	update_tab_shift_buttons();
}

void DlgSpikeDetect::update_tab_shift_buttons() const
{
	auto b_enable = TRUE;
	if (m_i_detect_parameters_dlg < 1)
		b_enable = FALSE;
	GetDlgItem(IDC_LEFTSHIFT)->EnableWindow(b_enable);
	b_enable = TRUE;
	if (m_i_detect_parameters_dlg >= m_p_detect_settings_array->get_size() - 1)
		b_enable = FALSE;
	GetDlgItem(IDC_RIGHTSHIFT)->EnableWindow(b_enable);
}
