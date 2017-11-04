
//////////////////////////////////////////////////////////////////////////////
// ADExperimentDlg dialog

#pragma once
#include "afxeditbrowsectrl.h"

class ADExperimentDlg : public CDialog
{
// Construction
public:
	ADExperimentDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	enum { IDD = IDD_AD_EXPERIMENTDLG };
	//CString	m_csComment;
	CString	m_csBasename;
	CString	m_csMoreComment;
	UINT	m_exptnumber;
	CString	m_csPathname;
	BOOL	m_bhidesubsequent;

	UINT m_IDinsect;
	BOOL m_bAuto;
	CComboBox m_coStrain;
	CComboBox m_coSex;
	CComboBox m_coSensillum;
	CComboBox m_coLocation;
	CComboBox m_coOperator;
	CComboBox m_coInsect;
	CComboBox m_coStimulus;
	CComboBox m_coConcentration;
	CComboBox m_coStimulus2;
	CComboBox m_coConcentration2;
	CComboBox m_coRepeat;
	CComboBox m_coRepeat2;
	CComboBox m_coExpt;
	CMFCEditBrowseCtrl m_mfcBrowsePath;	

	CString m_szFileName;
	BOOL	m_bFilename;

// Overrides
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation: set parameters 
	// data acquisition: set only parameter 1 (address of all parameters)
	// file: set all three parameters
public:
	OPTIONS_ACQDATA* m_pADC_options;		// data parameters (different options common)
	BOOL			m_bADexpt;		// set true if data acq / false if parmas from file
	CWaveFormat*	m_pwaveFormat;	// if data file, set this to file wave format
	CdbWaveDoc*		m_pdbDoc;
	BOOL			m_bEditMode;

protected:	
	int  SaveList(CComboBox* pCo, CStringArray* pS);
	void LoadList(CComboBox* pCo, CStringArray* pS, int isel, CDaoRecordset* pmSet = NULL);
	void EditComboBox(CComboBox* pCo);

	// Generated message map functions
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButtoninsectname();
	afx_msg void OnBnClickedButtonstrain();
	afx_msg void OnBnClickedButtonsex();
	afx_msg void OnBnClickedButtonsensillum();
	afx_msg void OnBnClickedButtonlocation();
	afx_msg void OnBnClickedButtonoperator();
	afx_msg void OnBnClickedButtonstimulus();
	afx_msg void OnBnClickedButtonconcentration();
	afx_msg void OnBnClickedButtonstimulus2();
	afx_msg void OnBnClickedButtonconcentration2();
	afx_msg void OnBnClickedButtonrepeat();
	afx_msg void OnBnClickedButtonrepeat2();
	afx_msg void OnBnClickedButtonexpt();
	afx_msg void OnEnKillfocusMfceditbrowse1();
};
