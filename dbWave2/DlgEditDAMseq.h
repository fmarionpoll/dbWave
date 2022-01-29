#pragma once
#include "afxwin.h"
#include "OUTPUTPARMS.h"

class CDlgEditDAMseq : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEditDAMseq)

public:
	CDlgEditDAMseq(CWnd* pParent = nullptr); // standard constructor
	~CDlgEditDAMseq() override;

	// Dialog Data
	enum { IDD = IDD_DA_MSEQ };

	OUTPUTPARMS m_outDParms;
	long m_mSeqRatio;
	long m_mSeqDelay;
	long m_mSeqSeed;
	CButton m_mSeqRandomSeed;

protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
	CWnd* m_pParent;

public:
	void OnOK() override;
	BOOL OnInitDialog() override;
	afx_msg void OnBnClickedMseqRandomSeed();

	DECLARE_MESSAGE_MAP()
};
