#include "StdAfx.h"
#include "dbWave.h"
#include "dbWaveDoc.h"
#include "DlgExportData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define EXPORT_DBWAVE	0
#define EXPORT_SAPID	1
#define EXPORT_TEXT		2
#define EXPORT_EXCEL	3

DlgExportData::DlgExportData(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgExportData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EXPORTAS, m_ComboExportas);
	DDX_Control(pDX, IDC_SOURCEFILES, m_filedroplist);
	DDX_Text(pDX, IDC_TIMEFIRST, m_timefirst);
	DDX_Text(pDX, IDC_TIMELAST, m_timelast);
	DDX_Text(pDX, IDC_CHANNELNUMBER, m_channelnumber);
	DDX_Text(pDX, IDC_RATIO, m_iundersample);
	DDV_MinMaxInt(pDX, m_iundersample, 1, 20000);
}

BEGIN_MESSAGE_MAP(DlgExportData, CDialog)
	ON_BN_CLICKED(IDC_SINGLECHANNEL, OnSinglechannel)
	ON_BN_CLICKED(IDC_ALLCHANNELS, OnAllchannels)
	ON_BN_CLICKED(IDC_ENTIREFILE, OnEntirefile)
	ON_CBN_SELCHANGE(IDC_EXPORTAS, OnSelchangeExportas)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_EXPORTALL, OnExportall)
END_MESSAGE_MAP()

void DlgExportData::UpdateStructFromControls()
{
	UpdateData(TRUE);

	iivO.exportType = m_ComboExportas.GetCurSel();
	iivO.bAllchannels = static_cast<CButton*>(GetDlgItem(IDC_ALLCHANNELS))->GetCheck();
	iivO.bSeparateComments = static_cast<CButton*>(GetDlgItem(IDC_SAVECOMMENTS))->GetCheck();
	iivO.bentireFile = static_cast<CButton*>(GetDlgItem(IDC_ENTIREFILE))->GetCheck();
	iivO.bincludeTime = static_cast<CButton*>(GetDlgItem(IDC_BTIMESTEPS))->GetCheck();
	iivO.selectedChannel = m_channelnumber;
	iivO.fTimefirst = m_timefirst;
	iivO.fTimelast = m_timelast;
}

BOOL DlgExportData::DestroyWindow()
{
	// restore initial conditions
	if (m_icurrentfile >= 0)
	{
		try
		{
			m_dbDoc->DB_SetCurrentRecordPosition(m_icurrentfile);
			m_dbDoc->OpenCurrentDataFile();
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
	CdbWaveApp* p_app; // handle to the instance
	p_app = static_cast<CdbWaveApp*>(AfxGetApp()); // pointer to application
	iivO = p_app->options_import; // copy structure / options

	// update dependent controls
	m_ComboExportas.SetCurSel(iivO.exportType); // combo-box
	if (iivO.exportType != 0) // hide if not sapid
	{
		GetDlgItem(IDC_SAVECOMMENTS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BTIMESTEPS)->ShowWindow(SW_SHOW);
	}
	static_cast<CButton*>(GetDlgItem(IDC_SAVECOMMENTS))->SetCheck(iivO.bSeparateComments);
	static_cast<CButton*>(GetDlgItem(IDC_BTIMESTEPS))->SetCheck(iivO.bincludeTime);

	static_cast<CButton*>(GetDlgItem(IDC_ENTIREFILE))->SetCheck(iivO.bentireFile);
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(!iivO.bentireFile);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(!iivO.bentireFile);

	if (m_dbDoc->m_pDat->GetpVTtags()->GetNTags() < 1)
	{
		m_timefirst = iivO.fTimefirst;
		m_timelast = iivO.fTimelast;
	}
	else
	{
		AcqDataDoc* pDat = m_dbDoc->m_pDat;
		m_timefirst = static_cast<float>(pDat->GetpVTtags()->GetTagLVal(0));
		if (pDat->GetpVTtags()->GetNTags() > 1)
			m_timelast = static_cast<float>(pDat->GetpVTtags()->GetTagLVal(1));
		else
			m_timelast = static_cast<float>(pDat->GetDOCchanLength());
		float chrate = pDat->GetpWaveFormat()->sampling_rate_per_channel;
		m_timefirst /= chrate;
		m_timelast /= chrate;
	}

	static_cast<CButton*>(GetDlgItem(IDC_ALLCHANNELS))->SetCheck(iivO.bAllchannels);
	static_cast<CButton*>(GetDlgItem(IDC_SINGLECHANNEL))->SetCheck(!iivO.bAllchannels);

	GetDlgItem(IDC_CHANNELNUMBER)->EnableWindow(!iivO.bAllchannels);
	m_channelnumber = iivO.selectedChannel; //  one channel

	// get filename(s) and select first in the list
	m_icurrentfile = m_dbDoc->DB_GetCurrentRecordPosition();
	if (m_bAllFiles)
	{
		int nfiles = m_dbDoc->DB_GetNRecords();
		for (int i = 0; i < nfiles; i++)
		{
			m_dbDoc->DB_SetCurrentRecordPosition(i);
			CString filename = m_dbDoc->DB_GetCurrentDatFileName();
			int icount = filename.GetLength() - filename.ReverseFind('\\') - 1;
			m_filedroplist.AddString(filename.Right(icount));
			m_filedroplist.SetItemData(i, i);
		}
		m_dbDoc->DB_SetCurrentRecordPosition(m_icurrentfile);
		m_filedroplist.SetCurSel(m_icurrentfile);
	}
	else
	{
		CString filename = m_dbDoc->DB_GetCurrentDatFileName();
		int icount = filename.GetLength() - filename.ReverseFind('\\') - 1;
		int i = m_filedroplist.AddString(filename.Right(icount));
		m_filedroplist.SetItemData(i, m_icurrentfile);
		m_filedroplist.SetCurSel(0);
	}
	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgExportData::OnOK()
{
	UpdateStructFromControls();
	// copy data into awave structure
	CdbWaveApp* p_app; // handle to the instance
	p_app = static_cast<CdbWaveApp*>(AfxGetApp()); // pointer to application
	p_app->options_import = iivO; // copy structure / options

	CDialog::OnOK();
}

void DlgExportData::OnSinglechannel()
{
	iivO.bAllchannels = FALSE;
	GetDlgItem(IDC_CHANNELNUMBER)->EnableWindow(TRUE);
}

void DlgExportData::OnAllchannels()
{
	iivO.bAllchannels = FALSE;
	GetDlgItem(IDC_CHANNELNUMBER)->EnableWindow(FALSE);
}

void DlgExportData::OnEntirefile()
{
	iivO.bentireFile = !iivO.bentireFile;
	GetDlgItem(IDC_TIMEFIRST)->EnableWindow(!iivO.bentireFile);
	GetDlgItem(IDC_TIMELAST)->EnableWindow(!iivO.bentireFile);
}

void DlgExportData::OnSelchangeExportas()
{
	UpdateData(TRUE); // convert result
	iivO.exportType = m_ComboExportas.GetCurSel();
	int bShow = SW_HIDE;
	if (iivO.exportType == 0)
		bShow = SW_SHOW;
	GetDlgItem(IDC_SAVECOMMENTS)->ShowWindow(bShow);
	GetDlgItem(IDC_BTIMESTEPS)->ShowWindow(!bShow);
}

//////////////////////////////////////////////////////////////
// export files

void DlgExportData::OnExportall()
{
	int nbitems = m_filedroplist.GetCount();
	CWaitCursor wait;

	for (int i = 0; i < nbitems; i++)
	{
		m_filedroplist.SetCurSel(i);
		UpdateData(FALSE);
		m_filedroplist.UpdateWindow();
		OnExport();
	}

	CString csdummy = _T("Done: \nAll file were exported");
	MessageBox(csdummy, _T("Export file"));
}

void DlgExportData::OnExport()
{
	Export();
	/*CString csdummy = "Done: \nFile exported as \n";
	csdummy += m_filedest;
	MessageBoxA(csdummy,"Export file");*/
}

void DlgExportData::Export()
{
	UpdateStructFromControls();

	// extract source file name and build destination file name
	int cursel = m_filedroplist.GetCurSel(); // get file name index
	int index = m_filedroplist.GetItemData(cursel); // get multidoc corresp index

	m_dbDoc->DB_SetCurrentRecordPosition(index);
	m_dbDoc->OpenCurrentDataFile();
	m_filesource = m_dbDoc->DB_GetCurrentDatFileName(); // now, extract path to source file

	int icount1 = m_filesource.ReverseFind('\\') + 1; // and extract that chain of chars
	CString csPath = m_filesource.Left(icount1); // store it into csPath
	int icount2 = m_filesource.ReverseFind('.'); // extract file name
	m_filedest = m_filesource.Mid(icount1, icount2 - icount1); // without file extension

	CString csDescript; // find file prefix and extension
	m_ComboExportas.GetWindowText(csDescript); // within drop down list
	int index1 = csDescript.Find('(') + 1; // first, get prefix
	int index2 = csDescript.Find('*'); // that is between "(" and "*"
	CString csPrefix = csDescript.Mid(index1, index2 - index1);
	index1 = csDescript.ReverseFind('.'); // then get new file extension
	index2 = csDescript.ReverseFind(')');
	CString csExt = csDescript.Mid(index1, index2 - index1);

	m_filedest = csPrefix + m_filedest + csExt; // lastly build, the new name
	m_filedest = csPath + m_filedest; // and add path

	// compute some parameters
	m_pDat = m_dbDoc->m_pDat; // pointer to data document
	CWaveFormat* pwaveFormat = m_pDat->GetpWaveFormat();
	if (iivO.bentireFile) // if all data, load
	{
		// intervals from data file
		mm_timefirst = 0.f;
		mm_timelast = pwaveFormat->duration;
		mm_lFirst = 0;
		mm_lLast = m_pDat->GetDOCchanLength();
	}
	else // else, convert time into
	{
		// file indexes
		mm_timefirst = m_timefirst;
		mm_timelast = m_timelast;
		if (mm_timelast < mm_timefirst)
			mm_timelast = pwaveFormat->duration;
		mm_lFirst = static_cast<long>(mm_timefirst * pwaveFormat->sampling_rate_per_channel);
		mm_lLast = static_cast<long>(mm_timelast * pwaveFormat->sampling_rate_per_channel);
	}

	mm_firstchan = 0; // loop through all chans
	mm_lastchan = pwaveFormat->scan_count - 1; // or only one
	if (!iivO.bAllchannels) // depending on this flag
	{
		mm_firstchan = iivO.selectedChannel; // flag set: change limits
		mm_lastchan = mm_firstchan;
	}

	mm_binzero = 0;

	// now that filenames are built, export the files
	switch (m_ComboExportas.GetCurSel())
	{
	case 0:
		ExportDataAsdbWaveFile();
		break;
	case 1: // sapid file
		ExportDataAsSapidFile();
		break;
	case 2: // txt file
		ExportDataAsTextFile();
		break;
	case 3: // excel file
		ExportDataAsExcelFile();
		break;
	default:
		break;
	}

	// delete current file and select next (eventually; if not possible, exit)
	m_filedroplist.DeleteString(cursel); // remove file name
	if (m_filedroplist.GetCount() > 0)
		m_filedroplist.SetCurSel(0); // select next file
	else
		OnOK(); // exit dialog box
}

// export data into a text file

BOOL DlgExportData::ExportDataAsTextFile()
{
	// open destination file
	CStdioFile dataDest; // destination file object
	CFileException fe; // trap exceptions
	if (!dataDest.Open(m_filedest, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText,
	                   &fe))
	{
		dataDest.Abort(); // file not found
		return FALSE; // and return
	}

	///////////////////////////////////
	// convert
	CString csCharBuf;
	CString cs_dummy;

	// LINE 1.......... data file name
	// LINE 2.......... date
	// LINE 3.......... file comment

	csCharBuf.Format(_T("file :\t%s\r\n"), (LPCTSTR)m_filesource);
	CWaveChanArray* pChanArray = m_pDat->GetpWavechanArray();
	CWaveFormat* pwaveFormat = m_pDat->GetpWaveFormat();

	CString csdate = (pwaveFormat->acqtime).Format("%#d %B %Y %X");
	cs_dummy.Format(_T("date :\t%s\r\n"), (LPCTSTR)csdate);
	csCharBuf += cs_dummy;
	cs_dummy.Format(_T("comment :\t%s\r\n"), (LPCTSTR)pwaveFormat->GetComments(_T("\t")));
	csCharBuf += cs_dummy;
	dataDest.Write(csCharBuf, csCharBuf.GetLength() * sizeof(TCHAR)); // write data
	csCharBuf.Empty();

	// LINE 4.......... start (s)
	// LINE 5.......... end   (s)

	// first interval (sec)
	csCharBuf.Format(_T("start (s):\t%f\r\nend   (s):\t%f\r\n"), mm_timefirst, mm_timelast);
	dataDest.Write(csCharBuf, csCharBuf.GetLength() * sizeof(TCHAR)); // write data
	csCharBuf.Empty();

	// LINE 6.......... Sampling rate (Hz)
	// LINE 7.......... A/D channels:
	// LINE 8.......... mV per bin for each channel

	csCharBuf.Format(_T("Sampling rate (Hz):\t%f\r\n"), pwaveFormat->sampling_rate_per_channel);
	cs_dummy.Format(_T("A/D channels :\r\n")); // header for chans
	csCharBuf += cs_dummy;
	cs_dummy.Format(_T("mV per bin:")); // line title
	csCharBuf += cs_dummy;

	int i;
	for (i = mm_firstchan; i <= mm_lastchan; i++) // loop through all chans
	{
		float VoltsperBin; // declare float
		m_pDat->GetWBVoltsperBin(i, &VoltsperBin, 0); // get value
		cs_dummy.Format(_T("\t%f"), static_cast<double>(VoltsperBin) * 1000.f); // copy to buffer
		csCharBuf += cs_dummy;
	}

	// LINE 9.......... comment for each channel

	cs_dummy.Format(_T("\r\ncomments"));
	csCharBuf += cs_dummy;
	dataDest.Write(csCharBuf, csCharBuf.GetLength() * sizeof(TCHAR)); // write data
	csCharBuf.Empty();

	for (i = mm_firstchan; i <= mm_lastchan; i++)
	{
		cs_dummy.Format(_T("\t%s"), (LPCTSTR)pChanArray->Get_p_channel(i)->am_csComment);
		csCharBuf += cs_dummy;
	}

	cs_dummy.Format(_T("\r\n")); // add CRLF
	csCharBuf += cs_dummy;
	dataDest.Write(csCharBuf, csCharBuf.GetLength() * sizeof(TCHAR)); // write data
	csCharBuf.Empty();

	// export data only if text
	if (iivO.exportType == EXPORT_TEXT)
	{
		// LINE 10.......... header for each channel

		if (iivO.bincludeTime)
		{
			cs_dummy.Format(_T("time (s)\t"));
			csCharBuf += cs_dummy;
		}
		for (i = mm_firstchan; i <= mm_lastchan; i++)
		{
			cs_dummy.Format(_T("chan_%i\t"), i);
			csCharBuf += cs_dummy;
		}
		csCharBuf = csCharBuf.Left(csCharBuf.GetLength() - 1);
		cs_dummy.Format(_T("\r\n")); // add CRLF
		csCharBuf += cs_dummy;
		dataDest.Write(csCharBuf, csCharBuf.GetLength() * sizeof(TCHAR)); // write data
		csCharBuf.Empty();

		// LINE .......... values

		short value;
		m_pDat->BGetVal(0, -1); // force reading buffer anew
		// loop over all values
		for (int ii_time = mm_lFirst; ii_time < mm_lLast; ii_time++)
		{
			if (iivO.bincludeTime) // add time stamp
			{
				cs_dummy.Format(_T("%li"), ii_time);
				csCharBuf += cs_dummy;
			}
			for (i = mm_firstchan; i <= mm_lastchan; i++) // loop through all chans
			{
				// get value and convert into ascii
				value = m_pDat->BGetVal(i, ii_time) - mm_binzero;
				cs_dummy.Format(_T("\t%i"), value);
				csCharBuf += cs_dummy;
			}
			csCharBuf += cs_dummy;
			dataDest.Write(csCharBuf, csCharBuf.GetLength() * sizeof(TCHAR)); // write data
			csCharBuf.Empty();
		}
	}

	///////////// close file
	dataDest.Close(); // close file
	return TRUE;
}

BOOL DlgExportData::ExportDataAsSapidFile()
{
	// open destination file
	CFile dataDest; // destination file object
	CFileException fe; // trap exceptions
	if (!dataDest.Open(m_filedest, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeBinary,
	                   &fe))
	{
		dataDest.Abort(); // file not found
		return FALSE; // and return
	}

	char* pdummy[] = {"awave32"}; // dummy data
	dataDest.Write(pdummy, 7); // write dummy data

	short value;
	//int dummy = m_pDat->BGetVal(0, -1);		// init reading data
	for (int ii_time = mm_lFirst; ii_time < mm_lLast; ii_time++)
	{
		for (auto i = mm_firstchan; i <= mm_lastchan; i++) // loop through all chans
		{
			// get value and convert into ascii
			value = m_pDat->BGetVal(i, ii_time) - mm_binzero;
			dataDest.Write(&value, sizeof(short));
		}
	}
	// last value is sampling rate
	value = static_cast<short>(m_pDat->GetpWaveFormat()->sampling_rate_per_channel);
	dataDest.Write(&value, sizeof(short));
	dataDest.Flush();
	dataDest.Close(); // close file

	// export file description in a separate text file
	if (iivO.bSeparateComments)
	{
		const auto filedest = m_filedest;
		m_filedest = m_filedest.Left(m_filedest.GetLength() - 3) + _T("TXT");
		ExportDataAsTextFile();
		m_filedest = filedest;
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

BOOL DlgExportData::ExportDataAsExcelFile()
{
	// variables to receive data & or text

	double fdouble;
	auto row = 0;
	auto col = 0;

	// open data file and create BOF record
	CFile data_dest; // destination file object
	CFileException fe; // trap exceptions
	if (!data_dest.Open(m_filedest, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeBinary,
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
	data_dest.Write(&wi, sizeof(WORD)); // worksheet identifer

	// write dimension record of the worksheet
	const WORD lastrow = 16384;
	const WORD lastcol = 4096;

	wi = 0;
	data_dest.Write(&wi, sizeof(WORD));
	wi = 8;
	data_dest.Write(&wi, sizeof(WORD));
	wi = 0;
	data_dest.Write(&wi, sizeof(WORD)); // first row
	wi = lastrow;
	data_dest.Write(&wi, sizeof(WORD)); // last row
	wi = 0;
	data_dest.Write(&wi, sizeof(WORD)); // first column
	wi = lastcol;
	data_dest.Write(&wi, sizeof(WORD)); // last column

	// LINE 1.......... data file name
	// LINE 2.......... date
	// LINE 3.......... file comment

	// file name
	save_BIFF(&data_dest, BIFF_CHARS, row, col, "file");
	col++;
	saveCString_BIFF(&data_dest, row, col, m_filesource);
	col--;
	row++;
	// date
	const auto pwaveFormat = m_pDat->GetpWaveFormat();
	const auto pchanArray = m_pDat->GetpWavechanArray();

	auto date = (pwaveFormat->acqtime).Format(_T("%#d %B %Y %X")); //("%c");
	save_BIFF(&data_dest, BIFF_CHARS, row, col, "date");
	col++;
	saveCString_BIFF(&data_dest, row, col, date);
	col--;
	row++;
	// comments
	save_BIFF(&data_dest, BIFF_CHARS, row, col, "comments");
	col++;
	date = pwaveFormat->GetComments(_T("\t"));
	saveCString_BIFF(&data_dest, row, col, date);
	col--;
	row++;

	// LINE 4.......... start (s)
	// LINE 5.......... end   (s)

	save_BIFF(&data_dest, BIFF_CHARS, row, col, "start (s)");
	col++;
	fdouble = mm_timefirst;
	save_BIFF(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&fdouble));
	col--;
	row++;

	save_BIFF(&data_dest, BIFF_CHARS, row, col, "end (s)");
	col++;
	fdouble = mm_timelast;
	save_BIFF(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&fdouble));
	col--;
	row++;

	// LINE 6.......... Sampling rate (Hz)
	// LINE 7.......... A/D channels:
	// LINE 8.......... mV per bin for each channel

	save_BIFF(&data_dest, BIFF_CHARS, row, col, "Sampling rate (Hz)");
	col++;
	fdouble = pwaveFormat->sampling_rate_per_channel;
	save_BIFF(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&fdouble));
	col--;
	row++;

	save_BIFF(&data_dest, BIFF_CHARS, row, col, "A/D channels :");
	row++;
	save_BIFF(&data_dest, BIFF_CHARS, row, col, "mV per bin:");
	col++;

	for (auto i = mm_firstchan; i <= mm_lastchan; i++) // loop through all chans
	{
		float voltsper_bin; // declare float
		m_pDat->GetWBVoltsperBin(i, &voltsper_bin, 0); // get value
		fdouble = voltsper_bin;
		save_BIFF(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&fdouble));
		col++;
	}
	row++;
	col = 0;

	// LINE 9.......... comment for each channel
	save_BIFF(&data_dest, BIFF_CHARS, row, col, "comments");
	col++;
	for (int i = mm_firstchan; i <= mm_lastchan; i++)
	{
		CWaveChan* pchan = pchanArray->Get_p_channel(i);
		CString comment = pchan->am_csComment;
		saveCString_BIFF(&data_dest, row, col, comment);
		col++;
	}
	col = 0;
	row++;
	row++;

	// loop to write data: write columns header, split cols if too much data

	auto ncolsperbout = mm_lastchan - mm_firstchan + 1; // nb column within 1 bout
	if (iivO.bincludeTime) // one more if time
		ncolsperbout++;
	const auto maxrow = 16383; // last row MAX
	const auto doclength = mm_lLast - mm_lFirst + 1; // compute how many rows are necess
	const long boutlength = maxrow - row; // nb data per bout
	int nbouts = doclength / boutlength; // nb bouts
	if (boutlength * nbouts < doclength) // take into account uneven division
		nbouts++;
	if (nbouts * ncolsperbout > 256) // clip if too much
	{
		AfxMessageBox(_T("Too much data: export file clipped"));
		nbouts = 256 / ncolsperbout;
		mm_lLast = nbouts * boutlength;
	}
	auto j0 = 0;
	if (iivO.bincludeTime)
		j0++;
	for (auto i = 0; i < nbouts; i++)
	{
		CString iterat;
		iterat.Format(_T("[%i]"), i);
		CString comment;
		if (iivO.bincludeTime)
		{
			comment = _T("time") + iterat;
			saveCString_BIFF(&data_dest, row, col, comment);
			col++;
		}
		for (auto j = j0; j < ncolsperbout; j++)
		{
			comment.Format(_T("chan_%i"), j - j0);
			comment += iterat;
		}
		col = 0;
		row++;

		// iterate to read data and export to Excel file
		auto ii_time = 0;
		float voltsper_bin;
		const double rate = pwaveFormat->sampling_rate_per_channel;
		auto dummy = m_pDat->BGetVal(0, -1); // init reading data

		for (int k = mm_lFirst; k < boutlength; k++)
		{
			ii_time = k;
			saveCString_BIFF(&data_dest, row, col, comment);
			col++;
		}
		for (auto i = 0; i < nbouts; i++)
		{
			if (ii_time >= mm_lLast)
				continue;
			if (iivO.bincludeTime)
			{
				fdouble = ii_time / rate;
				save_BIFF(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&fdouble));
				col++;
			}

			for (auto j = j0; j < ncolsperbout; j++)
			{
				m_pDat->GetWBVoltsperBin(j - j0, &voltsper_bin, 0);
				fdouble = (static_cast<double>(m_pDat->BGetVal(j - j0, ii_time)) - static_cast<double>(mm_binzero)) *
					static_cast<double>(voltsper_bin);
				save_BIFF(&data_dest, BIFF_FLOAT, row, col, reinterpret_cast<char*>(&fdouble));
				col++;
			}
			ii_time += boutlength;
		}
		row++;
		col = 0;
	}
	// Write EOF record, and closes the file

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

void DlgExportData::save_BIFF(CFile* fp, int type, int row, int col, char* data)
{
	ATTR attribute = {0x000000};
	WORD w1, w2, wrow, wcol;
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
	wrow = row;
	fp->Write(&wrow, sizeof(WORD));
	wcol = col;
	fp->Write(&wcol, sizeof(WORD));
	fp->Write(&attribute, 3);

	// write data
	char ichar;
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
		ichar = static_cast<char>(strlen(data));
		fp->Write(&ichar, 1);
		fp->Write(data, ichar);
		break;
	case BIFF_BOOL:
		ichar = *data ? 1 : 0;
		fp->Write(&ichar, 1);
		ichar = 0;
		fp->Write(&ichar, 1);
		break;
	default: ;
	}
}

void DlgExportData::saveCString_BIFF(CFile* fp, int row, int col, CString& data)
{
	ATTR attribute = {0x000000};
	WORD wi, wrow, wcol;

	// write record header
	wi = 4;
	fp->Write(&wi, sizeof(WORD));
	wi = 8 + data.GetLength();
	fp->Write(&wi, sizeof(WORD));
	wrow = row;
	fp->Write(&wrow, sizeof(WORD));
	wcol = col;
	fp->Write(&wcol, sizeof(WORD));
	fp->Write(&attribute, 3);
	char ichar = data.GetLength();
	fp->Write(&ichar, 1);
	fp->Write(data, ichar);
}

BOOL DlgExportData::ExportDataAsdbWaveFile()
{
	// create new file
	auto pDatDest = new AcqDataDoc;
	pDatDest->CreateAcqFile(m_filedest);

	// load data header and save it into dest file
	const auto pw_f_dest = pDatDest->GetpWaveFormat();
	auto pw_c_dest = pDatDest->GetpWavechanArray();
	const auto pw_f_source = m_pDat->GetpWaveFormat();
	const auto pw_c_source = m_pDat->GetpWavechanArray();
	pw_f_dest->Copy(pw_f_source);
	pw_c_dest->Copy(pw_c_source);

	const auto nchans = mm_lastchan - mm_firstchan + 1;
	if (pw_f_dest->scan_count != nchans)
	{
		const auto lastchannel = pw_f_dest->scan_count - 1;
		for (auto i = lastchannel; i > 0; i--)
		{
			if (i > mm_lastchan || i < mm_firstchan)
				pw_c_dest->ChanArray_removeAt(i);
		}
		ASSERT(nchans == pw_c_dest->ChanArray_getSize());
		pw_f_dest->scan_count = nchans;
	}

#define LEN 16384
	auto* p_data = new short[LEN];
	ASSERT(p_data != NULL);
	auto pdat = p_data;

	auto datlen = 0;
	pw_f_dest->sample_count = 0;
	pDatDest->AcqDoc_DataAppendStart();
	auto dummy = m_pDat->BGetVal(0, -1); // init reading data

	for (int ii_time = mm_lFirst; ii_time < mm_lLast; ii_time++)
	{
		for (auto i = mm_firstchan; i <= mm_lastchan; i++) // loop through all chans
		{
			*pdat = m_pDat->BGetVal(i, ii_time) - mm_binzero;
			datlen++;
			pdat++;
			if (datlen >= LEN)
			{
				pDatDest->AcqDoc_DataAppend(p_data, datlen * 2);
				datlen = 0;
				pdat = p_data;
			}
		}
	}
	if (datlen > 0) // save incomplete buffer
		pDatDest->AcqDoc_DataAppend(p_data, datlen * 2);

	// stop appending data, update dependent struct
	pDatDest->AcqDoc_DataAppendStop();

	// delete pdataDest object created here
	pDatDest->AcqCloseFile();

	delete pDatDest;
	delete[] p_data;

	// export file description in a separate text file
	if (iivO.bSeparateComments)
	{
		const auto filedest = m_filedest;
		m_filedest = m_filedest.Left(m_filedest.GetLength() - 3) + _T("TXT");
		ExportDataAsTextFile();
		m_filedest = filedest;
	}

	return TRUE;
}
