#pragma once

// CADInputParmsDlg dialog

#include "CyberAmp.h"
#include "CUSBPxxS1Ctl.h"
#include "GridCtrl\GridCtrl.h"

class CADInputParmsDlg : public CDialog
{
	DECLARE_DYNAMIC(CADInputParmsDlg)

public:
	CADInputParmsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CADInputParmsDlg();
	
// Dialog Data
	enum { IDD = IDD_AD_INPUTPARMSDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
// number of input channels
	int m_nacqchans;				// number of acquisition channels
	int m_maxchans;					// max nb for channel input number (0-7 or 0-15; exception for channel 16=DIN)
	int m_inputlistmax;				// dt9800 = 32 (set when creating the object)
	CSize m_OldSize;				// used to resize the form

public:
	CGridCtrl m_Grid;

// parameters passed:
public:
	// input/output data:
	CWaveFormat*	m_pwFormat;		// acquisition parameters
	CWaveChanArray*	m_pchArray;		// acquisition channels
	BOOL			m_bchainDialog;	// chain dialog (no= FALSE)
	// parameter set on exit to chain dialog
	UINT m_postmessage;				// launch assoc dialog

	BOOL m_bchantype;				// flag TRUE=single ended; false=differential - default: false
	int m_numchansMAXDI;			// = m_Analog.GetSSCaps(OLSSC_MAXDICHANS); default = 8
	int m_numchansMAXSE;			// = m_Analog.GetSSCaps(OLSSC_MAXSECHANS); default = 16
	BOOL m_bcommandAmplifier;		// change ampli settings on the fly (if present); default = none
	CUSBPxxS1Ctl* 		m_pAlligatorAmplifier;
	CPtrArray*			m_pAlligatorDevicePtrArray;

	
	// Implementation
protected:
	static TCHAR* pszRowTitle[];
	static TCHAR* pszHighPass[];
	static TCHAR* pszADGains[];
	static TCHAR* pszAmplifier[];
	static TCHAR* pszProbeType[];
	static TCHAR* pszEncoding[];
	static int   iEncoding[];

	int		m_rowADchannel;
	int		m_row_ADgain;
	int		m_row_headstagegain;
	int		m_row_ampgain;
	int		m_row_readonly;
	UINT	m_iNBins;
	float	m_xVoltsMax;

	BOOL	InitGridColumnDefaults(int col);
	void	AdjustGridSize();
	void	InitGridColumnReadOnlyFields(int col);
	void	InitADchannelCombo(int col, int iselect);
	void	LoadChanData(int i);
	void	SaveChanData(int i);
	void	SaveData();
	void	SetAmplifierParms(int col);
	void	GetAmplifierParms(int col);

public:
	virtual BOOL OnInitDialog();
	
	CComboBox m_resolutionCombo;		// A/D resolution: 8, 12, 16 bits (?)
	CComboBox m_encodingCombo;			// encoding mode (binary offset/straight/2's complement)
	
	afx_msg void OnEnChangeNacqchans();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedSingleended();
	afx_msg void OnBnClickedDifferential();
	afx_msg void OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnCbnSelchangeResolution();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedAdintervals();
};
