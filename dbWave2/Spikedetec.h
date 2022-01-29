#pragma once

#include "ChartWnd.h"
#include "chartdata.h"
#include "spikepar.h"

class CSpikeDetectDlg : public CDialog
{
	// Construction
public:
	CSpikeDetectDlg(CWnd* pParent = nullptr);

	// Dialog Data
	enum { IDD = IDD_SPKDETECTPARM };

	CSpkDetectArray* m_pDetectSettingsArray; // parameter set by caller
	AcqDataDoc* m_dbDoc; // parameter set by caller
	int m_iDetectParmsDlg; // parameter set by caller
	OPTIONS_VIEWDATA* mdPM; // browse options
	ChartData* m_pChartDataDetectWnd;
	ChartData* m_pChartDataSourceWnd;

protected:
	SPKDETECTPARM* m_pspkD;
	int m_scancount;

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
