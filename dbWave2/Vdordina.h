#pragma once

// vdordina.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDataViewOrdinatesDlg dialog

class CDataViewOrdinatesDlg : public CDialog
{
	// Construction
public:
	CDataViewOrdinatesDlg(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_ORDINATES };

	CComboBox m_chanSelect;
	int m_iUnit;
	float m_xcenter;
	float m_xmax;
	float m_xmin;

	ChartData* m_pChartDataWnd; // data Envelopes
	int m_nChanmax; // (global?) max nb of channels
	int m_Channel; // current channel
	BOOL m_bChanged; // a factor from current channel was changed

protected:
	float m_p10; // power of ten (multiplication factor to all parms)
	float m_voltsperpixel; // nb of volts per pixels, given current gain
	float m_VoltsperBin; // scale factor associated with current chan
	CWordArray m_settings; // storage for original values

	// Implementation
protected:
	void LoadChanlistData(int ilist); // load data from m_plinev
	void SaveChanlistData(int ilist); // change data to m_plinev
	void ChangeUnits(int newScale, BOOL bNew); // change power of ten
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Generated message map functions
	void OnOK() override;
	BOOL OnInitDialog() override;
	afx_msg void OnSelchangeVertunits();
	afx_msg void OnSelchangeChanselect();
	afx_msg void OnKillfocusVertMxMi();
	void OnCancel() override;
	afx_msg void OnKillfocusVertcenter();

	DECLARE_MESSAGE_MAP()
};
