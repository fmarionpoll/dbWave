#pragma once

#include <oltypes.h>
#include <olerrors.h>
#include <Olmem.h>
#include "dtacq32.h"

#define STRLEN	100				// general string length

class CDataTranslationBoardDlg : public CDialog
{
	DECLARE_DYNAMIC(CDataTranslationBoardDlg)

public:
	CDataTranslationBoardDlg(CWnd* pParent = NULL);
	virtual ~CDataTranslationBoardDlg();
	enum {IDD= IDD_DTBOARD_DLG};
	CComboBox	m_cboBoard;
	CStatic		m_boardInfo;
	CStatic		m_subsystems;
	CStatic		m_subsystemCapability;
	CComboBox	m_subsystem;

	CDTAcq32*	m_pAnalogIN;
	CDTAcq32*	m_pAnalogOUT;
	CDTAcq32*	m_pDigitalIN;
	CDTAcq32*	m_pDigitalOUT;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	// Implementation
protected:
	virtual BOOL OnInitDialog();
	BOOL FindDTOpenLayersBoards();
	afx_msg void OnBnClickedOk();
	afx_msg void OnSelchangeBoard();
	void GetBoardCapabilities();
	afx_msg void OnSelchangeSubsystem();

	DECLARE_MESSAGE_MAP()

private:
	UINT		m_atodCount;
	UINT		m_dtoaCount;
	UINT		m_dioCount;
	CWnd*		m_pParent;

	CString		m_boardName;
	char		m_str[STRLEN];
public:
	CStatic		m_ssNumerical;
};

