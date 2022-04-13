#pragma once

#include "SpikeClassListBox.h"
#include "ScrollBarEx.h"
#include "dbTableView.h"
#include <ChartData.h>

#include "Editctrl.h"
#include "OPTIONS_VIEWDATAMEASURE.h"
#include "SPKCLASSIF.h"
#include "SPKDETECTPARM.h"

class ViewSpikes : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewSpikes)
	ViewSpikes();
	~ViewSpikes() override;

	// Form Data
public:
	enum { IDD = IDD_VIEWSPIKES };

	float m_time_first = 0.f;
	float m_time_last = 0.f;
	int m_spike_index = -1;
	int m_spike_index_class = 0;
	float m_zoom = .2f;
	int m_class_source = 0;
	int m_class_destination = 1;
	BOOL m_b_reset_zoom = true;
	BOOL m_b_artefact = false;
	BOOL m_b_keep_same_class = false;
	float m_jitter_ms = 1.f;

	// Attributes
protected:
	ChartData m_ChartDataWnd;
	SpikeClassListBox m_spkClassListBox;
	int m_max_classes = 1;

	CEditCtrl mm_spike_index;
	CEditCtrl mm_spike_index_class;
	CEditCtrl mm_time_first; 
	CEditCtrl mm_time_last; 
	CEditCtrl mm_zoom;
	CEditCtrl mm_source_class;
	CEditCtrl mm_dest_class;
	CEditCtrl mm_jitter_ms;

	ScrollBarEx m_file_scroll; 
	SCROLLINFO m_file_scroll_infos{}; 

	int m_zoom_factor = 1; 
	HICON m_hBias = nullptr;
	HICON m_hZoom = nullptr;
	float m_y_scale_factor = 0.f; 
	int m_VBarMode = 0; 
	CScrollBar m_scroll_y; 

	AcqDataDoc* m_pDataDoc = nullptr; 

	BOOL m_bInitSourceView = true;
	int m_lFirst = 0;
	int m_lLast = -1;

	CDWordArray m_DW_intervals; // intervals to highlight spikes
	BOOL m_b_add_spike_mode = false;

	int m_yWE = 1;
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
	BOOL addSpiketoList(long ii_time, BOOL bcheck_if_otheraround);
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
	void OnActivateView(BOOL bActivate, CView* pActivateView, CView* p_deactivate_view) override;

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
