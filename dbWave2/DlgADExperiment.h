#pragma once
#include "afxeditbrowsectrl.h"
#include "dbWaveDoc.h"
#include "OPTIONS_ACQDATA.h"

class DlgADExperiment : public CDialog
{
	// Construction
public:
	DlgADExperiment(CWnd* pParent = nullptr); // standard constructor

	// Dialog Data
	enum { IDD = IDD_AD_EXPERIMENTDLG };

	CString		m_csBasename {};
	CString		m_csMoreComment{};
	UINT		m_exptnumber {0};
	UINT		m_insectnumber {0};
	CString		m_csPathname {};
	BOOL		m_bhidesubsequent{false};

	CComboBox	m_coStrain;
	CComboBox	m_coSex;
	CComboBox	m_coSensillum;
	CComboBox	m_coLocation;
	CComboBox	m_coOperator;
	CComboBox	m_coInsect;
	CComboBox	m_coStimulus;
	CComboBox	m_coConcentration;
	CComboBox	m_coStimulus2;
	CComboBox	m_coConcentration2;
	CComboBox	m_coRepeat;
	CComboBox	m_coRepeat2;
	CComboBox	m_coExpt;
	CMFCEditBrowseCtrl m_mfcBrowsePath;

	CString		m_szFileName{};
	BOOL		m_bFilename{false};

	// Overrides
public:
protected:
	void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support

public:
	OPTIONS_ACQDATA* m_pADC_options{nullptr};
	BOOL			m_bADexpt{true};
	CWaveFormat*	m_pwaveFormat{nullptr};
	CdbWaveDoc*		m_pdbDoc{nullptr};
	BOOL			m_bEditMode{false};

protected:
	int SaveList(CComboBox* pCo, CStringArray* p_spike_element);
	void LoadList(CComboBox* pCo, CStringArray* p_spike_element, int isel, CdbTableAssociated* pmSet = nullptr);
	void EditComboBox(CComboBox* pCo);

	// Generated message map functions
	void OnOK() override;
	void OnCancel() override;
	BOOL OnInitDialog() override;

public:
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
	afx_msg void OnBnClickedButtonNextid();

	DECLARE_MESSAGE_MAP()
};
