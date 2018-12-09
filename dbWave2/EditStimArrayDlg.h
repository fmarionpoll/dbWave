#pragma once
#include "Spikedoc.h"

// CEditStimArrayDlg dialog

class CEditStimArrayDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditStimArrayDlg)

public:
	CEditStimArrayDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CEditStimArrayDlg();

	// data passed by caller
	CPtrArray			m_pIntervalArrays;
	float				m_rate;
	CIntervalsAndLevels*	m_pstimsaved;

// Dialog Data
	enum { IDD = IDD_EDITSTIMULUS };

protected:
	CIntervalsAndLevels* m_pstim;

	virtual void	DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CImageList*		m_pimagelist;
	CStretchControl m_stretch;
	BOOL			m_binit;
	CEdit			m_csEdit;		// IDC_EDIT1
	float			m_value;		// IDC_EDIT1
	int				m_iItem;
	CListCtrl		m_stimarrayCtrl;// IDC_LISTSTIM	

	void			SelectItem(int i);
	void			ResetListOrder();
	void			TransferIntervalsArrayToDialogList(CIntervalsAndLevels* pstim);
	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedEdit();
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnBnClickedDelete();
	afx_msg void OnBnClickedInsert();
	afx_msg void OnBnClickedDelete3();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedPaste();
	afx_msg void OnBnClickedExport();
};
