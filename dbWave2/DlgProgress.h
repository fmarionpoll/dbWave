#pragma once


class DlgProgress : public CDialog
{
	// Construction / Destruction
public:
	DlgProgress(UINT nCaptionID = 0); // standard constructor
	~DlgProgress() override;

	BOOL Create(CWnd* pParent = nullptr);
	BOOL CheckCancelButton();
	// Progress Dialog manipulation
	void SetStatus(LPCTSTR lpszMessage);
	void SetRange(int nLower, int nUpper);
	int SetStep(int nStep);
	int SetPos(int nPos);
	int OffsetPos(int nPos);
	int StepIt();

	enum { IDD = IDD_PROGRESS };

	CProgressCtrl m_Progress;
public:
	BOOL DestroyWindow() override;
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	UINT m_nCaptionID{ IDP_PROGRESS_CAPTION };
	int m_nLower{ 0 };
	int m_nUpper{ 100 };
	int m_nStep{ 10 };

	BOOL m_bCancel{ false };
	BOOL m_bParentDisabled{ false };

	void ReEnableParent();

	void OnCancel() override;

	void OnOK() override
	{
	};
	void UpdatePercent(int nCurrent);
	void PumpMessages();

	// Generated message map functions
	BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
};
