#pragma once

#include "DataListCtrl.h"
#include "dbMainTable.h"
#include "ChartData.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "Editctrl.h"
#include "CSpkListTabCtrl.h"
#include "CViewDao.h"

//class CdbWaveCntrItem;
//class CdbMainTable;
//class CLineViewWnd;
//class CChartSpikeBarWnd;
//class CChartSpikeShapeWnd;

class CViewdbWave : public CViewDAO
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CViewdbWave)
	CViewdbWave();

public:
	enum { IDD = IDD_VIEWDBWAVE };

	// Attributes
public:
	CDataListCtrl		m_dataListCtrl;
	CSpkListTabCtrl		m_tabCtrl;
	BOOL				m_bvalidDat = false;
	BOOL				m_bvalidSpk = false;
	// Overrides
protected:
	virtual void		DoDataExchange(CDataExchange* pDX) override;
	virtual void		OnInitialUpdate() override;
	virtual void		OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;
	virtual void		OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	// Implementation
public:
	virtual				~CViewdbWave();

protected:
	BOOL				m_bAddMode = false;
	BOOL				m_bFilterON = true;
	int					m_dattransform = 0;
	float				m_timefirst = 0.;
	float				m_timelast = 0.;
	float				m_amplitudespan = 0.;
	int					m_spikeclass = 0;

	CEditCtrl			mm_spikeclass;		// selected spike class
	CEditCtrl			mm_timefirst;		// first abcissa value
	CEditCtrl			mm_timelast;		// last abcissa value
	CEditCtrl			mm_amplitudespan;	// amplitude
	OPTIONS_VIEWDATA*	m_options_viewdata = nullptr;

	void				updateControls();
	void				fillListBox();

public:
	void				DeleteRecords();
	
	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRecordPageup();
	afx_msg void OnRecordPagedown();
	afx_msg void OnClickMedianFilter();
	afx_msg void OnItemActivateListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkListctrl(NMHDR* pNMHDR, LRESULT* pResult);
public:
	afx_msg void OnLvnColumnclickListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedDisplaySpikes();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnEnChangeAmplitudespan();
	afx_msg void OnBnClickedCheckfilename();
	afx_msg void OnHdnEndtrackListctrl(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedRadioallclasses();
	afx_msg void OnBnClickedRadiooneclass();
	afx_msg void OnEnChangeSpikeclass();

	afx_msg void OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTab1(NMHDR* pNMHDR, LRESULT* pResult);
};
