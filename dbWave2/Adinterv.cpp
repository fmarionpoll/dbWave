// adinterv.cpp : implementation file
//

#include "stdafx.h"
#include "editctrl.h"
#include "dataheader_Atlab.h"
#include "adinterv.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// ADIntervalsDlg dialog


ADIntervalsDlg::ADIntervalsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(ADIntervalsDlg::IDD, pParent)
{
	m_bufferNitems = 0;
	m_adratechan = 0.0f;
	m_acqduration = 0.0f;
	m_sweepduration = 1.0f;
	m_bufferWsize = 0;
	m_undersamplefactor = 1;
	m_baudiblesound = FALSE;
	m_threshchan = 0;
	m_threshval = 0;
	m_ratemin=0.f;
	m_ratemax=0.f;
	m_bchainDialog = FALSE;
	m_pwaveFormat = nullptr;
	m_postmessage = 0;
	m_bufferWsizemax = 0;
}

void ADIntervalsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_NBUFFERS, m_bufferNitems);
	DDX_Text(pDX, IDC_ADRATECHAN, m_adratechan);
	DDX_Text(pDX, IDC_ACQDURATION, m_acqduration);
	DDX_Text(pDX, IDC_SWEEPDURATION, m_sweepduration);
	DDX_Text(pDX, IDC_BUFFERSIZE, m_bufferWsize);
	DDX_Text(pDX, IDC_EDIT1, m_undersamplefactor);
	DDV_MinMaxUInt(pDX, m_undersamplefactor, 1, 20000);
	DDX_Check(pDX, IDC_CHECK1, m_baudiblesound);
	DDX_Text(pDX, IDC_THRESHOLDCHAN, m_threshchan);
	DDX_Text(pDX, IDC_THRESHOLDVAL, m_threshval);
}

BEGIN_MESSAGE_MAP(ADIntervalsDlg, CDialog)
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


/////////////////////////////////////////////////////////////////////////////
// ADIntervalsDlg message handlers

void ADIntervalsDlg::OnAdchannels()
{
	m_postmessage = IDC_ADCHANNELS;
	OnOK();
}

void ADIntervalsDlg::OnOK()
{	
	// save parameters into CWaveFormat structure  
	UpdateData(TRUE);
	m_acqdef.chrate = m_adratechan;
	m_acqdef.buffersize = m_bufferWsize * m_acqdef.scan_count;
	m_acqdef.bufferNitems = m_bufferNitems;
	m_acqdef.sample_count = (long) (m_acqduration*m_acqdef.scan_count*m_adratechan);
	m_acqdef.bOnlineDisplay = ((CButton*) GetDlgItem(IDC_ONLINEDISPLAY))->GetCheck();
	m_acqdef.bADwritetofile= ((CButton*) GetDlgItem(IDC_WRITETODISK))->GetCheck();
	m_acqdef.data_flow = (GetCheckedRadioButton(IDC_CONTINUOUS, IDC_BURST)==IDC_CONTINUOUS)? 0: 1;
	m_acqdef.duration = m_acqduration;

	m_acqdef.trig_chan = m_threshchan;
	m_acqdef.trig_threshold= m_threshval;
	int iID = GetCheckedRadioButton(IDC_TRIGSOFT, IDC_TRIGINFTHRESHOLD);
	switch (iID)
	{
		case IDC_TRIGSOFT:			iID = OLx_TRG_SOFT; break;
		case IDC_TRIGKEYBOARD:		iID = OLx_TRG_EXTRA+1; break;
		case IDC_TRIGEXTERNAL:		iID = OLx_TRG_EXTERN; break;
		case IDC_TRIGSUPTHRESHOLD:	iID = OLx_TRG_THRESHPOS; break;
		case IDC_TRIGINFTHRESHOLD:	iID = OLx_TRG_THRESHNEG; break;
		default:					iID = OLx_TRG_SOFT; break;
	}
	m_acqdef.trig_mode = iID;

	// copy data to data acq def structure    
	*m_pwaveFormat = m_acqdef;
	CDialog::OnOK();
}

BOOL ADIntervalsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// display/hide chain dialog buttons
	int showWindow = SW_HIDE;
	if (m_bchainDialog)
		showWindow = SW_SHOW;
	GetDlgItem(IDC_ADCHANNELS)->ShowWindow(showWindow);
	GetDlgItem(IDC_ADINTERVALS)->ShowWindow(showWindow);

	// subclass edit controls	
	VERIFY(mm_adratechan.SubclassDlgItem(IDC_ADRATECHAN, this));
	VERIFY(mm_sweepduration.SubclassDlgItem(IDC_SWEEPDURATION, this));
	VERIFY(mm_acqduration.SubclassDlgItem(IDC_ACQDURATION, this));
	VERIFY(mm_bufferWsize.SubclassDlgItem(IDC_BUFFERSIZE, this));
	VERIFY(mm_bufferNitems.SubclassDlgItem(IDC_NBUFFERS, this));	

	// load data from document
	m_acqdef = *m_pwaveFormat;									// load parameters

	m_adratechan = m_acqdef.chrate;								// sampling rate (per chan)
	m_bufferNitems = m_acqdef.bufferNitems;
	m_bufferWsize = (UINT) (m_sweepduration*m_adratechan/m_bufferNitems) ;

	if (m_acqdef.sample_count == 0)
		m_acqdef.sample_count = m_bufferWsize*m_bufferNitems*m_acqdef.bufferNitems;
	m_acqduration= m_acqdef.sample_count / m_adratechan / m_acqdef.scan_count;
	
	// init parameters manually if there is no driver
	if (0.0f == m_ratemin) m_ratemin=0.1f;
	if (0.0f == m_ratemax) m_ratemax=50000.f;

	static_cast<CButton*>(GetDlgItem(IDC_ONLINEDISPLAY))->SetCheck(m_acqdef.bOnlineDisplay);
	static_cast<CButton*>(GetDlgItem(IDC_WRITETODISK))->SetCheck(m_acqdef.bADwritetofile);
	auto i_id = (m_acqdef.data_flow == 0)? IDC_CONTINUOUS: IDC_BURST;
	CheckRadioButton(IDC_CONTINUOUS, IDC_BURST, i_id);

	// trigger section
	BOOL flag = FALSE;
	switch (m_acqdef.trig_mode)
	{
		case OLx_TRG_SOFT:		i_id = IDC_TRIGSOFT; break;
		case OLx_TRG_EXTRA+1:	i_id = IDC_TRIGKEYBOARD; break;
		case OLx_TRG_EXTERN:	i_id = IDC_TRIGEXTERNAL; break;
		case OLx_TRG_THRESHPOS:	i_id = IDC_TRIGSUPTHRESHOLD; flag = TRUE; break;
		case OLx_TRG_THRESHNEG:	i_id = IDC_TRIGINFTHRESHOLD; flag = TRUE; break;
		default:				i_id = IDC_TRIGSOFT; break;
	}
	CheckRadioButton(IDC_TRIGSOFT, IDC_TRIGINFTHRESHOLD, i_id);
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(flag);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(flag);
	m_threshchan = m_acqdef.trig_chan;
	if (m_threshchan<0 || m_threshchan>7)
		m_threshchan = 0;
	m_threshval = m_acqdef.trig_threshold;
	if (m_threshval<0 || m_threshval>4095)
		m_threshval=2048;

	UpdateData(FALSE);	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// ------------------------------------------------------------------------
void ADIntervalsDlg::OnEnChangeAdratechan()
{
	if (!mm_adratechan.m_bEntryDone)
		return;

	switch (mm_adratechan.m_nChar)
	{
		case VK_UP:
		case VK_PRIOR:	m_adratechan++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_adratechan--;	break;
		case VK_RETURN: UpdateData(TRUE); break;
		default: break;
	}
	mm_adratechan.m_bEntryDone=FALSE;	// clear flag
	mm_adratechan.m_nChar=0;			// empty buffer
	mm_adratechan.SetSel(0, -1);		// select all text

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
	m_sweepduration = m_bufferWsize * m_bufferNitems / m_adratechan;
	UpdateData(FALSE);	
}


// ------------------------------------------------------------------
void ADIntervalsDlg::OnEnChangeDuration()
{
	if (!mm_sweepduration.m_bEntryDone)
		return;

	switch (mm_sweepduration.m_nChar)
	{				// load data from edit controls
		case VK_UP:
		case VK_PRIOR:	m_sweepduration++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_sweepduration--;	break;
		case VK_RETURN:	UpdateData(TRUE);	break;
		default: break;

	}
	mm_sweepduration.m_bEntryDone=FALSE;	// clear flag
	mm_sweepduration.m_nChar=0;			// empty buffer
	mm_sweepduration.SetSel(0, -1);		// select all text

	// check value and modifies dependent parameters
	// leave adratechan constant, modifies m_bufferWsize & m_bufferNitems ...?
	// first: try to adjust buffersize
	long ltotalpoints = (long) (m_sweepduration * m_adratechan);
	long lnbuffers = m_bufferNitems;
	long lWsize = ltotalpoints / lnbuffers;
	long lWsizemin = (long)(m_adratechan * 0.05); // minimum buffer size = 50 ms (!?)
	// corresponding buffer size is too much: add one buffer and decrease buffer size
	if (lWsize > (long) m_bufferWsizemax)
	{
		lnbuffers++;
		lWsize = ltotalpoints / lnbuffers;
	}
	// corresponding buffer size is too small: remove one buffer (if possible) and increase buffer size
	else if (lWsize < lWsizemin)
	{
		if (lnbuffers == 1)
		{
			AfxMessageBox(IDS_ACQDATA_SWEEPDURATIONLOW, NULL, MB_ICONEXCLAMATION | MB_OK);			
		}
		else
		{
			lnbuffers--;
			lWsize = ltotalpoints / lnbuffers;
		}			
	}
	m_bufferWsize = (WORD) lWsize;
	m_bufferNitems = (int) lnbuffers;
	m_sweepduration = lWsize * lnbuffers / m_adratechan;
	UpdateData(FALSE);	
}


// ------------------------------------------------------------------
void ADIntervalsDlg::OnEnChangeBuffersize()
{
	if (!mm_bufferWsize.m_bEntryDone)
		return;

	switch (mm_bufferWsize.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_bufferWsize++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_bufferWsize--;	break;
		default: break;
	}
	mm_bufferWsize.m_bEntryDone=FALSE;	// clear flag
	mm_bufferWsize.m_nChar=0;			// empty buffer
	mm_bufferWsize.SetSel(0, -1);		// select all text

	// check val and update dependent parameter
	WORD uiWsizemin = (WORD)(m_adratechan * 0.05); // minimum buffer size = 50 ms (!?)
	if (m_bufferWsize > m_bufferWsizemax)
		m_bufferWsize = m_bufferWsizemax;
	else if (m_bufferWsize < uiWsizemin)
		m_bufferWsize = uiWsizemin;	
	m_bufferWsize  = (m_bufferWsize /m_undersamplefactor) * m_undersamplefactor;
	if (m_bufferWsize == 0)
		m_bufferWsize = m_undersamplefactor;
	m_sweepduration = m_bufferWsize * m_bufferNitems / m_adratechan;
	UpdateData(FALSE);		
}


// -----------------------------------------------------------------------
void ADIntervalsDlg::OnEnChangeNbuffers()
{
	if (!mm_bufferNitems.m_bEntryDone)
		return;

	switch (mm_bufferNitems.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_bufferNitems++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_bufferNitems--;	break;
		default: break;
	}
	mm_bufferNitems.m_bEntryDone=FALSE;	// clear flag
	mm_bufferNitems.m_nChar=0;			// empty buffer
	mm_bufferNitems.SetSel(0, -1);		// select all text    
	// update dependent parameters
	if (m_bufferNitems < 1)
		m_bufferNitems = 1;
	m_sweepduration = m_bufferWsize * m_bufferNitems / m_adratechan;
	UpdateData(FALSE);	
	return;	
}


void ADIntervalsDlg::OnEnChangeAcqduration() 
{
	if (!mm_acqduration.m_bEntryDone)
		return;

	switch (mm_acqduration.m_nChar)
	{				// load data from edit controls
		case VK_RETURN:	UpdateData(TRUE);	break;
		case VK_UP:
		case VK_PRIOR:	m_acqduration++;	break;
		case VK_DOWN:
		case VK_NEXT:   m_acqduration--;	break;
		default: break;
	}
	mm_acqduration.m_bEntryDone=FALSE;	// clear flag
	mm_acqduration.m_nChar=0;			// empty buffer
	mm_acqduration.SetSel(0, -1);		// select all text
	// at least one buffer...
	float minduration = m_bufferWsize / m_adratechan;
	if (m_acqduration < minduration)
		m_acqduration = minduration;
	UpdateData(FALSE);
}


void ADIntervalsDlg::OnTrigthresholdOFF() 
{
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(FALSE);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(FALSE);
}

void ADIntervalsDlg::OnTrigthresholdON() 
{
	GetDlgItem(IDC_THRESHOLDCHAN)->EnableWindow(TRUE);
	GetDlgItem(IDC_THRESHOLDVAL)->EnableWindow(TRUE);	
}
