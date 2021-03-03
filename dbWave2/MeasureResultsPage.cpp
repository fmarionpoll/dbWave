// CMeasureOptionsPage.cpp : implementation file
//
// TODO : measure data and output to notedocview

#include "StdAfx.h"
#include "chart.h"
#include "ChartData.h"
//#include "Editctrl.h"
#include "NoteDoc.h"
#include "dbWaveDoc.h"
#include "resource.h"
#include "MeasureResultsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMeasureResultsPage, CPropertyPage)

/////////////////////////////////////////////////////////////////////////////
// CMeasureResultsPage property page

CMeasureResultsPage::CMeasureResultsPage() : CPropertyPage(CMeasureResultsPage::IDD), m_pdbDoc(nullptr)
{
	m_pChartDataWnd = nullptr;
}

CMeasureResultsPage::~CMeasureResultsPage()
{
}

void CMeasureResultsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RESULTS, m_CEditResults);
	DDX_Control(pDX, IDC_LIST1, m_listResults);
}

BEGIN_MESSAGE_MAP(CMeasureResultsPage, CPropertyPage)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
END_MESSAGE_MAP()

void CMeasureResultsPage::OnExport()
{
	CString csBuffer;
	// copy results from CListCtrl into text buffer
	m_CEditResults.GetWindowText(csBuffer);
	const auto p_app = static_cast<CdbWaveApp*>(AfxGetApp());

	CMultiDocTemplate* p_template = p_app->m_pNoteViewTemplate;
	const auto p_doc = p_template->OpenDocumentFile(nullptr);
	auto pos = p_doc->GetFirstViewPosition();
	const auto p_view = (CRichEditView*)p_doc->GetNextView(pos);
	auto& p_edit = p_view->GetRichEditCtrl();
	p_edit.SetWindowText(csBuffer);		// copy content of window into CString
}

void CMeasureResultsPage::OutputTitle()
{
	// prepare clistctrl column headers
	auto columns = m_listResults.InsertColumn(0, _T("#"), LVCFMT_LEFT, 20, 0) + 1;

	// column headers for vertical tags
	m_csTitle = _T("#");
	CString cs_cols = _T("");
	m_nbdatacols = 0;

	switch (m_pMO->wOption)
	{
		// ......................  vertical tags
	case 0:
		if (m_pMO->bDatalimits)
		{
			if (!m_pMO->btime)	cs_cols += _T("\tt1(mV)\tt2(mV)");
			else				cs_cols += _T("\tt1(mV)\tt1(s)\tt2(mV)\tt2(s)");
		}
		if (m_pMO->bDiffDatalimits)
		{
			cs_cols += _T("\tt2-t1(mV)");
			if (m_pMO->btime)	cs_cols += _T("\tt2-t1(s)");
		}
		if (m_pMO->bExtrema)
		{
			if (!m_pMO->btime)	cs_cols += _T("\tMax(mV)\tmin(mV)");
			else				cs_cols += _T("\tMax(mV)\tMax(s)\tmin(mV)\tmin(s)");
		}
		if (m_pMO->bDiffExtrema)
		{
			cs_cols += _T("\tDiff(mV)");
			if (m_pMO->btime)	cs_cols += _T("\tdiff(s)");
		}
		if (m_pMO->bHalfrisetime)	cs_cols += _T("\t1/2rise(s)");
		if (m_pMO->bHalfrecovery)	cs_cols += _T("\t1/2reco(s)");
		break;

		// ......................  horizontal cursors
	case 1:
		if (m_pMO->bDatalimits)		cs_cols += _T("\tv1(mV)\tv2(mV)");
		if (m_pMO->bDiffDatalimits)	cs_cols += _T("\tv2-v1(mV)");
		break;

		// ......................  rectangle area
	case 2:
		break;
		// ......................  detect stimulus and then measure
	case 3:
		break;
	default:
		break;
	}

	// now set columns - get nb of data channels CString
	if (!cs_cols.IsEmpty())
	{
		auto channel_first = 0;									// assume all data channels
		auto channel_last = m_pChartDataWnd->GetChanlistSize() - 1;	// requested
		const auto n_pixels_mv = (m_listResults.GetStringWidth(_T("0.000000")) * 3) / 2;

		if (!m_pMO->bAllChannels)					// else if flag set
		{											// restrict to a single chan
			channel_first = m_pMO->wSourceChan;				// single channel
			channel_last = channel_first;
		}
		m_nbdatacols = 0;
		auto psz_t = m_szT;
		TCHAR separators[] = _T("\t");		// separator = tab
		TCHAR* next_token = nullptr;

		for (auto channel = channel_first; channel <= channel_last; channel++)
		{
			auto cs = cs_cols;
			auto p_string = cs.GetBuffer(cs.GetLength() + 1);
			p_string++;					// skip first tab
			auto p_token = _tcstok_s(p_string, separators, &next_token);
			while (p_token != nullptr)
			{
				if (m_pMO->bAllChannels)
					wsprintf(m_szT, _T("ch%i-%s"), channel, p_token);
				else
					psz_t = p_token;
				columns = 1 + m_listResults.InsertColumn(columns, psz_t, LVCFMT_LEFT, n_pixels_mv, columns);
				m_csTitle += separators;
				m_csTitle += psz_t;
				p_token = _tcstok_s(nullptr, separators, &next_token);	// get next token
			}

			if (m_nbdatacols == 0)		// number of data columns
				m_nbdatacols = columns - 1;
			cs.ReleaseBuffer();			// release character buffer
		}
	}
	m_csTitle += _T("\r\n");
}

void CMeasureResultsPage::MeasureFromVTtags(const int channel)
{
	const auto n_tags = m_pChartDataWnd->m_VTtags.GetNTags();
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
			MeasureWithinInterval(channel, line, m_pChartDataWnd->m_VTtags.GetTagLVal(tag_first), m_pChartDataWnd->m_VTtags.GetTagLVal(tag_last));
			line++;
			tag_first = -1;
			tag_last = -1;
		}
	}

	// cope with isolated tags
	if (tag_first > 0 && tag_last < 0)
	{
		const auto l1 = m_pChartDataWnd->m_VTtags.GetTagLVal(tag_first);
		MeasureWithinInterval(channel, line, l1, l1);
	}
}

void CMeasureResultsPage::GetMaxMin(const int channel, long l_first, const long l_last)
{
	short* p_data;
	int n_channels;
	const auto p_buf = m_pdatDoc->LoadRawDataParams(&n_channels);
	const auto source_chan = m_pChartDataWnd->GetChanlistSourceChan(channel);
	const auto transform_mode = m_pChartDataWnd->GetChanlistTransformMode(channel);
	const auto span = m_pdatDoc->GetTransfDataSpan(transform_mode);

	// get first data point (init max and min)
	auto buf_chan_first = l_first;
	auto buf_chan_last = l_last;
	int offset;
	m_pdatDoc->LoadRawData(&buf_chan_first, &buf_chan_last, span);
	if (transform_mode > 0)
	{
		p_data = m_pdatDoc->LoadTransfData(l_first, buf_chan_last, transform_mode, source_chan);
		offset = 1;
	}
	else
	{
		p_data = p_buf + (l_first - buf_chan_first) * n_channels + source_chan;
		offset = n_channels;
	}
	m_max = *p_data;
	m_min = m_max;
	m_imax = l_first;
	m_imin = m_imax;
	m_first = m_max;

	// loop through data points
	while (l_first < l_last)
	{
		// load file data and get a pointer to these data
		buf_chan_first = l_first;
		buf_chan_last = l_last;
		m_pdatDoc->LoadRawData(&buf_chan_first, &buf_chan_last, span);
		if (l_last < buf_chan_last)
			buf_chan_last = l_last;
		if (transform_mode > 0)
			p_data = m_pdatDoc->LoadTransfData(l_first, buf_chan_last, transform_mode, source_chan);
		else
			p_data = p_buf + (l_first - buf_chan_first) * n_channels + source_chan;
		// now search for max and min
		for (int i = l_first; i <= buf_chan_last; i++)
		{
			m_last = *p_data;
			p_data += offset;
			if (m_last > m_max)
			{
				m_max = m_last;
				m_imax = i;
			}
			else if (m_last < m_min)
			{
				m_min = m_last;
				m_imin = i;
			}
		}
		// search ended, update variable
		l_first = buf_chan_last + 1;
	}
}

// output results into CListCtrl
void CMeasureResultsPage::MeasureWithinInterval(const int channel, const int line, const long l1, const long l2)
{
	// get scale factor for channel and sampling rate
	m_mVperBin = m_pChartDataWnd->GetChanlistVoltsperDataBin(channel) * 1000.0f;
	const auto rate = m_pdatDoc->GetpWaveFormat()->chrate;

	auto output_column = (m_col - 1) * m_nbdatacols + 1;		// output data into column icol
	auto item = m_listResults.GetItemCount();	// compute which line will receive data
	if (line >= item)
	{
		wsprintf(&m_szT[0], _T("%i"), item);		// if new line, print line nb
		m_listResults.InsertItem(item, m_szT);	// and insert a new line within table
	}
	else
		item = line;								// else set variable value

	// measure parameters / selected interval; save results within common vars
	GetMaxMin(channel, l1, l2);

	// output data according to options : data value at limits
	//m_szT[0] = '\t';								// prepare string for edit output
	CString cs_dummy;
	float	x_dummy;
	const CString cs_fmt(_T("\t%f"));
	if (m_pMO->bDatalimits)
	{
		x_dummy = static_cast<float>(m_first) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(l1) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}

		x_dummy = static_cast<float>(m_last) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(l2) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
	}

	if (m_pMO->bDiffDatalimits)
	{
		x_dummy = static_cast<float>(m_last - m_first) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = (static_cast<float>(l2) - l1) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
	}

	// measure max and min (value, time)
	if (m_pMO->bExtrema)
	{
		x_dummy = static_cast<float>(m_max) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(m_imax) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
		x_dummy = static_cast<float>(m_min) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(m_imin) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		}
	}

	// difference between extrema (value, time)
	if (m_pMO->bDiffExtrema)
	{
		x_dummy = static_cast<float>(m_max - m_min) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, output_column, cs_dummy); output_column++;
		if (m_pMO->btime)
		{
			x_dummy = static_cast<float>(m_imax - m_imin) / rate;
			cs_dummy.Format(cs_fmt, x_dummy);
			m_listResults.SetItemText(item, output_column, cs_dummy);
		}
	}

	// time necessary to reach half of the amplitude
	if (m_pMO->bHalfrisetime)
	{
	}

	// time necessary to regain half of the initial amplitude
	if (m_pMO->bHalfrecovery)
	{
	}
}

void CMeasureResultsPage::MeasureFromHZcur(int ichan)
{
	const auto number_of_tags = m_pChartDataWnd->m_HZtags.GetNTags();
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
			MeasureBetweenHZ(ichan, line, 
				m_pChartDataWnd->m_HZtags.GetValue(tag_first), 
				m_pChartDataWnd->m_HZtags.GetValue(tag_last));
			line++;
			tag_first = -1;
			tag_last = -1;
		}
	}

	// cope with isolated tags
	if (tag_first > 0 && tag_last < 0)
	{
		const auto v1 = m_pChartDataWnd->m_HZtags.GetValue(tag_first);
		MeasureBetweenHZ(ichan, line, v1, v1);
	}
}

// output results both into CEdit control (via pCopy) and within CListCtrl
void CMeasureResultsPage::MeasureBetweenHZ(const int channel, const int line, const int v1, const int v2)
{
	// get scale factor for channel and sampling rate
	m_mVperBin = m_pChartDataWnd->GetChanlistVoltsperDataBin(channel) * 1000.0f;

	auto column_1 = (m_col - 1) * m_nbdatacols + 1;		// output data into column icol
	auto item = m_listResults.GetItemCount();	// compute which line will receive data
	if (line >= item)
	{
		wsprintf(&m_szT[0], _T("%i"), item);	// if new line, print line nb
		m_listResults.InsertItem(item, m_szT);	// and insert a new line within table
	}
	else
		item = line;								// else set variable value

	// measure parameters / selected interval; save results within common vars
	//m_szT[0]='\t';								// prepare string for edit output
	CString cs_dummy;
	float	x_dummy;
	const CString cs_fmt(_T("\t%f"));

	// output data according to options : data value at limits
	if (m_pMO->bDatalimits)
	{
		x_dummy = static_cast<float>(v1) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, column_1, cs_dummy); column_1++;

		x_dummy = static_cast<float>(v2) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, column_1, cs_dummy); column_1++;
	}

	if (m_pMO->bDiffDatalimits)
	{
		x_dummy = static_cast<float>(v2 - v1) * m_mVperBin;
		cs_dummy.Format(cs_fmt, x_dummy);
		m_listResults.SetItemText(item, column_1, cs_dummy);
	}

	return;
}

void CMeasureResultsPage::MeasureFromRect(int ichan)
{
}

void CMeasureResultsPage::MeasureFromStim(int ichan)
{
}

/////////////////////////////////////////////////////////////////////////////
// CMeasureResultsPage message handlers

BOOL CMeasureResultsPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	MeasureParameters();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// measure parameters either from current file or from the entire series
BOOL CMeasureResultsPage::MeasureParameters()
{
	// compute file indexes
	const int current_file_index = m_pdbDoc->GetDB_CurrentRecordPosition();
	ASSERT(current_file_index >= 0);
	auto i_first = current_file_index;
	auto i_last = i_first;
	if (m_pMO->bAllFiles)
	{
		i_first = 0;
		i_last = m_pdbDoc->GetDB_NRecords() - 1;
	}

	// prepare listcontrol
//	while (m_listResults.DeleteColumn(0) != 0);
	m_listResults.DeleteAllItems();

	// prepare clipboard and Edit control (CEdit)
	if (OpenClipboard())
	{
		EmptyClipboard();		// prepare clipboard and copy text to buffer
		const DWORD dw_len = 32768;	// 32 Kb
		const auto h_copy = static_cast<HANDLE>(::GlobalAlloc(GHND, dw_len));
		if (h_copy == nullptr)
		{
			AfxMessageBox(_T("Memory low: unable to allocate memory"));
			return TRUE;
		}
		auto p_copy = static_cast<TCHAR*>(::GlobalLock(static_cast<HGLOBAL>(h_copy)));
		auto p_copy0 = p_copy;

		// export Ascii: begin //////////////////////////////////////////////
		BeginWaitCursor();
		OutputTitle();
		m_pdbDoc->SetDB_CurrentRecordPosition(i_first);
		const CString filename = m_pdbDoc->GetDB_CurrentDatFileName();
		auto* p_vd = new OPTIONS_VIEWDATA;
		ASSERT(p_vd != NULL);
		const CString cs_out = _T("**filename\tdate\ttime\tcomment\tchannel\r\n");

		for (auto i = i_first; i <= i_last; i++, m_pdbDoc->DBMoveNext())
		{
			// open data file
			m_pdbDoc->OpenCurrentDataFile();

			p_vd->bacqcomments = TRUE;		// global comment
			p_vd->bacqdate = TRUE;			// acquisition date
			p_vd->bacqtime = TRUE;			// acquisition time
			p_vd->bfilesize = FALSE;		// file size
			p_vd->bacqchcomment = FALSE;	// acq channel indiv comment
			p_vd->bacqchsetting = FALSE;	// acq chan indiv settings (gain, filter, etc)
			auto cs = cs_out;
			cs += m_pdbDoc->m_pDat->GetDataFileInfos(p_vd);
			p_copy += wsprintf(p_copy, _T("%s\r\n"), static_cast<LPCTSTR>(cs));

			// output title for this data set
			auto i_chan_0 = 0;
			auto i_chan_1 = m_pChartDataWnd->GetChanlistSize() - 1;
			if (!m_pMO->bAllChannels)
			{
				i_chan_0 = m_pMO->wSourceChan;
				i_chan_1 = i_chan_0;
			}
			if (i_chan_0 >= m_pChartDataWnd->GetChanlistSize())
				i_chan_0 = 0;
			if (i_chan_1 >= m_pChartDataWnd->GetChanlistSize())
				i_chan_1 = m_pChartDataWnd->GetChanlistSize() - 1;

			m_col = 1;
			for (auto i_chan = i_chan_0; i_chan <= i_chan_1; i_chan++)
			{
				// measure according to option
				switch (m_pMO->wOption)
				{
					// ......................  vertical tags
				case 0:	MeasureFromVTtags(i_chan); break;
					// ......................  horizontal cursors
				case 1:	MeasureFromHZcur(i_chan); break;
					// ......................  rectangle area
				case 2:	MeasureFromRect(i_chan); break;
					// ......................  detect stimulus and then measure
				case 3: MeasureFromStim(i_chan); break;
				default:
					break;
				}
				m_col++;
			}

			// transfer content of clistctrl to clipboard
			p_copy += wsprintf(p_copy, _T("%s"), (LPCTSTR)m_csTitle);
			const auto n_lines = m_listResults.GetItemCount();
			const auto n_columns = m_nbdatacols * (m_col - 1) + 1;
			for (auto item = 0; item < n_lines; item++)
			{
				for (auto column = 0; column < n_columns; column++)
				{
					auto cs_content = m_listResults.GetItemText(item, column);
					p_copy += wsprintf(p_copy, _T("%s\t"), static_cast<LPCTSTR>(cs_content));
				}
				p_copy--; *p_copy = '\r'; p_copy++;
				*p_copy = '\n'; p_copy++;
			}
			// if not, then next file will override the results computed before
		}

		*p_copy = 0;
		m_pdbDoc->SetDB_CurrentRecordPosition(current_file_index);
		//CString filename2 = m_pdbDoc->GetDB_CurrentDatFileName();
		m_pdbDoc->OpenCurrentDataFile();
		EndWaitCursor();     // it's done

		// export Ascii: end //////////////////////////////////////////////
		m_CEditResults.SetWindowText(p_copy0);
		::GlobalUnlock(static_cast<HGLOBAL>(h_copy));
		SetClipboardData(CF_TEXT, h_copy);
		CloseClipboard();		// close connect w.clipboard
		delete p_vd;				// delete temporary object
	}
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// remove tags on exit
void CMeasureResultsPage::OnOK()
{
	m_pChartDataWnd->Invalidate();
	CPropertyPage::OnOK();
}

// measure parameters each time this page is selected
BOOL CMeasureResultsPage::OnSetActive()
{
	MeasureParameters();
	return CPropertyPage::OnSetActive();
}