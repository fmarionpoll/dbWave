#include "StdAfx.h"
#include "dbWave.h"
#include "dbWaveDoc.h"
#include "DlgExportData.h"

#include "ADAcqDataDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EXPORT_DBWAVE	0
#define EXPORT_SAPID	1
#define EXPORT_TEXT		2
#define EXPORT_EXCEL	3

DlgExportData::DlgExportData(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

void DlgExportData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPORTAS, m_combo_export_as);
	DDX_Control(pDX, IDC_SOURCEFILES, m_file_drop_list);
	DDX_Text(pDX, IDC_TIMEFIRST, m_time_first);
	DDX_Text(pDX, IDC_TIMELAST, m_time_last);
	DDX_Text(pDX, IDC_CHANNELNUMBER, m_channel_number);
	DDX_Text(pDX, IDC_RATIO, m_i_under_sample);
	DDV_MinMaxInt(pDX, m_i_under_sample, 1, 20000);
}

BEGIN_MESSAGE_MAP(DlgExportData, CDialog)
	ON_BN_CLICKED(IDC_SINGLECHANNEL, on_single_channel)
	ON_BN_CLICKED(IDC_ALLCHANNELS, on_all_channels)
	ON_BN_CLICKED(IDC_ENTIREFILE, on_entire_file)
	ON_CBN_SELCHANGE(IDC_EXPORTAS, on_sel_change_export_as)
	ON_BN_CLICKED(IDC_EXPORT, on_export)
	ON_BN_CLICKED(IDC_EXPORTALL, on_export_all)
END_MESSAGE_MAP()

void DlgExportData::update_struct_from_controls()
{
	UpdateData(TRUE);

	iiv_o.export_type = m_combo_export_as.GetCurSel();
	iiv_o.all_channels =static_cast<boolean>(static_cast<CButton*>(GetDlgItem(IDC_ALLCHANNELS))->GetCheck());
	iiv_o.separate_comments = static_cast<boolean>(static_cast<CButton*>(GetDlgItem(IDC_SAVECOMMENTS))->GetCheck());
	iiv_o.entire_file =static_cast<boolean>(static_cast<CButton*>(GetDlgItem(IDC_ENTIREFILE))->GetCheck());
	iiv_o.include_time =  static_cast<boolean>(static_cast<CButton*>(GetDlgItem(IDC_BTIMESTEPS))->GetCheck());
	iiv_o.selected_channel = m_channel_number;
	iiv_o.time_first = m_time_first;
	iiv_o.time_last = m_time_last;
}

BOOL DlgExportData::DestroyWindow()
{
	// restore initial conditions
	if (m_i_current_file >= 0)
	{
		try
		{
			m_db_doc->db_set_current_record_position(m_i_current_file);
			m_db_doc->open_current_data_file();
		}
		catch (CDaoException* e)
		{
			DisplayDaoException(e, 1);
			e->Delete();
		}
	}
	return CDialog::DestroyWindow();
}

BOOL DlgExportData::OnInitDialog()
{
	CDialog::OnInitDialog();

	// extra initialization
	// handle to the instance
	const CdbWaveApp* p_app = static_cast<CdbWaveApp*>(AfxGetApp()); // pointer to application
	iiv_o = p_app->options_import; // copy structure / options

	// update dependent controls
	m_combo_export_as.SetCurSel(iiv_o.export_type); // combo-box
	if (iiv_o.export_type != 0) // hide if not sapid
	{
		GetDlgItem(IDC_SAVECOMMENTS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTIMESTEPS)->ShowWindow(SW_SHOW);
	}
	static_cast<CButton*>(GetDlgItem(IDC_SAVECOMMENTS))->SetCheck(iiv_o.separate_comments);
	static_cast<CButton*>(GetDlgItem(IDC_BTIMESTEPS))->SetCheck(iiv_o.include_time);

	static_cast<CButton*>(GetDlgItem(IDC_ENTIREFILE))->SetCheck(iiv_o.entire_file);
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(!iiv_o.entire_file);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(!iiv_o.entire_file);

	if (m_db_doc->m_p_data_doc->get_vt_tags_list()->get_tag_list_size() < 1)
	{
		m_time_first = iiv_o.time_first;
		m_time_last = iiv_o.time_last;
	}
	else
	{
		AcqDataDoc* pDat = m_db_doc->m_p_data_doc;
		m_time_first = static_cast<float>(pDat->get_vt_tags_list()->get_tag_value_long(0));
		if (pDat->get_vt_tags_list()->get_tag_list_size() > 1)
			m_time_last = static_cast<float>(pDat->get_vt_tags_list()->get_tag_value_long(1));
		else
			m_time_last = static_cast<float>(pDat->get_doc_channel_length());
		const float ch_rate = pDat->get_wave_format()->sampling_rate_per_channel;
		m_time_first /= ch_rate;
		m_time_last /= ch_rate;
	}

	static_cast<CButton*>(GetDlgItem(IDC_ALLCHANNELS))->SetCheck(iiv_o.all_channels);
	static_cast<CButton*>(GetDlgItem(IDC_SINGLECHANNEL))->SetCheck(!iiv_o.all_channels);

	GetDlgItem(IDC_CHANNELNUMBER)->EnableWindow(!iiv_o.all_channels);
	m_channel_number = iiv_o.selected_channel; //  one channel

	// get filename(s) and select first in the list
	m_i_current_file = m_db_doc->db_get_current_record_position();
	if (m_b_all_files)
	{
		const int n_files = m_db_doc->db_get_records_count();
		for (int i = 0; i < n_files; i++)
		{
			m_db_doc->db_set_current_record_position(i);
			CString filename = m_db_doc->db_get_current_dat_file_name();
			const int i_count = filename.GetLength() - filename.ReverseFind('\\') - 1;
			m_file_drop_list.AddString(filename.Right(i_count));
			m_file_drop_list.SetItemData(i, i);
		}
		m_db_doc->db_set_current_record_position(m_i_current_file);
		m_file_drop_list.SetCurSel(m_i_current_file);
	}
	else
	{
		CString filename = m_db_doc->db_get_current_dat_file_name();
		const int i_count = filename.GetLength() - filename.ReverseFind('\\') - 1;
		int i = m_file_drop_list.AddString(filename.Right(i_count));
		m_file_drop_list.SetItemData(i, m_i_current_file);
		m_file_drop_list.SetCurSel(0);
	}
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgExportData::OnOK()
{
	update_struct_from_controls();

	// handle to the instance
	auto p_app = static_cast<CdbWaveApp*>(AfxGetApp()); 
	p_app->options_import = iiv_o; 

	CDialog::OnOK();
}

void DlgExportData::on_single_channel()
{
	iiv_o.all_channels = FALSE;
	GetDlgItem(IDC_CHANNELNUMBER)->EnableWindow(TRUE);
}

void DlgExportData::on_all_channels()
{
	iiv_o.all_channels = FALSE;
	GetDlgItem(IDC_CHANNELNUMBER)->EnableWindow(FALSE);
}

void DlgExportData::on_entire_file()
{
	iiv_o.entire_file = !iiv_o.entire_file;
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(!iiv_o.entire_file);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(!iiv_o.entire_file);
}

void DlgExportData::on_sel_change_export_as()
{
	UpdateData(TRUE); 
	iiv_o.export_type =static_cast<WORD>(m_combo_export_as.GetCurSel());
	int b_show = SW_HIDE;
	if (iiv_o.export_type == 0)
		b_show = SW_SHOW;
	GetDlgItem(IDC_SAVECOMMENTS)->ShowWindow(b_show);
	GetDlgItem(IDC_BTIMESTEPS)->ShowWindow(!b_show);
}

void DlgExportData::on_export_all()
{
	const int nb_items = m_file_drop_list.GetCount();
	CWaitCursor wait;

	for (int i = 0; i < nb_items; i++)
	{
		m_file_drop_list.SetCurSel(i);
		UpdateData(FALSE);
		m_file_drop_list.UpdateWindow();
		on_export();
	}

	const CString cs_dummy = _T("Done: \nAll file were exported");
	MessageBox(cs_dummy, _T("Export file"));
}

void DlgExportData::on_export()
{
	Export();
}

void DlgExportData::Export()
{
	update_struct_from_controls();

	// extract source file name and build destination file name
	const int cur_sel = m_file_drop_list.GetCurSel(); 
	const int index = static_cast<int>(m_file_drop_list.GetItemData(cur_sel)); 
	m_db_doc->db_set_current_record_position(index);
	m_db_doc->open_current_data_file();
	m_file_source = m_db_doc->db_get_current_dat_file_name(); // now, extract path to source file

	const int i_count1 = m_file_source.ReverseFind('\\') + 1; // and extract that chain of chars
	const CString cs_path = m_file_source.Left(i_count1); // store it into cs_path
	const int i_count2 = m_file_source.ReverseFind('.'); // extract file name
	m_file_dest = m_file_source.Mid(i_count1, i_count2 - i_count1); // without file extension

	CString cs_description; // find file prefix and extension
	m_combo_export_as.GetWindowText(cs_description); // within drop down list
	int index1 = cs_description.Find('(') + 1; // first, get prefix
	int index2 = cs_description.Find('*'); // that is between "(" and "*"
	const CString csPrefix = cs_description.Mid(index1, index2 - index1);
	index1 = cs_description.ReverseFind('.'); // then get new file extension
	index2 = cs_description.ReverseFind(')');
	const CString csExt = cs_description.Mid(index1, index2 - index1);

	m_file_dest = csPrefix + m_file_dest + csExt; // lastly build, the new name
	m_file_dest = cs_path + m_file_dest; // and add path

	// compute some parameters
	m_p_dat = m_db_doc->m_p_data_doc; // pointer to data document
	const CWaveFormat* p_wave_format = m_p_dat->get_wave_format();
	if (iiv_o.entire_file) // if all data, load
	{
		// intervals from data file
		mm_time_first = 0.f;
		mm_time_last = p_wave_format->duration;
		mm_l_first = 0;
		mm_l_last = m_p_dat->get_doc_channel_length();
	}
	else // else, convert time into
	{
		// file indexes
		mm_time_first = m_time_first;
		mm_time_last = m_time_last;
		if (mm_time_last < mm_time_first)
			mm_time_last = p_wave_format->duration;
		mm_l_first = static_cast<long>(mm_time_first * p_wave_format->sampling_rate_per_channel);
		mm_l_last = static_cast<long>(mm_time_last * p_wave_format->sampling_rate_per_channel);
	}

	mm_first_chan = 0; // loop through all chans
	mm_last_chan = p_wave_format->scan_count - 1; // or only one
	if (!iiv_o.all_channels) // depending on this flag
	{
		mm_first_chan = iiv_o.selected_channel; // flag set: change limits
		mm_last_chan = mm_first_chan;
	}

	mm_bin_zero = 0;

	// now that filenames are built, export the files
	switch (m_combo_export_as.GetCurSel())
	{
	case 0:
		export_data_as_db_wave_file();
		break;
	case 1: // sapid file
		export_data_as_sapid_file();
		break;
	case 2: // txt file
		export_data_as_text_file();
		break;
	case 3: // excel file
		export_data_as_excel_file();
		break;
	default:
		break;
	}

	// delete current file and select next (eventually; if not possible, exit)
	m_file_drop_list.DeleteString(cur_sel); 
	if (m_file_drop_list.GetCount() > 0)
		m_file_drop_list.SetCurSel(0); 
	else
		OnOK(); // exit dialog box
}

// export data into a text file

BOOL DlgExportData::export_data_as_text_file()
{
	// open destination file
	CStdioFile data_dest; 
	CFileException fe;
	if (!data_dest.Open(m_file_dest, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText,
	                   &fe))
	{
		data_dest.Abort(); 
		return FALSE; 
	}

	CString cs_char_buf;
	CString cs_dummy;

	// LINE 1.......... data file name
	// LINE 2.......... date
	// LINE 3.......... file comment

	cs_char_buf.Format(_T("file :\t%s\r\n"), (LPCTSTR)m_file_source);
	const CWaveChanArray* p_chan_array = m_p_dat->get_wave_channels_array();
	const CWaveFormat* p_wave_format = m_p_dat->get_wave_format();

	const CString cs_date = (p_wave_format->acquisition_time).Format("%#d %B %Y %X");
	cs_dummy.Format(_T("date :\t%s\r\n"), (LPCTSTR)cs_date);
	cs_char_buf += cs_dummy;
	cs_dummy.Format(_T("comment :\t%s\r\n"), (LPCTSTR)p_wave_format->get_comments(_T("\t")));
	cs_char_buf += cs_dummy;
	data_dest.Write(cs_char_buf, cs_char_buf.GetLength() * sizeof(TCHAR)); // write data
	cs_char_buf.Empty();

	// LINE 4.......... start (s)
	// LINE 5.......... end   (s)

	// first interval (sec)
	cs_char_buf.Format(_T("start (s):\t%f\r\nend   (s):\t%f\r\n"), mm_time_first, mm_time_last);
	data_dest.Write(cs_char_buf, cs_char_buf.GetLength() * sizeof(TCHAR)); 
	cs_char_buf.Empty();

	// LINE 6.......... Sampling rate (Hz)
	// LINE 7.......... A/D channels:
	// LINE 8.......... mV per bin for each channel

	cs_char_buf.Format(_T("Sampling rate (Hz):\t%f\r\n"), p_wave_format->sampling_rate_per_channel);
	cs_dummy.Format(_T("A/D channels :\r\n")); 
	cs_char_buf += cs_dummy;
	cs_dummy.Format(_T("mV per bin:")); // line title
	cs_char_buf += cs_dummy;

	int i;
	for (i = mm_first_chan; i <= mm_last_chan; i++) 
	{
		float volts_per_bin; 
		m_p_dat->get_volts_per_bin(i, &volts_per_bin, 0); 
		cs_dummy.Format(_T("\t%f"), static_cast<double>(volts_per_bin) * 1000.f); 
		cs_char_buf += cs_dummy;
	}

	// LINE 9.......... comment for each channel

	cs_dummy.Format(_T("\r\ncomments"));
	cs_char_buf += cs_dummy;
	data_dest.Write(cs_char_buf, cs_char_buf.GetLength() * sizeof(TCHAR)); 
	cs_char_buf.Empty();

	for (i = mm_first_chan; i <= mm_last_chan; i++)
	{
		cs_dummy.Format(_T("\t%s"), (LPCTSTR)p_chan_array->get_p_channel(i)->am_csComment);
		cs_char_buf += cs_dummy;
	}

	cs_dummy.Format(_T("\r\n")); // add CRLF
	cs_char_buf += cs_dummy;
	data_dest.Write(cs_char_buf, cs_char_buf.GetLength() * sizeof(TCHAR)); 
	cs_char_buf.Empty();

	// export data only if text
	if (iiv_o.export_type == EXPORT_TEXT)
	{
		// LINE 10.......... header for each channel

		if (iiv_o.include_time)
		{
			cs_dummy.Format(_T("time (s)\t"));
			cs_char_buf += cs_dummy;
		}
		for (i = mm_first_chan; i <= mm_last_chan; i++)
		{
			cs_dummy.Format(_T("chan_%i\t"), i);
			cs_char_buf += cs_dummy;
		}
		cs_char_buf = cs_char_buf.Left(cs_char_buf.GetLength() - 1);
		cs_dummy.Format(_T("\r\n"));
		cs_char_buf += cs_dummy;
		data_dest.Write(cs_char_buf, cs_char_buf.GetLength() * sizeof(TCHAR)); 
		cs_char_buf.Empty();

		// LINE .......... values

		m_p_dat->get_value_from_buffer(0, -1);
		// loop over all values
		for (int ii_time = mm_l_first; ii_time < mm_l_last; ii_time++)
		{
			if (iiv_o.include_time) // add time stamp
			{
				cs_dummy.Format(_T("%li"), ii_time);
				cs_char_buf += cs_dummy;
			}
			for (i = mm_first_chan; i <= mm_last_chan; i++) 
			{
				// get value and convert into ascii
				const short value = m_p_dat->get_value_from_buffer(i, ii_time) - mm_bin_zero;
				cs_dummy.Format(_T("\t%i"), value);
				cs_char_buf += cs_dummy;
			}
			cs_char_buf += cs_dummy;
			data_dest.Write(cs_char_buf, cs_char_buf.GetLength() * sizeof(TCHAR)); 
			cs_char_buf.Empty();
		}
	}

	data_dest.Close();
	return TRUE;
}

BOOL DlgExportData::export_data_as_sapid_file()
{
	// open destination file
	CFile data_dest;
	CFileException fe; 
	if (!data_dest.Open(m_file_dest, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeBinary,
	                   &fe))
	{
		data_dest.Abort(); 
		return FALSE; 
	}

	char* p_dummy[] = {"awave32"}; 
	data_dest.Write(p_dummy, 7); 

	short value;
	for (int ii_time = mm_l_first; ii_time < mm_l_last; ii_time++)
	{
		for (auto i = mm_first_chan; i <= mm_last_chan; i++) 
		{
			// get value and convert into ascii
			value =  m_p_dat->get_value_from_buffer(i, ii_time) - static_cast<short>(mm_bin_zero);
			data_dest.Write(&value, sizeof(short));
		}
	}
	// last value is sampling rate
	value = static_cast<short>(m_p_dat->get_wave_format()->sampling_rate_per_channel);
	data_dest.Write(&value, sizeof(short));
	data_dest.Flush();
	data_dest.Close(); // close file

	// export file description in a separate text file
	if (iiv_o.separate_comments)
	{
		const auto file_dest = m_file_dest;
		m_file_dest = m_file_dest.Left(m_file_dest.GetLength() - 3) + _T("TXT");
		export_data_as_text_file();
		m_file_dest = file_dest;
	}

	return TRUE;
}

// export Excel files
// according to a sample described in the MSDN April 1996
// BIFF samples R/W binary interchange file format (Microsoft SDK samples)
#define BIFF_BLANK		1
#define	BIFF_WORD		2
#define	BIFF_FLOAT		4
#define	BIFF_CHARS		8
#define	BIFF_BOOL		16

BOOL DlgExportData::export_data_as_excel_file()
{
	// variables to receive data & or text

	double f_double;
	auto row = 0;
	auto col = 0;

	// open data file and create BOF record
	CFile data_dest; // destination file object
	CFileException fe; // trap exceptions
	if (!data_dest.Open(m_file_dest, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeBinary,
	                    &fe))
	{
		data_dest.Abort(); // file not found
		return FALSE; // and return
	}

	// write BOF record
	WORD wi;
	wi = 9;
	data_dest.Write(&wi, sizeof(WORD)); //  version=BIFF2, bof record
	wi = 4;
	data_dest.Write(&wi, sizeof(WORD)); // ?
	wi = 2;
	data_dest.Write(&wi, sizeof(WORD)); //
	wi = 0x10;
	data_dest.Write(&wi, sizeof(WORD)); // worksheet identifier

	// write dimension record of the worksheet
	constexpr WORD last_row = 16384;
	constexpr WORD last_col = 4096;

	wi = 0;
	data_dest.Write(&wi, sizeof(WORD));
	wi = 8;
	data_dest.Write(&wi, sizeof(WORD));
	wi = 0;
	data_dest.Write(&wi, sizeof(WORD)); // first row
	wi = last_row;
	data_dest.Write(&wi, sizeof(WORD)); // last row
	wi = 0;
	data_dest.Write(&wi, sizeof(WORD)); // first column
	wi = last_col;
	data_dest.Write(&wi, sizeof(WORD)); // last column

	// LINE 1.......... data file name
	// LINE 2.......... date
	// LINE 3.......... file comment

	// file name
	save_biff(&data_dest, BIFF_CHARS, row, col, "file");
	col++;
	save_c_string_biff(&data_dest, row, col, m_file_source);
	col--;
	row++;
	// date
	const auto p_wave_format = m_p_dat->get_wave_format();
	const auto p_chan_array = m_p_dat->get_wave_channels_array();

	auto date = (p_wave_format->acquisition_time).Format(_T("%#d %B %Y %X")); 
	save_biff(&data_dest, BIFF_CHARS, row, col, "date");
	col++;
	save_c_string_biff(&data_dest, row, col, date);
	col--;
	row++;
	// comments
	save_biff(&data_dest, BIFF_CHARS, row, col, "comments");
	col++;
	date = p_wave_format->get_comments(_T("\t"));
	save_c_string_biff(&data_dest, row, col, date);
	col--;
	row++;

	// LINE 4.......... start (s)
	// LINE 5.......... end   (s)

	save_biff(&data_dest, BIFF_CHARS, row, col, "start (s)");
	col++;
	f_double = mm_time_first;
	save_biff(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&f_double));
	col--;
	row++;

	save_biff(&data_dest, BIFF_CHARS, row, col, "end (s)");
	col++;
	f_double = mm_time_last;
	save_biff(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&f_double));
	col--;
	row++;

	// LINE 6.......... Sampling rate (Hz)
	// LINE 7.......... A/D channels:
	// LINE 8.......... mV per bin for each channel

	save_biff(&data_dest, BIFF_CHARS, row, col, "Sampling rate (Hz)");
	col++;
	f_double = p_wave_format->sampling_rate_per_channel;
	save_biff(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&f_double));
	col--;
	row++;

	save_biff(&data_dest, BIFF_CHARS, row, col, "A/D channels :");
	row++;
	save_biff(&data_dest, BIFF_CHARS, row, col, "mV per bin:");
	col++;

	for (auto i = mm_first_chan; i <= mm_last_chan; i++) // loop through all chans
	{
		float volts_per_bin; // declare float
		m_p_dat->get_volts_per_bin(i, &volts_per_bin, 0); // get value
		f_double = volts_per_bin;
		save_biff(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&f_double));
		col++;
	}
	row++;
	col = 0;

	// LINE 9.......... comment for each channel
	save_biff(&data_dest, BIFF_CHARS, row, col, "comments");
	col++;
	for (int i = mm_first_chan; i <= mm_last_chan; i++)
	{
		CWaveChan* p_chan = p_chan_array->get_p_channel(i);
		CString comment = p_chan->am_csComment;
		save_c_string_biff(&data_dest, row, col, comment);
		col++;
	}
	col = 0;
	row++;
	row++;

	// loop to write data: write columns header, split cols if too much data

	auto n_cols_per_bout = mm_last_chan - mm_first_chan + 1; // nb column within 1 bout
	if (iiv_o.include_time) 
		n_cols_per_bout++;
	constexpr auto max_row = 16383; 
	const auto doc_length = mm_l_last - mm_l_first + 1; 
	const long bout_length = max_row - row;
	int n_bouts = doc_length / bout_length; 
	if (bout_length * n_bouts < doc_length) 
		n_bouts++;
	if (n_bouts * n_cols_per_bout > 256)
	{
		AfxMessageBox(_T("Too much data: export file clipped"));
		n_bouts = 256 / n_cols_per_bout;
		mm_l_last = n_bouts * bout_length;
	}
	auto j0 = 0;
	if (iiv_o.include_time)
		j0++;
	for (auto i = 0; i < n_bouts; i++)
	{
		CString iteration;
		iteration.Format(_T("[%i]"), i);
		CString comment;
		if (iiv_o.include_time)
		{
			comment = _T("time") + iteration;
			save_c_string_biff(&data_dest, row, col, comment);
			col++;
		}
		for (auto j = j0; j < n_cols_per_bout; j++)
		{
			comment.Format(_T("chan_%i"), j - j0);
			comment += iteration;
		}
		col = 0;
		row++;

		// iterate to read data and export to Excel file
		auto ii_time = 0;
		float volts_per_bin;
		const double rate = p_wave_format->sampling_rate_per_channel;
		auto dummy = m_p_dat->get_value_from_buffer(0, -1); 

		for (int k = mm_l_first; k < bout_length; k++)
		{
			ii_time = k;
			save_c_string_biff(&data_dest, row, col, comment);
			col++;
		}
		for (auto i = 0; i < n_bouts; i++)
		{
			if (ii_time >= mm_l_last)
				continue;
			if (iiv_o.include_time)
			{
				f_double = ii_time / rate;
				save_biff(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&f_double));
				col++;
			}

			for (auto j = j0; j < n_cols_per_bout; j++)
			{
				m_p_dat->get_volts_per_bin(j - j0, &volts_per_bin, 0);
				f_double = (static_cast<double>(m_p_dat->get_value_from_buffer(j - j0, ii_time)) - static_cast<double>(mm_bin_zero)) *
					static_cast<double>(volts_per_bin);
				save_biff(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&f_double));
				col++;
			}
			ii_time += bout_length;
		}
		row++;
		col = 0;
	}

	unsigned long int wwi = 0x000000a;
	data_dest.Write(&wwi, sizeof(unsigned long int));
	data_dest.Flush();

	return TRUE;
}

// save data to a BIFF_ file

using ATTR = struct
{
	unsigned long a : 24;
};

void DlgExportData::save_biff(CFile* fp, const int type, const int row, const int col, const char* data)
{
	constexpr ATTR attribute = {0x000000};
	WORD w1, w2, w_row, w_col;
	switch (type)
	{
	case BIFF_BLANK: w1 = 1;
		w2 = 7;
		break; // blank cell
	case BIFF_WORD: w1 = 2;
		w2 = 9;
		break; // integer data
	case BIFF_FLOAT: w1 = 3;
		w2 = 15;
		break; // number (IEEE floating type)
	case BIFF_CHARS: w1 = 4;
		w2 = 8 + static_cast<WORD>(strlen(data));
		break; // label
	case BIFF_BOOL: w1 = 5;
		w2 = 9;
		break; // boolean
	}

	// write record header
	fp->Write(&w1, sizeof(WORD));
	fp->Write(&w2, sizeof(WORD));
	w_row = static_cast<WORD>(row);
	fp->Write(&w_row, sizeof(WORD));
	w_col = static_cast<WORD>(col);
	fp->Write(&w_col, sizeof(WORD));
	fp->Write(&attribute, 3);

	// write data
	char i_char;
	switch (type)
	{
	case BIFF_BLANK:
		break; // blank cell
	case BIFF_WORD:
		fp->Write(data, sizeof(short));
		break; // integer data
	case BIFF_FLOAT:
		fp->Write(data, 8);
		break; // number (IEEE floating type)
	case BIFF_CHARS:
		i_char = static_cast<char>(strlen(data));
		fp->Write(&i_char, 1);
		fp->Write(data, i_char);
		break;
	case BIFF_BOOL:
		i_char = *data ? 1 : 0;
		fp->Write(&i_char, 1);
		i_char = 0;
		fp->Write(&i_char, 1);
		break;
	default: ;
	}
}

void DlgExportData::save_c_string_biff(CFile* fp, const int row, const int col, const CString& data)
{
	constexpr ATTR attribute = {0x000000};
	WORD wi, w_row, w_col;

	// write record header
	wi = 4;
	fp->Write(&wi, sizeof(WORD));
	wi = 8 + data.GetLength();
	fp->Write(&wi, sizeof(WORD));
	w_row =static_cast<WORD>(row);
	fp->Write(&w_row, sizeof(WORD));
	w_col = static_cast<WORD>(col);
	fp->Write(&w_col, sizeof(WORD));
	fp->Write(&attribute, 3);
	const char i_char = data.GetLength();
	fp->Write(&i_char, 1);
	fp->Write(data, i_char);
}

BOOL DlgExportData::export_data_as_db_wave_file()
{
	// create new file
	const auto pDatDest = new ADAcqDataDoc;
	pDatDest->acq_create_file(m_file_dest);

	// load data header and save it into dest file
	const auto pw_f_dest = pDatDest->get_wave_format();
	const auto pw_c_dest = pDatDest->get_wave_channels_array();
	const auto pw_f_source = m_p_dat->get_wave_format();
	const auto pw_c_source = m_p_dat->get_wave_channels_array();
	pw_f_dest->copy(pw_f_source);
	pw_c_dest->Copy(pw_c_source);

	const auto n_channels = mm_last_chan - mm_first_chan + 1;
	if (pw_f_dest->scan_count != n_channels)
	{
		const auto last_channel = pw_f_dest->scan_count - 1;
		for (auto i = last_channel; i > 0; i--)
		{
			if (i > mm_last_chan || i < mm_first_chan)
				pw_c_dest->chan_array_remove_at(i);
		}
		ASSERT(n_channels == pw_c_dest->chan_array_get_size());
		pw_f_dest->scan_count = n_channels;
	}

#define LEN 16384
	auto* p_data = new short[LEN];
	ASSERT(p_data != NULL);
	auto p_dat = p_data;

	auto dat_len = 0;
	pw_f_dest->sample_count = 0;
	pDatDest->AcqDoc_DataAppendStart();
	auto dummy = m_p_dat->get_value_from_buffer(0, -1); 

	for (int ii_time = mm_l_first; ii_time < mm_l_last; ii_time++)
	{
		for (auto i = mm_first_chan; i <= mm_last_chan; i++) 
		{
			*p_dat = m_p_dat->get_value_from_buffer(i, ii_time) - mm_bin_zero;
			dat_len++;
			p_dat++;
			if (dat_len >= LEN)
			{
				pDatDest->AcqDoc_DataAppend(p_data, dat_len * 2);
				dat_len = 0;
				p_dat = p_data;
			}
		}
	}
	if (dat_len > 0) // save incomplete buffer
		pDatDest->AcqDoc_DataAppend(p_data, dat_len * 2);

	// stop appending data, update dependent struct
	pDatDest->AcqDoc_DataAppendStop();

	// delete pdataDest object created here
	pDatDest->acq_close_file();

	delete pDatDest;
	delete[] p_data;

	// export file description in a separate text file
	if (iiv_o.separate_comments)
	{
		const auto file_dest = m_file_dest;
		m_file_dest = m_file_dest.Left(m_file_dest.GetLength() - 3) + _T("TXT");
		export_data_as_text_file();
		m_file_dest = file_dest;
	}

	return TRUE;
}
