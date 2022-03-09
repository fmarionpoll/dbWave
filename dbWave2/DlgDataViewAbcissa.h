#pragma once


class DlgDataViewAbcissa : public CDialog
{
	// Construction
public:
	DlgDataViewAbcissa(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_ABCISSA };

	float m_firstAbcissa{ 0.f };
	float m_frameDuration{ 0.f };
	float m_lastAbcissa{ 0.f };
	int m_abcissaUnitIndex{ -1 };
	float m_centerAbcissa{ 0.f };
	float m_abcissaScale{ 1.f };
	float m_veryLastAbcissa{ 1.f };
	int m_previousIndex{ 1 };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void CheckLimits();

	// Generated message map functions
	afx_msg void OnSelchangeAbcissaunits();
	void OnOK() override;
	afx_msg void OnKillfocusAbcissa();
	afx_msg void OnKillfocusDuration();
	afx_msg void OnKillfocusCenter();

	DECLARE_MESSAGE_MAP()
};
