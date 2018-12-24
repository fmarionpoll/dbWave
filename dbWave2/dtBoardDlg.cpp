#include "StdAfx.h"
#include "resource.h"
#include "dbWave.h"
#include "dtBoardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CDataTranslationBoardDlg, CDialog)

CDataTranslationBoardDlg::CDataTranslationBoardDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataTranslationBoardDlg::IDD, pParent), m_subssystemIN(0), m_subsystemelementIN(0), m_atodCount(0),
	  m_dtoaCount(0)
{
	m_pDTAcq32 = nullptr;
	m_pAnalogIN = nullptr;
	m_pAnalogOUT = nullptr;
	m_nsubsystems = 0;
}

CDataTranslationBoardDlg::~CDataTranslationBoardDlg()
{
	const UINT ui_num_boards = m_pDTAcq32->GetNumBoards();
	if (ui_num_boards > 0) {
		m_pAnalogIN->SetSubSysType(m_subssystemIN);
	}
}

void CDataTranslationBoardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOARD, m_cbBoard);
	DDX_Control(pDX, IDC_LIST_BOARDCAPS, m_listBoardCaps);
	DDX_Control(pDX, IDC_LIST_SSNUM, m_listSSNumCaps);
	DDX_Control(pDX, IDC_LIST_SSCAPS, m_listSSYNCaps);
}

BEGIN_MESSAGE_MAP(CDataTranslationBoardDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_BOARD, &CDataTranslationBoardDlg::OnSelchangeBoard)
	ON_LBN_SELCHANGE(IDC_LIST_BOARDCAPS, &CDataTranslationBoardDlg::OnLbnSelchangeListBoardcaps)
END_MESSAGE_MAP()

BOOL CDataTranslationBoardDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_subssystemIN			= m_pAnalogIN->GetSubSysType();

	m_pDTAcq32 = m_pAnalogIN;
	FindDTOpenLayersBoards();
	OnSelchangeBoard();
	return TRUE;
}

BOOL CDataTranslationBoardDlg::FindDTOpenLayersBoards()
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


void CDataTranslationBoardDlg::OnSelchangeBoard()
{
	const auto isel = m_cbBoard.GetCurSel();
	m_cbBoard.GetLBText(isel, m_boardName);
	m_nsubsystems = GetBoardCapabilities(); 
}

#define SS_LIST_SIZE 6
#define	SS_CODES {OLSS_AD, OLSS_DA, OLSS_DIN, OLSS_DOUT, OLSS_CT, OLSS_SRL}
#define	SS_LIST	 {OLDC_ADELEMENTS, OLDC_DAELEMENTS, OLDC_DINELEMENTS, OLDC_DOUTELEMENTS, OLDC_CTELEMENTS, OLDC_SRLELEMENTS}
#define SS_TEXT	 {	_T("Analog Inputs"), _T("Analog outputs"), _T("Digital Inputs"), _T("Digital Outputs"),	_T("Counter/Trigger"), _T("Serial Port")}

int CDataTranslationBoardDlg::GetBoardCapabilities()
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


void CDataTranslationBoardDlg::ChangeSubsystem(int index)
{
	int ss_codes[SS_LIST_SIZE] = SS_CODES;
	const DWORD ss_info = ss_codes[index];
	const int numitems= m_listBoardCaps.GetItemData(index);
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

#define	SUB_COUNT	36
#define	SUB_CAP		{OLSSC_SUP_AUTO_CALIBRATE, OLSSC_SUP_SINGLEVALUE, OLSSC_SUP_CONTINUOUS, OLSSC_SUP_CONTINUOUS_PRETRIG, OLSSC_SUP_CONTINUOUS_ABOUTTRIG, OLSSC_SUP_RANDOM_CGL, OLSSC_SUP_SEQUENTIAL_CGL, OLSSC_SUP_ZEROSEQUENTIAL_CGL, OLSSC_SUP_SIMULTANEOUS_SH, OLSSC_SUP_SIMULTANEOUS_START, OLSSC_SUP_SYNCHRONIZATION, OLSSC_SUP_SIMULTANEOUS_CLOCKING, OLSSC_SUP_PAUSE, OLSSC_SUP_POSTMESSAGE, OLSSC_SUP_INPROCESSFLUSH, OLSSC_SUP_BUFFERING, OLSSC_SUP_WRPSINGLE, OLSSC_SUP_WRPMULTIPLE, OLSSC_SUP_WRPWAVEFORM, OLSSC_SUP_WRPWAVEFORM_ONLY, OLSSC_SUP_GAPFREE_NODMA, OLSSC_SUP_GAPFREE_SINGLEDMA, OLSSC_SUP_GAPFREE_DUALDMA, OLSSC_SUP_TRIGSCAN, OLSSC_MAXMULTISCAN, OLSS_SUP_RETRIGGER_SCAN_PER_TRIGGER, OLSS_SUP_RETRIGGER_INTERNAL, OLSSC_SUP_RETRIGGER_EXTRA, OLSSC_SUP_INTERRUPT, OLSSC_SUP_SINGLEENDED, OLSSC_SUP_DIFFERENTIAL, OLSSC_SUP_BINARY, OLSSC_SUP_2SCOMP, OLSSC_RETURNS_FLOATS, OLSSC_NUMRANGES}
#define	SUB_TEXT	{_T("Automatic calibration"), _T("Single sampling"), _T("Continuous sampling"), _T("Continuous pretriggered"), _T("Continuous about triggered"), _T("Random channels allowed"), _T("Sequential channels allowed"),  _T("Zero first channel allowed"),  _T("Hold channel required"),  _T("Simultaneous start"),  _T("Prog. synchronization"),  _T("Simultaneous clocking"), _T("Pausing"), _T("Post messages"), _T("In-process buffering"), _T("Buffering"), _T("Single-buffer wrap"), _T("Multi-buffer wrap"),  _T("Waveform generation"),  _T("FIFI Waveform only"), _T("Gap-free without DMA"), _T("Gap-free single DMA"), _T("Gap-free dual DMA"), _T("Triggered scans"),  _T("Multiple scans"),  _T("Scan-per-trigger"), _T("Internal retriggering"), _T("Extra retriggering"), _T("Interrupt-driven I/O"),  _T("Single-ended"), _T("Differential"), _T("Binary encoding"),  _T("Twos-Complement"),  _T("Floating point"), _T("Multiple voltage ranges")}

void CDataTranslationBoardDlg::GetSubsystemYNCapabilities(int numitems)
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

#define SS_NUM_COUNT 10
#define SS_NUM_CAP	{OLSSC_MAXSECHANS, OLSSC_MAXDICHANS, OLSSC_NUMGAINS, OLSSC_CGLDEPTH, OLSSC_NUMDMACHANS, OLSSC_NUMFILTERS, OLSSC_NUMRANGES, OLSSC_NUMRESOLUTIONS, OLSSC_FIFO_SIZE_IN_K, OLSSC_NUMCHANNELS}
#define SS_NUM_TEXT	{_T("Single-ended channels"), _T("Differential channels"), _T("Gain selections"), _T("Channel gain list"),_T("DMA channels"), _T("Filters"), _T("Ranges"), _T("Resolutions"), _T("FIFO (KB)"), _T("I/O Channels")}

void CDataTranslationBoardDlg::GetSubsystemNumericalCapabilities(int numitems)
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

void CDataTranslationBoardDlg::OnLbnSelchangeListBoardcaps()
{
	const auto isel = m_listBoardCaps.GetCurSel();
	if (isel >= 0) {
		ChangeSubsystem(isel);
	}
}
