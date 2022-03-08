#pragma once

#include "SpikeClassListBox.h"
#include "ScrollBarEx.h"
#include "dbTableView.h"

class CViewSpikes : public CdbTableView
{
protected:
	DECLARE_DYNCREATE(CViewSpikes)
	CViewSpikes();
	~CViewSpikes() override;

	// Form Data
public:
	enum { IDD = IDD_VIEWSPIKES };

	float m_timefirst = 0.f;
	float m_timelast = 0.f;
	int m_spikeno = -1;
	int m_spikenoclass = 0;
	float m_zoom = .2f;
	int m_sourceclass = 0;
	int m_destclass = 1;
	BOOL m_bresetzoom = true;
	BOOL m_bartefact = false;
	BOOL m_bKeepSameClass = false;
	float m_jitter_ms = 1.f;

	// Attributes
protected:
	ChartData m_ChartDataWnd; // data display
	CSpikeClassListBox m_spkClassListBox; // listbox of spike classes
	int m_maxclasses = 1;

	CEditCtrl mm_spikeno;
	CEditCtrl mm_spikenoclass;
	CEditCtrl mm_timefirst; // first abcissa value
	CEditCtrl mm_timelast; // last abcissa value
	CEditCtrl mm_zoom; // zoom factor
	CEditCtrl mm_sourceclass;
	CEditCtrl mm_destclass;
	CEditCtrl mm_jitter_ms;

	CScrollBarEx m_filescroll; // data position within file
	SCROLLINFO m_filescroll_infos{}; // infos for scrollbar

	int m_zoominteger = 1; // zoom length (nb data acq points)
	HICON m_hBias = nullptr;
	HICON m_hZoom = nullptr;
	float m_yscaleFactor = 0.f; // div factor for y bar
	int m_VBarMode = 0; // flag V scrollbar state
	CScrollBar m_scrolly; // V scrollbar

	AcqDataDoc* m_pDataDoc = nullptr; // data document pointer

	BOOL m_bInitSourceView = true;
	int m_lFirst = 0;
	int m_lLast = -1;

	CDWordArray m_DWintervals; // intervals to highlight spikes
	BOOL m_baddspikemode = false;

	int m_yWE = 1; // offset and gain to display spikes
	int m_yWO = 0;
	int m_ptVT = -1;
	CRect m_rectVTtrack = CRect(0, 0, 0, 0);
	float m_jitter = 0.f;
	BOOL m_bdummy = TRUE;

	// Implementation
protected:
	void updateFileParameters(BOOL bUpdateInterface = TRUE);
	void updateLegends(BOOL bUpdateInterface);
	void updateDataFile(BOOL bUpdateInterface);
	void updateSpikeFile(BOOL bUpdateInterface);
	void updateGainScroll();
	void updateBiasScroll();
	void adjustYZoomToMaxMin(BOOL bForceSearchMaxMin);
	void selectSpike(int spikeno);
	void defineSubClassedItems();
	void defineStretchParameters();
	void zoomOnPresetInterval(int iistart);
	void scrollGain(UINT nSBCode, UINT nPos);
	void scrollBias(UINT nSBCode, UINT nPos);
	void setVBarMode(short bMode);
	BOOL addSpiketoList(long iitime, BOOL bcheck_if_otheraround);
	void setAddspikesMode(int mousecursorType);
	void selectSpkList(int icursel);
	void setTrackRectangle();
	void scrollFile(UINT nSBCode, UINT nPos);
	void updateFileScroll();
	void centerDataDisplayOnSpike(int spikeno);

	// public interface to view
public:
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;

	// page format printing parameters (pixel unit)
	OPTIONS_VIEWDATA* options_viewdata = nullptr; // view data options
	OPTIONS_VIEWDATAMEASURE* mdMO = nullptr; // measure options
	SPKCLASSIF* m_psC = nullptr;
	SPKDETECTPARM* m_pspkDP = nullptr; // spike detection parameters

protected:
	void PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo);
	CString PrintConvertFileIndex(long l_first, long l_last);
	void PrintComputePageSize();
	CString PrintGetFileInfos();
	CString PrintBars(CDC* p_dc, CRect* rect);
	long PrintGetFileSeriesIndexFromPage(int page, int* file);
	float PrintChangeUnit(float xVal, CString* xUnit, float* xScalefactor);

protected:
	// Generated message map functions
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatCentercurve();
	afx_msg void OnFormatGainadjust();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnEnChangeNOspike();
	afx_msg void OnEnChangeSpikenoclass();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditCopy();
	afx_msg void OnZoom();
	afx_msg void OnEnChangeZoom();
	afx_msg void OnEnChangeSourceclass();
	afx_msg void OnEnChangeDestclass();
	afx_msg void OnFormatPreviousframe();
	afx_msg void OnFormatNextframe();
	afx_msg void OnGAINbutton();
	afx_msg void OnBIASbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnArtefact();
	afx_msg void OnEnChangeJitter();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnHScrollLeft();
	afx_msg void OnHScrollRight();
	afx_msg void OnBnClickedSameclass();

	DECLARE_MESSAGE_MAP()
};
