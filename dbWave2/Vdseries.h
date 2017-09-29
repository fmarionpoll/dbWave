#pragma once

// vdseries.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDataSeriesDlg dialog

class CDataSeriesDlg : public CDialog
{
// Construction
public:
	CDataSeriesDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_DATASERIES };
	CButton	m_deleteseries;
	CButton	m_defineseries;
	CComboBox	m_transform;
	CComboBox	m_ordinates;
	CListBox	m_listseries;
	CString		m_name;
	int			m_ispan;
	CLineViewWnd*	m_lineview;	// data contours
	CAcqDataDoc*	m_pdbDoc;		// data document
	int				m_listindex;// same as lineview:Chanlist
	
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeListseries();
	afx_msg void OnClickedDeleteseries();
	afx_msg void OnClickedDefineseries();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelchangeTransform();
	afx_msg void OnEnChangeEdit1();
	DECLARE_MESSAGE_MAP()
};

