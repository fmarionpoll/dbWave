// spikedetec.cpp : implementation file
//
// caller must set:
// m_iDetectParmsDlgDlg	= index spk detection parameters set to edit
// m_spkDA = address of structure containing array of spk detection parameters structures

#include "stdafx.h"
#include "dbWave.h"
#include "resource.h"
#include ".\spikedetec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpikeDetectDlg dialog


CSpikeDetectDlg::CSpikeDetectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpikeDetectDlg::IDD, pParent)
{

}

void CSpikeDetectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PARAMETERSTAB, m_cParameterTabCtrl);
}

BEGIN_MESSAGE_MAP(CSpikeDetectDlg, CDialog)
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


//////////////////////////////////////////////////////////////////////////////
// CSpikeDetectDlg message handlers

BOOL CSpikeDetectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	// init chan list, select first detection channel
	CString comment;
	((CComboBox*)GetDlgItem(IDC_DETECTCHAN))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_EXTRACTCHAN))->ResetContent();

	CWaveFormat* pwaveFormat = m_dbDoc->GetpWaveFormat();
	CWaveChanArray* pchanArray = m_dbDoc->GetpWavechanArray();
	int chanmax = pwaveFormat->scan_count;
	m_scancount = chanmax;

	// load list of channels into combo boxes
	for (int i =0; i<chanmax; i++)
	{
		comment.Format(_T("%i"), i);		// channel index CString
		CWaveChan* pchan = pchanArray->GetWaveChan(i);
		comment += _T(" - ") + pchan->am_csComment;
		VERIFY(((CComboBox*)GetDlgItem(IDC_DETECTCHAN))->AddString(comment) != CB_ERR);
		VERIFY(((CComboBox*)GetDlgItem(IDC_EXTRACTCHAN))->AddString(comment) != CB_ERR);
	}

	// load list of transform methods
	chanmax = m_dbDoc->GetTransfDataNTypes();
	((CComboBox*)GetDlgItem(IDC_DETECTTRANSFORM))->ResetContent();
	((CComboBox*)GetDlgItem(IDC_EXTRACTTRANSFORM))->ResetContent();
	for (int i=0; i<chanmax; i++)
	{
		VERIFY(((CComboBox*)GetDlgItem(IDC_DETECTTRANSFORM))->AddString(m_dbDoc->GetTransfDataName(i)) != CB_ERR);
		VERIFY(((CComboBox*)GetDlgItem(IDC_EXTRACTTRANSFORM))->AddString(m_dbDoc->GetTransfDataName(i)) != CB_ERR);
	}

	// load list of detection parameters 
	for (int i = 0; i< m_pDetectSettingsArray->GetSize(); i++)
	{
		CString cs;
		cs.Format(_T("#%i "), i);
		cs += (m_pDetectSettingsArray->GetItem(i))->comment;
		m_cParameterTabCtrl.InsertItem(i, cs);
	}

	// fill dialog with values from array
	LoadChanParameters(m_iDetectParmsDlg);
	UpdateTabShiftButtons();

	return TRUE;  	// return TRUE  unless you set the focus to a control
}

void CSpikeDetectDlg::SaveChanParameters(int chan)
{
	m_pspkD = m_pDetectSettingsArray->GetItem(chan);
	GetDlgItem(IDC_COMMENT)->GetWindowText(m_pspkD->comment);
	mdPM->bDetectWhileBrowse = ((CButton*)GetDlgItem(IDC_DETECTBROWSE))->GetCheck();

	// spikes detection parameters
	BOOL flag2 = ((CButton*)GetDlgItem(IDC_DETECTFROMCHAN))->GetCheck();
	m_pspkD->detectFrom = flag2 ? 0 : 1;
	m_pspkD->detectChan = ((CComboBox*)GetDlgItem(IDC_DETECTCHAN))->GetCurSel();
	m_pspkD->detectTransform=((CComboBox*)GetDlgItem(IDC_DETECTTRANSFORM))->GetCurSel();
	m_pspkD->detectThreshold=GetDlgItemInt(IDC_DETECTTHRESHOLD);

	// detect spikes
	if (((CButton*) GetDlgItem(IDC_SPIKESRADIO))->GetCheck() == BST_CHECKED)
	{
		m_pspkD->detectWhat = 0;
		m_pspkD->extractChan= ((CComboBox*)GetDlgItem(IDC_EXTRACTCHAN))->GetCurSel();
		m_pspkD->extractTransform=((CComboBox*)GetDlgItem(IDC_EXTRACTTRANSFORM))->GetCurSel();
		m_pspkD->extractNpoints=GetDlgItemInt(IDC_SPIKENPOINTS);
		m_pspkD->prethreshold=GetDlgItemInt(IDC_PRETHRESHOLD);
		m_pspkD->refractory=GetDlgItemInt(IDC_REFRACTORY);
	}
	// detect stimulus
	else
	{
		m_pspkD->detectWhat = 1;
		m_pspkD->detectMode = ((CComboBox*)GetDlgItem(IDC_STIMDETECTMODE))->GetCurSel();
		m_pspkD->extractChan= m_pspkD->detectChan;
	}
}

void CSpikeDetectDlg::LoadChanParameters(int chan)
{
	m_iDetectParmsDlg = chan;
	m_pspkD = m_pDetectSettingsArray->GetItem(chan);
	GetDlgItem(IDC_COMMENT)->SetWindowText(m_pspkD->comment);

	if (m_pspkD->detectWhat == 0)
	{
		((CButton*) GetDlgItem(IDC_SPIKESRADIO))->SetCheck(BST_CHECKED);
		((CButton*) GetDlgItem(IDC_STIMRADIO))->SetCheck(BST_UNCHECKED);
	}
	else
	{
		((CButton*) GetDlgItem(IDC_SPIKESRADIO))->SetCheck(BST_UNCHECKED);
		((CButton*) GetDlgItem(IDC_STIMRADIO))->SetCheck(BST_CHECKED);
	}
	SetDlgInterfaceState(m_pspkD->detectWhat);
	
	// spikes detection parameters
	BOOL flag = (m_pspkD->detectFrom == 0);
	((CButton*)GetDlgItem(IDC_DETECTFROMCHAN))->SetCheck(flag);
	((CButton*)GetDlgItem(IDC_DETECTFROMTAG))->SetCheck(!flag);
	DisplayDetectFromChan();
	((CComboBox*)GetDlgItem(IDC_DETECTCHAN))->SetCurSel(m_pspkD->detectChan);
	((CComboBox*)GetDlgItem(IDC_DETECTTRANSFORM))->SetCurSel(m_pspkD->detectTransform);
	SetDlgItemInt(IDC_DETECTTHRESHOLD, m_pspkD->detectThreshold);

	((CComboBox*)GetDlgItem(IDC_EXTRACTCHAN))->SetCurSel(m_pspkD->extractChan);	
	((CComboBox*)GetDlgItem(IDC_EXTRACTTRANSFORM))->SetCurSel(m_pspkD->extractTransform);
	SetDlgItemInt(IDC_SPIKENPOINTS, m_pspkD->extractNpoints);
	SetDlgItemInt(IDC_PRETHRESHOLD, m_pspkD->prethreshold);
	SetDlgItemInt(IDC_REFRACTORY, m_pspkD->refractory);
	((CButton*)GetDlgItem(IDC_DETECTBROWSE))->SetCheck(mdPM->bDetectWhileBrowse);

	// stimulus detection parameters
	((CComboBox*)GetDlgItem(IDC_STIMDETECTMODE))->SetCurSel(m_pspkD->detectMode);

	// select proper tab
	m_cParameterTabCtrl.SetCurSel(chan);
}

void CSpikeDetectDlg::OnOK()
{
	// save spike detection parameters
	SaveChanParameters(m_iDetectParmsDlg);
	CDialog::OnOK();
}


void CSpikeDetectDlg::DisplayDetectFromChan()
{
	BOOL flag = FALSE;
	if (((CButton*)GetDlgItem(IDC_DETECTFROMCHAN))->GetCheck())
		flag = TRUE;
	GetDlgItem(IDC_DETECTCHAN)->EnableWindow(flag);
	GetDlgItem(IDC_DETECTTRANSFORM)->EnableWindow(flag);
	GetDlgItem(IDC_DETECTTHRESHOLD)->EnableWindow(flag);
}

void CSpikeDetectDlg::OnDetectfromtag() 
{
	DisplayDetectFromChan();	
}

void CSpikeDetectDlg::OnDetectfromchan() 
{
	DisplayDetectFromChan();
}

void CSpikeDetectDlg::OnTcnSelchangeParameterstab(NMHDR *pNMHDR, LRESULT *pResult)
{
	SaveChanParameters(m_iDetectParmsDlg);
	*pResult = 0;
	int ichan = m_cParameterTabCtrl.GetCurSel();
	LoadChanParameters(ichan);
	Invalidate();

	// update sourceview
	UpdateSourceView();
	UpdateTabShiftButtons();
}

void CSpikeDetectDlg::SetDlgInterfaceState(int detectWhat)
{
	BOOL bSpikeDetectItems = TRUE;
	BOOL bStimDetectItems = FALSE;
	if (detectWhat == 1)
	{
		bSpikeDetectItems = FALSE;
		bStimDetectItems = TRUE;
	}
	GetDlgItem(IDC_DETECTFROMTAG)->EnableWindow(bSpikeDetectItems);
	GetDlgItem(IDC_DETECTFROMCHAN)->EnableWindow(bSpikeDetectItems);
	GetDlgItem(IDC_EXTRACTCHAN)->EnableWindow(bSpikeDetectItems);
	GetDlgItem(IDC_EXTRACTTRANSFORM)->EnableWindow(bSpikeDetectItems);
	GetDlgItem(IDC_SPIKENPOINTS)->EnableWindow(bSpikeDetectItems);
	GetDlgItem(IDC_PRETHRESHOLD)->EnableWindow(bSpikeDetectItems);
	GetDlgItem(IDC_REFRACTORY)->EnableWindow(bSpikeDetectItems);

	GetDlgItem(IDC_STIMDETECTMODE)->EnableWindow(bStimDetectItems);
}

void CSpikeDetectDlg::OnBnClickedSpikesradio()
{
	m_pspkD->detectWhat = 0;
	SetDlgInterfaceState(m_pspkD->detectWhat);
}

void CSpikeDetectDlg::OnBnClickedStimradio()
{
	m_pspkD->detectWhat = 1;
	SetDlgInterfaceState(m_pspkD->detectWhat);
}


void CSpikeDetectDlg::OnBnClickedAddparambttn()
{
	// save current parameters set
	SaveChanParameters(m_iDetectParmsDlg);

	// Add parameter set
	int ilast = m_pDetectSettingsArray->AddItem() -1;
	CString cs;
	cs.Format(_T("set #%i"), ilast);
	m_cParameterTabCtrl.InsertItem(ilast, cs);
	LoadChanParameters(ilast);
}

void CSpikeDetectDlg::OnBnClickedDelparambttn()
{
	int ilast = m_pDetectSettingsArray->RemoveItem(m_iDetectParmsDlg)-1;
	m_cParameterTabCtrl.DeleteItem(m_iDetectParmsDlg);
	LoadChanParameters(ilast);
	Invalidate();
}

void CSpikeDetectDlg::OnEnChangeDetectthreshold()
{
	int ithreshold = GetDlgItemInt(IDC_DETECTTHRESHOLD);
	m_pdisplayDetect->MoveHZtagtoVal(0, ithreshold);
}

void CSpikeDetectDlg::OnCbnSelchangeDetectchan()
{
	UpdateSourceView();
}

void CSpikeDetectDlg::UpdateSourceView()
{
	int icursel  = ((CComboBox*) GetDlgItem(IDC_DETECTCHAN))->GetCurSel(); 
	int icursel2 = ((CComboBox*) GetDlgItem(IDC_DETECTTRANSFORM))->GetCurSel();
	int icursel3 = ((CComboBox*) GetDlgItem(IDC_EXTRACTCHAN))->GetCurSel();
	m_pdisplayDetect->SetChanlistOrdinates(0, icursel, icursel2);
	m_pdisplayDetect->GetDataFromDoc();
	m_pdisplayDetect->AutoZoomChan(0);
	m_pdisplayDetect->Invalidate();
}


void CSpikeDetectDlg::OnCbnSelchangeDetecttransform()
{
	UpdateSourceView();
}

void CSpikeDetectDlg::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	int ithreshold = GetDlgItemInt(IDC_DETECTTHRESHOLD);
	ithreshold -= MulDiv(m_pdisplayDetect->GetChanlistYextent(0), pNMUpDown->iDelta, 10);
	SetDlgItemInt(IDC_DETECTTHRESHOLD, ithreshold);
	
	*pResult = 0;
}

void CSpikeDetectDlg::OnCbnSelchangeExtractchan()
{
	UpdateSourceView();
}

void CSpikeDetectDlg::OnEnChangeComment()
{
	CString cs;
	GetDlgItem(IDC_COMMENT)->GetWindowText(cs);
	SetTabComment(m_iDetectParmsDlg, cs);
}

void CSpikeDetectDlg::SetTabComment(int i, CString& cs)
{
	TCITEM tcItem;
	tcItem.mask = TCIF_TEXT;
	CString cs1;
	cs1.Format(_T("#%i %s"), m_iDetectParmsDlg, (LPCTSTR) cs);
	//  Set the new text for the item.
	tcItem.pszText = cs1.GetBuffer(2);
	m_cParameterTabCtrl.SetItem(i, &tcItem);
}


// move current parameter settings one slot more

void CSpikeDetectDlg::OnBnClickedShiftright()
{
	int isource = m_iDetectParmsDlg;
	int idest = isource +1;
	if (idest < m_pDetectSettingsArray->GetSize())
		ExchangeParms(isource, idest);
}

void CSpikeDetectDlg::OnBnClickedShiftleft()
{
	int isource = m_iDetectParmsDlg;
	int idest = isource -1;
	if (idest <= 0)
		ExchangeParms(isource, idest);
}

void CSpikeDetectDlg::ExchangeParms(int isource, int idest)
{
	SaveChanParameters(isource);				// save current data
	SPKDETECTPARM* pSP		= new SPKDETECTPARM;		// create temporary data to exchange
	SPKDETECTPARM* pSource	= m_pDetectSettingsArray->GetItem(isource);
	SPKDETECTPARM* pDest	= m_pDetectSettingsArray->GetItem(idest);
	*pSP = *pSource;
	*pSource = *pDest;  
	*pDest = *pSP;
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

void CSpikeDetectDlg::UpdateTabShiftButtons()
{
	BOOL bEnable=TRUE;
	if (m_iDetectParmsDlg < 1)
		bEnable = FALSE;
	GetDlgItem(IDC_LEFTSHIFT)->EnableWindow(bEnable);
	bEnable = TRUE;
	if (m_iDetectParmsDlg >= m_pDetectSettingsArray->GetSize()-1)
		bEnable = FALSE;
	GetDlgItem(IDC_RIGHTSHIFT)->EnableWindow(bEnable);
}