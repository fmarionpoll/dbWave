#pragma once

#include <oltypes.h>
#include <olerrors.h>
#include <Olmem.h>
#include <oldaapi.h>
#include "dtacq32.h"
#include "afxwin.h"

class CDataTranslationBoardDlg : public CDialog
{
	DECLARE_DYNAMIC(CDataTranslationBoardDlg)

public:
	CDataTranslationBoardDlg(CWnd* pParent = NULL);
	virtual ~CDataTranslationBoardDlg();
	enum {IDD= IDD_DTBOARD_DLG};

	CDTAcq32*	m_pAnalogIN;
private:
	UINT		m_atodCount;
	UINT		m_dtoaCount;
	UINT		m_dioCount;
	int			m_nsubsystems;
	CString		m_boardName;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	// Implementation
protected:
	virtual BOOL OnInitDialog();
	BOOL FindDTOpenLayersBoards();
	int  GetBoardCapabilities();
	void GetSubsystemYNCapabilities();
	void GetSubsystemNumericalCapabilities();

	afx_msg void OnSelchangeBoard();
	afx_msg void OnSelchangeSubsystem();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox	m_cbBoard;
	CComboBox	m_cbSubsystem;
	CListBox	m_listBoardCaps;
	CListBox	m_listSSNumCaps;
	CListBox	m_listSSYNCaps;
};

