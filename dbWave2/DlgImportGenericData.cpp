#include "StdAfx.h"
#include "resource.h"
#include "DlgImportGenericData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDlgImportGenericData::CDlgImportGenericData(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent), m_pfilenameArray(nullptr)
{
	m_adChannelComment = _T("");
	m_adChannelGain = 0.0f;
	m_adChannelChan = 0;
	m_nbADchannels = 0;
	m_nbRuns = 0;
	m_samplingrate = 0.0f;
	m_skipNbytes = 0;
	m_voltageMax = 0.0f;
	m_voltageMin = 0.0f;
	m_csFileTitle = _T("");
	m_bpreviewON = FALSE;
	bConvert = FALSE;
	m_bpreviewON = FALSE;
	m_fileold = _T("");
	m_bChanged = FALSE;
	m_bimportall = FALSE;
	piivO = nullptr;
}

void CDlgImportGenericData::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILELIST, m_filedroplist);
	DDX_Control(pDX, IDC_PRECISION, m_ComboPrecision);
	DDX_Text(pDX, IDC_CHANNELCOMMENT, m_adChannelComment);
	DDX_Text(pDX, IDC_CHANNELGAIN, m_adChannelGain);
	DDX_Text(pDX, IDC_CHANNELNO, m_adChannelChan);
	DDX_Text(pDX, IDC_NUMBEROFCHANNELS, m_nbADchannels);
	DDX_Text(pDX, IDC_NUMBEROFRUNS, m_nbRuns);
	DDX_Text(pDX, IDC_SAMPLINGRATE, m_samplingrate);
	DDX_Text(pDX, IDC_SKIPNBYTES, m_skipNbytes);
	DDX_Text(pDX, IDC_VMAX, m_voltageMax);
	DDX_Text(pDX, IDC_VMIN, m_voltageMin);
	DDX_Text(pDX, IDC_FILETITLE, m_csFileTitle);
	DDX_Check(pDX, IDC_CHECK1, m_bpreviewON);
}

BEGIN_MESSAGE_MAP(CDlgImportGenericData, CDialog)
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

BOOL CDlgImportGenericData::OnInitDialog()
{
	CDialog::OnInitDialog(); // normal initialization CSpinButtonCtrl

	// subclass edit control
	VERIFY(mm_nbADchannels.SubclassDlgItem(IDC_NUMBEROFCHANNELS, this));
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
		static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->SetCheck(piivO->bSapid3_5);
		if (!piivO->bPreview)
			GetDlgItem(IDC_DISPLAYSOURCE)->ShowWindow(SW_HIDE);
	}

	// copy relevant data into controls
	m_adChannelChan = 1;
	UpdateControlsFromStruct();
	// convert selected file if options say so
	if (bConvert && piivO->bPreview)
	{
		m_bpreviewON = piivO->bPreview;
		// get options as set by caller
		static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(piivO->bPreview);
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

void CDlgImportGenericData::EnableRunParameters()
{
	int IDC_button = IDC_SINGLERUN; // check button concerning structure of
	if (!piivO->bSingleRun) // file; either a single acquisition bout
		IDC_button = IDC_MULTIPLERUNS; // or more than one
	CheckRadioButton(IDC_SINGLERUN, IDC_MULTIPLERUNS, IDC_button);
	GetDlgItem(IDC_NUMBEROFRUNS)->EnableWindow(!piivO->bSingleRun); // edit window
	GetDlgItem(IDC_STATIC1)->EnableWindow(!piivO->bSingleRun); // static
	static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->SetCheck(piivO->bPreview);
	m_bChanged = TRUE;
}

// copy values from structure iivO into controls
// this routine is called when the dialog box is opened

void CDlgImportGenericData::UpdateControlsFromStruct()
{
	// copy structure content into controls
	m_nbRuns = piivO->nbRuns; // load number of repetitions & enable corresp
	GetDlgItem(IDC_SAMPLINGRATE)->EnableWindow(!piivO->bSapid3_5);
	EnableRunParameters(); // enable dependent dlg items
	m_samplingrate = piivO->samplingRate; // sampling rate per channel (in Herz)
	m_nbADchannels = piivO->nbChannels; // number of data acquisition channels
	piivO->pwave_chan_array->ChanArray_setSize(piivO->nbChannels);

	int IDC_button = IDC_OFFSETBINARY; // check button concerning data encoding mode
	if (piivO->encodingMode > 0)
		IDC_button = IDC_TWOSCOMPLEMENT;
	CheckRadioButton(IDC_OFFSETBINARY, IDC_TWOSCOMPLEMENT, IDC_button);

	// set precision combox box
	int precisionindex = 1; // default = 1 (12 bits)
	if (piivO->bitsPrecision == 16) // 16 bits = 2nd position
		precisionindex = 2;
	else if (piivO->bitsPrecision == 8) // 8 bits = initial position
		precisionindex = 0;
	else if (piivO->bitsPrecision == 24) // 24 bits
		precisionindex = 3;
	else if (piivO->bitsPrecision == 32) // 32 bits
		precisionindex = 4;
	m_ComboPrecision.SetCurSel(precisionindex);

	m_voltageMax = piivO->voltageMax; // set voltage max
	m_voltageMin = piivO->voltageMin; // set voltage min
	m_skipNbytes = piivO->skipNbytes; // set "skip n bytes"
	m_csFileTitle = piivO->title; // file global comment
	CWaveChan* pChannel = piivO->pwave_chan_array->Get_p_channel(m_adChannelChan - 1);
	m_adChannelGain = static_cast<float>(pChannel->am_gaintotal); // set gain
	m_adChannelComment = pChannel->am_csComment; // and comment

	// set limits associated spin controls
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN2))->SetRange(1, 1024);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange(1, m_nbADchannels);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN3))->SetRange(1, 32767);

	UpdateData(FALSE); // refresh controls
	m_bChanged = FALSE;
}

// copy values from controls into structure iivO
// this routine is called on exit

void CDlgImportGenericData::UpdateStructFromControls()
{
	UpdateData(TRUE);
	// copy controls into structure
	if (IsDlgButtonChecked(IDC_SINGLERUN))
		piivO->bSingleRun = TRUE;
	else
	{
		piivO->bSingleRun = FALSE;
		piivO->nbRuns = m_nbRuns;
	}
	piivO->samplingRate = m_samplingrate; // sampling rate per channel (in Herz)
	piivO->nbChannels = m_nbADchannels; // number of data acquisition channels
	// encoding
	if (IsDlgButtonChecked(IDC_OFFSETBINARY))
		piivO->encodingMode = OLx_ENC_BINARY;
	else
		piivO->encodingMode = OLx_ENC_2SCOMP;

	// precision is updated when a change is made

	piivO->voltageMax = m_voltageMax; // set voltage max
	piivO->voltageMin = m_voltageMin; // set voltage min
	piivO->skipNbytes = m_skipNbytes; // set "skip n bytes"
	piivO->title = m_csFileTitle; // file global comment

	CWaveChan* pChannel = piivO->pwave_chan_array->Get_p_channel(m_adChannelChan - 1);
	pChannel->am_gaintotal = m_adChannelGain; // set gain
	pChannel->am_gainamplifier = pChannel->am_gaintotal;
	pChannel->am_gainAD = 1;
	pChannel->am_gainpre = 1;
	pChannel->am_gainpost = 1;
	pChannel->am_gainheadstage = 1;
	pChannel->am_csComment = m_adChannelComment; // and comment

	// adjust size of chan descriptors array
	piivO->pwave_chan_array->ChanArray_setSize(m_nbADchannels);

	m_bChanged = FALSE;
}

// exit dialog box
// save iivO parameters
// copy names of the files converted back into the name array

void CDlgImportGenericData::OnOK()
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

void CDlgImportGenericData::OnSelchangePrecision()
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
	if (value != piivO->bitsPrecision)
	{
		piivO->bitsPrecision = value;
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

// On check : set flag single / multiple run

void CDlgImportGenericData::OnMultipleruns()
{
	if (IsDlgButtonChecked(IDC_MULTIPLERUNS)
		&& piivO->bSingleRun != FALSE)
	{
		piivO->bSingleRun = FALSE;
		EnableRunParameters();
		// UpdatePreview();
	}
	m_bChanged = TRUE;
}

void CDlgImportGenericData::OnSinglerun()
{
	if (IsDlgButtonChecked(IDC_SINGLERUN)
		&& piivO->bSingleRun != TRUE)
	{
		piivO->bSingleRun = TRUE;
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

void CDlgImportGenericData::OnEnChangeNumberofchannels()
{
	if (!mm_nbADchannels.m_bEntryDone)
		return;

	UINT nbADchannels = m_nbADchannels;
	switch (mm_nbADchannels.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE); // load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_nbADchannels++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_nbADchannels--;
		break;
	}

	mm_nbADchannels.m_bEntryDone = FALSE;
	mm_nbADchannels.m_nChar = 0;
	mm_nbADchannels.SetSel(0, -1); //select all text

	// modif limit max of chan parameters
	if (m_nbADchannels < 1) // check that there is at least one chan
		m_nbADchannels = 1; // then change the limit of the spin

	// action if value has changed
	if (m_nbADchannels != nbADchannels)
	{
		piivO->nbChannels = m_nbADchannels;
		piivO->pwave_chan_array->ChanArray_setSize(m_nbADchannels);
		static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_SPIN1))->SetRange(1, m_nbADchannels);
		if (m_adChannelChan > m_nbADchannels) // and update dependent chan no
		{
			m_adChannelChan = m_nbADchannels;
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

void CDlgImportGenericData::OnEnChangeChannelno()
{
	if (!mm_adChannelChan.m_bEntryDone)
		return;

	UINT previouschan = m_adChannelChan;
	switch (mm_adChannelChan.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE); // load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_adChannelChan++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_adChannelChan--;
		break;
	}

	// check limits of m_adChannelChan
	if (m_adChannelChan < 1)
		m_adChannelChan = 1;
	if (m_adChannelChan > m_nbADchannels)
		m_adChannelChan = m_nbADchannels;
	if (m_adChannelChan != previouschan)
	{
		// save previous data
		previouschan = m_adChannelChan;
		UpdateData(TRUE); // load data from controls
		m_adChannelChan = previouschan;
		CWaveChan* pChannel = piivO->pwave_chan_array->Get_p_channel(previouschan - 1);
		pChannel->am_gaintotal = m_adChannelGain; // set gain
		pChannel->am_gainamplifier = pChannel->am_gaintotal;
		pChannel->am_csComment = m_adChannelComment; // and comment
		// point to new channel: add new descriptors if necessary
		piivO->pwave_chan_array->ChanArray_setSize(m_nbADchannels);

		// load data from new current channel
		pChannel = piivO->pwave_chan_array->Get_p_channel(m_adChannelChan - 1);
		m_adChannelGain = static_cast<float>(pChannel->am_gaintotal); // set gain
		m_adChannelComment = pChannel->am_csComment; // and comment
		UpdateData(FALSE);
	}

	mm_adChannelChan.m_bEntryDone = FALSE;
	mm_adChannelChan.m_nChar = 0;
	mm_adChannelChan.SetSel(0, -1); //select all text
	m_bChanged = TRUE;
}

// change size of data header... ie offset from start of file to data

void CDlgImportGenericData::OnEnChangeSkipnbytes()
{
	if (!mm_skipNbytes.m_bEntryDone)
		return;

	UINT skipNbytes = m_skipNbytes;
	switch (mm_skipNbytes.m_nChar)
	{
	case VK_RETURN:
		UpdateData(TRUE); // load data from edit controls
		break;
	case VK_UP:
	case VK_PRIOR:
		m_skipNbytes++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		m_skipNbytes--;
		break;
	}

	// check limits of m_adChannelChan
	if (m_skipNbytes < 0)
		m_skipNbytes = 0;
	UpdateData(FALSE);

	if (m_skipNbytes != skipNbytes)
	{
		piivO->skipNbytes = m_skipNbytes;
		UpdatePreview();
	}
	mm_skipNbytes.m_bEntryDone = FALSE;
	mm_skipNbytes.m_nChar = 0;
	mm_skipNbytes.SetSel(0, -1); //select all text
	m_bChanged = TRUE;
}

// spin control associated with Edit control "nb of data channels"
// convert notification message into flags adapted to the CEditCtrl

void CDlgImportGenericData::OnDeltaposSpinNbChannels(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown->iDelta > 0)
		mm_nbADchannels.m_nChar = VK_UP;
	else
		mm_nbADchannels.m_nChar = VK_DOWN;
	mm_nbADchannels.m_bEntryDone = TRUE;
	*pResult = 0;
}

// spin control associated with channel no
// same remarks as above

void CDlgImportGenericData::OnDeltaposSpinNoChan(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	if (pNMUpDown->iDelta > 0)
		mm_adChannelChan.m_nChar = VK_UP;
	else
		mm_adChannelChan.m_nChar = VK_DOWN;
	mm_adChannelChan.m_bEntryDone = TRUE;
	*pResult = 0;
}

void CDlgImportGenericData::OnDeltaposSkipNbytes(NMHDR* pNMHDR, LRESULT* pResult)
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

void CDlgImportGenericData::OnImport()
{
	if (!m_bimportall)
		UpdateStructFromControls();

	int cursel = m_filedroplist.GetCurSel();
	int index = m_filedroplist.GetItemData(cursel);
	SetFileNames(index);

	// convert file
	UpdateWaveDescriptors(&m_AcqDataFile);
	m_AcqDataFile.SaveAs(m_filedest);
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

void CDlgImportGenericData::OnImportall()
{
	m_bimportall = TRUE;
	UpdateStructFromControls();
	int nbitems = m_filedroplist.GetCount();
	for (int i = 0; i < nbitems; i++)
		OnImport();
	m_bimportall = FALSE;
}

// update preview file

void CDlgImportGenericData::UpdatePreview()
{
	// convert data
	if (m_filesource != m_fileold)
	{
		m_AcqDataFile.openAcqFile(m_filesource);
		m_fileold = m_filesource;
	}
	m_AcqDataFile.SetReadingBufferDirty(); // invalidate data buffer
	UpdateWaveDescriptors(&m_AcqDataFile);

	// display data
	if (!m_bpreviewON)
		return;

	int ndocchans = m_AcqDataFile.GetpWaveFormat()->scan_count;
	m_ChartDataWnd.AttachDataFile(&m_AcqDataFile);
	CSize lsize = m_ChartDataWnd.GetRectSize();
	m_ChartDataWnd.ResizeChannels(lsize.cx, m_AcqDataFile.GetDOCchanLength());
	if (m_ChartDataWnd.GetChanlistSize() < ndocchans) // add envelopes if necessary
	{
		for (int jdocchan = 1; jdocchan < ndocchans; jdocchan++)
			m_ChartDataWnd.AddChanlistItem(jdocchan, 0);
	}
	// load data from document and adjust display
	m_ChartDataWnd.GetDataFromDoc(0, m_AcqDataFile.GetDOCchanLength() - 1);
	for (int i = 0; i < ndocchans; i++)
	{
		int max, min;
		CChanlistItem* chan = m_ChartDataWnd.GetChanlistItem(i);
		chan->GetMaxMin(&max, &min);
		// split curves if requested by options
		int iextent = MulDiv(max - min + 1, 11, 10);
		int izero = (max + min) / 2;
		chan->SetYextent(iextent);
		chan->SetYzero(izero);
	}
	m_ChartDataWnd.Invalidate();
}

void CDlgImportGenericData::OnSetPreview()
{
	// get latest from controls
	piivO->bPreview = static_cast<CButton*>(GetDlgItem(IDC_CHECK1))->GetCheck();
	int n_cmd_show = SW_SHOW;
	if (!piivO->bPreview)
		n_cmd_show = SW_HIDE;
	GetDlgItem(IDC_DISPLAYSOURCE)->ShowWindow(n_cmd_show);

	// if preview OFF: hide display source
	if (!piivO->bPreview && m_bpreviewON)
	{
		m_bpreviewON = FALSE;
	}
	else if (piivO->bPreview && !m_bpreviewON)
	{
		// build current file
		SetFileNames(m_filedroplist.GetItemData(m_filedroplist.GetCurSel()));
		m_bpreviewON = TRUE;
		UpdatePreview();
	}
}

void CDlgImportGenericData::OnSelchangeFilelist()
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

void CDlgImportGenericData::OnOffsetbinary()
{
	UINT state = IsDlgButtonChecked(IDC_OFFSETBINARY);
	if (state == 1
		&& piivO->encodingMode != OLx_ENC_BINARY)
	{
		piivO->encodingMode = OLx_ENC_BINARY;
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

void CDlgImportGenericData::OnTwoscomplement()
{
	UINT state = IsDlgButtonChecked(IDC_TWOSCOMPLEMENT);
	if (state == 1
		&& piivO->encodingMode != OLx_ENC_2SCOMP)
	{
		piivO->encodingMode = OLx_ENC_2SCOMP;
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

void CDlgImportGenericData::SetFileNames(int index)
{
	m_filesource = m_pfilenameArray->GetAt(index);
	int icount = m_filesource.ReverseFind('\\') + 1;
	CString csPath = m_filesource.Left(icount);
	int icount2 = m_filesource.Find('.') - icount + 1;
	m_filedest = _T("AW_") + m_filesource.Mid(icount, icount2);
	m_filedest = csPath + m_filedest;
	m_filedest += _T("dat");
}

void CDlgImportGenericData::OnSapid3_5()
{
	// get latest from controls
	if (piivO->bSapid3_5 != static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->GetCheck())
	{
		piivO->bSapid3_5 = static_cast<CButton*>(GetDlgItem(IDC_CHECKSAPID))->GetCheck();
		if (piivO->bSapid3_5)
		{
			piivO->bSingleRun = TRUE;
			piivO->bitsPrecision = 12;
			piivO->skipNbytes = 7;
			piivO->nbChannels = 1;
			UpdateControlsFromStruct();
		}
		GetDlgItem(IDC_SAMPLINGRATE)->EnableWindow(!piivO->bSapid3_5);
		UpdatePreview();
	}
	m_bChanged = TRUE;
}

// UpdateWaveDescriptors()
//
// Update parameters that describe the structure of the data within data file
// load parameters from user selected options

void CDlgImportGenericData::UpdateWaveDescriptors(AcqDataDoc* pDataF)
{
	CFileStatus status;
	pDataF->m_pXFile->GetStatus(status);
	pDataF->SetbOffsetToData(m_skipNbytes);

	pDataF->m_pWBuf->WBDatachanSetnum(piivO->nbChannels);
	CWaveFormat* pwF = pDataF->GetpWaveFormat();

	// define parameters within CWaveFormat
	pwF->cs_comment = piivO->title;
	pwF->acqtime = status.m_ctime; // use CFile creation time
	pwF->fullscale_volts = piivO->voltageMax - piivO->voltageMin;
	pwF->mode_encoding = piivO->encodingMode;
	long binspan = 4096;
	switch (piivO->bitsPrecision)
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
	if (piivO->encodingMode == 0) // OLx_ENC_BINARY
		pwF->binzero = binspan / 2;

	// copy ACQCHAN directly from iivO
	*(pDataF->GetpWavechanArray()) = *(piivO->pwave_chan_array);

	// UNUSED PARAMETERS FROM iivO :
	//			BOOL	bSingleRun;
	//			short	nbRuns;

	// read data and copy into CDataFileAWAVE
	ULONGLONG lCompteur = status.m_size - piivO->skipNbytes; // get size of data
	if (piivO->bSapid3_5) // special case SAPID
	{
		lCompteur -= 2; // last word = sampling rate
		WORD wrate; // get rate from source file
		pDataF->m_pXFile->Seek(-2, CFile::end); // position and read
		pDataF->m_pXFile->Read(&wrate, sizeof(WORD));
		piivO->samplingRate = wrate; // transfer rate
	}
	pwF->chrate = piivO->samplingRate;
	pwF->sample_count = static_cast<long>(lCompteur / 2);
	m_AcqDataFile.ReadDataInfos();
}

//////////////////////////////////////////////////////////////////////////
// convert generic data file
// - open data file
// - create new CDataFileAWAVE object
// - set header parameters from iivO parameters
// - transfer data from source file into destination
// - close file and delete CDataFileAWAVE object

void CDlgImportGenericData::OnEnChangeChannelcomment()
{
	m_bChanged = TRUE;
}
