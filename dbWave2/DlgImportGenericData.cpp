#include "StdAfx.h"
#include "resource.h"
#include "DlgImportGenericData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgImportGenericData::DlgImportGenericData(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgImportGenericData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILELIST, m_filedroplist);
	DDX_Control(pDX, IDC_PRECISION, m_ComboPrecision);
	DDX_Text(pDX, IDC_CHANNELCOMMENT, m_adChannelComment);
	DDX_Text(pDX, IDC_CHANNELGAIN, m_adChannelGain);
	DDX_Text(pDX, IDC_CHANNELNO, m_adChannelChan);
	DDX_Text(pDX, IDC_NUMBEROFCHANNELS, m_nb_AD_channels);
	DDX_Text(pDX, IDC_NUMBEROFRUNS, m_nbRuns);
	DDX_Text(pDX, IDC_SAMPLINGRATE, m_samplingrate);
	DDX_Text(pDX, IDC_SKIPNBYTES, m_skipNbytes);
	DDX_Text(pDX, IDC_VMAX, m_voltageMax);
	DDX_Text(pDX, IDC_VMIN, m_voltageMin);
	DDX_Text(pDX, IDC_FILETITLE, m_csFileTitle);
	DDX_Check(pDX, IDC_CHECK1, m_bpreviewON);
}

BEGIN_MESSAGE_MAP(DlgImportGenericData, CDialog)
	ON_CBN_SELCHANGE(IDC_PRECISION, OnSelchangePrecision)
	ON_EN_CHANGE(IDC_NUMBEROFCHANNELS, OnEnChangeNumberofchannels)
	ON_BN_CLICKED(IDC_MULTIPLERUNS, OnMultipleruns)
	ON_BN_CLICKED(IDC_SINGLERUN, OnSinglerun)
	ON_EN_CHANGE(IDC_CHANNELNO, OnEnChangeChannelno)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, OnDeltaposSpinNbChannels)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpinNoChan)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_IMPORTALL, OnImportall)
	ON_BN_CLICKED(IDC_CHECK1, OnSetPreview)
	ON_CBN_SELCHANGE(IDC_FILELIST, OnSelchangeFilelist)
	ON_BN_CLICKED(IDC_OFFSETBINARY, OnOffsetbinary)
	ON_BN_CLICKED(IDC_TWOSCOMPLEMENT, OnTwoscomplement)
	ON_EN_CHANGE(IDC_SKIPNBYTES, OnEnChangeSkipnbytes)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN3, OnDeltaposSkipNbytes)
	ON_BN_CLICKED(IDC_CHECKSAPID, OnSapid3_5)
	ON_EN_CHANGE(IDC_CHANNELCOMMENT, OnEnChangeChannelcomment)
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
	VERIFY(mm_nb_AD_channels.SubclassDlgItem(IDC_NUMBEROFCHANNELS, this));
	VERIFY(mm_adChannelChan.SubclassDlgItem(IDC_CHANNELNO, this));
	VERIFY(mm_skipNbytes.SubclassDlgItem(IDC_SKIPNBYTES, this));
	VERIFY(m_ChartDataWnd.SubclassDlgItem(IDC_DISPLAYSOURCE, this));

	// inhibit buttons linked to conversion if requested
	int nIDFocus;
	if (!bConvert)
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
		int lastindex = m_pfilenameArray->GetSize();
		for (int i = 0; i < lastindex; i++)
		{
			CString filename = m_pfilenameArray->GetAt(i);
			int icount = filename.GetLength() - filename.ReverseFind('\\') - 1;
			m_filedroplist.AddString(filename.Right(icount));
			m_filedroplist.SetItemData(i, i);
		}
		m_filedroplist.SetCurSel(0); // select first item in the list
		nIDFocus = IDC_IMPORT;
		static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->SetCheck(piivO->is_sapid_3_5);
		if (!piivO->preview_requested)
			GetDlgItem(IDC_DISPLAYSOURCE)->ShowWindow(SW_HIDE);
	}

	// copy relevant data into controls
	m_adChannelChan = 1;
	UpdateControlsFromStruct();
	// convert selected file if options say so
	if (bConvert && piivO->preview_requested)
	{
		m_bpreviewON = piivO->preview_requested;
		// get options as set by caller
		static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(piivO->preview_requested);
		SetFileNames(0);
		UpdatePreview();
	}

	GetDlgItem(nIDFocus)->SetFocus();
	return FALSE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

// when user check "multiple runs", enable dependent controls: "nbruns" and
// corresponding edit control
// otherwise disable them

void DlgImportGenericData::EnableRunParameters()
{
	int IDC_button = IDC_SINGLERUN; // check button concerning structure of
	if (!piivO->is_single_run) // file; either a single acquisition bout
		IDC_button = IDC_MULTIPLERUNS; // or more than one
	CheckRadioButton(IDC_SINGLERUN, IDC_MULTIPLERUNS, IDC_button);
	GetDlgItem(IDC_NUMBEROFRUNS)->EnableWindow(!piivO->is_single_run); // edit window
	GetDlgItem(IDC_STATIC1)->EnableWindow(!piivO->is_single_run); // static
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(piivO->preview_requested);
	m_bChanged = TRUE;
}

// copy values from structure iivO into controls
// this routine is called when the dialog box is opened

void DlgImportGenericData::UpdateControlsFromStruct()
{
	// copy structure content into controls
	m_nbRuns = piivO->nb_runs; // load number of repetitions & enable corresp
	GetDlgItem(IDC_SAMPLINGRATE)->EnableWindow(!piivO->is_sapid_3_5);
	EnableRunParameters(); // enable dependent dlg items
	m_samplingrate = piivO->sampling_rate; // sampling rate per channel (in Herz)
	m_nb_AD_channels = piivO->nb_channels; // number of data acquisition channels
	piivO->p_wave_chan_array->chan_array_set_size(piivO->nb_channels);

	int IDC_button = IDC_OFFSETBINARY; // check button concerning data encoding mode
	if (piivO->encoding_mode > 0)
		IDC_button = IDC_TWOSCOMPLEMENT;
	CheckRadioButton(IDC_OFFSETBINARY, IDC_TWOSCOMPLEMENT, IDC_button);

	// set precision combox box
	int precisionindex = 1; // default = 1 (12 bits)
	if (piivO->bits_precision == 16) // 16 bits = 2nd position
		precisionindex = 2;
	else if (piivO->bits_precision == 8) // 8 bits = initial position
		precisionindex = 0;
	else if (piivO->bits_precision == 24) // 24 bits
		precisionindex = 3;
	else if (piivO->bits_precision == 32) // 32 bits
		precisionindex = 4;
	m_ComboPrecision.SetCurSel(precisionindex);

	m_voltageMax = piivO->voltage_max; // set voltage max
	m_voltageMin = piivO->voltage_min; // set voltage min
	m_skipNbytes = piivO->skip_n_bytes; // set "skip n bytes"
	m_csFileTitle = piivO->title; // file global comment
	CWaveChan* pChannel = piivO->p_wave_chan_array->get_p_channel(m_adChannelChan - 1);
	m_adChannelGain = static_cast<float>(pChannel->am_gaintotal); // set gain
	m_adChannelComment = pChannel->am_csComment; // and comment

	// set limits associated spin controls
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN2))->SetRange(1, 1024);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange(1, m_nb_AD_channels);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN3))->SetRange(1, 32767);

	UpdateData(FALSE); // refresh controls
	m_bChanged = FALSE;
}

// copy values from controls into structure iivO
// this routine is called on exit

void DlgImportGenericData::UpdateStructFromControls()
{
	UpdateData(TRUE);
	// copy controls into structure
	if (IsDlgButtonChecked(IDC_SINGLERUN))
		piivO->is_single_run = TRUE;
	else
	{
		piivO->is_single_run = FALSE;
		piivO->nb_runs = m_nbRuns;
	}
	piivO->sampling_rate = m_samplingrate; // sampling rate per channel (in Herz)
	piivO->nb_channels = m_nb_AD_channels; // number of data acquisition channels
	// encoding
	if (IsDlgButtonChecked(IDC_OFFSETBINARY))
		piivO->encoding_mode = OLx_ENC_BINARY;
	else
		piivO->encoding_mode = OLx_ENC_2SCOMP;

	// precision is updated when a change is made

	piivO->voltage_max = m_voltageMax; // set voltage max
	piivO->voltage_min = m_voltageMin; // set voltage min
	piivO->skip_n_bytes = m_skipNbytes; // set "skip n bytes"
	piivO->title = m_csFileTitle; // file global comment

	CWaveChan* pChannel = piivO->p_wave_chan_array->get_p_channel(m_adChannelChan - 1);
	pChannel->am_gaintotal = m_adChannelGain; // set gain
	pChannel->am_amplifiergain = pChannel->am_gaintotal;
	pChannel->am_gainAD = 1;
	pChannel->am_gainpre = 1;
	pChannel->am_gainpost = 1;
	pChannel->am_gainheadstage = 1;
	pChannel->am_csComment = m_adChannelComment; // and comment

	// adjust size of chan descriptors array
	piivO->p_wave_chan_array->chan_array_set_size(m_nb_AD_channels);

	m_bChanged = FALSE;
}

// exit dialog box
// save iivO parameters
// copy names of the files converted back into the name array

void DlgImportGenericData::OnOK()
{
	if (m_bChanged)
		UpdateStructFromControls(); // update structures with options

	// remove names that were not converted
	if (bConvert)
	{
		int lastindex = m_pfilenameArray->GetUpperBound();
		for (int i = lastindex; i >= 0; i--) // loop backwards
		{
			CString filename = m_pfilenameArray->GetAt(i);
			int icount = filename.Find(_T("AW_"));
			if (icount < 0) // delete unconverted file name
				m_pfilenameArray->RemoveAt(i);
		}
	}
	// base class routine...
	CDialog::OnOK();
}

// change parameter describing the precision of the data
// convert CCombobox selection into a number of bits, store result into iivO

void DlgImportGenericData::OnSelchangePrecision()
{
	UpdateData(TRUE); // load data from controls
	// set precision combox box
	int precisionindex = m_ComboPrecision.GetCurSel();
	int value; // convert corresponding precision  value
	switch (precisionindex)
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
	if (value != piivO->bits_precision)
	{
		piivO->bits_precision = value;
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

// On check : set flag single / multiple run

void DlgImportGenericData::OnMultipleruns()
{
	if (IsDlgButtonChecked(IDC_MULTIPLERUNS)
		&& piivO->is_single_run != FALSE)
	{
		piivO->is_single_run = FALSE;
		EnableRunParameters();
		// UpdatePreview();
	}
	m_bChanged = TRUE;
}

void DlgImportGenericData::OnSinglerun()
{
	if (IsDlgButtonChecked(IDC_SINGLERUN)
		&& piivO->is_single_run != TRUE)
	{
		piivO->is_single_run = TRUE;
		EnableRunParameters();
		// UpdatePreview();
	}
	m_bChanged = TRUE;
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

void DlgImportGenericData::OnEnChangeNumberofchannels()
{
	if (!mm_nb_AD_channels.m_bEntryDone)
		return;

	const UINT nb_AD_channels = m_nb_AD_channels;
	mm_nb_AD_channels.OnEnChange(this, m_nb_AD_channels, 1, -1);

	// modif limit max of chan parameters
	if (m_nb_AD_channels < 1) // check that there is at least one chan
		m_nb_AD_channels = 1; // then change the limit of the spin

	// action if value has changed
	if (m_nb_AD_channels != nb_AD_channels)
	{
		piivO->nb_channels = m_nb_AD_channels;
		piivO->p_wave_chan_array->chan_array_set_size(m_nb_AD_channels);
		static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange(1, m_nb_AD_channels);
		if (m_adChannelChan > m_nb_AD_channels) // and update dependent chan no
		{
			m_adChannelChan = m_nb_AD_channels;
			UpdateData(FALSE);
		}
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

// change channel number
//
// each data acquisition channel is associated with descriptors
// which can be modified by the user. This control allow the selection
// of one descriptor
// same comments as with OnEnChangeNumberofchannels()
// operations:
// - evaluate and clip irrelevant values
// - save current parameters
// - load parameters for new channel

void DlgImportGenericData::OnEnChangeChannelno()
{
	if (!mm_adChannelChan.m_bEntryDone)
		return;

	UINT previous_channel = m_adChannelChan;
	mm_adChannelChan.OnEnChange(this, m_adChannelChan, 1, -1);

	// check limits of m_adChannelChan
	if (m_adChannelChan < 1)
		m_adChannelChan = 1;
	if (m_adChannelChan > m_nb_AD_channels)
		m_adChannelChan = m_nb_AD_channels;
	if (m_adChannelChan != previous_channel)
	{
		// save previous data
		previous_channel = m_adChannelChan;
		UpdateData(TRUE); // load data from controls
		m_adChannelChan = previous_channel;
		CWaveChan* pChannel = piivO->p_wave_chan_array->get_p_channel(previous_channel - 1);
		pChannel->am_gaintotal = m_adChannelGain; // set gain
		pChannel->am_amplifiergain = pChannel->am_gaintotal;
		pChannel->am_csComment = m_adChannelComment; // and comment
		// point to new channel: add new descriptors if necessary
		piivO->p_wave_chan_array->chan_array_set_size(m_nb_AD_channels);

		// load data from new current channel
		pChannel = piivO->p_wave_chan_array->get_p_channel(m_adChannelChan - 1);
		m_adChannelGain = static_cast<float>(pChannel->am_gaintotal); // set gain
		m_adChannelComment = pChannel->am_csComment; // and comment
		UpdateData(FALSE);
	}

	m_bChanged = TRUE;
}

// change size of data header... ie offset from start of file to data

void DlgImportGenericData::OnEnChangeSkipnbytes()
{
	if (!mm_skipNbytes.m_bEntryDone)
		return;

	UINT skipNbytes = m_skipNbytes;
	mm_skipNbytes.OnEnChange(this, m_skipNbytes, 1, -1);

	// check limits of m_adChannelChan
	if (m_skipNbytes < 0)
		m_skipNbytes = 0;
	UpdateData(FALSE);

	if (m_skipNbytes != skipNbytes)
	{
		piivO->skip_n_bytes = m_skipNbytes;
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

// spin control associated with Edit control "nb of data channels"
// convert notification message into flags adapted to the CEditCtrl

void DlgImportGenericData::OnDeltaposSpinNbChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown->iDelta > 0)
		mm_nb_AD_channels.m_nChar = VK_UP;
	else
		mm_nb_AD_channels.m_nChar = VK_DOWN;
	mm_nb_AD_channels.m_bEntryDone = TRUE;
	*pResult = 0;
}

// spin control associated with channel no
// same remarks as above

void DlgImportGenericData::OnDeltaposSpinNoChan(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown->iDelta > 0)
		mm_adChannelChan.m_nChar = VK_UP;
	else
		mm_adChannelChan.m_nChar = VK_DOWN;
	mm_adChannelChan.m_bEntryDone = TRUE;
	*pResult = 0;
}

void DlgImportGenericData::OnDeltaposSkipNbytes(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown->iDelta > 0)
		mm_skipNbytes.m_nChar = VK_UP;
	else
		mm_skipNbytes.m_nChar = VK_DOWN;
	mm_skipNbytes.m_bEntryDone = TRUE;
	*pResult = 0;
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

void DlgImportGenericData::OnImport()
{
	if (!m_bimportall)
		UpdateStructFromControls();

	int cursel = m_filedroplist.GetCurSel();
	int index = m_filedroplist.GetItemData(cursel);
	SetFileNames(index);

	// convert file
	UpdateWaveDescriptors(&m_AcqDataFile);
	m_AcqDataFile.save_as(m_filedest);
	m_pfilenameArray->SetAt(index, m_filedest);

	// delete drop list item CComboBox
	m_filedroplist.DeleteString(cursel);
	if (m_filedroplist.GetCount() > 0)
	{
		m_filedroplist.SetCurSel(0);
		OnSelchangeFilelist();
	}
	else
		OnOK();
}

// import all files at once: loop through all members of the combo box

void DlgImportGenericData::OnImportall()
{
	m_bimportall = TRUE;
	UpdateStructFromControls();
	int nbitems = m_filedroplist.GetCount();
	for (int i = 0; i < nbitems; i++)
		OnImport();
	m_bimportall = FALSE;
}

// update preview file

void DlgImportGenericData::UpdatePreview()
{
	// convert data
	if (m_filesource != m_fileold)
	{
		m_AcqDataFile.open_acq_file(m_filesource);
		m_fileold = m_filesource;
	}
	m_AcqDataFile.set_reading_buffer_dirty(); // invalidate data buffer
	UpdateWaveDescriptors(&m_AcqDataFile);

	// display data
	if (!m_bpreviewON)
		return;

	int ndocchans = m_AcqDataFile.get_waveformat()->scan_count;
	m_ChartDataWnd.AttachDataFile(&m_AcqDataFile);
	CSize lsize = m_ChartDataWnd.get_rect_size();
	m_ChartDataWnd.resize_channels(lsize.cx, m_AcqDataFile.get_doc_channel_length());
	if (m_ChartDataWnd.get_channel_list_size() < ndocchans) // add envelopes if necessary
	{
		for (int jdocchan = 1; jdocchan < ndocchans; jdocchan++)
			m_ChartDataWnd.add_channel_list_item(jdocchan, 0);
	}
	// load data from document and adjust display
	m_ChartDataWnd.get_data_from_doc(0, m_AcqDataFile.get_doc_channel_length() - 1);
	for (int i = 0; i < ndocchans; i++)
	{
		int max, min;
		CChanlistItem* chan = m_ChartDataWnd.get_channel_list_item(i);
		chan->GetMaxMin(&max, &min);
		// split curves if requested by options
		int iextent = MulDiv(max - min + 1, 11, 10);
		int izero = (max + min) / 2;
		chan->SetYextent(iextent);
		chan->SetYzero(izero);
	}
	m_ChartDataWnd.Invalidate();
}

void DlgImportGenericData::OnSetPreview()
{
	// get latest from controls
	piivO->preview_requested = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	int n_cmd_show = SW_SHOW;
	if (!piivO->preview_requested)
		n_cmd_show = SW_HIDE;
	GetDlgItem(IDC_DISPLAYSOURCE)->ShowWindow(n_cmd_show);

	// if preview OFF: hide display source
	if (!piivO->preview_requested && m_bpreviewON)
	{
		m_bpreviewON = FALSE;
	}
	else if (piivO->preview_requested && !m_bpreviewON)
	{
		// build current file
		SetFileNames(m_filedroplist.GetItemData(m_filedroplist.GetCurSel()));
		m_bpreviewON = TRUE;
		UpdatePreview();
	}
}

void DlgImportGenericData::OnSelchangeFilelist()
{
	int cursel = m_filedroplist.GetCurSel();
	int index = m_filedroplist.GetItemData(cursel);
	CString filesource = m_pfilenameArray->GetAt(index);
	if (filesource.CompareNoCase(m_filesource) != 0)
	{
		SetFileNames(index);
		UpdatePreview();
	}
}

void DlgImportGenericData::OnOffsetbinary()
{
	UINT state = IsDlgButtonChecked(IDC_OFFSETBINARY);
	if (state == 1
		&& piivO->encoding_mode != OLx_ENC_BINARY)
	{
		piivO->encoding_mode = OLx_ENC_BINARY;
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

void DlgImportGenericData::OnTwoscomplement()
{
	UINT state = IsDlgButtonChecked(IDC_TWOSCOMPLEMENT);
	if (state == 1
		&& piivO->encoding_mode != OLx_ENC_2SCOMP)
	{
		piivO->encoding_mode = OLx_ENC_2SCOMP;
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

void DlgImportGenericData::SetFileNames(int index)
{
	m_filesource = m_pfilenameArray->GetAt(index);
	int icount = m_filesource.ReverseFind('\\') + 1;
	CString csPath = m_filesource.Left(icount);
	int icount2 = m_filesource.Find('.') - icount + 1;
	m_filedest = _T("AW_") + m_filesource.Mid(icount, icount2);
	m_filedest = csPath + m_filedest;
	m_filedest += _T("dat");
}

void DlgImportGenericData::OnSapid3_5()
{
	// get latest from controls
	if (piivO->is_sapid_3_5 != static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->GetCheck())
	{
		piivO->is_sapid_3_5 = static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->GetCheck();
		if (piivO->is_sapid_3_5)
		{
			piivO->is_single_run = TRUE;
			piivO->bits_precision = 12;
			piivO->skip_n_bytes = 7;
			piivO->nb_channels = 1;
			UpdateControlsFromStruct();
		}
		GetDlgItem(IDC_SAMPLINGRATE)->EnableWindow(!piivO->is_sapid_3_5);
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

// UpdateWaveDescriptors()
//
// Update parameters that describe the structure of the data within data file
// load parameters from user selected options

void DlgImportGenericData::UpdateWaveDescriptors(AcqDataDoc* pDataF)
{
	CFileStatus status;
	pDataF->m_pXFile->GetStatus(status);
	pDataF->set_offset_to_data(m_skipNbytes);

	pDataF->m_pWBuf->create_buffer_with_n_channels(piivO->nb_channels);
	CWaveFormat* pwF = pDataF->get_waveformat();

	// define parameters within CWaveFormat
	pwF->cs_comment = piivO->title;
	pwF->acqtime = status.m_ctime; // use CFile creation time
	pwF->fullscale_volts = piivO->voltage_max - piivO->voltage_min;
	pwF->mode_encoding = piivO->encoding_mode;
	long binspan = 4096;
	switch (piivO->bits_precision)
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
	pwF->binspan = binspan;
	pwF->binzero = 0;
	if (piivO->encoding_mode == 0) // OLx_ENC_BINARY
		pwF->binzero = binspan / 2;

	// copy ACQCHAN directly from iivO
	pDataF->get_wavechan_array()->Copy(piivO->p_wave_chan_array);

	// UNUSED PARAMETERS FROM iivO :
	//			BOOL	bSingleRun;
	//			short	nbRuns;

	// read data and copy into CDataFileAWAVE
	ULONGLONG lCompteur = status.m_size - piivO->skip_n_bytes; // get size of data
	if (piivO->is_sapid_3_5) // special case SAPID
	{
		lCompteur -= 2; // last word = sampling rate
		WORD wrate; // get rate from source file
		pDataF->m_pXFile->Seek(-2, CFile::end); // position and read
		pDataF->m_pXFile->Read(&wrate, sizeof(WORD));
		piivO->sampling_rate = wrate; // transfer rate
	}
	pwF->sampling_rate_per_channel = piivO->sampling_rate;
	pwF->sample_count = static_cast<long>(lCompteur / 2);
	m_AcqDataFile.read_data_infos();
}

//////////////////////////////////////////////////////////////////////////
// convert generic data file
// - open data file
// - create new CDataFileAWAVE object
// - set header parameters from iivO parameters
// - transfer data from source file into destination
// - close file and delete CDataFileAWAVE object

void DlgImportGenericData::OnEnChangeChannelcomment()
{
	m_bChanged = TRUE;
}
