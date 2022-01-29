#pragma once
#include "afxwin.h"
#include "OUTPUTPARMS.h"

class CDlgEditDAMseq : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEditDAMseq)

public:
	CDlgEditDAMseq(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDlgEditDAMseq();

	// Dialog Data
	enum { IDD = IDD_DA_MSEQ };
	OUTPUTPARMS	m_outDParms;
	long		m_mSeqRatio;
	long		m_mSeqDelay;
	long		m_mSeqSeed;
	CButton		m_mSeqRandomSeed;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CWnd* m_pParent;

public:
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedMseqRandomSeed();

	DECLARE_MESSAGE_MAP()
};
