// vdordina.cpp : implementation file
//

#include "StdAfx.h"
//#include "Cscale.h"
//#include "chart.h"
#include "ChartData.h"

#include "resource.h"
#include "Vdordina.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CDataViewOrdinatesDlg dialog
//
// this dialog displays interdependent values
// the values changes the curves display
// curves parameters are: channel (1 selected or all)
//						zero (internal representation) * voltsperbin
//						extent (internal) * voltsperbin
//
// the parameters displayed are not the internal values but
// m_xmax		max amplitude of curve selected (top of the window) * voltfactor
// m_xmin		min amplitude (..) * voltfactor
// m_xcenter	amplitude of the zero * voltfactor
//
// these parameters are mirrored by parameters without attenuation
// m_max (volts) = m_xmax / voltfactor
// m_min (volts) = m_xmin / voltfactor
// m_center (volts) = m_xcenter / voltfactor
//
// the formula to convert these params into internal parameters are
//    (if zero volt = 2048)
// zero = ((m_max+m_min)/2) / voltsperbin + 2048
//				(2048 = 4096(full scale)/2: 12 bits encoding)
// extent = (max-min) / voltsperbin
// voltsperpixel = extent * voltsperbin / lineview.GetRectHeight()
//
// therefore when one parameter is changed, others are modified accordingly:
// (0=unchanged, 1=changed)
//	change ---------- center---max-----min---
//	chan				1		1		1
//	center				1		1		1
//	max					1		1		0
//	min					1		0		1

CDataViewOrdinatesDlg::CDataViewOrdinatesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent), m_pChartDataWnd(nullptr), m_nChanmax(0), m_Channel(0), m_bChanged(0),
	  m_p10(0), m_voltsperpixel(0), m_VoltsperBin(0)
{
	m_iUnit = -1;
	m_xcenter = 0.0f;
	m_xmax = 0.0f;
	m_xmin = 0.0f;
}

void CDataViewOrdinatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHANSELECT, m_chanSelect);
	DDX_CBIndex(pDX, IDC_VERTUNITS, m_iUnit);
	DDX_Text(pDX, IDC_VERTCENTER, m_xcenter);
	DDX_Text(pDX, IDC_VERTMAX, m_xmax);
	DDX_Text(pDX, IDC_VERTMIN, m_xmin);
	if (m_bChanged)
	{
		SaveChanlistData(m_Channel);
		m_bChanged = FALSE;
	}
}

BEGIN_MESSAGE_MAP(CDataViewOrdinatesDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_VERTUNITS, OnSelchangeVertunits)
	ON_CBN_SELCHANGE(IDC_CHANSELECT, OnSelchangeChanselect)
	ON_EN_KILLFOCUS(IDC_VERTMAX, OnKillfocusVertMxMi)
	ON_EN_KILLFOCUS(IDC_VERTCENTER, OnKillfocusVertcenter)
	ON_EN_KILLFOCUS(IDC_VERTMIN, OnKillfocusVertMxMi)
END_MESSAGE_MAP()

BOOL CDataViewOrdinatesDlg::OnInitDialog()
{
	// save extent and zero before calling base class
	// this is an awful patch. But necessary. For some reason really unclear to
	// me, the parameters (zero and extent) of channel 0 / chanlist / lineview
	// are MODIFIED after calling the base class CDialog::OnInitDialog
	int i;
	m_nChanmax = m_pChartDataWnd->GetChanlistSize(); // nb of data channels
	for (i = 0; i < m_nChanmax; i++) // browse through all chans
	{
		CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(i);
		m_settings.Add(chan->GetYzero()); // save zero
		m_settings.Add(chan->GetYextent()); // save extent
	}
	CDialog::OnInitDialog();

	// load channel description CComboBox
	int j = 0; // index to restore parms
	for (i = 0; i < m_nChanmax; i++) // browse through all chans again
	{
		CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(i);
		chan->SetYzero(m_settings.GetAt(j));
		j++; // restore zero
		chan->SetYextent(m_settings.GetAt(j));
		j++; // restore extent
		m_chanSelect.AddString(chan->GetComment()); // load comment/chan
	}
	m_chanSelect.SetCurSel(m_Channel); // select chan zero

	LoadChanlistData(m_Channel); // compute data in volts
	m_iUnit = 1; // select "mV" as unit within combo box
	static_cast<CComboBox*>(GetDlgItem(IDC_VERTUNITS))->SetCurSel(m_iUnit);
	ChangeUnits(m_iUnit, TRUE); // adapt data/volts -> mv and fill controls
	UpdateData(FALSE);
	return TRUE;
}

void CDataViewOrdinatesDlg::LoadChanlistData(int i)
{
	CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(i);
	// compute max and min from zero and extent
	m_VoltsperBin = chan->GetVoltsperDataBin();
	const auto zero = static_cast<float>(chan->GetYzero());
	const auto extent = static_cast<float>(chan->GetYextent());
	//const auto binzero =  static_cast<float>(0.0)  /*m_plinev->GetChanlistBinZero(i)*/;

	m_xcenter = (zero/*- binzero*/) * m_VoltsperBin / m_p10;
	const auto xextent = (extent * m_VoltsperBin / m_p10) / 2.f;
	m_xmin = m_xcenter - xextent;
	m_xmax = m_xcenter + xextent;

	m_bChanged = FALSE;
}

void CDataViewOrdinatesDlg::SaveChanlistData(int indexlist)
{
	const auto bCheck = static_cast<CButton*>(GetDlgItem(IDC_CHECKALL))->GetCheck();
	auto indexfirst = indexlist; // prepare for loop (first chan)
	auto indexlast = indexlist + 1; // ibid (ending chan)
	if (bCheck) // if bCheck (TRUE) then
	{
		indexfirst = 0; // loop through all channels
		indexlast = m_nChanmax; // 0 to max
	}

	for (auto j = indexfirst; j < indexlast; j++)
	{
		CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(j);
		const auto vper_bin = chan->GetVoltsperDataBin();
		const auto xzero = ((m_xmax + m_xmin) / 2.f) * m_p10;
		const auto xextent = (m_xmax - m_xmin) * m_p10;
		auto i = static_cast<int>(xzero / vper_bin) + chan->GetDataBinZero();
		chan->SetYzero(i);
		i = static_cast<int>(xextent / vper_bin);
		chan->SetYextent(i);
	}
	m_pChartDataWnd->Invalidate();
}

void CDataViewOrdinatesDlg::ChangeUnits(int iUnit, BOOL bNew)
{
	auto newp10 = m_p10;
	if (bNew)
	{
		switch (iUnit)
		{
		case 0: newp10 = 1.0f;
			break; // volts
		case 1: newp10 = 0.001f;
			break; // millivolts
		case 2: newp10 = 0.000001f;
			break; // microvolts
		default: newp10 = 1.0f;
			break;
		}
	}

	if (newp10 != m_p10)
	{
		// convert content of control edit printing scaled values
		m_xcenter = (m_xcenter * m_p10) / newp10;
		m_xmax = (m_xmax * m_p10) / newp10;
		m_xmin = (m_xmin * m_p10) / newp10;
		m_p10 = newp10;
	}
}

void CDataViewOrdinatesDlg::OnSelchangeVertunits()
{
	m_iUnit = static_cast<CComboBox*>(GetDlgItem(IDC_VERTUNITS))->GetCurSel();
	ChangeUnits(m_iUnit, TRUE);
	UpdateData(FALSE);
}

void CDataViewOrdinatesDlg::OnSelchangeChanselect()
{
	if (m_bChanged)
		SaveChanlistData(m_Channel); // save current data
	const auto new_index = static_cast<CComboBox*>(GetDlgItem(IDC_CHANSELECT))->GetCurSel();
	m_Channel = new_index; // new chan selected
	LoadChanlistData(m_Channel); // load new data from lineview
	ChangeUnits(m_iUnit, FALSE); // convert params
	UpdateData(FALSE);
}

void CDataViewOrdinatesDlg::OnKillfocusVertcenter()
{
	auto diff = m_xcenter;
	UpdateData(TRUE);
	if (diff == m_xcenter)
		return;
	diff = m_xcenter - diff;
	m_xmax += diff;
	m_xmin += diff;

	CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(m_Channel);
	const auto vper_bin = chan->GetVoltsperDataBin();
	const auto xzero = ((m_xmax + m_xmin) / 2.f) * m_p10;
	const auto zero = static_cast<int>(xzero / vper_bin) + chan->GetDataBinZero();
	chan->SetYzero(zero);
	m_bChanged = TRUE;
	UpdateData(FALSE);
}

void CDataViewOrdinatesDlg::OnKillfocusVertMxMi()
{
	UpdateData(TRUE);

	// save into lineview and reload to change scale and voltsperpixel
	CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(m_Channel);
	const auto vper_bin = chan->GetVoltsperDataBin();
	const auto xzero = ((m_xmax + m_xmin) / 2.f) * m_p10;
	const auto xextent = (m_xmax - m_xmin) * m_p10;
	const auto zero = static_cast<int>(xzero / vper_bin) + chan->GetDataBinZero();
	const auto extent = static_cast<int>(xextent / vper_bin);

	chan->SetYzero(zero);
	chan->SetYextent(extent);
	LoadChanlistData(m_Channel);
	ChangeUnits(m_iUnit, FALSE);

	m_bChanged = TRUE;
	UpdateData(FALSE);
}

void CDataViewOrdinatesDlg::OnCancel()
{
	// restore extent of the channels
	m_nChanmax = m_pChartDataWnd->GetChanlistSize(); // nb of data channels
	auto j = 0;
	for (auto i = 0; i < m_nChanmax; i++)
	{
		CChanlistItem* chan = m_pChartDataWnd->GetChanlistItem(i);
		chan->SetYzero(m_settings[j]);
		j++;
		chan->SetYextent(m_settings[j]);
		j++;
	}
	CDialog::OnCancel();
}

void CDataViewOrdinatesDlg::OnOK()
{
	// trap CR to validate current field
	switch (GetFocus()->GetDlgCtrlID())
	{
	// select was on OK
	case 1:
		{
			const auto b_check = static_cast<CButton*>(GetDlgItem(IDC_CHECKALL))->GetCheck();
			// exit, do not store changes of current channel -- nothing was changed
			if (m_bChanged || b_check)
				SaveChanlistData(m_Channel);
			CDialog::OnOK();
		}
		break;
	// trap return
	case IDC_CHANSELECT:
		break;
	case IDC_VERTCENTER:
		OnKillfocusVertcenter();
		break;
	case IDC_VERTUNITS:
		break;
	case IDC_VERTMAX:
	case IDC_VERTMIN:
		OnKillfocusVertMxMi();
		break;
	default:
		CDialog::OnOK();
		break;
	}
}
