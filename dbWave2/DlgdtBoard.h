#pragma once

#include "./include/DataTranslation/OLDAAPI.H"
#include "./include/DataTranslation/OLTYPES.H"
#include "./include/DataTranslation/OLERRORS.H"
#include "./include/DataTranslation/Olmem.h"
#include "dtacq32.h"
#include "afxwin.h"

class CDlgDataTranslationBoard : public CDialog
{
	DECLARE_DYNAMIC(CDlgDataTranslationBoard)

public:
	CDlgDataTranslationBoard(CWnd* pParent = nullptr);
	virtual ~CDlgDataTranslationBoard();
	enum { IDD = IDD_DTBOARD_DLG };

	CComboBox	m_cbBoard;
	CListBox	m_listBoardCaps;
	CListBox	m_listSSNumCaps;
	CListBox	m_listSSYNCaps;

	CDTAcq32* m_pAnalogIN;
	short		m_subssystemIN;
	short		m_subsystemelementIN;
	CDTAcq32* m_pAnalogOUT;
	CDTAcq32* m_pDTAcq32;

private:
	UINT		m_atodCount;
	UINT		m_dtoaCount;
	int			m_nsubsystems;
	CString		m_boardName;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	// Implementation
protected:
	virtual BOOL OnInitDialog();
	BOOL FindDTOpenLayersBoards();
	int  GetBoardCapabilities();
	void GetSubsystemYNCapabilities(int numitems);
	void GetSubsystemNumericalCapabilities(int numitems);
	void ChangeSubsystem(int index);

	afx_msg void OnSelchangeBoard();
	afx_msg void OnLbnSelchangeListBoardcaps();

	DECLARE_MESSAGE_MAP()

};
