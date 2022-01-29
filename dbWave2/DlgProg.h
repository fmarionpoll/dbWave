#pragma once


class CDlgProgress : public CDialog
{
	// Construction / Destruction
public:
	CDlgProgress(UINT nCaptionID = 0); // standard constructor
	~CDlgProgress() override;

	BOOL Create(CWnd* pParent = nullptr);

	// Checking for Cancel button
	BOOL CheckCancelButton();
	// Progress Dialog manipulation
	void SetStatus(LPCTSTR lpszMessage);
	void SetRange(int nLower, int nUpper);
	int SetStep(int nStep);
	int SetPos(int nPos);
	int OffsetPos(int nPos);
	int StepIt();

	// Dialog Data
	enum { IDD = IDD_PROGRESS };

	CProgressCtrl m_Progress;

	// Overrides
public:
	BOOL DestroyWindow() override;
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

	// Implementation
protected:
	UINT m_nCaptionID;
	int m_nLower;
	int m_nUpper;
	int m_nStep;

	BOOL m_bCancel;
	BOOL m_bParentDisabled;

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
