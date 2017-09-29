#pragma once
#include "spikedoc.h"

// CEditStimArrayDlg dialog

class CEditStimArrayDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditStimArrayDlg)

public:
	CEditStimArrayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditStimArrayDlg();

	// data passed by caller
	CStimLevelSeries	m_stim;
	float				m_rate;
	CStimLevelSeries*	m_pstimsaved;

// Dialog Data
	enum { IDD = IDD_EDITSTIMULUS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CImageList*		m_pimagelist;
	CStretchControl m_stretch;
	BOOL			m_binit;
	CEdit			m_csEdit;
	int				m_iItem;
	CListCtrl		m_stimarrayCtrl;
	void			SelectItem(int i);
	void			ResetListOrder();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedEdit();
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedDelete3();
	float m_value;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedPaste();
};
