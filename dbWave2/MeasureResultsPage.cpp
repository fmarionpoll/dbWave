
// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "ChartWnd.h"
#include "ChartData.h"
#include "NoteDoc.h"
#include "dbWaveDoc.h"
#include "resource.h"
#include "MeasureResultsPage.h"

#include "dbWave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureResultsPage, CPropertyPage)



CMeasureResultsPage::CMeasureResultsPage() : CPropertyPage(IDD), m_pdb_doc(nullptr)
{
	m_p_chart_data_wnd = nullptr;
}

CMeasureResultsPage::~CMeasureResultsPage()
= default;

void CMeasureResultsPage::DoDataExchange(CDataExchange* p_dx)
{
	CPropertyPage::DoDataExchange(p_dx);
	DDX_Control(p_dx, IDC_RESULTS, m_c_edit_results);
	DDX_Control(p_dx, IDC_LIST1, m_list_results);
}

BEGIN_MESSAGE_MAP(CMeasureResultsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_EXPORT, on_export)
END_MESSAGE_MAP()

void CMeasureResultsPage::on_export()
{
	CString csBuffer;
	// copy results from CListCtrl into text buffer
	m_c_edit_results.GetWindowText(csBuffer);
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());

	CMultiDocTemplate* p_template = p_app->m_note_view_template;
	const auto p_doc = p_template->OpenDocumentFile(nullptr);
	auto pos = p_doc->GetFirstViewPosition();
	const auto p_view = static_cast<CRichEditView*>(p_doc->GetNextView(pos));
	auto& p_edit = p_view->GetRichEditCtrl();
	p_edit.SetWindowText(csBuffer); // copy content of window into CString
}

void CMeasureResultsPage::output_title()
{
	// prepare list ctrl column headers
	auto columns = m_list_results.InsertColumn(0, _T("#"), LVCFMT_LEFT, 20, 0) + 1;

	// column headers for vertical tags
	m_cs_title_ = _T("#");
	CString cs_cols = _T("");
	m_nb_data_cols_ = 0;

	switch (m_p_options_measure->w_option)
	{
	// ......................  vertical tags
	case 0:
		if (m_p_options_measure->b_data_limits)
		{
			if (!m_p_options_measure->b_time) cs_cols += _T("\tt1(mV)\tt2(mV)");
			else cs_cols += _T("\tt1(mV)\tt1(s)\tt2(mV)\tt2(s)");
		}
		if (m_p_options_measure->b_diff_data_limits)
		{
			cs_cols += _T("\tt2-t1(mV)");
			if (m_p_options_measure->b_time) cs_cols += _T("\tt2-t1(s)");
		}
		if (m_p_options_measure->b_extrema)
		{
			if (!m_p_options_measure->b_time) cs_cols += _T("\tMax(mV)\tmin(mV)");
			else cs_cols += _T("\tMax(mV)\tMax(s)\tmin(mV)\tmin(s)");
		}
		if (m_p_options_measure->b_diff_extrema)
		{
			cs_cols += _T("\tDiff(mV)");
			if (m_p_options_measure->b_time) cs_cols += _T("\tdiff(s)");
		}
		if (m_p_options_measure->b_half_rise_time) cs_cols += _T("\t1/2rise(s)");
		if (m_p_options_measure->b_half_recovery) cs_cols += _T("\t1/2reco(s)");
		break;

	// ......................  horizontal cursors
	case 1:
		if (m_p_options_measure->b_data_limits) cs_cols += _T("\tv1(mV)\tv2(mV)");
		if (m_p_options_measure->b_diff_data_limits) cs_cols += _T("\tv2-v1(mV)");
		break;

	// ......................  rectangle area
	case 2:
	// ......................  detect stimulus and then measure
	case 3:
	default:
		break;
	}

	// now set columns - get nb of data channels CString
	if (!cs_cols.IsEmpty())
	{
		auto channel_first = 0; // assume all data channels
		auto channel_last = m_p_chart_data_wnd->get_channel_list_size() - 1; // requested
		const auto n_pixels_mv = (m_list_results.GetStringWidth(_T("0.000000")) * 3) / 2;

		if (!m_p_options_measure->b_all_channels) // else if flag set
		{
			// restrict to a single chan
			channel_first = m_p_options_measure->w_source_channel; // single channel
			channel_last = channel_first;
		}
		m_nb_data_cols_ = 0;
		auto psz_t = m_sz_t_;
		TCHAR separators[] = _T("\t"); // separator = tab
		TCHAR* next_token = nullptr;

		for (auto channel = channel_first; channel <= channel_last; channel++)
		{
			auto cs = cs_cols;
			auto p_string = cs.GetBuffer(cs.GetLength() + 1);
			p_string++; // skip first tab
			auto p_token = _tcstok_s(p_string, separators, &next_token);
			while (p_token != nullptr)
			{
				if (m_p_options_measure->b_all_channels)
					wsprintf(m_sz_t_, _T("ch%i-%s"), channel, p_token);
				else
					psz_t = p_token;
				columns = 1 + m_list_results.InsertColumn(columns, psz_t, LVCFMT_LEFT, n_pixels_mv, columns);
				m_cs_title_ += separators;
				m_cs_title_ += psz_t;
				p_token = _tcstok_s(nullptr, separators, &next_token); // get next token
			}

			if (m_nb_data_cols_ == 0) // number of data columns
				m_nb_data_cols_ = columns - 1;
			cs.ReleaseBuffer(); // release character buffer
		}
	}
	m_cs_title_ += _T("\r\n");
}

void CMeasureResultsPage::measure_from_vertical_tags(const int channel)
{
	const auto n_tags = m_p_chart_data_wnd->vt_tags.get_tag_list_size();
	auto tag_first = -1;
	auto tag_last = -1;

	// assume that VT is ordered; measure only between pairs
	auto line = 0;
	for (auto i = 0; i < n_tags; i++)
	{
		// search first tag
		if (tag_first < 0)
		{
			tag_first = i;
			continue;
		}
		// second tag found: measure
		if (tag_last < 0)
		{
			tag_last = i;
			measure_within_interval(channel, line, m_p_chart_data_wnd->vt_tags.get_tag_value_long(tag_first),
			                      m_p_chart_data_wnd->vt_tags.get_tag_value_long(tag_last));
			line++;
			tag_first = -1;
			tag_last = -1;
		}
	}

	// cope with isolated tags
	if (tag_first > 0 && tag_last < 0)
	{
		const auto l1 = m_p_chart_data_wnd->vt_tags.get_tag_value_long(tag_first);
		measure_within_interval(channel, line, l1, l1);
	}
}

void CMeasureResultsPage::get_max_min(const int channel, long l_first, const long l_last)
{
	short* p_data;
	int n_channels = m_p_dat_doc->get_scan_count();
	const auto p_buf = m_p_dat_doc->get_raw_data_buffer();
	const CChanlistItem* channel_list_item = m_p_chart_data_wnd->get_channel_list_item(channel);
	const auto source_chan = channel_list_item->get_source_chan();
	const auto transform_mode = channel_list_item->get_transform_mode();
	const auto span = AcqDataDoc::get_transformed_data_span(transform_mode);

	// get first data point (init max and min)
	auto buf_chan_first = l_first;
	auto buf_chan_last = l_last;
	int offset;
	m_p_dat_doc->load_raw_data(&buf_chan_first, &buf_chan_last, span);
	if (transform_mode > 0)
	{
		p_data = m_p_dat_doc->load_transformed_data(l_first, buf_chan_last, transform_mode, source_chan);
		offset = 1;
	}
	else
	{
		p_data = p_buf + (l_first - buf_chan_first) * n_channels + source_chan;
		offset = n_channels;
	}
	m_max_ = *p_data;
	m_min_ = m_max_;
	m_imax_ = l_first;
	i_min_ = m_imax_;
	m_first_ = m_max_;

	// loop through data points
	while (l_first < l_last)
	{
		// load file data and get a pointer to these data
		buf_chan_first = l_first;
		buf_chan_last = l_last;
		m_p_dat_doc->load_raw_data(&buf_chan_first, &buf_chan_last, span);
		if (l_last < buf_chan_last)
			buf_chan_last = l_last;
		if (transform_mode > 0)
			p_data = m_p_dat_doc->load_transformed_data(l_first, buf_chan_last, transform_mode, source_chan);
		else
			p_data = p_buf + (l_first - buf_chan_first) * n_channels + source_chan;
		// now search for max and min
		for (int i = l_first; i <= buf_chan_last; i++)
		{
			m_last_ = *p_data;
			p_data += offset;
			if (m_last_ > m_max_)
			{
				m_max_ = m_last_;
				m_imax_ = i;
			}
			else if (m_last_ < m_min_)
			{
				m_min_ = m_last_;
				i_min_ = i;
			}
		}
		// search ended, update variable
		l_first = buf_chan_last + 1;
	}
}

// output results into CListCtrl
void CMeasureResultsPage::measure_within_interval(const int channel, const int line, const long l1, const long l2)
{
	// get scale factor for channel and sampling rate
	m_mv_per_bin_ = m_p_chart_data_wnd->get_channel_list_item(channel)->get_volts_per_bin() * 1000.0f;
	const auto rate = m_p_dat_doc->get_wave_format()->sampling_rate_per_channel;

	auto output_column = (m_col_ - 1) * m_nb_data_cols_ + 1; // output data into column i_col
	auto item = m_list_results.GetItemCount(); // compute which line will receive data
	if (line >= item)
	{
		wsprintf(&m_sz_t_[0], _T("%i"), item); // if new line, print line nb
		m_list_results.InsertItem(item, m_sz_t_); // and insert a new line within table
	}
	else
		item = line; // else set variable value

	// measure parameters / selected interval; save results within common vars
	get_max_min(channel, l1, l2);

	// output data according to options : data value at limits
	//m_szT[0] = '\t';								// prepare string for edit output
	CString cs_dummy;
	float x_dummy;
	const CString cs_fmt(_T("\t%f"));
	if (m_p_options_measure->b_data_limits)
	{
		x_dummy = static_cast<float>(m_first_) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, output_column, cs_dummy);
		output_column++;
		if (m_p_options_measure->b_time)
		{
			x_dummy = static_cast<float>(l1) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_list_results.SetItemText(item, output_column, cs_dummy);
			output_column++;
		}

		x_dummy = static_cast<float>(m_last_) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, output_column, cs_dummy);
		output_column++;
		if (m_p_options_measure->b_time)
		{
			x_dummy = static_cast<float>(l2) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_list_results.SetItemText(item, output_column, cs_dummy);
			output_column++;
		}
	}

	if (m_p_options_measure->b_diff_data_limits)
	{
		x_dummy = static_cast<float>(m_last_ - m_first_) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, output_column, cs_dummy);
		output_column++;
		if (m_p_options_measure->b_time)
		{
			x_dummy = (static_cast<float>(l2) - static_cast<float>(l1)) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_list_results.SetItemText(item, output_column, cs_dummy);
			output_column++;
		}
	}

	// measure max and min (value, time)
	if (m_p_options_measure->b_extrema)
	{
		x_dummy = static_cast<float>(m_max_) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, output_column, cs_dummy);
		output_column++;
		if (m_p_options_measure->b_time)
		{
			x_dummy = static_cast<float>(m_imax_) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_list_results.SetItemText(item, output_column, cs_dummy);
			output_column++;
		}
		x_dummy = static_cast<float>(m_min_) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, output_column, cs_dummy);
		output_column++;
		if (m_p_options_measure->b_time)
		{
			x_dummy = static_cast<float>(i_min_) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_list_results.SetItemText(item, output_column, cs_dummy);
			output_column++;
		}
	}

	// difference between extrema (value, time)
	if (m_p_options_measure->b_diff_extrema)
	{
		x_dummy = static_cast<float>(m_max_ - m_min_) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, output_column, cs_dummy);
		output_column++;
		if (m_p_options_measure->b_time)
		{
			x_dummy = static_cast<float>(m_imax_ - i_min_) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_list_results.SetItemText(item, output_column, cs_dummy);
		}
	}

	// time necessary to reach half of the amplitude
	if (m_p_options_measure->b_half_rise_time)
	{
	}

	// time necessary to regain half of the initial amplitude
	if (m_p_options_measure->b_half_recovery)
	{
	}
}

void CMeasureResultsPage::measure_from_horizontal_cursors(int i_chan)
{
	const auto number_of_tags = m_p_chart_data_wnd->hz_tags.get_tag_list_size();
	auto tag_first = -1;
	auto tag_last = -1;

	// assume that HZ is ordered; measure only between pairs
	int line = 0;
	for (int i = 0; i < number_of_tags; i++)
	{
		// search first tag
		if (tag_first < 0)
		{
			tag_first = i;
			continue;
		}
		// second tag found: measure
		if (tag_last < 0)
		{
			tag_last = i;
			measure_between_hz(i_chan, line,
			                 m_p_chart_data_wnd->hz_tags.get_value_int(tag_first),
			                 m_p_chart_data_wnd->hz_tags.get_value_int(tag_last));
			line++;
			tag_first = -1;
			tag_last = -1;
		}
	}

	// cope with isolated tags
	if (tag_first > 0 && tag_last < 0)
	{
		const auto v1 = m_p_chart_data_wnd->hz_tags.get_value_int(tag_first);
		measure_between_hz(i_chan, line, v1, v1);
	}
}

// output results both into CEdit control (via pCopy) and within CListCtrl
void CMeasureResultsPage::measure_between_hz(const int channel, const int line, const int v1, const int v2)
{
	// get scale factor for channel and sampling rate
	m_mv_per_bin_ = m_p_chart_data_wnd->get_channel_list_item(channel)->get_volts_per_bin() * 1000.0f;

	auto column_1 = (m_col_ - 1) * m_nb_data_cols_ + 1; // output data into column icol
	auto item = m_list_results.GetItemCount(); // compute which line will receive data
	if (line >= item)
	{
		wsprintf(&m_sz_t_[0], _T("%i"), item); // if new line, print line nb
		m_list_results.InsertItem(item, m_sz_t_); // and insert a new line within table
	}
	else
		item = line; // else set variable value

	// measure parameters / selected interval; save results within common vars
	//m_szT[0]='\t';								// prepare string for edit output
	CString cs_dummy;
	float x_dummy;
	const CString cs_fmt(_T("\t%f"));

	// output data according to options : data value at limits
	if (m_p_options_measure->b_data_limits)
	{
		x_dummy = static_cast<float>(v1) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, column_1, cs_dummy);
		column_1++;

		x_dummy = static_cast<float>(v2) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, column_1, cs_dummy);
		column_1++;
	}

	if (m_p_options_measure->b_diff_data_limits)
	{
		x_dummy = static_cast<float>(v2 - v1) * m_mv_per_bin_;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_list_results.SetItemText(item, column_1, cs_dummy);
	}
}

void CMeasureResultsPage::measure_from_rectangle(int i_chan)
{
}

void CMeasureResultsPage::measure_from_stimulus(int i_chan)
{
}

/////////////////////////////////////////////////////////////////////////////
// CMeasureResultsPage message handlers

BOOL CMeasureResultsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	measure_parameters();

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// measure parameters either from current file or from the entire series
BOOL CMeasureResultsPage::measure_parameters()
{
	// compute file indexes
	const int current_file_index = m_pdb_doc->db_get_current_record_position();
	ASSERT(current_file_index >= 0);
	auto i_first = current_file_index;
	auto i_last = i_first;
	if (m_p_options_measure->b_all_files)
	{
		i_first = 0;
		i_last = m_pdb_doc->db_get_records_count() - 1;
	}

	// prepare list_control
	//	while (m_listResults.DeleteColumn(0) != 0);
	m_list_results.DeleteAllItems();

	// prepare clipboard and Edit control (CEdit)
	if (OpenClipboard())
	{
		EmptyClipboard(); // prepare clipboard and copy text to buffer
		constexpr DWORD dw_len = 32768; // 32 Kb
		const auto h_copy = GlobalAlloc(GHND, dw_len);
		if (h_copy == nullptr)
		{
			AfxMessageBox(_T("Memory low: unable to allocate memory"));
			return TRUE;
		}
		auto p_copy = static_cast<TCHAR*>(GlobalLock(h_copy));
		auto p_copy0 = p_copy;

		// export Ascii: begin
		BeginWaitCursor();
		output_title();
		BOOL b_success = m_pdb_doc->db_set_current_record_position(i_first);
		const CString filename = m_pdb_doc->db_get_current_dat_file_name();
		auto* p_vd = new options_view_data;
		ASSERT(p_vd != NULL);
		const CString cs_out = _T("**filename\tdate\ttime\tcomment\tchannel\r\n");

		for (auto i = i_first; i <= i_last; i++, m_pdb_doc->db_move_next())
		{
			// open data file
			m_pdb_doc->open_current_data_file();

			p_vd->b_acq_comments = TRUE; // global comment
			p_vd->b_acq_date = TRUE; // acquisition date
			p_vd->b_acq_time = TRUE; // acquisition time
			p_vd->b_file_size = FALSE; // file size
			p_vd->b_acq_channel_comment = FALSE; // acq channel individual comment
			p_vd->b_acq_channel_setting = FALSE; // acq chan individual settings (gain, filter, etc.)
			auto cs = cs_out;
			cs += m_pdb_doc->m_p_data_doc->get_data_file_infos(p_vd);
			p_copy += wsprintf(p_copy, _T("%s\r\n"), static_cast<LPCTSTR>(cs));

			// output title for this data set
			auto i_chan_0 = 0;
			auto i_chan_1 = m_p_chart_data_wnd->get_channel_list_size() - 1;
			if (!m_p_options_measure->b_all_channels)
			{
				i_chan_0 = m_p_options_measure->w_source_channel;
				i_chan_1 = i_chan_0;
			}
			if (i_chan_0 >= m_p_chart_data_wnd->get_channel_list_size())
				i_chan_0 = 0;
			if (i_chan_1 >= m_p_chart_data_wnd->get_channel_list_size())
				i_chan_1 = m_p_chart_data_wnd->get_channel_list_size() - 1;

			m_col_ = 1;
			for (auto i_chan = i_chan_0; i_chan <= i_chan_1; i_chan++)
			{
				// measure according to option
				switch (m_p_options_measure->w_option)
				{
				case 0: measure_from_vertical_tags(i_chan);
					break;
				case 1: measure_from_horizontal_cursors(i_chan);
					break;
				case 2: measure_from_rectangle(i_chan);
					break;
				case 3: measure_from_stimulus(i_chan);
					break;
				default:
					break;
				}
				m_col_++;
			}

			// transfer content of list_ctrl to clipboard
			p_copy += wsprintf(p_copy, _T("%s"), static_cast<LPCTSTR>(m_cs_title_));
			const auto n_lines = m_list_results.GetItemCount();
			const auto n_columns = m_nb_data_cols_ * (m_col_ - 1) + 1;
			for (auto item = 0; item < n_lines; item++)
			{
				for (auto column = 0; column < n_columns; column++)
				{
					auto cs_content = m_list_results.GetItemText(item, column);
					p_copy += wsprintf(p_copy, _T("%s\t"), static_cast<LPCTSTR>(cs_content));
				}
				p_copy--;
				*p_copy = '\r';
				p_copy++;
				*p_copy = '\n';
				p_copy++;
			}
			// if not, then next file will override the results computed before
		}

		*p_copy = 0;
		b_success = m_pdb_doc->db_set_current_record_position(current_file_index);
		m_pdb_doc->open_current_data_file();
		EndWaitCursor(); // it's done

		// export Ascii: end //////////////////////////////////////////////
		m_c_edit_results.SetWindowText(p_copy0);
		GlobalUnlock(h_copy);
		SetClipboardData(CF_TEXT, h_copy);
		CloseClipboard(); // close connect w.clipboard
		delete p_vd; // delete temporary object
	}
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// remove tags on exit
void CMeasureResultsPage::OnOK()
{
	m_p_chart_data_wnd->Invalidate();
	CPropertyPage::OnOK();
}

// measure parameters each time this page is selected
BOOL CMeasureResultsPage::OnSetActive()
{
	measure_parameters();
	return CPropertyPage::OnSetActive();
}
