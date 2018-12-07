#pragma once
#include "afxwin.h"

class CEditDAMseqDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEditDAMseqDlg)

public:
	CEditDAMseqDlg (CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditDAMseqDlg();

	// Dialog Data
	enum { IDD = IDD_DA_MSEQ };
	OUTPUTPARMS	m_outDParms;
	long		m_mSeqRatio;
	long		m_mSeqDelay;
	long		m_mSeqSeed;
	CButton		m_mSeqRandomSeed;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CWnd*		m_pParent;

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedMseqRandomSeed();
};
