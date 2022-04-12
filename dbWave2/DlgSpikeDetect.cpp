// spikedetec.cpp : implementation file
//
// caller must set:
// m_iDetectParmsDlgDlg	= index spk detection parameters set to edit
// m_spkDA = address of structure containing array of spk detection parameters structures

#include "StdAfx.h"
#include "resource.h"
#include "DlgSpikeDetect.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgSpikeDetect::DlgSpikeDetect(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
}

void DlgSpikeDetect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PARAMETERSTAB, m_cParameterTabCtrl);
}

BEGIN_MESSAGE_MAP(DlgSpikeDetect, CDialog)
	ON_BN_CLICKED(IDC_DETECTFROMTAG, OnDetectfromtag)
	ON_BN_CLICKED(IDC_DETECTFROMCHAN, OnDetectfromchan)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PARAMETERSTAB, OnTcnSelchangeParameterstab)
	ON_BN_CLICKED(IDC_SPIKESRADIO, OnBnClickedSpikesradio)
	ON_BN_CLICKED(IDC_STIMRADIO, OnBnClickedStimradio)
	ON_BN_CLICKED(IDC_ADDPARAMBTTN, OnBnClickedAddparambttn)
	ON_BN_CLICKED(IDC_DELPARAMBTTN, OnBnClickedDelparambttn)
	ON_EN_CHANGE(IDC_DETECTTHRESHOLD, OnEnChangeDetectthreshold)
	ON_CBN_SELCHANGE(IDC_DETECTCHAN, OnCbnSelchangeDetectchan)
	ON_CBN_SELCHANGE(IDC_DETECTTRANSFORM, OnCbnSelchangeDetecttransform)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_CBN_SELCHANGE(IDC_EXTRACTCHAN, OnCbnSelchangeExtractchan)
	ON_EN_CHANGE(IDC_COMMENT, OnEnChangeComment)
	ON_BN_CLICKED(IDC_RIGHTSHIFT, OnBnClickedShiftright)
	ON_BN_CLICKED(IDC_LEFTSHIFT, OnBnClickedShiftleft)
END_MESSAGE_MAP()

BOOL DlgSpikeDetect::OnInitDialog()
{
	CDialog::OnInitDialog();
	// init chan list, select first detection channel
	CString comment;
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->ResetContent();
	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTCHAN))->ResetContent();

	const auto pwave_format = m_dbDoc->GetpWaveFormat();
	const auto pchan_array = m_dbDoc->GetpWavechanArray();
	int chanmax = pwave_format->scan_count;
	m_scancount = chanmax;

	// load list of channels into combo boxes
	for (auto i = 0; i < chanmax; i++)
	{
		comment.Format(_T("%i"), i); // channel index CString
		const auto pchan = pchan_array->Get_p_channel(i);
		comment += _T(" - ") + pchan->am_csComment;
		VERIFY(((CComboBox*)GetDlgItem(IDC_DETECTCHAN))->AddString(comment) != CB_ERR);
		VERIFY(((CComboBox*)GetDlgItem(IDC_EXTRACTCHAN))->AddString(comment) != CB_ERR);
	}

	// load list of transform methods
	chanmax = m_dbDoc->GetTransfDataNTypes();
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->ResetContent();
	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTTRANSFORM))->ResetContent();
	for (int i = 0; i < chanmax; i++)
	{
		VERIFY(((CComboBox*)GetDlgItem(IDC_DETECTTRANSFORM))->AddString(m_dbDoc->GetTransfDataName(i)) != CB_ERR);
		VERIFY(((CComboBox*)GetDlgItem(IDC_EXTRACTTRANSFORM))->AddString(m_dbDoc->GetTransfDataName(i)) != CB_ERR);
	}

	// load list of detection parameters
	for (int i = 0; i < m_pDetectSettingsArray->GetSize(); i++)
	{
		CString cs;
		cs.Format(_T("#%i "), i);
		cs += (m_pDetectSettingsArray->GetItem(i))->comment;
		m_cParameterTabCtrl.InsertItem(i, cs);
	}

	// fill dialog with values from array
	LoadChanParameters(m_iDetectParmsDlg);
	UpdateTabShiftButtons();

	return TRUE; // return TRUE  unless you set the focus to a control
}

void DlgSpikeDetect::SaveChanParameters(int chan)
{
	m_pspkD = m_pDetectSettingsArray->GetItem(chan);
	GetDlgItem(IDC_COMMENT)->GetWindowText(m_pspkD->comment);
	mdPM->bDetectWhileBrowse = static_cast<CButton*>(GetDlgItem(IDC_DETECTBROWSE))->GetCheck();

	// spikes detection parameters
	const auto flag2 = static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMCHAN))->GetCheck();
	m_pspkD->detect_from = flag2 ? 0 : 1;
	m_pspkD->detect_channel = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->GetCurSel();
	m_pspkD->detect_transform = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->GetCurSel();
	m_pspkD->detect_threshold_bin = GetDlgItemInt(IDC_DETECTTHRESHOLD);

	// detect spikes
	if (static_cast<CButton*>(GetDlgItem(IDC_SPIKESRADIO))->GetCheck() == BST_CHECKED)
	{
		m_pspkD->detect_what = DETECT_SPIKES;
		m_pspkD->extract_channel = static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTCHAN))->GetCurSel();
		m_pspkD->extract_transform = static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTTRANSFORM))->GetCurSel();
		m_pspkD->extract_n_points = GetDlgItemInt(IDC_SPIKENPOINTS);
		m_pspkD->detect_pre_threshold = GetDlgItemInt(IDC_PRETHRESHOLD);
		m_pspkD->detect_refractory_period = GetDlgItemInt(IDC_REFRACTORY);
	}
	// detect stimulus
	else
	{
		m_pspkD->detect_what = DETECT_STIMULUS;
		m_pspkD->detect_mode = static_cast<CComboBox*>(GetDlgItem(IDC_STIMDETECTMODE))->GetCurSel();
		m_pspkD->extract_channel = m_pspkD->detect_channel;
	}
}

void DlgSpikeDetect::LoadChanParameters(int chan)
{
	m_iDetectParmsDlg = chan;
	m_pspkD = m_pDetectSettingsArray->GetItem(chan);
	GetDlgItem(IDC_COMMENT)->SetWindowText(m_pspkD->comment);

	if (m_pspkD->detect_what == DETECT_SPIKES)
	{
		static_cast<CButton*>(GetDlgItem(IDC_SPIKESRADIO))->SetCheck(BST_CHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_STIMRADIO))->SetCheck(BST_UNCHECKED);
	}
	else
	{
		static_cast<CButton*>(GetDlgItem(IDC_SPIKESRADIO))->SetCheck(BST_UNCHECKED);
		static_cast<CButton*>(GetDlgItem(IDC_STIMRADIO))->SetCheck(BST_CHECKED);
	}
	SetDlgInterfaceState(m_pspkD->detect_what);

	// spikes detection parameters
	const BOOL flag = (m_pspkD->detect_from == 0);
	static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMCHAN))->SetCheck(flag);
	static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMTAG))->SetCheck(!flag);
	DisplayDetectFromChan();
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->SetCurSel(m_pspkD->detect_channel);
	static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->SetCurSel(m_pspkD->detect_transform);
	SetDlgItemInt(IDC_DETECTTHRESHOLD, m_pspkD->detect_threshold_bin);

	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTCHAN))->SetCurSel(m_pspkD->extract_channel);
	static_cast<CComboBox*>(GetDlgItem(IDC_EXTRACTTRANSFORM))->SetCurSel(m_pspkD->extract_transform);
	SetDlgItemInt(IDC_SPIKENPOINTS, m_pspkD->extract_n_points);
	SetDlgItemInt(IDC_PRETHRESHOLD, m_pspkD->detect_pre_threshold);
	SetDlgItemInt(IDC_REFRACTORY, m_pspkD->detect_refractory_period);
	static_cast<CButton*>(GetDlgItem(IDC_DETECTBROWSE))->SetCheck(mdPM->bDetectWhileBrowse);

	// stimulus detection parameters
	static_cast<CComboBox*>(GetDlgItem(IDC_STIMDETECTMODE))->SetCurSel(m_pspkD->detect_mode);

	// select proper tab
	m_cParameterTabCtrl.SetCurSel(chan);
}

void DlgSpikeDetect::OnOK()
{
	// save spike detection parameters
	SaveChanParameters(m_iDetectParmsDlg);
	CDialog::OnOK();
}

void DlgSpikeDetect::DisplayDetectFromChan()
{
	BOOL flag = FALSE;
	if (static_cast<CButton*>(GetDlgItem(IDC_DETECTFROMCHAN))->GetCheck())
		flag = TRUE;
	GetDlgItem(IDC_DETECTCHAN)->EnableWindow(flag);
	GetDlgItem(IDC_DETECTTRANSFORM)->EnableWindow(flag);
	GetDlgItem(IDC_DETECTTHRESHOLD)->EnableWindow(flag);
}

void DlgSpikeDetect::OnDetectfromtag()
{
	DisplayDetectFromChan();
}

void DlgSpikeDetect::OnDetectfromchan()
{
	DisplayDetectFromChan();
}

void DlgSpikeDetect::OnTcnSelchangeParameterstab(NMHDR* pNMHDR, LRESULT* pResult)
{
	SaveChanParameters(m_iDetectParmsDlg);
	*pResult = 0;
	const auto ichan = m_cParameterTabCtrl.GetCurSel();
	LoadChanParameters(ichan);
	Invalidate();

	// update sourceview
	UpdateSourceView();
	UpdateTabShiftButtons();
}

void DlgSpikeDetect::SetDlgInterfaceState(int detectWhat)
{
	auto b_spike_detect_items = TRUE;
	auto b_stim_detect_items = FALSE;
	if (detectWhat == 1)
	{
		b_spike_detect_items = FALSE;
		b_stim_detect_items = TRUE;
	}
	GetDlgItem(IDC_DETECTFROMTAG)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_DETECTFROMCHAN)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_EXTRACTCHAN)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_EXTRACTTRANSFORM)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_SPIKENPOINTS)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_PRETHRESHOLD)->EnableWindow(b_spike_detect_items);
	GetDlgItem(IDC_REFRACTORY)->EnableWindow(b_spike_detect_items);

	GetDlgItem(IDC_STIMDETECTMODE)->EnableWindow(b_stim_detect_items);
}

void DlgSpikeDetect::OnBnClickedSpikesradio()
{
	m_pspkD->detect_what = DETECT_SPIKES;
	SetDlgInterfaceState(m_pspkD->detect_what);
}

void DlgSpikeDetect::OnBnClickedStimradio()
{
	m_pspkD->detect_what = DETECT_STIMULUS;
	SetDlgInterfaceState(m_pspkD->detect_what);
}

void DlgSpikeDetect::OnBnClickedAddparambttn()
{
	// save current parameters set
	SaveChanParameters(m_iDetectParmsDlg);

	// Add parameter set
	const auto ilast = m_pDetectSettingsArray->AddItem() - 1;
	CString cs;
	cs.Format(_T("set #%i"), ilast);
	m_cParameterTabCtrl.InsertItem(ilast, cs);
	LoadChanParameters(ilast);
}

void DlgSpikeDetect::OnBnClickedDelparambttn()
{
	const auto ilast = m_pDetectSettingsArray->RemoveItem(m_iDetectParmsDlg) - 1;
	m_cParameterTabCtrl.DeleteItem(m_iDetectParmsDlg);
	LoadChanParameters(ilast);
	Invalidate();
}

void DlgSpikeDetect::OnEnChangeDetectthreshold()
{
	const int ithreshold = GetDlgItemInt(IDC_DETECTTHRESHOLD);
	m_pChartDataDetectWnd->MoveHZtagtoVal(0, ithreshold);
}

void DlgSpikeDetect::OnCbnSelchangeDetectchan()
{
	UpdateSourceView();
}

void DlgSpikeDetect::UpdateSourceView()
{
	const auto icursel = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTCHAN))->GetCurSel();
	const auto icursel2 = static_cast<CComboBox*>(GetDlgItem(IDC_DETECTTRANSFORM))->GetCurSel();
	// TODO not used??
	//int icursel3 = ((CComboBox*) GetDlgItem(IDC_EXTRACTCHAN))->GetCurSel();
	m_pChartDataDetectWnd->SetChanlistOrdinates(0, icursel, icursel2);
	m_pChartDataDetectWnd->GetDataFromDoc();
	m_pChartDataDetectWnd->AutoZoomChan(0);
	m_pChartDataDetectWnd->Invalidate();
}

void DlgSpikeDetect::OnCbnSelchangeDetecttransform()
{
	UpdateSourceView();
}

void DlgSpikeDetect::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult)
{
	const auto p_nm_up_down = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	int ithreshold = GetDlgItemInt(IDC_DETECTTHRESHOLD);
	ithreshold -= MulDiv(m_pChartDataDetectWnd->GetChanlistItem(0)->GetYextent(), p_nm_up_down->iDelta, 10);
	SetDlgItemInt(IDC_DETECTTHRESHOLD, ithreshold);

	*pResult = 0;
}

void DlgSpikeDetect::OnCbnSelchangeExtractchan()
{
	UpdateSourceView();
}

void DlgSpikeDetect::OnEnChangeComment()
{
	CString cs;
	GetDlgItem(IDC_COMMENT)->GetWindowText(cs);
	SetTabComment(m_iDetectParmsDlg, cs);
}

void DlgSpikeDetect::SetTabComment(int i, CString& cs)
{
	TCITEM tc_item;
	tc_item.mask = TCIF_TEXT;
	CString cs1;
	cs1.Format(_T("#%i %s"), m_iDetectParmsDlg, (LPCTSTR)cs);
	//  Set the new text for the item.
	tc_item.pszText = cs1.GetBuffer(2);
	m_cParameterTabCtrl.SetItem(i, &tc_item);
}

// move current parameter settings one slot more

void DlgSpikeDetect::OnBnClickedShiftright()
{
	const auto isource = m_iDetectParmsDlg;
	const auto idest = isource + 1;
	if (idest < m_pDetectSettingsArray->GetSize())
		ExchangeParms(isource, idest);
}

void DlgSpikeDetect::OnBnClickedShiftleft()
{
	const auto isource = m_iDetectParmsDlg;
	const auto idest = isource - 1;
	if (idest <= 0)
		ExchangeParms(isource, idest);
}

void DlgSpikeDetect::ExchangeParms(int isource, int idest)
{
	SaveChanParameters(isource); // save current data
	const auto p_sp = new SPKDETECTPARM; // create temporary data to exchange
	const auto p_source = m_pDetectSettingsArray->GetItem(isource);
	const auto p_destination = m_pDetectSettingsArray->GetItem(idest);
	*p_sp = *p_source;
	*p_source = *p_destination;
	*p_destination = *p_sp;
	m_iDetectParmsDlg = idest;
	LoadChanParameters(idest);
	SetTabComment(isource, (m_pDetectSettingsArray->GetItem(isource))->comment);
	SetTabComment(idest, (m_pDetectSettingsArray->GetItem(idest))->comment);

	// update interface
	m_cParameterTabCtrl.SetCurSel(idest);
	Invalidate();
	UpdateSourceView();
	UpdateTabShiftButtons();
}

void DlgSpikeDetect::UpdateTabShiftButtons()
{
	auto b_enable = TRUE;
	if (m_iDetectParmsDlg < 1)
		b_enable = FALSE;
	GetDlgItem(IDC_LEFTSHIFT)->EnableWindow(b_enable);
	b_enable = TRUE;
	if (m_iDetectParmsDlg >= m_pDetectSettingsArray->GetSize() - 1)
		b_enable = FALSE;
	GetDlgItem(IDC_RIGHTSHIFT)->EnableWindow(b_enable);
}
