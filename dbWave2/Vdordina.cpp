// vdordina.cpp : implementation file
//

#include "stdafx.h"

#include "cscale.h"
#include "scopescr.h"
#include "lineview.h"

#include "resource.h"
#include "vdordina.h"

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
// voltsperpixel = extent * voltsperbin / lineview.Height()
//
// therefore when one parameter is changed, others are modified accordingly:
// (0=unchanged, 1=changed)
//	change ---------- center---max-----min---
//	chan				1		1		1
//	center				1		1		1
//	max					1		1		0
//	min					1		0		1



CDataViewOrdinatesDlg::CDataViewOrdinatesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataViewOrdinatesDlg::IDD, pParent)
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

/////////////////////////////////////////////////////////////////////////////
// CDataViewOrdinatesDlg message handlers



// --------------------------------------------------------------------------
// OnInitDialog
// --------------------------------------------------------------------------
BOOL CDataViewOrdinatesDlg::OnInitDialog()
{
	// save extent and zero before calling base class 
	// this is an awful patch. But necessary. For some reason really unclear to
	// me, the parameters (zero and extent) of channel 0 / chanlist / lineview 
	// are MODIFIED after calling the base class CDialog::OnInitDialog!!!!!!
	// therefore, I save all parameters just before and restore then after...
	int i;
	m_nChanmax = m_plinev->GetChanlistSize();		// nb of data channels
	for (i=0; i < m_nChanmax; i++)					// browse through all chans
	{
		m_settings.Add(m_plinev->GetChanlistYzero(i));	// save zero
		m_settings.Add(m_plinev->GetChanlistYextent(i));// save extent
	}

	// call to base class routine. Should be done in first according to Microsoft doc.
    CDialog::OnInitDialog();
	
	// load channel description CComboBox
	int j = 0;														// index to restore parms
	for (i=0; i < m_nChanmax; i++)									// browse through all chans again
	{
		m_plinev->SetChanlistYzero(i, m_settings.GetAt(j)); j++;		// restore zero
		m_plinev->SetChanlistYextent(i, m_settings.GetAt(j)); j++;	// restore extent
		m_chanSelect.AddString(m_plinev->GetChanlistComment(i));	// load comment/chan
	}
    m_chanSelect.SetCurSel(m_Channel);				// select chan zero
    
    // load chan data
    LoadChanlistData(m_Channel);					// compute data in volts
	m_iUnit = 1;									// select "mV" as unit within combo box
	((CComboBox*)GetDlgItem(IDC_VERTUNITS))->SetCurSel(m_iUnit);
    ChangeUnits(m_iUnit, TRUE);						// adapt data/volts -> mv and fill controls

    // update content of the edit controls
    UpdateData(FALSE);								// pass parameters to real controls
	return TRUE;
}


// --------------------------------------------------------------------------
// LoadChanData(int ID)
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::LoadChanlistData(int i)
{
	// compute max and min from zero and extent
	m_VoltsperBin =  m_plinev->GetChanlistVoltsperBin(i);
	float zero = (float) m_plinev->GetChanlistYzero(i);
	float extent =  (float) m_plinev->GetChanlistYextent(i);
	float binzero =  (float)0.0  /*m_plinev->GetChanlistBinZero(i)*/;

	m_xcenter = (zero/*- binzero*/) * m_VoltsperBin / m_p10;
	float xextent = (extent * m_VoltsperBin / m_p10)/2.f;
	m_xmin = m_xcenter - xextent;
	m_xmax = m_xcenter + xextent;

	m_bChanged = FALSE;						// tell updatedata to not save new params
}

// --------------------------------------------------------------------------
// SaveChanData(int ID)
// convert dialog box params into lineview parameters, change corresp vals
// in lineview, update curves display
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::SaveChanlistData(int indexlist)
{
	int bCheck = ((CButton*)GetDlgItem(IDC_CHECKALL))->GetCheck();
	int indexfirst = indexlist;				// prepare for loop (first chan)
	int indexlast = indexlist+1;			// ibid (ending chan)
	if (bCheck)								// if bCheck (TRUE) then
	{
		indexfirst = 0;						// loop through all channels
		indexlast = m_nChanmax;				// 0 to max
	}

	for (int j = indexfirst; j<indexlast; j++)
	{
		float VperBin =  m_plinev->GetChanlistVoltsperBin(j);
		float xzero = ((m_xmax + m_xmin)/2.f) * m_p10;
		float xextent = (m_xmax - m_xmin) * m_p10;
		int i = (int) (xzero/VperBin) + 	m_plinev->GetChanlistBinZero(j);
		m_plinev->SetChanlistYzero(j,i);		// change zero
		i = (int) (xextent /VperBin);
		m_plinev->SetChanlistYextent(j, i);	// change extent
	}
	m_plinev->Invalidate();
}

// --------------------------------------------------------------------------
// ChangeUnits(int iUnit, BOOL bNew)
// convert volts into unit chosen by the user
//		iUnit = 0 (volts), 1 (mV), 2 (µV)
//		bNew: TRUE=compute new m_p10; 
//			  FALSE(default)= keep old, set new
// save factor in m_p10
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::ChangeUnits(int iUnit, BOOL bNew)
{
	float newp10 = m_p10;
	if (bNew)
	{
		switch (iUnit)
		{
		case 0:	newp10 = 1.0f;break; 		// volts
		case 1:	newp10 = 0.001f;break;		// millivolts
		case 2:	newp10 = 0.000001f;break;	// microvolts
		default:newp10 = 1.0f;break;
		}
	}

	if (newp10 !=  m_p10)
	{
		// convert content of control edit printing scaled values
		m_xcenter = (m_xcenter*m_p10)/newp10;
		m_xmax = (m_xmax *m_p10)/newp10;
		m_xmin = (m_xmin *m_p10)/newp10;
		m_p10 = newp10;
	}
}

// --------------------------------------------------------------------------
// ChangeSelect -- vertical units: center, extent, max, min
// unit is V, mV or uV
// get param and update all other controls
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::OnSelchangeVertunits()
{
	m_iUnit = ((CComboBox*)GetDlgItem(IDC_VERTUNITS))->GetCurSel();
	ChangeUnits(m_iUnit, TRUE);
	UpdateData(FALSE);
}

// --------------------------------------------------------------------------
// ChangeSelect --  select another data channel
// modify center, max, min 
// PS:		no effect if "ALL" is checked
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::OnSelchangeChanselect()
{
	if (m_bChanged)
		SaveChanlistData(m_Channel);	// save current data
	int newIndex = ((CComboBox*)GetDlgItem(IDC_CHANSELECT))->GetCurSel();
	m_Channel = newIndex;				// new chan selected
	LoadChanlistData(m_Channel);		// load new data from lineview
	ChangeUnits(m_iUnit, FALSE);		// convert params
	UpdateData(FALSE);	
}

// --------------------------------------------------------------------------
// change center
// modifies max and min
// modif zero, extent unchanged
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::OnKillfocusVertcenter()
{
	float diff = m_xcenter;
	UpdateData(TRUE);
	if (diff == m_xcenter)
		return;
	diff = m_xcenter - diff;
	m_xmax += diff;
	m_xmin += diff;
	
	float VperBin =  m_plinev->GetChanlistVoltsperBin(m_Channel);
	float xzero = ((m_xmax + m_xmin)/2.f) * m_p10;	
	int zero = (int) (xzero/VperBin) + 	m_plinev->GetChanlistBinZero(m_Channel);
	m_plinev->SetChanlistYzero(m_Channel,zero);		// change zero

	// convert into current volts units
	m_bChanged = TRUE;
	UpdateData(FALSE);	
}

// --------------------------------------------------------------------------
// change max or min
// modifies center
// modif zero & extent
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::OnKillfocusVertMxMi()
{
	UpdateData(TRUE);

	// save into lineview and reload to change scale and voltsperpixel
	float VperBin =  m_plinev->GetChanlistVoltsperBin(m_Channel);
	float xzero = ((m_xmax + m_xmin)/2.f) * m_p10;
	float xextent = (m_xmax - m_xmin) * m_p10;
	int zero = (int) (xzero/VperBin) + 	m_plinev->GetChanlistBinZero(m_Channel);
	int extent = (int) (xextent /VperBin);

	m_plinev->SetChanlistYzero(m_Channel, zero);
	m_plinev->SetChanlistYextent(m_Channel, extent);
	LoadChanlistData(m_Channel);	// reload params
	ChangeUnits(m_iUnit, FALSE);	// convert into current volts units

	m_bChanged = TRUE;		
	UpdateData(FALSE);
}

// --------------------------------------------------------------------------
// OnCancel
// restore all lineview parameters and exit
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::OnCancel()
{
	// restore extent of the channels
	m_nChanmax = m_plinev->GetChanlistSize(); // nb of data channels
	int j = 0;
	for (int i = 0; i<m_nChanmax; i++)
	{
		m_plinev->SetChanlistYzero(i, (int) m_settings[j]);
		j++;
		m_plinev->SetChanlistYextent(i, (int) m_settings[j]);
		j++;
	}	
	CDialog::OnCancel();
}

// --------------------------------------------------------------------------
// OnOK
// save current settings and exit
// --------------------------------------------------------------------------
void CDataViewOrdinatesDlg::OnOK()
{
	// trap CR to validate current field
	switch (GetFocus()->GetDlgCtrlID())
	{
		// select was on OK
		case 1:
			{
			int bCheck = ((CButton*)GetDlgItem(IDC_CHECKALL))->GetCheck();
			// exit, do not store changes of current channel -- nothing was changed
			if (m_bChanged || bCheck)
				SaveChanlistData(m_Channel);
				// save current channel before if bModif && checked...
			}
			CDialog::OnOK();
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
