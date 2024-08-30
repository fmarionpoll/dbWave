#pragma once
#include "afxwin.h"
#include "output_parameters.h"

class DlgEditDAMseq : public CDialogEx
{
	DECLARE_DYNAMIC(DlgEditDAMseq)

public:
	DlgEditDAMseq(CWnd* p_parent = nullptr); // standard constructor
	~DlgEditDAMseq() override;

	// Dialog Data
	enum { IDD = IDD_DA_MSEQ };

	output_parameters m_out_d_parameters;
	long m_m_seq_ratio {0};
	long m_m_seq_delay {0};
	long m_m_seq_seed {0};
	CButton m_m_seq_random_seed;

protected:
	void DoDataExchange(CDataExchange* p_dx) override; // DDX/DDV support
	CWnd* m_p_parent_;

public:
	void OnOK() override;
	BOOL OnInitDialog() override;
	afx_msg void on_bn_clicked_m_seq_random_seed();

	DECLARE_MESSAGE_MAP()
};
