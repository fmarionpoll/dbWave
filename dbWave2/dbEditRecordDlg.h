#pragma once

// CdbEditRecordDlg dialog

class CdbWaveDoc;
class CdbMainTable;

class CdbEditRecordDlg : public CDialog
{
	DECLARE_DYNAMIC(CdbEditRecordDlg)

public:
	CdbEditRecordDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CdbEditRecordDlg();

	CdbWaveDoc*		m_pdbDoc;
	CdbMainTable*	m_pSet;
	BOOL			m_bshowIDC_NEXT;
	BOOL			m_bshowIDC_PREVIOUS;

// Dialog Data
	enum { IDD = IDD_EDITRECORDDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void	PopulateControls();
	void	PopulateCombo_WithText(CDaoRecordset& linkedtableSet, CComboBox& combo, int iID);
	void	PopulateCombo_WithNumbers(CComboBox& combo, CArray<long, long>* pIDarray, long& lvar);
	void	UpdateSetFromCombo(CDaoRecordset& linkedtableSet, CComboBox& combo, long& iIDset);
	void	EditChangeItem_IndirectField(int IDC);
	void	EditChangeItem_MainField(int IDC);
	DB_ITEMDESC* GetItemDescriptors(int IDC);
	void	UpdateDatabaseFromDialog();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	CComboBox m_ctlexpt;
	CComboBox m_ctlinsectID;
	CComboBox m_ctlSensillumID;
	CComboBox m_ctlstim;
	CComboBox m_ctlconc;
	CComboBox m_ctlstim2;
	CComboBox m_ctlconc2;
	CComboBox m_ctlinsect;
	CComboBox m_ctlsensillum;
	CComboBox m_ctllocation;
	CComboBox m_ctlOperator;
	CComboBox m_ctlpathdat;
	CComboBox m_ctlpathspk;
	CComboBox m_ctlstrain;
	CComboBox m_ctlsex;
	CComboBox m_ctlrepeat;
	CComboBox m_ctlrepeat2;
	CComboBox m_ctlflag;
	
	CString m_cs_more;
	CString m_csnameDat;
	CString m_csnameSpk;
	
	afx_msg void OnBnClickedButtoninsectid();
	afx_msg void OnBnClickedButtonsensillumid();
	afx_msg void OnBnClickedButtonstimulus();
	afx_msg void OnBnClickedButtonconcentration();
	afx_msg void OnBnClickedButtonstimulus2();
	afx_msg void OnBnClickedButtonconcentration2();
	afx_msg void OnBnClickedButtoninsectname();
	afx_msg void OnBnClickedButtonstrain();
	afx_msg void OnBnClickedButtonsex();
	afx_msg void OnBnClickedButtonsensillum();
	afx_msg void OnBnClickedButtonlocation();
	afx_msg void OnBnClickedButtonoperator();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedSynchrosingle();
	afx_msg void OnBnClickedSynchroall();
	afx_msg void OnBnClickedPrevious();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedButtonrepeat();
	afx_msg void OnBnClickedButtonrepeat2();
	afx_msg void OnBnClickedButtonflag();
	afx_msg void OnBnClickedButtonexpt2();
};
