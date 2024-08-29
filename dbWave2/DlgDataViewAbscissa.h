#pragma once


class DlgDataViewAbscissa : public CDialog
{
	// Construction
public:
	DlgDataViewAbscissa(CWnd* p_parent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_ABCISSA };

	float m_first_abscissa{ 0.f };
	float m_frame_duration{ 0.f };
	float m_last_abscissa{ 0.f };
	int m_abscissa_unit_index{ -1 };
	float m_center_abscissa{ 0.f };
	float m_abscissa_scale{ 1.f };
	float m_very_last_abscissa{ 1.f };
	int m_previous_index{ 1 };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* p_dx) override; 
	BOOL OnInitDialog() override;
	void check_limits();

	// Generated message map functions
	afx_msg void on_sel_change_abscissa_units();
	void OnOK() override;
	afx_msg void on_kill_focus_abscissa();
	afx_msg void on_kill_focus_duration();
	afx_msg void on_kill_focus_center();

	DECLARE_MESSAGE_MAP()
};
