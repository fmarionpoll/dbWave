#pragma once
#include "ChartData.h"
#include "ChartSpikeBar.h"
#include "ChartSpikeShape.h"
#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "./Controls/cdxCRotBevelLine.h"
#include "dbTableView.h"
#include "Editctrl.h"


class ViewSpikeDetection : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewSpikeDetection)
	ViewSpikeDetection();

	// Form Data
public:
	enum { IDD = IDD_VIEWSPKDETECTION };

	RulerBar m_xspkdscale;
	CComboBox m_CBdetectWhat;
	CComboBox m_CBdetectChan;
	CComboBox m_CBtransform;
	CComboBox m_CBtransform2;
	float m_timefirst = 0.f;
	float m_timelast = 0.f;
	CString m_datacomments{};
	int m_spike_index = -1;
	BOOL m_bartefact = false;
	float m_thresholdval = 0.f;
	int m_ichanselected = 0;
	int m_ichanselected2 = 0;
	cdxCRotBevelLine m_bevel1;
	cdxCRotBevelLine m_bevel2;
	cdxCRotBevelLine m_bevel3;

	// form variables
protected:
	int m_scan_count_doc = -1;
	CDWordArray m_DWintervals;

	ChartData m_chart_data_filtered;
	ChartData m_chart_data_source;
	ChartSpikeBar m_chart_spike_bar;
	ChartSpikeShape m_chart_spike_shape;

	ScrollBarEx m_filescroll;
	SCROLLINFO m_filescroll_infos{};

	CEditCtrl mm_spikeno;
	CEditCtrl mm_thresholdval;
	CEditCtrl mm_timefirst;
	CEditCtrl mm_timelast;
	CEditCtrl mm_spkWndDuration;
	CEditCtrl mm_spkWndAmplitude;
	CEditCtrl mm_ichanselected;
	CEditCtrl mm_ichanselected2;

	int m_zoom_integer = 0;
	SPKDETECTARRAY* m_pArrayFromApp = nullptr;
	SpikeDetectArray m_spk_detect_array_current;
	SPKDETECTPARM* m_p_detect_parameters = nullptr;
	int m_i_detect_parameters = 0;

	OPTIONS_VIEWDATA* options_view_data = nullptr;
	OPTIONS_VIEWDATAMEASURE* options_view_data_measure = nullptr;

	float m_samplingRate = 0.f;
	float m_spkWndAmplitude = 1.f;
	float m_spkWndDuration = 6.f;
	BOOL m_bValidThreshold = false;
	BOOL m_bDetected = false;

	int m_cursor_state = 0;

public:
	void SetViewMouseCursor(int cursor_mode)
	{
		m_chart_spike_bar.set_mouse_cursor_type(cursor_mode);
		m_chart_spike_shape.set_mouse_cursor_type(cursor_mode);
		m_chart_data_filtered.set_mouse_cursor_type(cursor_mode);
		m_chart_data_source.set_mouse_cursor_type(cursor_mode);
	}

	// Implementation
protected:
	void detect_all(BOOL bAll); // detect from current set of parms or from all
	int	 detect_method_1(WORD channel_index); // spike detection, method 1 / m_spkD chan channel
	int  detect_stimulus_1(int channel_index); // stimulus detection
	void serialize_windows_state(BOOL save, int tab_index = -1);
	void align_display_to_current_spike();
	void highlight_spikes(BOOL flag = TRUE);
	void scroll_file(UINT nSBCode, UINT nPos);
	void select_spike_no(int spike_index, BOOL bMultipleSelection);
	BOOL check_detection_settings();

	void update_combo_box();
	void update_combos_detect_and_transforms();
	void define_stretch_parameters();
	void define_sub_classed_items();

	void update_detection_parameters();
	void update_detection_controls();
	void update_detection_settings(int iSelParms);
	void update_file_parameters(BOOL bUpdateInterface = TRUE);
	boolean update_data_file(BOOL bUpdateInterface);
	void update_spike_file(BOOL bUpdateInterface = TRUE);
	void update_file_scroll();
	void update_legends();
	void update_number_of_spikes();
	void update_legend_detection_wnd();
	void update_spike_shape_window_scale(BOOL b_set_from_controls = TRUE);
	void update_VT_tags();
	void update_spike_display();

	// public interface to view
public:
	// Overrides
	BOOL OnMove(UINT nIDMoveCommand) override;
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnActivateView(BOOL activate, CView* activated_view, CView* de_activated_view) override;

	// Implementation
public:
	~ViewSpikeDetection() override;
	// print view
protected:
	void PrintFileBottomPage(CDC* p_dc, const CPrintInfo* pInfo);
	CString PrintConvertFileIndex(long l_first, long l_last);
	void PrintComputePageSize();

	CString PrintGetFileInfos();
	CString PrintDataBars(CDC* p_dc, ChartData* pDataChartWnd, const CRect* prect);
	CString PrintSpkShapeBars(CDC* p_dc, const CRect* prect, BOOL bAll);
	void PrintCreateFont();
	BOOL PrintGetFileSeriesIndexFromPage(int page, int& file_number, long& l_first);
	BOOL PrintGetNextRow(int& file_index, long& l_first, long& very_last);
	int PrintGetNPages();
	void PrintDataCartridge(CDC* p_dc, ChartData* pDataChartWnd, CRect* prect, BOOL bComments);

	// gain and bias setting: data and functions
	HICON m_hBias = nullptr;
	HICON m_hZoom = nullptr;
	CScrollBar m_scrolly;
	float m_yscaleFactor = 0.f;
	int m_VBarMode = 0;

	HICON m_hBias2 = nullptr;
	HICON m_hZoom2 = nullptr;
	CScrollBar m_scrolly2;
	float m_yscaleFactor2 = 0.f;
	int m_VBarMode2 = 0;

	void OnGainScroll(UINT nSBCode, UINT nPos, int iID);
	void OnBiasScroll(UINT nSBCode, UINT nPos, int iID);
	void UpdateGainScroll(int iID);
	void UpdateBiasScroll(int iID);
	void SetVBarMode(short bMode, int iID);
	void update_tabs();

	// Generated message map functions
public:
	afx_msg void OnMeasureAll();
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFirstFrame();
	afx_msg void OnLastFrame();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSelchangeDetectchan();
	afx_msg void OnSelchangeTransform();
	afx_msg void OnSelchangeDetectMode();
	afx_msg void OnClear();
	afx_msg void OnEnChangeSpikeno();
	afx_msg void OnArtefact();
	afx_msg void OnFormatXscale();
	afx_msg void OnFormatAlldata();
	afx_msg void OnFormatYscaleCentercurve();
	afx_msg void OnFormatYscaleGainadjust();
	afx_msg void OnFormatSplitcurves();
	afx_msg void OnEnChangeThresholdval();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnToolsDetectionparameters();
	afx_msg void OnToolsEdittransformspikes();
	afx_msg void OnToolsDataseries();
	afx_msg void OnEditCopy();
	afx_msg void OnFileSave();
	afx_msg void OnMeasure();

	afx_msg void OnBnClickedBiasbutton();
	afx_msg void OnBnClickedGainbutton();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEnChangeSpkWndAmplitude();
	afx_msg void OnEnChangeSpkWndLength();
	afx_msg void OnBnClickedLocatebttn();

	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedClearAll();
	afx_msg void OnToolsEditstimulus();
	afx_msg void OnEnChangeChanselected();
	afx_msg void OnBnClickedGain2();
	afx_msg void OnBnClickedBias2();
	afx_msg void OnEnChangeChanselected2();
	afx_msg void OnCbnSelchangeTransform2();

	DECLARE_MESSAGE_MAP()
};
