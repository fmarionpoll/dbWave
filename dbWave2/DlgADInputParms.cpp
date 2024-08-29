#include "StdAfx.h"
#include <Olxdadefs.h>
#include "GridCtrl/GridCell.h"
#include "GridCtrl/GridCellCombo_FMP.h"
#include "GridCtrl/GridCellNumeric.h"
#include "GridCtrl/GridCellCheck.h"
#include "DlgADInputParms.h"

#include "AcqWaveChan.h"
#include "ColorNames.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Static data : rows headers, content of combo_boxes (high pass filter, AD gain, amplifier, head_stage)
// pszRowTitle: content of the row header
// if string starts with char, print BOLD and left aligned
// if string starts with space, print normal and right aligned
// the numbers below are used in a "switch" to take action according to the row item
// within OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)

const TCHAR* DlgADInputs::psz_row_title_[] = {
	_T("Title"),
	// 1: // title
	_T("A/D channel (0-7) "),
	_T(" gain"),
	// 2: // A/D channel (0-7), (0-16) 3: // A/D gain
	_T("Amplifier"),
	_T(" channel"),
	_T(" gain"),
	_T(" notch filter"),
	_T(" IN+ filter (Hz)"),
	_T(" DC Offset (mV)"), //_T(" IN- filter (Hz)"),
	_T(" low pass filter (Hz)"),
	// 4: // ampli 5: // ampli out chan 6: // ampli gain 7: // notch 8:// IN+ 9: // IN- 10: //low pass
	_T("Probe"),
	_T(" gain"),
	// 11: headstage 12: // headstage gain
	_T("Signal max/min (mV)"),
	_T(" total gain"),
	_T(" resolution (µV)"),
	// 13: // max range 14: // total gain 15:// bin resolution
	_T("")
};

const TCHAR* DlgADInputs::psz_high_pass_[] = {
	_T("GND"),_T("DC"),_T("0.1"),_T("1"),_T("10"),_T("30"),_T("100"),_T("300"),
	_T(""),_T(""),_T(""),_T(""),_T(""),_T(""),_T(""),_T("")
};

const TCHAR* DlgADInputs::psz_ad_gains_[] = {_T("1"),_T("2"),_T("4"),_T("8"),_T(""),_T(""),_T(""),_T("")};

const TCHAR* DlgADInputs::psz_amplifier_[] = {
	_T("CyberAmp 320"),_T("Neurolog"),_T("IDAC"),_T("Dagan"),_T("Alligator"), _T("none"),_T(""),_T("")
};

const TCHAR* DlgADInputs::psz_probe_type_[] = {
	_T("Syntech"),_T("DTP02"),_T("AI401"),_T("Neurolog"),_T("AD575"),_T("Dagan"),_T("none"),_T("")
};

const TCHAR* DlgADInputs::psz_encoding_[] = {
	_T("Offset binary"),_T("Two's complement"),_T("Unknown"),_T(""),_T(""),_T(""),_T(""),_T("")
};

int DlgADInputs::i_encoding_[] = {OLx_ENC_BINARY, OLx_ENC_2SCOMP, -1};

IMPLEMENT_DYNAMIC(DlgADInputs, CDialog)

DlgADInputs::DlgADInputs(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

DlgADInputs::~DlgADInputs()
= default;

void DlgADInputs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NACQCHANS, m_n_acq_channels_);
	DDX_Text(pDX, IDC_MAXLIST, m_input_list_max_);
	DDX_Control(pDX, IDC_GRID, m_grid);
	DDX_Control(pDX, IDC_RESOLUTION, m_resolution_combo);
	DDX_Control(pDX, IDC_ENCODING, m_encoding_combo);
}

BEGIN_MESSAGE_MAP(DlgADInputs, CDialog)
	ON_EN_CHANGE(IDC_NACQCHANS, on_en_change_n_acq_channels)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_SINGLEENDED, on_bn_clicked_single_ended)
	ON_BN_CLICKED(IDC_DIFFERENTIAL, on_bn_clicked_differential)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID, on_grid_end_edit)
	ON_CBN_SELCHANGE(IDC_RESOLUTION, on_cbn_sel_change_resolution)
	ON_BN_CLICKED(IDOK, on_bn_clicked_ok)
	ON_BN_CLICKED(IDC_ADINTERVALS, on_bn_clicked_ad_intervals)
END_MESSAGE_MAP()

// CADInputParmsDlg message handlers

BOOL DlgADInputs::OnInitDialog()
{
	CDialog::OnInitDialog();
	is_ad_changed = false;

	CRect rect;
	GetClientRect(rect);
	m_old_size_ = CSize(rect.Width(), rect.Height());

	// display/hide chain dialog buttons
	auto show_window = SW_HIDE;
	if (b_chain_dialog)
		show_window = SW_SHOW;
	GetDlgItem(IDC_ADCHANNELS)->ShowWindow(show_window);
	GetDlgItem(IDC_ADINTERVALS)->ShowWindow(show_window);

	// display nb of available channels according to parameter
	if (channel_type == OLx_CHNT_SINGLEENDED)
	{
		static_cast<CButton*>(GetDlgItem(IDC_SINGLEENDED))->SetCheck(BST_CHECKED);
		m_max_channels_ = n_channels_max_se;
	}
	else
	{
		static_cast<CButton*>(GetDlgItem(IDC_DIFFERENTIAL))->SetCheck(BST_CHECKED);
		m_max_channels_ = n_channels_max_di;
	}

	// set spin  max list: dt9800 = up to 32 channels
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange(1, m_input_list_max_);

	// load encoding
	int i = 0;
	do
	{
		auto j = m_encoding_combo.AddString(psz_encoding_[i]);
		m_encoding_combo.SetItemData(j, static_cast<DWORD>(i_encoding_[i]));
		i++;
	}
	while (psz_encoding_[i] != _T(""));

	// eventually load data
	load_ad_parameters_from_pw_format();

	// initialise grid properties
	m_grid.EnableDragAndDrop(TRUE);
	m_grid.GetDefaultCell(FALSE, FALSE)->SetBackClr(col_white);
	m_grid.SetColumnCount(m_n_acq_channels_ + 1);
	m_grid.SetFixedColumnCount(1);
	m_grid.SetFixedRowCount(1);

	init_row_headers();
	init_columns();

	m_grid.AutoSizeColumn(0);
	m_grid.AutoSizeRows();
	m_grid.ExpandRowsToFit();

	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgADInputs::on_en_change_n_acq_channels()
{
	if (!IsWindow(m_grid.m_hWnd))
		return;
	UpdateData(TRUE);

	// make sure that the number of channels is appropriate
	if (m_n_acq_channels_ < 1)
		m_n_acq_channels_ = 1;
	if (m_n_acq_channels_ > m_input_list_max_)
		m_n_acq_channels_ = m_input_list_max_;

	// update title of row 2 and refresh cell
	CString cs;
	cs.Format(_T("A/D channel (0-%i)"), m_max_channels_ - 1);
	m_grid.SetItemText(m_row_ad_channel_, 0, cs);
	m_grid.RedrawCell(m_row_ad_channel_, 0, nullptr);

	// update combos / acq channels
	for (int i = 1; i <= m_n_acq_channels_; i++)
		init_ad_channel_combo(i, -1);

	adjust_grid_size();
	UpdateData(FALSE);
	is_ad_changed = true;
}

void DlgADInputs::adjust_grid_size()
{
	const auto n_old_columns = m_grid.GetColumnCount();
	// reduce size of the grid; delete last columns
	if (n_old_columns > m_n_acq_channels_ + 1)
	{
		for (auto i = n_old_columns - 1; i > m_n_acq_channels_; i--)
			m_grid.DeleteColumn(i);
		m_grid.Refresh();
	}
	// increase size of the grid
	else if (n_old_columns < m_n_acq_channels_ + 1)
	{
		m_grid.SetColumnCount(m_n_acq_channels_ + 1);
		for (int i = n_old_columns; i < m_n_acq_channels_ + 1; i++)
			init_grid_column_defaults(i);
		m_grid.Refresh();
	}
}

BOOL CALLBACK EnumProc(const HWND h_wnd, LPARAM lParam)
{
	const auto p_wnd = CWnd::FromHandle(h_wnd);
	const auto* p_translate = reinterpret_cast<CSize*>(lParam);

	auto* p_dlg = static_cast<DlgADInputs*>(p_wnd->GetParent());
	if (!p_dlg) return FALSE;

	CRect rect;
	p_wnd->GetWindowRect(rect);
	p_dlg->ScreenToClient(rect);
	if (h_wnd == p_dlg->m_grid.GetSafeHwnd())
	{
		// move grid
		if (((rect.top >= 7 && p_translate->cy > 0) || rect.Height() > 20) && ((rect.left >= 7 && p_translate->cx > 0)
			|| rect.Width() > 20))
			p_wnd->MoveWindow(rect.left, rect.top, rect.Width() + p_translate->cx, rect.Height() + p_translate->cy,
			                  FALSE);
		else
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top + p_translate->cy, rect.Width(), rect.Height(),
			                  FALSE);
	}
	else
	{
		const auto i_id = p_wnd->GetDlgCtrlID();
		switch (i_id)
		{
		case IDC_SIZEBOX: // move size box in the lower right corner
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top + p_translate->cy,
			                  rect.Width(), rect.Height(), FALSE);
			break;
		case IDOK: // other windows to move with the right size switch
		case IDCANCEL:
		case IDC_ADCHANNELS:
		case IDC_ADINTERVALS:
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top, rect.Width(), rect.Height(), FALSE);
			break;
		default:
			break;
		}
	}
	p_dlg->Invalidate();

	return TRUE;
}

void DlgADInputs::OnSize(UINT n_type, int cx, int cy)
{
	CDialog::OnSize(n_type, cx, cy);
	if (cx <= 1 || cy <= 1)
		return;

	CSize translate(cx - m_old_size_.cx, cy - m_old_size_.cy);
	EnumChildWindows(GetSafeHwnd(), EnumProc, reinterpret_cast<LPARAM>(&translate));
	m_old_size_ = CSize(cx, cy);

	CWnd* p_wnd = GetDlgItem(IDC_SIZEBOX);
	if (p_wnd)
		p_wnd->ShowWindow((n_type == SIZE_MAXIMIZED) ? SW_HIDE : SW_SHOW);
}

void DlgADInputs::init_ad_channel_combo(const int col, int i_select) const
{
	// build string array with channel list
	CStringArray csArrayOptions;
	CString cs;
	for (int i = 0; i < m_max_channels_; i++)
	{
		cs.Format(_T("%i"), i);
		csArrayOptions.Add(cs);
	}
	csArrayOptions.Add(_T("16 (Din)"));

	// select cell and corresponding combo
	const auto p_cell = static_cast<GridCellComboFMP*>(m_grid.GetCell(m_row_ad_channel_, col));
	if (p_cell != nullptr)
	{
		if (i_select < 0)
			i_select = p_cell->GetCurSel();
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN); 
		p_cell->SetCurSel(i_select);
	}
}

BOOL DlgADInputs::init_grid_column_defaults(int col)
{
	if (col < 1 || col >= m_grid.GetColumnCount())
		return FALSE;

	// get font and create bold face
	auto p_font = m_grid.GetFont();
	ASSERT(p_font != NULL);
	LOGFONT lf;
	p_font->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;

	// set columns headers
	auto row = 0;
	GV_ITEM item;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.row = row;
	item.col = col;
	item.nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
	item.strText.Format(_T("ch %i"), col);
	m_grid.SetItem(&item);
	m_grid.SetItemFont(row, col, &lf);

	// set cells content
	//"Title",
	row++;
	item.row = row;
	item.strText.Format(_T("channel %i"), col);
	item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
	m_grid.SetItem(&item);

	//"A/D channel" - (col-1)
	row++;
	m_row_ad_channel_ = row;
	if (m_grid.SetCellType(m_row_ad_channel_, col, RUNTIME_CLASS(GridCellComboFMP)))
		init_ad_channel_combo(col, col - 1);

	// "A/D gain" - combo
	row++;
	m_row_ad_gain_ = row;
	if (m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellComboFMP)))
	{
		CStringArray csArrayOptions;
		auto i = 0;
		do
		{
			csArrayOptions.Add(psz_ad_gains_[i]);
			i++;
		}
		while (psz_ad_gains_[i] != _T(""));
		auto* p_cell = static_cast<GridCellComboFMP*>(m_grid.GetCell(row, col));
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN);
		// init value
		if (col > 1)
			m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(0);
	}

	// "Amplifier type" - combo
	row++;
	if (m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellComboFMP)))
	{
		CStringArray cs_array_options;
		auto i = 0;
		do
		{
			cs_array_options.Add(psz_amplifier_[i]);
			i++;
		}
		while (psz_amplifier_[i] != _T(""));
		auto* p_cell = static_cast<GridCellComboFMP*>(m_grid.GetCell(row, col));
		p_cell->SetOptions(cs_array_options);
		p_cell->SetStyle(CBS_DROPDOWN);
		// init value
		if (col > 1)
			m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(0);
	}

	// " output channel" - numeric
	row++;
	m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellNumeric));
	// init value
	item.row = row;
	item.strText.Format(_T("%d"), col);
	m_grid.SetItem(&item);

	// " gain" - numeric
	row++;
	m_row_amp_gain_ = row; // save index for later
	m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellNumeric));
	// init value
	if (col > 1)
		m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
	else
		m_grid.SetItemText(row, col, _T("500"));

	// " notch filter" - check box
	row++;
	item.strText = _T("50 Hz");
	item.row = row;
	m_grid.SetItem(&item);
	m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellCheck));

	// " IN+" - combo
	row++;
	if (m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellComboFMP)))
	{
		CStringArray csArrayOptions;
		int i = 0;
		do
		{
			csArrayOptions.Add(psz_high_pass_[i]);
			i++;
		}
		while (psz_high_pass_[i] != _T(""));
		auto p_cell = static_cast<GridCellComboFMP*>(m_grid.GetCell(row, col));
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN);
		// init value
		if (col > 1)
			m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(1);
	}

	// DC Offset (mV) " IN-" - normal - float
	row++;
	m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCell));
	if (col > 1)
		m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
	else
		m_grid.SetItemText(row, col, _T("0.00"));

	// " low pass filter" - numeric
	row++;
	m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellNumeric));
	if (col > 1)
		m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
	else
		m_grid.SetItemText(row, col, _T("2800"));

	// "Probe type" - combo
	row++;
	if (m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellComboFMP)))
	{
		CStringArray csArrayOptions;
		int i = 0;
		do
		{
			csArrayOptions.Add(psz_probe_type_[i]);
			i++;
		}
		while (psz_probe_type_[i] != _T(""));

		auto p_cell = static_cast<GridCellComboFMP*>(m_grid.GetCell(row, col));
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN);
		// init value
		if (col > 1)
			m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(0);
	}

	// " gain" - numeric
	row++;
	m_row_head_stage_gain_ = row; // save index for later
	m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellNumeric));
	// init value
	if (col > 1)
		m_grid.SetItemText(row, col, m_grid.GetItemText(row, col - 1));
	else
		m_grid.SetItemText(row, col, _T("10"));

	// read only / numeric fields
	m_row_readonly_ = row + 1;
	const auto clr = m_grid.GetDefaultCell(TRUE, TRUE)->GetBackClr();
	for (auto i = 0; i < 3; i++)
	{
		row++;
		m_grid.SetCellType(row, col, RUNTIME_CLASS(GridCellNumeric));
		m_grid.SetItemState(row, col, m_grid.GetItemState(1, 1) | GVIS_READONLY);
		m_grid.SetItemBkColour(row, col, clr);
	}
	display_channel_read_only_fields(col);

	return TRUE;
}

void DlgADInputs::on_bn_clicked_single_ended()
{
	m_max_channels_ = n_channels_max_se;
	channel_type = OLx_CHNT_SINGLEENDED;
	on_en_change_n_acq_channels();
	is_ad_changed = true;
}

void DlgADInputs::on_bn_clicked_differential()
{
	m_max_channels_ = n_channels_max_di;
	channel_type = OLx_CHNT_DIFFERENTIAL;
	on_en_change_n_acq_channels();
	is_ad_changed = true;
}

void DlgADInputs::on_grid_end_edit(NMHDR* p_notify_struct, LRESULT* p_result)
{
	const auto p_item = reinterpret_cast<NM_GRIDVIEW*>(p_notify_struct);
	switch (p_item->iRow)
	{
	//case 1: // title - no action
	case 2: // A/D channel (0-7), (0-15)
		is_ad_changed = true;
		break;
	case 3: // A/D gain
		display_channel_read_only_fields(p_item->iColumn);
		is_ad_changed = true;
		break;

	case 4: // ampli
	case 5: // ampli out chan
	case 6: // ampli gain
	case 7: // notch
	case 8: // IN+
	case 9: // low pass
	case 10:
		save_grid_to_wavechan_data(p_item->iColumn);
		set_amplifier_parameters(p_item->iColumn);
		display_channel_read_only_fields(p_item->iColumn);
		break;
	//case 11: // headstage
	case 12:	// headstage gain
		display_channel_read_only_fields(p_item->iColumn);
		break;
	//case 13: // max range
	//case 14: // total gain
	//case 15: // bin resolution

	default:
		break;
	}
	// Accept change and refresh grid
	*p_result = TRUE;
	m_grid.Refresh();
}

void DlgADInputs::on_cbn_sel_change_resolution()
{
	CString cs;
	m_resolution_combo.GetWindowText(cs);
	const auto n_bits = _ttoi(cs);
	const auto old_n_bins = m_i_n_bins_;
	m_i_n_bins_ = 1 << n_bits; // 2 exp (n bits)
	// change input resolution if value has changed
	if (m_i_n_bins_ != old_n_bins)
	{
		for (auto i = 1; i <= m_n_acq_channels_; i++)
			display_channel_read_only_fields(i);
		m_grid.Refresh();
	}
	is_ad_changed = true;
}

// load column data (index = 1, n)
void DlgADInputs::load_grid_with_wavechan_data(const int col)
{
	// select channel and load parameters CComboBox
	ASSERT(col <= m_pch_array->chan_array_get_size());
	const auto p_chan = m_pch_array->get_p_channel(col - 1);

	// settings for the col
	GV_ITEM item;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.col = col;

	display_channel_ad_card(item, p_chan);
	get_amplifier_parameters(p_chan);
	display_channel_amplifier(item, p_chan);
	display_channel_probe(item, p_chan);
	display_channel_read_only_fields(col);
}

void DlgADInputs::save_data()
{
	// if number of acq channels changed, change corresponding structures
	if (m_n_acq_channels_ != m_pw_format->scan_count)
	{
		m_pw_format->scan_count = static_cast<short>(m_n_acq_channels_);
		m_pch_array->chan_array_set_size(m_n_acq_channels_);
		const WORD ch_buffer_size = m_pw_format->buffer_size / m_n_acq_channels_;
		m_pw_format->buffer_size = ch_buffer_size * m_n_acq_channels_;
	}
	// general acq parameters
	GetDlgItem(IDC_ADCARDNAME)->GetWindowText(m_pw_format->cs_ad_card_name);
	m_pw_format->bin_span = static_cast<long>(m_i_n_bins_);

	// save each channel
	for (auto i = 1; i <= m_n_acq_channels_; i++)
	{
		save_grid_to_wavechan_data(i);
		set_amplifier_parameters(i);
	}
}

// save column data (index = 1, n)
void DlgADInputs::save_grid_to_wavechan_data(const int col) const
{
	// select channel and load parameters CComboBox
	if (m_pch_array->chan_array_get_size() < col)
		m_pch_array->chan_array_set_size(col);
	const auto p_chan = m_pch_array->get_p_channel(col - 1);

	// AD channel comment
	auto row = 1;
	p_chan->am_csComment = m_grid.GetItemText(row, col);
	// "AD input channel" - combo
	row++;
	auto cs = m_grid.GetItemText(row, col);
	p_chan->am_adchannel = _ttoi(cs); // digital channels "16 [DIN]" is OK because of the space after "16"

	// " AD gain" - combo
	row++;
	cs = m_grid.GetItemText(row, col);
	p_chan->am_gainAD = _ttoi(cs);

	// amplifier name
	row++;
	p_chan->am_csamplifier = m_grid.GetItemText(row, col);

	// amplifier channel
	row++;
	cs = m_grid.GetItemText(row, col);
	p_chan->am_amplifierchan = _ttoi(cs);

	// amplifier gain
	row++;
	cs = m_grid.GetItemText(row, col);
	p_chan->am_gainpre = _ttoi(cs);
	p_chan->am_gainpost = 1;

	// amplifier notch filter
	row++;
	p_chan->am_notchfilt = static_cast<GridCellCheck*>(m_grid.GetCell(row, col))->GetCheck();

	// amplifier high pass filter (IN+)
	row++;
	p_chan->am_csInputpos = m_grid.GetItemText(row, col);

	// DC Offset(mV) // IN-
	row++;
	cs = m_grid.GetItemText(row, col);
	p_chan->am_offset = static_cast<float>(_tstof(cs));

	// amplifier low pass filter
	row++;
	cs = m_grid.GetItemText(row, col);
	p_chan->am_lowpass = _ttoi(cs);

	// head stage
	row++;
	p_chan->am_csheadstage = m_grid.GetItemText(row, col);

	// head stage gain
	row++;
	cs = m_grid.GetItemText(row, col);
	p_chan->am_gainheadstage = _ttoi(cs);
	p_chan->am_amplifiergain = static_cast<double>(p_chan->am_gainheadstage) * static_cast<double>(p_chan->am_gainpre) *
		static_cast<double>(p_chan->am_gainpost);
	p_chan->am_gaintotal = p_chan->am_amplifiergain * p_chan->am_gainAD;
	// compute dependent parameters
	p_chan->am_resolutionV = m_pw_format->full_scale_volts / p_chan->am_gaintotal / m_pw_format->bin_span;
}

void DlgADInputs::on_bn_clicked_ok()
{
	UpdateData(TRUE);
	save_data();
	OnOK();
}

void DlgADInputs::on_bn_clicked_ad_intervals()
{
	m_postmessage = IDC_ADINTERVALS;
	UpdateData(TRUE);
	save_data();
	OnOK();
}

void DlgADInputs::set_amplifier_parameters(const int col) const
{
	if (!b_command_amplifier)
		return;

	// transfer data into structure
	const auto p_chan = m_pch_array->get_p_channel(col - 1);

	if (p_chan->am_csamplifier.Find(_T("CyberAmp")) >= 0
		|| p_chan->am_csamplifier.Find(_T("Axon")) >= 0)
	{
		CyberAmp cyber_amp;
		if (cyber_amp.Initialize() != NULL)
			return;

		p_chan->am_csInputneg = psz_high_pass_[0];
		/*auto error_code = */
		cyber_amp.SetWaveChanParms(p_chan);
	}

	if (p_chan->am_csamplifier.Find(_T("Alligator")) >= 0 && m_alligator_amplifier != nullptr)
	{
		if (m_alligator_amplifier->IsValidHandle())
			m_alligator_amplifier->SetWaveChanParms(p_chan);
	}
}

void DlgADInputs::get_amplifier_parameters(CWaveChan* p_chan) const
{
	if (!b_command_amplifier)
		return;

	// exit if cyberAmp not declared - if not, exit
	if (p_chan->am_csamplifier.Find(_T("CyberAmp")) >= 0
		|| p_chan->am_csamplifier.Find(_T("Axon")) >= 0)
	{
		CyberAmp cyber_amp;
		if (cyber_amp.Initialize() != NULL)
			return;
		cyber_amp.GetWaveChanParms(p_chan);
	}

	if (p_chan->am_csamplifier.Find(_T("Alligator")) >= 0 && m_alligator_amplifier != nullptr)
	{
		if (m_alligator_amplifier->IsValidHandle())
			m_alligator_amplifier->GetWaveChanParms(p_chan);
	}
}

void DlgADInputs::load_ad_parameters_from_pw_format()
{
	if (m_pw_format == nullptr)
	{
		m_n_acq_channels_ = 1;
		m_resolution_combo.SetCurSel(1); // 12 bits
		m_i_n_bins_ = 1 << 12; // 2 exp (n bits)
		m_encoding_combo.SetCurSel(0);
	}
	else
	{
		m_n_acq_channels_ = m_pw_format->scan_count;
		GetDlgItem(IDC_ADCARDNAME)->SetWindowText(m_pw_format->cs_ad_card_name);

		// get precision and compute corresponding nb of bits
		m_i_n_bins_ = m_pw_format->bin_span + 1;
		int i_bins = static_cast<int>(m_i_n_bins_);
		auto n_bits = 0;
		do
		{
			i_bins = i_bins / 2;
			n_bits++;
		}
		while (i_bins > 1);

		// select corresponding entry in the combobox
		CString cs;
		cs.Format(_T("%i"), n_bits);
		const auto i_found = m_resolution_combo.FindStringExact(-1, cs);
		if (i_found != CB_ERR)
			m_resolution_combo.SetCurSel(i_found);

		// select encoding mode according to parameter in waveFormat
		int i_current;
		for (i_current = 0; i_current < m_encoding_combo.GetCount(); i_current++)
		{
			if (m_encoding_combo.GetItemData(i_current) == static_cast<DWORD>(m_pw_format->mode_encoding))
				break;
		}
		if (i_current == m_encoding_combo.GetCount())
			i_current--;
		m_encoding_combo.SetCurSel(i_current);
	}
}

void DlgADInputs::init_row_headers()
{
	// get font and create bold face
	const auto p_font = m_grid.GetFont();
	ASSERT(p_font != NULL);
	LOGFONT lf;
	p_font->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;

	// set row headers
	auto n_rows = 0;
	do
	{
		n_rows++;
	}
	while (psz_row_title_[n_rows - 1] != _T(""));
	m_grid.SetRowCount(n_rows);

	// init row headers with descriptors
	int row = 0;
	do
	{
		constexpr int col = 0;
		row++;
		m_grid.SetItemText(row, col, psz_row_title_[row - 1]);
		if (psz_row_title_[row - 1][0] == ' ')
			m_grid.SetItemFormat(row, col, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		else
			m_grid.SetItemFont(row, col, &lf);
	}
	while (psz_row_title_[row] != _T(""));
	CString cs;
	cs.Format(_T("A/D channel (0-%i)"), m_max_channels_ - 1);
	m_grid.SetItemText(2, 0, cs);
}

void DlgADInputs::init_columns()
{
	// init data columns
	for (int col = 1; col <= m_n_acq_channels_; col++)
	{
		init_grid_column_defaults(col);
		if (m_pw_format != nullptr)
			load_grid_with_wavechan_data(col);
	}
}

void DlgADInputs::display_channel_ad_card(GV_ITEM& item, const CWaveChan* p_chan)
{
	item.row = 1;
	item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
	item.strText = p_chan->am_csComment;
	if (p_chan->am_csComment != _T(""))
		m_grid.SetItem(&item);

	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_adchannel);
	m_grid.SetItem(&item);

	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_gainAD);
	m_grid.SetItem(&item);
}

void DlgADInputs::display_channel_amplifier(GV_ITEM& item, const CWaveChan* p_chan)
{
	item.row = 4;
	item.strText = p_chan->am_csamplifier;
	m_grid.SetItem(&item);
	// amplifier channel
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_amplifierchan);
	m_grid.SetItem(&item);
	// amplifier gain
	item.row++;
	const auto gain = static_cast<double>(p_chan->am_gaintotal) / (static_cast<double>(p_chan->am_gainheadstage) *
		static_cast<double>(p_chan->am_gainAD));
	auto i_gain = static_cast<int>(gain);
	if (i_gain == 0)
		i_gain = 1;
	item.strText.Format(_T("%d"), i_gain);
	m_grid.SetItem(&item);
	// amplifier notch filter
	item.row++;
	static_cast<GridCellCheck*>(m_grid.GetCell(item.row, item.col))->SetCheck(p_chan->am_notchfilt);
	// IN+
	item.row++;
	item.strText = p_chan->am_csInputpos;
	m_grid.SetItem(&item);
	// DC Offset (mV) // IN-
	item.row++;
	//Item.strText=p chan->am_cs Input neg;
	item.strText.Format(_T("%0.3f"), p_chan->am_offset);
	m_grid.SetItem(&item);
	// amplifier low pass filter
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_lowpass);
	m_grid.SetItem(&item);
}

void DlgADInputs::display_channel_probe(GV_ITEM& item, const CWaveChan* p_chan)
{
	item.row = 11;
	item.strText = p_chan->am_csheadstage;
	m_grid.SetItem(&item);
	// head stage gain
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_gainheadstage);
	m_grid.SetItem(&item);
}

void DlgADInputs::display_channel_read_only_fields(int col)
{
	// "Signal total gain" - read only / numeric
	int row = m_row_readonly_;
	CString cs = m_grid.GetItemText(m_row_amp_gain_, col);
	auto gain = _ttoi(cs);
	cs = m_grid.GetItemText(m_row_head_stage_gain_, col);
	gain *= _ttoi(cs);
	cs = m_grid.GetItemText(m_row_ad_gain_, col);
	gain *= _ttoi(cs);
	cs.Format(_T("%1.3f"), (m_x_volts_max_ * 1000.f / static_cast<float>(gain)));
	m_grid.SetItemText(row, col, cs);

	// "total gain" - read only / numeric
	row++;
	cs.Format(_T("%li"), gain); // = amp gain * probe gain * AD gain
	m_grid.SetItemText(row, col, cs);

	// " resolution (µV)" - read only
	row++;
	cs.Format(_T("%1.3f"), (m_x_volts_max_
		          * static_cast<float>(1.E6)
		          / static_cast<float>(gain) * 2
		          / static_cast<float>(m_i_n_bins_)));
	m_grid.SetItemText(row, col, cs);
}
