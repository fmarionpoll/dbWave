#include "StdAfx.h"
#include "resource.h"
#include "CDTBoardParameters.h"
#include "DlgdtBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDlgDataTranslationBoard, CDialog)

CDlgDataTranslationBoard::CDlgDataTranslationBoard(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgDataTranslationBoard::IDD, pParent)
{
}

CDlgDataTranslationBoard::~CDlgDataTranslationBoard()
{
	const UINT ui_num_boards = m_pDTAcq32->GetNumBoards();
	if (ui_num_boards > 0) {
		m_pAnalogIN->SetSubSysType(m_subssystemIN);
	}
}

void CDlgDataTranslationBoard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOARD, m_cbBoard);
	DDX_Control(pDX, IDC_LIST_BOARDCAPS, m_listBoardCaps);
	DDX_Control(pDX, IDC_LIST_SSNUM, m_listSSNumCaps);
	DDX_Control(pDX, IDC_LIST_SSCAPS, m_listSSYNCaps);
}

BEGIN_MESSAGE_MAP(CDlgDataTranslationBoard, CDialog)
	ON_CBN_SELCHANGE(IDC_BOARD, &CDlgDataTranslationBoard::OnSelchangeBoard)
	ON_LBN_SELCHANGE(IDC_LIST_BOARDCAPS, &CDlgDataTranslationBoard::OnLbnSelchangeListBoardcaps)
END_MESSAGE_MAP()

BOOL CDlgDataTranslationBoard::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_subssystemIN = m_pAnalogIN->GetSubSysType();
	m_pDTAcq32 = m_pAnalogIN;
	FindDTOpenLayersBoards();
	OnSelchangeBoard();
	return TRUE;
}

BOOL CDlgDataTranslationBoard::FindDTOpenLayersBoards()
{
	m_cbBoard.ResetContent();
	const UINT ui_num_boards = m_pDTAcq32->GetNumBoards();
	const auto flag = (ui_num_boards > 0 ? TRUE : FALSE);
	if (ui_num_boards == 0)
		m_cbBoard.AddString(_T("No Board"));
	else
	{
		for (UINT i = 0; i < ui_num_boards; i++)
			m_cbBoard.AddString(m_pDTAcq32->GetBoardList(i));
	}
	m_cbBoard.SetCurSel(0);
	return flag;
}

void CDlgDataTranslationBoard::OnSelchangeBoard()
{
	const auto isel = m_cbBoard.GetCurSel();
	m_cbBoard.GetLBText(isel, m_boardName);
	m_nsubsystems = GetBoardCapabilities();
}

#define SS_LIST_SIZE 6
#define	SS_CODES {OLSS_AD, OLSS_DA, OLSS_DIN, OLSS_DOUT, OLSS_CT, OLSS_SRL}
#define	SS_LIST	 {OLDC_ADELEMENTS, OLDC_DAELEMENTS, OLDC_DINELEMENTS,OLDC_DOUTELEMENTS, OLDC_CTELEMENTS, OLDC_SRLELEMENTS}
#define SS_TEXT	 {_T("Analog Inputs"), _T("Analog outputs"), _T("Digital Inputs"),_T("Digital Outputs"),	_T("Counter/Trigger"), _T("Serial Port")}

int CDlgDataTranslationBoard::GetBoardCapabilities()
{
	int nsubsystems = 0;

	try {
		m_pDTAcq32->SetBoard(m_boardName);
		m_listBoardCaps.ResetContent();

		nsubsystems = m_pDTAcq32->GetNumSubSystems();

		CString subsystem_text[SS_LIST_SIZE] = SS_TEXT;
		int ss_list[SS_LIST_SIZE] = SS_LIST;

		for (auto i = 0; i < SS_LIST_SIZE; i++)
		{
			const int number = m_pDTAcq32->GetDevCaps(ss_list[i]);
			CString cs;
			cs.Format(_T("\n  %i: "), number);
			auto board_text = cs + subsystem_text[i];
			m_listBoardCaps.AddString(board_text);
			m_listBoardCaps.SetItemData(i, number);
		}
	}
	catch (COleDispatchException* e)
	{
		CString my_error;
		my_error.Format(_T("DT-Open Layers Error: %i "), static_cast<int>(e->m_scError));
		my_error += e->m_strDescription;
		AfxMessageBox(my_error);
		e->Delete();
		return nsubsystems;
	}
	return nsubsystems;
}

void CDlgDataTranslationBoard::ChangeSubsystem(int index)
{
	int ss_codes[SS_LIST_SIZE] = SS_CODES;
	const DWORD ss_info = ss_codes[index];
	const int numitems = m_listBoardCaps.GetItemData(index);
	const auto ol_ss = (ss_info & 0xffff);
	//UINT ui_element = (ss_info >> 16) & 0xff;
	if (numitems > 0 && static_cast<unsigned long>(m_pDTAcq32->GetSubSysType()) != ol_ss) {
		try {
			if (ol_ss == static_cast<unsigned long>(m_pAnalogOUT->GetSubSysType()))
				m_pDTAcq32 = m_pAnalogOUT;
			else
				m_pDTAcq32 = m_pAnalogIN;
			m_pDTAcq32->SetSubSysType(static_cast<short>(ol_ss));
			m_pDTAcq32->SetSubSysElement(0);
		}
		catch (COleDispatchException* e)
		{
			CString myError;
			myError.Format(_T("DT-Open Layers Error: %i "), static_cast<int>(e->m_scError));
			myError += e->m_strDescription;
			AfxMessageBox(myError);
			e->Delete();
			return;
		}
	}
	GetSubsystemYNCapabilities(numitems);
	GetSubsystemNumericalCapabilities(numitems);
}

void CDlgDataTranslationBoard::GetSubsystemYNCapabilities(int numitems)
{
	m_listSSYNCaps.ResetContent();
	if (numitems > 0) {
		OLSSC	olssc[SUB_COUNT] = SUB_CAP;
		CString	cap_text[SUB_COUNT] = SUB_TEXT;
		try {
			for (UINT i = 0; i < SUB_COUNT; i++)
				if (m_pDTAcq32->GetSSCaps(olssc[i]) != 0)
					m_listSSYNCaps.AddString(cap_text[i]);
		}
		catch (COleDispatchException* e)
		{
			CString my_error;
			my_error.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); my_error += e->m_strDescription;
			AfxMessageBox(my_error);
			e->Delete();
		}
	}
	m_listSSYNCaps.Invalidate();
}

void CDlgDataTranslationBoard::GetSubsystemNumericalCapabilities(int numitems)
{
	m_listSSNumCaps.ResetContent();
	if (numitems > 0) {
		OLSSC	olssc_num[SS_NUM_COUNT] = SS_NUM_CAP;
		CString	num_text[SS_NUM_COUNT] = SS_NUM_TEXT;
		try {
			for (UINT i = 0; i < SS_NUM_COUNT; i++)
			{
				const UINT capability = m_pDTAcq32->GetSSCaps(olssc_num[i]);
				if (capability != 0)
				{
					CString cs;
					cs.Format(_T(": %d"), capability);
					m_listSSNumCaps.AddString(num_text[i] + cs);
				}
			}
		}
		catch (COleDispatchException* e)
		{
			CString my_error;
			my_error.Format(_T("DT-Open Layers Error: %i "), (int)e->m_scError); my_error += e->m_strDescription;
			AfxMessageBox(my_error);
			e->Delete();
		}
	}
	m_listSSNumCaps.Invalidate();
}

void CDlgDataTranslationBoard::OnLbnSelchangeListBoardcaps()
{
	const auto isel = m_listBoardCaps.GetCurSel();
	if (isel >= 0) {
		ChangeSubsystem(isel);
	}
}