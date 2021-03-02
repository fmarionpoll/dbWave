#pragma once


class CDlgProgress : public CDialog
{
	// Construction / Destruction
public:
	CDlgProgress(UINT nCaptionID = 0);   // standard constructor
	~CDlgProgress();

	BOOL Create(CWnd* pParent = nullptr);

	// Checking for Cancel button
	BOOL CheckCancelButton();
	// Progress Dialog manipulation
	void SetStatus(LPCTSTR lpszMessage);
	void SetRange(int nLower, int nUpper);
	int  SetStep(int nStep);
	int  SetPos(int nPos);
	int  OffsetPos(int nPos);
	int  StepIt();

	// Dialog Data
	enum { IDD = IDD_PROGRESS };
	CProgressCtrl	m_Progress;

	// Overrides
public:
	virtual BOOL DestroyWindow();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	UINT m_nCaptionID;
	int m_nLower;
	int m_nUpper;
	int m_nStep;

	BOOL m_bCancel;
	BOOL m_bParentDisabled;

	void ReEnableParent();

	virtual void OnCancel();
	virtual void OnOK() {};
	void UpdatePercent(int nCurrent);
	void PumpMessages();

	// Generated message map functions
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
