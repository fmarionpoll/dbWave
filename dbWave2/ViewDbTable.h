#pragma once
#include <afxdao.h>
#pragma warning(disable : 4995)
#include "SpikeListTabCtrl.h"
#include "dbTableMain.h"
#include "StretchControls.h"
#include "dbWaveDoc.h"


class ViewDbTable : public CDaoRecordView
{
	DECLARE_DYNAMIC(ViewDbTable)

protected:
	ViewDbTable(LPCTSTR lpsz_template_name);
	ViewDbTable(UINT n_id_template);
	~ViewDbTable() override;

public:
	CdbTableMain* p_db_table_main{ nullptr };
	boolean m_auto_detect { false };
	boolean m_auto_increment {false};

	CdbWaveDoc* GetDocument();
	CDaoRecordset* OnGetRecordset() override;
	BOOL OnMove(UINT n_id_move_command) override;
	void OnDraw(CDC* p_dc) override;
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;

	CSpikeDoc* p_spk_doc { nullptr };
	SpikeList* p_spk_list{ nullptr };
	void save_current_spk_file();
	void increment_spike_flag();
	SpikeListTabCtrl spk_list_tab_ctrl {};

#ifdef _DEBUG
	void AssertValid() const override;
#ifndef _WIN32_WCE
	void Dump(CDumpContext& dc) const override;
#endif
#endif
	// print view
protected:
	CRect pixels_margin_;				
	int file_0_ {0};
	long l_first_0_{ 0 };
	long l_last0_{ 0 };
	int pixels_count_0_{ 0 };
	int files_count_{ 0 };
	int n_rows_per_page_{ 0 };	// USER: nb files/page
	long l_print_first_{ 0 };	
	long l_print_len_{ 0 };		// nb pts per line
	long print_first_{ 0 };
	long print_last_{ 0 };
	BOOL is_printing_ { false };
	CRect rect_data_;
	CRect rect_spike_;

	// printer parameters
	TEXTMETRIC text_metric_ {}; 
	LOGFONT log_font_ {}; 
	CFont* p_old_font_ { nullptr };
	CFont font_print_; 
	CRect print_rect_;

	BOOL OnPreparePrinting(CPrintInfo* p_info) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnPrint(CDC* p_dc, CPrintInfo* p_info) override;
	void OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactive_view) override;

	// parameters for OnSize
	CStretchControl stretch_ {};
	BOOL b_init_ {false};

protected:
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void on_nm_click_tab1(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void on_tcn_sel_change_tab1(NMHDR* p_nmhdr, LRESULT* p_result);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* ViewDbTable::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
