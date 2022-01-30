#pragma once
#include "ChartData.h"


class DlgDataViewOrdinates : public CDialog
{
public:
	DlgDataViewOrdinates(CWnd* pParent = nullptr); 

	// Dialog Data
	enum { IDD = IDD_ORDINATES };

	CComboBox m_chanSelect;
	int m_iUnit{ -1 };
	float m_xcenter{ 0.f };
	float m_xmax{ 0.f };
	float m_xmin{ 0.f };

	ChartData* m_pChartDataWnd{ nullptr }; 
	int m_nChanmax{ 0 };
	int m_Channel{ 0 }; 
	BOOL m_bChanged{ false }; 

protected:
	float m_p10{ 0.f }; // power of ten (multiplication factor to all parms)
	float m_voltsperpixel{ 0.f }; // nb of volts per pixels, given current gain
	float m_VoltsperBin{ 0.f }; // scale factor associated with current chan
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
