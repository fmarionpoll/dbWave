#pragma once


class DlgProgress : public CDialog
{
	// Construction / Destruction
public:
	DlgProgress(UINT n_caption_id = 0); // standard constructor
	~DlgProgress() override;

	BOOL Create(CWnd* p_parent = nullptr);
	BOOL check_cancel_button();

	// Progress Dialog manipulation
	void set_status(LPCTSTR lpsz_message) const;
	void set_range(int n_lower, int n_upper);
	int set_step(int n_step);
	int set_pos(int n_pos);
	int offset_pos(int n_pos);
	int step_it();

	enum { IDD = IDD_PROGRESS };

	CProgressCtrl m_progress;
public:
	BOOL DestroyWindow() override;
protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support

	// Implementation
protected:
	UINT m_n_caption_id_{ IDP_PROGRESS_CAPTION };
	int m_n_lower_{ 0 };
	int m_n_upper_{ 100 };
	int m_n_step_{ 10 };

	BOOL m_b_cancel_{ false };
	BOOL m_b_parent_disabled_{ false };

	void re_enable_parent();

	void OnCancel() override;

	void OnOK() override
	{
	};
	void update_percent(int n_current) const;
	void pump_messages();

	// Generated message map functions
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
