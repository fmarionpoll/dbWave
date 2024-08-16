#pragma once
#include <afxdao.h>
#pragma warning(disable : 4995)
#include "CSpkListTabCtrl.h"
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
	CSpkListTabCtrl spk_list_tab_ctrl {};

#ifdef _DEBUG
	void AssertValid() const override;
#ifndef _WIN32_WCE
	void Dump(CDumpContext& dc) const override;
#endif
#endif
	// print view
protected:
	CRect m_margin_;				// margins (pixels)
	int m_file_0_ {0};				// current file
	long m_l_first_0_{ 0 };
	long m_l_last0_{ 0 };
	int m_pixels_count_0_{ 0 };
	int m_files_count_{ 0 };		// nb of files in doc
	int m_nb_rows_per_page_{ 0 };	// USER: nb files/page
	long m_l_print_first_{ 0 };		// file index of first pt
	long m_l_print_len_{ 0 };		// nb pts per line
	long m_print_first_{ 0 };
	long m_print_last_{ 0 };
	BOOL m_b_is_printing_ { false };
	CRect rect_data_;
	CRect rect_spike_;

	// printer parameters
	TEXTMETRIC m_t_metric_ {}; 
	LOGFONT m_log_font_ {}; 
	CFont* m_p_old_font_ { nullptr };
	CFont m_font_print_; 
	CRect m_print_rect_;

	BOOL OnPreparePrinting(CPrintInfo* p_info) override;
	void OnBeginPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnEndPrinting(CDC* p_dc, CPrintInfo* p_info) override;
	void OnPrint(CDC* p_dc, CPrintInfo* p_info) override;
	void OnActivateView(BOOL b_activate, CView* p_activate_view, CView* p_deactive_view) override;

	// parameters for OnSize
	CStretchControl m_stretch_ {};
	BOOL m_b_init_ {false};

protected:
	afx_msg void OnSize(UINT n_type, int cx, int cy);
	afx_msg void OnNMClickTab1(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg void OnTcnSelchangeTab1(NMHDR* p_nmhdr, LRESULT* p_result);

	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dataView.cpp
inline CdbWaveDoc* ViewDbTable::GetDocument()
{
	return (CdbWaveDoc*)m_pDocument;
}
#endif
