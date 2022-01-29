#pragma once
#include "dtacq32.h"


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

	CDTAcq32*	m_pAnalogIN = nullptr;
	CDTAcq32*	m_pAnalogOUT = nullptr;
	CDTAcq32*	m_pDTAcq32 = nullptr;
	short		m_subssystemIN = 0;
	short		m_subsystemelementIN = 0;
	
private:
	UINT		m_atodCount = 0;
	UINT		m_dtoaCount = 0;
	int			m_nsubsystems = 0;
	CString		m_boardName;


protected:
	void DoDataExchange(CDataExchange* pDX) override;
	// Implementation
	BOOL OnInitDialog() override;
	BOOL FindDTOpenLayersBoards();
	int  GetBoardCapabilities();
	void GetSubsystemYNCapabilities(int numitems);
	void GetSubsystemNumericalCapabilities(int numitems);
	void ChangeSubsystem(int index);
	void DispatchException(COleDispatchException* e);

	afx_msg void OnSelchangeBoard();
	afx_msg void OnLbnSelchangeListBoardcaps();

	DECLARE_MESSAGE_MAP()

};
