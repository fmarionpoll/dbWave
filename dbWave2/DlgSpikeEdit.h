#pragma once

#include "ChartData.h"
#include "ChartSpikeShape.h"

class DlgSpikeEdit : public CDialog
{
	// Construction
public:
	DlgSpikeEdit(CWnd* pParent = nullptr);

	// Dialog Data
	enum { IDD = IDD_EDITSPIKE };

	int m_spikeclass{ 0 };
	int m_spikeno{ 0 };
	BOOL m_bartefact{ false };
	int m_displayratio{ 0 };
	int m_yvextent{ 0 };

	CEditCtrl mm_spikeno;
	CEditCtrl mm_spikeclass;
	CEditCtrl mm_displayratio;
	CEditCtrl mm_yvextent;

	CdbWaveDoc* m_pdbWaveDoc{ nullptr };
	int m_spikeChan{ 0 };
	CWnd* m_parent{ nullptr }; 
	int m_xextent{ 0 };
	int m_yextent{ 0 };
	int m_xzero{ 0 };
	int m_yzero{ 0 };
	BOOL m_bchanged{ 0 };

protected:
	CSpikeList* m_pSpkList{ nullptr }; // spike list
	AcqDataDoc* m_pAcqDatDoc{ nullptr }; // source data doc cDocument
	int m_spkpretrig{ 0 };
	int m_spklen{ 0 };
	int m_viewdatalen{ 0 };
	CDWordArray m_DWintervals; // intervals to highlight spikes / CLineviewWnd
	CChartSpikeShapeWnd m_SpkChartWnd; // all spikes in displayspikes
	ChartData m_ChartDataWnd; // source data
	long m_iitimeold{ 0 };
	long m_iitime{ 0 };
	CScrollBar m_HScroll;
	CScrollBar m_VScroll;
	SCROLLINFO m_HScroll_infos{}; // infos for scrollbar
	SCROLLINFO m_VScroll_infos{};

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	void LoadSpikeParms();
	void LoadSourceData();
	void LoadSpikeFromData(int shift);
	void UpdateSpikeScroll();

	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void OnEnChangeSpikeno();
	afx_msg void OnEnChangeSpikeclass();
	afx_msg void OnArtefact();
	afx_msg void OnEnChangeDisplayratio();
	afx_msg void OnEnChangeYextent();
	afx_msg void OnDestroy();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	DECLARE_MESSAGE_MAP()
};
