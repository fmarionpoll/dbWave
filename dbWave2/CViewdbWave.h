#pragma once

#include "DataListCtrl.h"
#include "Editctrl.h"
#include "CViewDao.h"


class CViewdbWave : public CViewDAO
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CViewdbWave)
	CViewdbWave();
	~CViewdbWave() override;

public:
	enum { IDD = IDD_VIEWDBWAVE };

	// Attributes
public:
	CDataListCtrl m_dataListCtrl;

protected:
	CEditCtrl mm_spikeclass; // selected spike class
	CEditCtrl mm_timefirst; // first abcissa value
	CEditCtrl mm_timelast; // last abcissa value
	CEditCtrl mm_amplitudespan; // amplitude
	OPTIONS_VIEWDATA* m_options_viewdata = nullptr;

	BOOL m_bvalidDat = false;
	BOOL m_bvalidSpk = false;
	BOOL m_bAddMode = false;
	BOOL m_bFilterON = true;
	int m_dattransform = 0;
	float m_timefirst = 0.;
	float m_timelast = 0.;
	float m_amplitudespan = 0.;
	int m_spikeclass = 0;

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void updateControls();
	void fillListBox();

public:
	void DeleteRecords();

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
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
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
};
