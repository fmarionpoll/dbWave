#include "StdAfx.h"
#include "Editctrl.h"
#include "DlgADIntervals.h"

#include <Olxdadefs.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DlgADIntervals::DlgADIntervals(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgADIntervals::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_NBUFFERS, m_bufferNitems);
	DDX_Text(pDX, IDC_ADRATECHAN, m_adratechan);
	DDX_Text(pDX, IDC_ACQDURATION, m_duration_to_acquire);
	DDX_Text(pDX, IDC_SWEEPDURATION, m_sweepduration);
	DDX_Text(pDX, IDC_BUFFERSIZE, m_bufferWsize);
	DDX_Text(pDX, IDC_EDIT1, m_undersamplefactor);
	DDV_MinMaxUInt(pDX, m_undersamplefactor, 1, 20000);
	DDX_Check(pDX, IDC_CHECK1, m_baudiblesound);
	DDX_Text(pDX, IDC_THRESHOLDCHAN, m_threshchan);
	DDX_Text(pDX, IDC_THRESHOLDVAL, m_threshval);
}

BEGIN_MESSAGE_MAP(DlgADIntervals, CDialog)
	ON_BN_CLICKED(IDC_ADCHANNELS, OnAdchannels)
	ON_EN_CHANGE(IDC_ADRATECHAN, OnEnChangeAdratechan)
	ON_EN_CHANGE(IDC_SWEEPDURATION, OnEnChangeDuration)
	ON_EN_CHANGE(IDC_BUFFERSIZE, OnEnChangeBuffersize)
	ON_EN_CHANGE(IDC_NBUFFERS, OnEnChangeNbuffers)
	ON_EN_CHANGE(IDC_ACQDURATION, OnEnChangeAcqduration)
	ON_BN_CLICKED(IDC_TRIGSOFT, OnTrigthresholdOFF)
	ON_BN_CLICKED(IDC_TRIGSUPTHRESHOLD, OnTrigthresholdON)
	ON_BN_CLICKED(IDC_TRIGKEYBOARD, OnTrigthresholdOFF)
	ON_BN_CLICKED(IDC_TRIGEXTERNAL, OnTrigthresholdOFF)
	ON_BN_CLICKED(IDC_TRIGINFTHRESHOLD, OnTrigthresholdON)
END_MESSAGE_MAP()

void DlgADIntervals::OnAdchannels()
{
	m_postmessage = IDC_ADCHANNELS;
	OnOK();
}

void DlgADIntervals::OnOK()
{
	UpdateData(TRUE);
	m_acqdef.sampling_rate_per_channel = m_adratechan;
	m_acqdef.buffersize = WORD(m_bufferWsize * UINT(m_acqdef.scan_count));
	m_acqdef.bufferNitems = short(m_bufferNitems);
	m_acqdef.sample_count = static_cast<long>(m_duration_to_acquire * float(m_acqdef.scan_count) * m_adratechan);
	m_acqdef.bOnlineDisplay = static_cast<CButton*>(GetDlgItem(IDC_ONLINEDISPLAY))->GetCheck();
	m_acqdef.bADwritetofile = static_cast<CButton*>(GetDlgItem(IDC_WRITETODISK))->GetCheck();
	m_acqdef.data_flow = (GetCheckedRadioButton(IDC_CONTINUOUS, IDC_BURST) == IDC_CONTINUOUS) ? 0 : 1;
	m_acqdef.duration = m_duration_to_acquire;

	m_acqdef.trig_chan = short(m_threshchan);
	m_acqdef.trig_threshold = short(m_threshval);
	auto i_id = short(GetCheckedRadioButton(IDC_TRIGSOFT, IDC_TRIGINFTHRESHOLD));
	switch (i_id)
	{
	case IDC_TRIGSOFT: i_id = OLx_TRG_SOFT;
		break;
	case IDC_TRIGKEYBOARD: i_id = OLx_TRG_EXTRA + 1;
		break;
	case IDC_TRIGEXTERNAL: i_id = OLx_TRG_EXTERN;
		break;
	case IDC_TRIGSUPTHRESHOLD: i_id = OLx_TRG_THRESHPOS;
		break;
	case IDC_TRIGINFTHRESHOLD: i_id = OLx_TRG_THRESHNEG;
		break;
	default: i_id = OLx_TRG_SOFT;
		break;
	}
	m_acqdef.trig_mode = i_id;

	m_pwaveFormat->Copy(&m_acqdef);
	CDialog::OnOK();
}

BOOL DlgADIntervals::OnInitDialog()
{
	CDialog::OnInitDialog();

	// display/hide chain dialog buttons
	auto show_window = SW_HIDE;
	if (m_bchainDialog)
		show_window = SW_SHOW;
	GetDlgItem(IDC_ADCHANNELS)->ShowWindow(show_window);
	GetDlgItem(IDC_ADINTERVALS)->ShowWindow(show_window);

	// subclass edit controls
	VERIFY(mm_adratechan.SubclassDlgItem(IDC_ADRATECHAN, this));
	VERIFY(mm_sweepduration.SubclassDlgItem(IDC_SWEEPDURATION, this));
	VERIFY(mm_acqduration.SubclassDlgItem(IDC_ACQDURATION, this));
	VERIFY(mm_bufferWsize.SubclassDlgItem(IDC_BUFFERSIZE, this));
	VERIFY(mm_bufferNitems.SubclassDlgItem(IDC_NBUFFERS, this));

	// load data from document
	m_acqdef.Copy(m_pwaveFormat);
	m_adratechan = m_acqdef.sampling_rate_per_channel;
	m_bufferNitems = m_acqdef.bufferNitems;
	m_bufferWsize = static_cast<UINT>(m_sweepduration * m_adratechan / float(m_bufferNitems));

	// init parameters manually if there is no driver
	if (0.0f == m_ratemin) m_ratemin = 0.1f;
	if (0.0f == m_ratemax) m_ratemax = 50000.f;

	static_cast<CButton*>(GetDlgItem(IDC_ONLINEDISPLAY))->SetCheck(m_acqdef.bOnlineDisplay);
	static_cast<CButton*>(GetDlgItem(IDC_WRITETODISK))->SetCheck(m_acqdef.bADwritetofile);
	auto i_id = (m_acqdef.data_flow == 0) ? IDC_CONTINUOUS : IDC_BURST;
	CheckRadioButton(IDC_CONTINUOUS, IDC_BURST, i_id);

	// trigger section
	auto flag = FALSE;
	switch (m_acqdef.trig_mode)
	{
	case OLx_TRG_SOFT: i_id = IDC_TRIGSOFT;
		break;
	case OLx_TRG_EXTRA + 1: i_id = IDC_TRIGKEYBOARD;
		break;
	case OLx_TRG_EXTERN: i_id = IDC_TRIGEXTERNAL;
		break;
	case OLx_TRG_THRESHPOS: i_id = IDC_TRIGSUPTHRESHOLD;
		flag = TRUE;
		break;
	case OLx_TRG_THRESHNEG: i_id = IDC_TRIGINFTHRESHOLD;
		flag = TRUE;
		break;
	default: i_id = IDC_TRIGSOFT;
		break;
	}
	CheckRadioButton(IDC_TRIGSOFT, IDC_TRIGINFTHRESHOLD, i_id);
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(flag);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(flag);
	m_threshchan = m_acqdef.trig_chan;
	if (m_threshchan < 0 || m_threshchan > 7)
		m_threshchan = 0;
	m_threshval = m_acqdef.trig_threshold;
	if (m_threshval < 0 || m_threshval > 4095)
		m_threshval = 2048;

	UpdateData(FALSE);
	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgADIntervals::OnEnChangeAdratechan()
{
	if (mm_adratechan.m_bEntryDone)
	{
		switch (mm_adratechan.m_nChar)
		{
		case VK_UP:
		case VK_PRIOR: m_adratechan++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_adratechan--;
			break;
		case VK_RETURN: UpdateData(TRUE);
			break;
		default: break;
		}
		mm_adratechan.m_bEntryDone = FALSE; // clear flag
		mm_adratechan.m_nChar = 0; // empty buffer
		mm_adratechan.SetSel(0, -1); // select all text

		// check value and modifies dependent parameters
		if (m_adratechan < m_ratemin)
		{
			m_adratechan = m_ratemin;
			MessageBeep(MB_ICONEXCLAMATION);
		}
		if (m_adratechan > m_ratemax)
		{
			m_adratechan = m_ratemax;
			MessageBeep(MB_ICONEXCLAMATION);
		}
		m_sweepduration = float(m_bufferWsize) * float(m_bufferNitems) / m_adratechan;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::OnEnChangeDuration()
{
	if (mm_sweepduration.m_bEntryDone)
	{
		switch (mm_sweepduration.m_nChar)
		{
			// load data from edit controls
		case VK_UP:
		case VK_PRIOR: m_sweepduration++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_sweepduration--;
			break;
		case VK_RETURN: UpdateData(TRUE);
			break;
		default: break;
		}
		mm_sweepduration.m_bEntryDone = FALSE; // clear flag
		mm_sweepduration.m_nChar = 0; // empty buffer
		mm_sweepduration.SetSel(0, -1); // select all text

		// check value and modifies dependent parameters
		// leave adratechan constant, modifies m_bufferWsize & m_bufferNitems ...?
		// first: try to adjust buffersize
		const auto l_total_points = static_cast<long>(m_sweepduration * m_adratechan);
		long n_buffers = m_bufferNitems;
		auto l_w_size = l_total_points / n_buffers;
		const auto l_w_size_min = static_cast<long>(m_adratechan * 0.05f); // minimum buffer size = 50 ms (!?)
		// corresponding buffer size is too much: add one buffer and decrease buffer size
		if (l_w_size > static_cast<long>(m_bufferWsizemax))
		{
			n_buffers++;
			l_w_size = l_total_points / n_buffers;
		}
		// corresponding buffer size is too small: remove one buffer (if possible) and increase buffer size
		else if (l_w_size < l_w_size_min)
		{
			if (n_buffers == 1)
			{
				AfxMessageBox(IDS_ACQDATA_SWEEPDURATIONLOW, NULL, MB_ICONEXCLAMATION | MB_OK);
			}
			else
			{
				n_buffers--;
				l_w_size = l_total_points / n_buffers;
			}
		}
		m_bufferWsize = static_cast<WORD>(l_w_size);
		m_bufferNitems = static_cast<int>(n_buffers);
		m_sweepduration = float(l_w_size) * float(n_buffers) / m_adratechan;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::OnEnChangeBuffersize()
{
	if (mm_bufferWsize.m_bEntryDone)
	{
		switch (mm_bufferWsize.m_nChar)
		{
			// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_bufferWsize++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_bufferWsize--;
			break;
		default: break;
		}
		mm_bufferWsize.m_bEntryDone = FALSE; // clear flag
		mm_bufferWsize.m_nChar = 0; // empty buffer
		mm_bufferWsize.SetSel(0, -1); // select all text

		// check value and update dependent parameter
		const auto ui_w_size_min = static_cast<WORD>(m_adratechan * 0.05f); // minimum buffer size = 50 ms (!?)
		if (m_bufferWsize > m_bufferWsizemax)
			m_bufferWsize = m_bufferWsizemax;
		else if (m_bufferWsize < ui_w_size_min)
			m_bufferWsize = ui_w_size_min;
		m_bufferWsize = (m_bufferWsize / m_undersamplefactor) * m_undersamplefactor;
		if (m_bufferWsize == 0)
			m_bufferWsize = m_undersamplefactor;
		m_sweepduration = float(m_bufferWsize * m_bufferNitems) / m_adratechan;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::OnEnChangeNbuffers()
{
	if (mm_bufferNitems.m_bEntryDone)
	{
		switch (mm_bufferNitems.m_nChar)
		{
			// load data from edit controls
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_bufferNitems++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_bufferNitems--;
			break;
		default: break;
		}
		mm_bufferNitems.m_bEntryDone = FALSE; // clear flag
		mm_bufferNitems.m_nChar = 0; // empty buffer
		mm_bufferNitems.SetSel(0, -1); // select all text
		// update dependent parameters
		if (m_bufferNitems < 1)
			m_bufferNitems = 1;
		m_sweepduration = float(m_bufferWsize * m_bufferNitems) / m_adratechan;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::OnEnChangeAcqduration()
{
	if (mm_acqduration.m_bEntryDone)
	{
		switch (mm_acqduration.m_nChar)
		{
		case VK_RETURN: UpdateData(TRUE);
			break;
		case VK_UP:
		case VK_PRIOR: m_duration_to_acquire++;
			break;
		case VK_DOWN:
		case VK_NEXT: m_duration_to_acquire--;
			break;
		default: break;
		}
		mm_acqduration.m_bEntryDone = FALSE; 
		mm_acqduration.m_nChar = 0; 
		mm_acqduration.SetSel(0, -1);
		const auto min_duration = float(m_bufferWsize) / m_adratechan;
		if (m_duration_to_acquire < min_duration)
			m_duration_to_acquire = min_duration;
		UpdateData(FALSE);
	}
}

void DlgADIntervals::OnTrigthresholdOFF()
{
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(FALSE);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(FALSE);
}

void DlgADIntervals::OnTrigthresholdON()
{
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(TRUE);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(TRUE);
}
