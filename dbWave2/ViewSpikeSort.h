#pragma once
#include "ChartSpikeBar.h"
#include "ChartSpikeHist.h"
#include "ChartSpikeShape.h"
#include "ChartSpikeXY.h"
#include "dbTableView.h"
#include "Editctrl.h"
#include "ScrollBarEx.h"
#include "SPKCLASSIF.h"


class ViewSpikeSort : public dbTableView
{
	DECLARE_DYNCREATE(ViewSpikeSort)
protected:
	ViewSpikeSort();
	~ViewSpikeSort() override;

	// Form Data
public:
	enum { IDD = IDD_VIEWSPKSORT1 };

	CComboBox m_CBparameter;

	float m_t1 = 0.f;
	float m_t2 = 0.f;
	float m_tunit = 1000.f; // 1=s, 1000f=ms, 1e6=us
	float m_lower = 0.f;
	float m_upper = 0.f;
	int m_sourceclass = 0;
	int m_destinationclass = 0;
	float m_timeFirst = 0.f;
	float m_timeLast = 0.f;
	float m_mVMax = 0.f;
	float m_mVMin = 0.f;
	BOOL m_bAllFiles = false;
	int m_spikeno = -1;
	int m_spikenoclass = 0;
	float m_txyright = 1.f;
	float m_txyleft = 0.f;
	float m_mVbin = 0.01f;

	// Attributes
protected:
	ChartSpikeHist yhistogram_wnd_; // spike histogram
	ChartSpikeXY xygraph_wnd_; // points with spike height or different measures
	ChartSpikeShapeWnd m_ChartSpkWnd_Shape; // spike shapes
	ChartSpikeBar m_ChartSpkWnd_Bar; // bars with spike height

	CEditCtrl mm_t1;
	CEditCtrl mm_t2;
	CEditCtrl mm_lower;
	CEditCtrl mm_upper;
	CEditCtrl mm_sourceclass;
	CEditCtrl mm_destinationclass;
	CEditCtrl mm_timeFirst;
	CEditCtrl mm_timeLast;
	CEditCtrl mm_mVMin;
	CEditCtrl mm_mVMax;
	CEditCtrl mm_spikeno;
	CEditCtrl mm_spikenoclass;
	CEditCtrl mm_txyright;
	CEditCtrl mm_txyleft;
	CEditCtrl mm_mVbin;

	ScrollBarEx m_filescroll; // data position within file
	SCROLLINFO m_filescroll_infos{}; // infos for scrollbar

	SPKCLASSIF* m_psC{};
	OPTIONS_VIEWDATA* m_pOptionsViewData{};

	int m_itaglow{};
	int m_itagup{};
	int m_ixyright{};
	int m_ixyleft{};

	int m_spkhistupper{};
	int m_spkhistlower{};
	int m_spkformtagleft{};
	int m_spkformtagright{};

	float m_vunit = 1000.f; // 1=V, 1000f=mV, 1e6=uV
	float m_delta{};
	int m_parmmax{}; // max of array m_measure_y1
	int m_parmmin{}; // min of array m_measure_y1
	BOOL m_bvalidextrema = false; // tells if m_parmmax & m_parmmin are valid
	long m_lFirst{}; // display first
	long m_lLast{}; // display last
	BOOL m_bMeasureDone = false; // flag m_measure_y1 valid
	int m_divAmplitudeBy = 1; // value to adjust changes in amplitude / filter(s)

	// Operations
public:
	void SetViewMouseCursor(int cursormode)
	{
		xygraph_wnd_.SetMouseCursorType(cursormode);
		m_ChartSpkWnd_Shape.SetMouseCursorType(cursormode);
	}

	// Overrides
public:
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void defineSubClassedItems();
	void defineStretchParameters();
	void OnInitialUpdate() override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	// Implementation
protected:
	void updateFileParameters();
	void updateSpikeFile();
	void updateLegends();
	void selectSpikeFromCurrentList(int spikeno);
	void updateGain();
	void updateFileScroll();
	void scrollFile(UINT nSBCode, UINT nPos);
	void selectSpkList(int icursel);
	void activateMode4();
	void buildHistogram();
	void unflagAllSpikes();

	// Generated message map functions
public:
	afx_msg void OnSelchangeParameter();
	afx_msg void OnSort();
	afx_msg LRESULT OnMyMessage(WPARAM code, LPARAM lParam);
	afx_msg void OnMeasure();
	afx_msg void OnFormatAlldata();

	afx_msg void OnFormatCentercurve();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnSelectAllFiles();
	afx_msg void OnToolsAlignspikes();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	afx_msg void OnEnChangelower();
	afx_msg void OnEnChangeupper();
	afx_msg void OnEnChangeT1();
	afx_msg void OnEnChangeT2();
	afx_msg void OnEnChangeSourceclass();
	afx_msg void OnEnChangeDestinationclass();
	afx_msg void OnEnChangetimeFirst();
	afx_msg void OnEnChangetimeLast();
	afx_msg void OnEnChangemVMin();
	afx_msg void OnEnChangemVMax();
	afx_msg void OnEnChangeNOspike();
	afx_msg void OnEnChangeSpikenoclass();
	afx_msg void OnEnChangeEditleft2();
	afx_msg void OnEnChangeEditright2();
	afx_msg void OnEnChangeNBins();

	DECLARE_MESSAGE_MAP()
};
