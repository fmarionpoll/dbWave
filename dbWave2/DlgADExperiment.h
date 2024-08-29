#pragma once
#include "afxeditbrowsectrl.h"
#include "dbWaveDoc.h"
#include "options_input.h"

class DlgADExperiment : public CDialog
{
	// Construction
public:
	DlgADExperiment(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_AD_EXPERIMENT };

	CString		m_cs_basename {};
	CString		m_cs_more_comment{};
	UINT		m_experiment_number {0};
	UINT		m_insect_number {0};
	CString		m_cs_pathname {};
	BOOL		m_b_hide_subsequent{false};

	CComboBox	m_co_strain;
	CComboBox	m_co_sex;
	CComboBox	m_co_sensillum;
	CComboBox	m_co_location;
	CComboBox	m_co_operator;
	CComboBox	m_co_insect;
	CComboBox	m_co_stimulus;
	CComboBox	m_co_concentration;
	CComboBox	m_co_stimulus2;
	CComboBox	m_co_concentration2;
	CComboBox	m_co_repeat;
	CComboBox	m_co_repeat2;
	CComboBox	m_co_experiment;
	CMFCEditBrowseCtrl m_mfc_browse_path;

	CString		m_sz_file_name{};
	BOOL		m_b_filename{false};

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

public:
	options_input* 	options_input{nullptr};
	BOOL			b_ad_experiment{true};
	CWaveFormat*	p_wave_format{nullptr};
	CdbWaveDoc*		p_db_doc{nullptr};
	BOOL			b_edit_mode{false};

protected:
	static int save_list(CComboBox* p_co, CStringArray* p_spike_element);
	static void load_list(CComboBox* p_co, const CStringArray* p_spike_element, int i_sel, CdbTableAssociated* pm_set = nullptr);
	void edit_combo_box(CComboBox* p_co);

	// Generated message map functions
	void OnOK() override;
	void OnCancel() override;
	BOOL OnInitDialog() override;

public:
	afx_msg void on_bn_clicked_button_insect_name();
	afx_msg void on_bn_clicked_button_strain();
	afx_msg void on_bn_clicked_button_sex();
	afx_msg void on_bn_clicked_button_sensillum();
	afx_msg void on_bn_clicked_button_location();
	afx_msg void on_bn_clicked_button_operator();
	afx_msg void on_bn_clicked_button_stimulus();
	afx_msg void on_bn_clicked_button_concentration();
	afx_msg void on_bn_clicked_button_stimulus2();
	afx_msg void on_bn_clicked_button_concentration2();
	afx_msg void on_bn_clicked_button_repeat();
	afx_msg void on_bn_clicked_button_repeat2();
	afx_msg void on_bn_clicked_button_experiment();
	afx_msg void on_en_kill_focus_mfc_edit_browse1();
	afx_msg void on_bn_clicked_button_next_id();

	DECLARE_MESSAGE_MAP()
};
