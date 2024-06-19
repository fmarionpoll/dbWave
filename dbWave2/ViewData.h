#pragma once

#include "RulerBar.h"
#include "ScrollBarEx.h"
#include "afxwin.h"
#include "ChartData.h"
#include "Editctrl.h"
#include "dbTableView.h"


class ViewData : public dbTableView
{
protected:
	DECLARE_DYNCREATE(ViewData)
	ViewData();

	// Form Data
public:
	enum { IDD = IDD_VIEWDATA };

	int m_channel_selected = 0;
	float m_first_Hz_cursor = 0.;
	float m_second_Hz_cursor = 0.;
	float m_difference_second_minus_first = 0.;
	float m_time_first_abcissa = 0.;
	float m_time_last_abcissa = 0.;
	float m_floatNDigits = 1000.; // 10(000) -> n digits displayed
	BOOL m_bInitComment = true;

	ChartData m_ChartDataWnd{}; 
	CEditCtrl mm_first_Hz_cursor; 
	CEditCtrl mm_second_Hz_cursor;
	CEditCtrl mm_difference_second_minus_first; 
	CEditCtrl mm_time_first_abcissa;
	CEditCtrl mm_time_last_abcissa;
	CComboBox m_comboSelectChan;
	RulerBar m_ADC_yRulerBar;
	RulerBar m_ADC_xRulerBar;

protected:
	// parameters related to data display and to document
	AcqDataDoc* m_p_dat_Doc = nullptr;
	BOOL m_b_valid_doc_ = false;
	float m_samplingRate = 1.;
	int m_cursor_state = 0;
	int m_VBar_pixel_ratio = 30; 
	int m_HBar_pixel_ratio = 10; 
	int m_current_file_index = 0;

	HICON m_hBias = nullptr;
	HICON m_hZoom = nullptr;
	int scan_count = 0;
	float channel_rate = 0.;

	OPTIONS_VIEWDATA* options_view_data = nullptr;
	OPTIONS_VIEWDATAMEASURE* mdMO = nullptr;

protected:
	void PrintFileBottomPage(CDC* p_dc, CPrintInfo* pInfo);
	CString convert_file_index(long l_first, long l_last);
	void ComputePrinterPageSize();
	CString get_file_infos();
	CString PrintBars(CDC* p_dc, CRect* rect);
	BOOL get_file_series_index_from_page(int page, int& file_number, long& l_first);
	BOOL PrintGetNextRow(int& file_number, long& l_first, long& very_last);
	void save_modified_file();
	void update_file_parameters(BOOL b_update_interface = TRUE);
	void update_channels_display_parameters();
	void chain_dialog(WORD i_id);
	int print_get_n_pages();

	BOOL m_bCommonScale = false;

protected:
	ScrollBarEx m_file_scroll_bar; // data position within file
	SCROLLINFO m_file_scroll_bar_infos{}; // infos for scrollbar
	int m_VBarMode = 0; // flag V scrollbar state
	CScrollBar m_scrolly; // V scrollbar

	void OnFileScroll(UINT nSBCode, UINT nPos);
	void OnGainScroll(UINT nSBCode, UINT nPos);
	void UpdateYExtent(int ichan, int yextent);
	void UpdateYZero(int ichan, int ybias);
	void OnBiasScroll(UINT nSBCode, UINT nPos);
	void UpdateGainScroll();
	void UpdateBiasScroll();
	void SetVBarMode(int bMode);
	void UpdateFileScroll();
	void UpdateLegends(int legends_options);
	void UpdateHZtagsVal();
	void set_cursor_associated_windows();
	void update_channel(int channel);
	void MeasureProperties(int item);

	// Overrides
protected:
	void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;
	void DoDataExchange(CDataExchange* pDX) override;
	void OnInitialUpdate() override;
	BOOL OnPreparePrinting(CPrintInfo* pInfo) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnPrint(CDC* p_dc, CPrintInfo* pInfo) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* pInfo) override;

	void define_sub_classed_items();
	void define_stretch_parameters();

	// Implementation
public:
	~ViewData() override;

	afx_msg void OnClickedBias();
	afx_msg void OnClickedGain();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFormatXscale();
	afx_msg void OnFormatYscale();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnToolsDataseries();
	afx_msg void ADC_OnHardwareChannelsDlg();
	afx_msg void ADC_OnHardwareIntervalsDlg();
	afx_msg void OnCenterCurve();
	afx_msg void OnGainAdjustCurve();
	afx_msg void OnSplitCurves();
	afx_msg void OnFirstFrame();
	afx_msg void OnLastFrame();
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnViewAlldata();
	afx_msg void OnFormatDataseriesattributes();
	afx_msg void OnToolsMeasuremode();
	afx_msg void OnToolsMeasure();
	afx_msg void OnToolsVerticaltags();
	afx_msg void OnToolsHorizontalcursors();
	afx_msg void OnUpdateToolsHorizontalcursors(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolsVerticaltags(CCmdUI* pCmdUI);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void ADC_OnHardwareDefineexperiment();
	afx_msg void OnEnChangeTimefirst();
	afx_msg void OnEnChangeTimelast();
	afx_msg void OnCbnSelchangeCombochan();

	DECLARE_MESSAGE_MAP()
};
