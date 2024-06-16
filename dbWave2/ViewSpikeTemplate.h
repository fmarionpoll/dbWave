#pragma once
#include "ChartSpikeShape.h"
#include "dbTableView.h"
#include "Editctrl.h"
#include "OPTIONS_VIEWDATAMEASURE.h"
#include "SPKCLASSIF.h"
#include "TemplateListWnd.h"


class ViewSpikeTemplates : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewSpikeTemplates)
	ViewSpikeTemplates();
	~ViewSpikeTemplates() override;
public:
	enum { IDD = IDD_VIEWSPKTEMPLATES };

	float m_t1 = 0.f;
	float m_t2 = 6.f;
	float m_tunit = 1000.f; // 1=s, 1000f=ms, 1e6=us
	float m_timefirst = 0.;
	float m_timelast = 0.;
	int m_hitrate = 0;
	float m_ktolerance = 0.;
	int m_spikenoclass = 0;
	int m_hitratesort = 0;
	int m_ifirstsortedclass = 0;
	BOOL m_bAllFiles = false;
	BOOL m_ballTempl = false;
	BOOL m_ballSort = false;
	BOOL m_bDisplaySingleClass = false;
	CTabCtrl m_tab1Ctrl{};

protected:
	CEditCtrl mm_t1;
	CEditCtrl mm_t2;
	CEditCtrl mm_hitrate;
	CEditCtrl mm_hitratesort;
	CEditCtrl mm_ktolerance;
	CEditCtrl mm_spikenoclass;
	CEditCtrl mm_timefirst; // first abscissa value
	CEditCtrl mm_timelast; // last abscissa value
	CEditCtrl mm_ifirstsortedclass;

	CTemplateListWnd m_avgList{};
	CTemplateListWnd m_templList{};
	CTemplateListWnd m_avgAllList{};

	ChartSpikeShape m_ChartSpkWnd_Shape; // all spikes in displayspikes

	OPTIONS_VIEWDATA* mdPM = nullptr; // view data options
	OPTIONS_VIEWDATAMEASURE* mdMO = nullptr; // measure options
	SPKCLASSIF* m_psC = nullptr; // sort parameters
	SCROLLINFO m_scrollFilePos_infos{};
	long m_lFirst = 0;
	long m_lLast = 0;
	int m_spkformtagleft = 0;
	int m_spkformtagright = 0;
	int m_spikeno = -1;

public:
	void SetViewMouseCursor(int cursormode) { m_ChartSpkWnd_Shape.SetMouseCursorType(cursormode); }

public:
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;

	// Implementation
protected:
	void defineSubClassedItems();
	void defineStretchParameters();
	void updateFileParameters(); // reset parameters for new file
	void updateTemplates();
	void updateLegends();
	void selectSpike(short spikeno);
	void updateScrollBar();
	void selectSpikeList(int icur);
	void editSpikeClass(int controlID, int controlItem);
	void displayAvg(BOOL ballfiles, CTemplateListWnd* pTPList); //, CImageList* pImList);
	void sortSpikes();
	void updateCtrlTab1(int iselect);
	void setExtentZeroAllDisplay(int extent, int zero);
	void updateSpikeFile();
	float convertSpikeIndexToTime(int index) { return index * m_tunit / m_pSpkList->get_acq_sampling_rate(); }
	int convertTimeToSpikeIndex(float time) { return static_cast<int>(time * m_pSpkList->get_acq_sampling_rate() / m_tunit); }

public:
	// Generated message map functions
	afx_msg void OnEnChangeclassno();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnFormatCentercurve();
	afx_msg void OnBuildTemplates();
	afx_msg void OnEnChangeHitrate();
	afx_msg void OnEnChangeTolerance();
	afx_msg void OnEnChangeHitrateSort();
	afx_msg void OnKeydownTemplateList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheck1();
	afx_msg void OnDestroy();

	//afx_msg void OnAllClasses();
	//afx_msg void OnSingleClass();
	//afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnLButtonClickedTab(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnBnClickedSort();
	afx_msg void OnBnClickedDisplay();
	afx_msg void OnEnChangeIfirstsortedclass();

	afx_msg void OnTcnSelchangeTab2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTab2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedDisplaysingleclass();
	afx_msg void OnEnChangeT1();
	afx_msg void OnEnChangeT2();

	DECLARE_MESSAGE_MAP()
};
