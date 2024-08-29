#pragma once
#include "afxwin.h"

// CDAChannelsDlg dialog

#define DA_SEQUENCEWAVE	0
#define DA_SINEWAVE		1
#define DA_SQUAREWAVE	2
#define DA_TRIANGLEWAVE	3
#define DA_MSEQWAVE		4
#define DA_NOISEWAVE	5
#define	DA_FILEWAVE		6
#define DA_CONSTANT		7
#define DA_LINEWAVE		8
#define DA_ZERO			9
#define DA_ONE			10

class DlgDAChannels : public CDialogEx
{
	DECLARE_DYNAMIC(DlgDAChannels)

public:
	DlgDAChannels(CWnd* p_parent = nullptr); 
	~DlgDAChannels() override;

	// Dialog Data
	enum { IDD = IDD_DA_CHANNELS };

	CArray<output_parameters, output_parameters> output_params_array;
	CArray<CComboBox*, CComboBox*> combobox_ptr_array;
	BOOL m_bChannel2;

protected:
	void DoDataExchange(CDataExchange* p_dx) override; 
	void edit_sequence(int i_id, int channel);
	void fill_combo(int channel);
	void select_combo_item_from_output_parameters_array(int i_val);
	void on_bn_clicked_button_source(int channel);
	void on_cbn_sel_change_combo_source(int i_val);

	static CString combo_text_[];
	static DWORD combo_val_[];

public:
	CIntervals m_stimulus_saved;
	float m_sampling_rate;
	int m_i_sel_digital;

	BOOL OnInitDialog() override;

	afx_msg void on_bn_clicked_ok();

	afx_msg void on_bn_clicked_check_chan0();
	afx_msg void on_bn_clicked_check_chan1();
	afx_msg void on_bn_clicked_check_chan2();

	afx_msg void on_cbn_sel_change_combo_source0();
	afx_msg void on_cbn_sel_change_combo_source1();
	afx_msg void on_cbn_sel_change_combo_source2();

	afx_msg void on_bn_clicked_button_source0();
	afx_msg void on_bn_clicked_button_source1();
	afx_msg void on_bn_clicked_button_source2();
	afx_msg void on_bn_clicked_button_source3();
	afx_msg void on_bn_clicked_button_source4();
	afx_msg void on_bn_clicked_button_source5();
	afx_msg void on_bn_clicked_button_source6();
	afx_msg void on_bn_clicked_button_source7();
	afx_msg void on_bn_clicked_button_source8();
	afx_msg void on_bn_clicked_button_source9();
	afx_msg void on_cbn_sel_change_combo_source3();
	afx_msg void on_cbn_sel_change_combo_source4();
	afx_msg void on_cbn_sel_change_combo_source5();
	afx_msg void on_cbn_sel_change_combo_source6();
	afx_msg void on_cbn_sel_change_combo_source7();
	afx_msg void on_cbn_sel_change_combo_source8();
	afx_msg void on_cbn_sel_change_combo_source9();

	DECLARE_MESSAGE_MAP()
};
