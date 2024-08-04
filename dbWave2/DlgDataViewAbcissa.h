#pragma once


class DlgDataViewAbscissa : public CDialog
{
	// Construction
public:
	DlgDataViewAbscissa(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_ABCISSA };

	float m_firstAbscissa{ 0.f };
	float m_frameDuration{ 0.f };
	float m_lastAbscissa{ 0.f };
	int m_abscissaUnitIndex{ -1 };
	float m_centerAbscissa{ 0.f };
	float m_abscissaScale{ 1.f };
	float m_veryLastAbscissa{ 1.f };
	int m_previousIndex{ 1 };

	// Implementation
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	BOOL OnInitDialog() override;
	void CheckLimits();

	// Generated message map functions
	afx_msg void OnSelchangeAbscissaunits();
	void OnOK() override;
	afx_msg void OnKillfocusAbscissa();
	afx_msg void OnKillfocusDuration();
	afx_msg void OnKillfocusCenter();

	DECLARE_MESSAGE_MAP()
};
