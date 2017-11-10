#include "stdafx.h"
#include "dbWave.h"
#include "dtBoardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDataTranslationBoardDlg, CDialog)

CDataTranslationBoardDlg::CDataTranslationBoardDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataTranslationBoardDlg::IDD, pParent)
{
	m_pParent = pParent;
}

CDataTranslationBoardDlg::~CDataTranslationBoardDlg()
{
}

void CDataTranslationBoardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOARD, m_cboBoard);
	DDX_Control(pDX, IDC_BOARD_INFO, m_boardInfo);
	DDX_Control(pDX, IDC_SUBSYSTEM, m_subsystem);
	DDX_Control(pDX, IDC_SS_CAP, m_subsystemCapability);
	DDX_Control(pDX, IDC_SS_NUMERICAL, m_ssNumerical);
}

BEGIN_MESSAGE_MAP(CDataTranslationBoardDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_BOARD, &CDataTranslationBoardDlg::OnSelchangeBoard)
	ON_CBN_SELCHANGE(IDC_SUBSYSTEM, &CDataTranslationBoardDlg::OnSelchangeSubsystem)
END_MESSAGE_MAP()

BOOL CDataTranslationBoardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	FindDTOpenLayersBoards();

	// Get information about the initial selection
	OnSelchangeBoard();
	return TRUE;
}

BOOL CDataTranslationBoardDlg::FindDTOpenLayersBoards()
{
	m_cboBoard.ResetContent();
	UINT uiNumBoards = m_pAnalogIN->GetNumBoards();
	BOOL flag = (uiNumBoards > 0 ? TRUE : FALSE);
	if (uiNumBoards == 0)
		m_cboBoard.AddString(_T("No Board"));
	else
	{
		for (UINT i = 0; i < uiNumBoards; i++)
			m_cboBoard.AddString(m_pAnalogIN->GetBoardList(i));
	}
	int isel = 0;
	m_cboBoard.SetCurSel(isel);
	return flag;
}

void CDataTranslationBoardDlg::OnBnClickedOk()
{
}

void CDataTranslationBoardDlg::OnSelchangeBoard()
{
	int isel = m_cboBoard.GetCurSel();
	m_cboBoard.GetLBText(isel, m_boardName);

	// Clear the subsystem information
	m_subsystem.ResetContent();
	m_subsystem.AddString(_T("No subsystems"));
	m_subsystem.SetCurSel(0);
	m_subsystemCapability.SetWindowText (_T("Nothing"));
	m_ssNumerical.SetWindowText(_T("Nothing"));

	GetBoardCapabilities();

	
	//// List all the subsystems
	//m_subsystem.ResetContent();
	//ec = olDaEnumSubSystems(board.hdrvr, DassProc, (LPARAM)&m_subsystem);
	//m_subsystem.SetCurSel(0);
	//OnSelchangeSubsystem();
}

void CDataTranslationBoardDlg::GetBoardCapabilities()
{
	//// Find the board's capabilities
	//olDaGetBoardInfo(board.name, driver_name, model_name, MAX_BOARD_NAME_LENGTH, &instance_number);
	//sprintf(m_str, "Driver: %s\nModel: %s\nBoard index: %d\n", driver_name, model_name, instance_number);
	//board_text = m_str;
	//OLDC capability[CAPABILITY_LIST_SIZE] = CAPABILITY_LIST;
	//CString capability_text[CAPABILITY_LIST_SIZE] = CAPABILITY_TEXT;
	//ECODE ec = olDaInitialize(board.name, &board.hdrvr);
	//for (UINT i = 0; i<CAPABILITY_LIST_SIZE; i++) {
	//	UINT number;
	//	ec = olDaGetDevCaps(board.hdrvr, capability[i], &number);
	//	sprintf(m_str, "%s %d\n", capability_text[i], number);
	//	board_text += m_str;
	//}
	//m_boardInfo.SetWindowTextA(board_text);
}

void CDataTranslationBoardDlg::OnSelchangeSubsystem()
{
}


