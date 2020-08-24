#pragma once

// vdabcissa.h : header file

/////////////////////////////////////////////////////////////////////////////
// CDataViewAbcissaDlg dialog

class CDataViewAbcissaDlg : public CDialog
{
	// Construction
public:
	CDataViewAbcissaDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ABCISSADLG };
	float	m_firstAbcissa;
	float	m_frameDuration;
	float	m_lastAbcissa;
	int		m_abcissaUnitIndex;
	float	m_centerAbcissa;
	float   m_abcissaScale;
	float	m_veryLastAbcissa;
	int		m_previousIndex;

	// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	void CheckLimits();

	// Generated message map functions
	afx_msg void OnSelchangeAbcissaunits();
	virtual void OnOK();
	afx_msg void OnKillfocusAbcissa();
	afx_msg void OnKillfocusDuration();
	afx_msg void OnKillfocusCenter();
	DECLARE_MESSAGE_MAP()
};
