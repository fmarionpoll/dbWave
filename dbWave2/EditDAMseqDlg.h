#pragma once
#include "afxwin.h"

class CEditDAMseqDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditDAMseqDlg)

public:
	CEditDAMseqDlg (CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditDAMseqDlg();

	// Dialog Data
	enum { IDD = IDD_DA_MSEQ };
	OUTPUTPARMS	m_outDParms;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CWnd*		m_pParent;

	DECLARE_MESSAGE_MAP()
public:

	long	m_mSeqRatio;
	double	m_mSeqAmplitude;
	double	m_mSeqOffset;
	long	m_mSeqDelay;
	long	m_mSeqSeed;
	CButton m_mSeqRandomSeed;

	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedMseqRandomSeed();
	afx_msg void OnEnChangeEdit1();
};
