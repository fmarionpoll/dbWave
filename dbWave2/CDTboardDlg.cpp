
#include "stdafx.h"
#include <oltypes.h>
#include <olerrors.h>
#include <Olmem.h>
#include "dbWave.h"

#include "CDtBoardDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ***********************************************************
// Note: The only known compatible boards for both A/D and D/A
// are the DT9812, DT9834 and DT9836
//************************************************************

BOOL CALLBACK GetDriver(LPSTR lpszName,LPSTR lpszEntry,LPARAM lParam )   
/* this is the callback function of olDaEnumBoards, it gets the 
strings of the Open Layers board and attempts to initialize
the board.  If successful, enumeration is halted. */
{
   BOARD* pBoard=(BOARD*)(LPVOID)lParam;
   pBoard->name = lpszName;
   pBoard->entry = lpszEntry;
   CString name = pBoard->name;
   PTSTR pname = name;
   pBoard->status = olDaInitialize(pname, &pBoard->hdrvr);
   if (pBoard->hdrvr != NULL)
      return FALSE;
   return TRUE;	
}

BOOL CALLBACK DassProc(LPSTR lpszName,OLSS OlSs,UINT uiElement,LPARAM lParam)
{
	// This is the callback function of olDaEnumSubSystems
	CComboBox* pSubs=(CComboBox*)(LPVOID)lParam;
	if (lpszName==NULL) {
		// Stop enumerating
		return FALSE;
	}
	CString subsystem_type[SUBSYSTEM_COUNT] = SUBSYSTEM_TYPE;
	OLSS subsystem_list[SUBSYSTEM_COUNT-1]  = SUBSYSTEM_LIST;
	char str[STRLEN];
	sprintf(str, "%s #%ld - ",lpszName,uiElement);

	CString subsystem_text = str;
	UINT i=0;
	for (i=0;i<SUBSYSTEM_COUNT-1;i++) {
		if (OlSs==subsystem_list[i]) {
			subsystem_text+=subsystem_type[i];
		}
	}
	// Last option is "Unknown"
	if (i==SUBSYSTEM_COUNT) {
		subsystem_text+=subsystem_type[i];
	}
	// Add the text to the combobox
	pSubs->AddString(subsystem_text);
	// Find its index
	int count=pSubs->GetCount();
	// Element number in upper WORD, subsystem type in lower WORD
	DWORD ss_info=(uiElement<<16)+OlSs;
	pSubs->SetItemData(count-1,ss_info);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDTBoardDlg dialog
CDTBoardDlg::CDTBoardDlg(CWnd* pParent)
	: CDialog(CDTBoardDlg::IDD, pParent)
{
	m_pParent=pParent;
}

void CDTBoardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOARD, m_cboBoard);
	DDX_Control(pDX, IDC_BOARD_INFO, m_boardInfo);
	DDX_Control(pDX, IDC_SUBSYSTEM, m_subsystem);
	DDX_Control(pDX, IDC_SS_CAP, m_subsystemCapability);
	DDX_Control(pDX, IDC_SS_NUMERICAL, m_ssNumerical);
}

BEGIN_MESSAGE_MAP(CDTBoardDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDTBoardDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDTBoardDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_BOARD, &CDTBoardDlg::OnSelchangeBoard)
	ON_CBN_SELCHANGE(IDC_SUBSYSTEM, &CDTBoardDlg::OnSelchangeSubsystem)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDTBoardDlg functions
CString CDTBoardDlg::GetBoardName()
{
	return m_strBoardName;
}

void CDTBoardDlg::SetBoardName(CString board_name)
{
	m_strBoardName=board_name;
}

/////////////////////////////////////////////////////////////////////////////
// CDTBoardDlg message handlers

BOOL CDTBoardDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_cboBoard.ResetContent();
	m_cboBoard.AddString(_T("No Board"));
	BOARD board;
	if (OLNOERROR==olDaEnumBoards(GetDriver,(LPARAM)&board)) {
		m_cboBoard.AddString(board.name);
	}
	if (m_cboBoard.GetCount()<=1) {
		MESSAGE(MESS_INFORM,"No data acquisition boards found");
		m_cboBoard.SetCurSel(0);
		return TRUE;
	}
	m_cboBoard.SelectString(0,board.name);
	// Get information about the initial selection
	OnSelchangeBoard();
	return TRUE;
}

void CDTBoardDlg::OnBnClickedOk()
{
	char board_name[MAX_BOARD_NAME_LENGTH];
	UINT board_index=m_cboBoard.GetCurSel();
	m_cboBoard.GetLBText(board_index,board_name);
	m_strBoardName=board_name;
	CDialog::OnOK();
}

void CDTBoardDlg::OnBnClickedCancel()
{
	m_strBoardName.Empty();
	CDialog::OnCancel();
}

void CDTBoardDlg::OnSelchangeBoard()
{
	// Clear the subsystem information
	m_subsystem.ResetContent();
	m_subsystem.AddString("No subsystems");
	m_subsystem.SetCurSel(0);
	m_subsystemCapability.SetWindowTextA("Nothing");
	m_ssNumerical.SetWindowTextA("Nothing");
	// Get the selected board
	BOARD board;
	CString board_text;
	char driver_name[MAX_BOARD_NAME_LENGTH];
	char model_name[MAX_BOARD_NAME_LENGTH];
	UINT instance_number;
	UINT board_index=m_cboBoard.GetCurSel();
	m_cboBoard.GetLBText(board_index,board.name);
	if (board_index==0 || board.name[0]==0) {
		m_boardInfo.SetWindowTextA("No board selected");
		return;
	}
	// Find the board's capabilities
	olDaGetBoardInfo(board.name,driver_name,model_name,MAX_BOARD_NAME_LENGTH,&instance_number);
	sprintf(m_str,"Driver: %s\nModel: %s\nBoard index: %d\n",driver_name,model_name,instance_number);
	board_text=m_str;
	OLDC capability[CAPABILITY_LIST_SIZE]=CAPABILITY_LIST;
	CString capability_text[CAPABILITY_LIST_SIZE]=CAPABILITY_TEXT;
	ECODE ec=olDaInitialize(board.name,&board.hdrvr);
	for (UINT i=0;i<CAPABILITY_LIST_SIZE;i++) {
		UINT number;
		ec=olDaGetDevCaps(board.hdrvr,capability[i],&number);
		sprintf(m_str,"%s %d\n",capability_text[i],number);
		board_text+=m_str;
	}
	m_boardInfo.SetWindowTextA(board_text);
	// List all the subsystems
	m_subsystem.ResetContent();
	ec=olDaEnumSubSystems(board.hdrvr,DassProc,(LPARAM)&m_subsystem);
	m_subsystem.SetCurSel(0);
	OnSelchangeSubsystem();
}

void CDTBoardDlg::OnSelchangeSubsystem()
{
	m_subsystemCapability.SetWindowTextA("Nothing");
	m_ssNumerical.SetWindowTextA("Nothing");
	// Get the board info
	BOARD board;
	UINT board_index=m_cboBoard.GetCurSel();
	m_cboBoard.GetLBText(board_index,board.name);
	if (board_index==0 || board.name[0]==0) {
		m_boardInfo.SetWindowTextA("No board selected");
		return;
	}
	char driver_name[MAX_BOARD_NAME_LENGTH];
	char model_name[MAX_BOARD_NAME_LENGTH];
	UINT instance_number;
	olDaGetBoardInfo(board.name,driver_name,model_name,MAX_BOARD_NAME_LENGTH,&instance_number);
	if (OLNOERROR!=olDaInitialize(board.name,&board.hdrvr)) {
		return;
	}
	// Get the selected subsystem
	int index=m_subsystem.GetCurSel();
	DWORD ss_info=m_subsystem.GetItemData(index);
	OLSS OlSs=(OLSS)(ss_info&0xffff);
	UINT uiElement=(ss_info>>16)&0xff;
	HDASS ss_handle;
	if (OLNOERROR!=olDaGetDASS(board.hdrvr,OlSs,uiElement,&ss_handle)) {
		// Try releasing it once
		olDaReleaseDASS(ss_handle);
		olDaReset(ss_handle);
		if (OLNOERROR!=olDaGetDASS(board.hdrvr,OlSs,uiElement,&ss_handle)) {
			// Give up
			return;
		}
	}
	// Subsystem yes/no capabilities
	CString capabilities;
	capabilities.Empty();
	OLSSC	olssc[SUB_COUNT]=SUB_CAP;
	CString	cap_text[SUB_COUNT]=SUB_TEXT;
	for (UINT i=0;i<SUB_COUNT;i++) {
		UINT capability;
		if (OLNOERROR==olDaGetSSCaps(ss_handle,olssc[i],&capability)) {
			if (capability!=0) {
				capabilities+=cap_text[i];
				capabilities+="\n";
			}
		}
	}
	m_subsystemCapability.SetWindowTextA(capabilities);
	// Subsystem numerical capabilities
	capabilities.Empty();
	OLSSC	olssc_num[SS_NUM_COUNT]=SS_NUM_CAP;
	CString	num_text[SS_NUM_COUNT]=SS_NUM_TEXT;
	for (UINT i=0;i<SS_NUM_COUNT;i++) {
		UINT capability;
		if (OLNOERROR==olDaGetSSCaps(ss_handle,olssc_num[i],&capability)) {
			if (capability!=0) {
				sprintf(m_str,"%s=%ld\n",num_text[i],capability);
				capabilities+=m_str;
			}
		}
	}
	m_ssNumerical.SetWindowTextA(capabilities);
	olDaReleaseDASS(ss_handle);
}
