#pragma once
#include "dtacq32.h"


class DlgDataTranslationBoard : public CDialog
{
	DECLARE_DYNAMIC(DlgDataTranslationBoard)

public:
	DlgDataTranslationBoard(CWnd* p_parent = nullptr);
	~DlgDataTranslationBoard() override;

	enum { IDD = IDD_DTBOARD };

	CComboBox m_cb_board;
	CListBox m_list_board_caps;
	CListBox m_list_ss_num_caps;
	CListBox m_list_ss_caps;

	CDTAcq32* m_p_analog_in {nullptr} ;
	CDTAcq32* m_p_analog_out{ nullptr };
	CDTAcq32* m_p_dt_acq32{ nullptr };
	short m_subsystem_in {0};
	short m_subsystem_element_in {0};

private:
	//UINT m_a_to_d_count_ {0};
	//UINT m_d_to_a_count_ {0};
	int m_n_subsystems_ {0};
	CString m_board_name_;


protected:
	void DoDataExchange(CDataExchange* pDX) override;
	
	BOOL OnInitDialog() override;
	BOOL find_dt_open_layers_boards();
	int get_board_capabilities();
	void get_subsystem_yn_capabilities(int num_items);
	void get_subsystem_numerical_capabilities(int num_items);
	void change_subsystem(int index);
	static void DispatchException(COleDispatchException* e);

	afx_msg void on_sel_change_board();
	afx_msg void on_lbn_sel_change_list_board_caps();

	DECLARE_MESSAGE_MAP()
};
