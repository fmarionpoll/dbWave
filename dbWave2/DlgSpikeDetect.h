#pragma once

#include "ChartWnd.h"
#include "ChartData.h"
#include "spikepar.h"

class DlgSpikeDetect : public CDialog
{
	// Construction
public:
	DlgSpikeDetect(CWnd* pParent = nullptr);

	// Dialog Data
	enum { IDD = IDD_SPKDETECTPARM };

	CSpkDetectArray* m_pDetectSettingsArray{ nullptr }; // parameter set by caller
	AcqDataDoc* m_dbDoc{ nullptr }; 
	int m_iDetectParmsDlg{ 0 };
	OPTIONS_VIEWDATA* mdPM{ nullptr };
	ChartData* m_pChartDataDetectWnd{ nullptr };
	ChartData* m_pChartDataSourceWnd{ nullptr };

protected:
	SPKDETECTPARM* m_pspkD{ nullptr };
	int m_scancount{ 0 };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	void LoadChanParameters(int chan);
	void SaveChanParameters(int chan);
	void DisplayDetectFromChan();
	void SetDlgInterfaceState(int detectWhat);
	void UpdateSourceView();
	void ExchangeParms(int isource, int idest);
	void SetTabComment(int i, CString& cs);
	void UpdateTabShiftButtons();

	BOOL OnInitDialog() override;
	void OnOK() override;
	afx_msg void OnDetectfromtag();
	afx_msg void OnDetectfromchan();

public:
	CTabCtrl m_cParameterTabCtrl;
	afx_msg void OnTcnSelchangeParameterstab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedSpikesradio();
	afx_msg void OnBnClickedStimradio();
	afx_msg void OnBnClickedAddparambttn();
	afx_msg void OnBnClickedDelparambttn();
	afx_msg void OnEnChangeDetectthreshold();
	afx_msg void OnCbnSelchangeDetectchan();
	afx_msg void OnCbnSelchangeDetecttransform();
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCbnSelchangeExtractchan();
	afx_msg void OnEnChangeComment();
	afx_msg void OnBnClickedShiftright();
	afx_msg void OnBnClickedShiftleft();

	DECLARE_MESSAGE_MAP()
};
