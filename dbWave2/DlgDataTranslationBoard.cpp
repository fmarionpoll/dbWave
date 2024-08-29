#include "StdAfx.h"
#include "resource.h"
#include "CDTBoardParameters.h"
#include "DlgDataTranslationBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(DlgDataTranslationBoard, CDialog)

DlgDataTranslationBoard::DlgDataTranslationBoard(CWnd* p_parent /*=NULL*/)
	: CDialog(IDD, p_parent)
{
}

DlgDataTranslationBoard::~DlgDataTranslationBoard()
{
	const UINT ui_num_boards = m_p_dt_acq32->GetNumBoards();
	if (ui_num_boards > 0)
	{
		m_p_analog_in->SetSubSysType(m_subsystem_in);
	}
}

void DlgDataTranslationBoard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOARD, m_cb_board);
	DDX_Control(pDX, IDC_LIST_BOARDCAPS, m_list_board_caps);
	DDX_Control(pDX, IDC_LIST_SSNUM, m_list_ss_num_caps);
	DDX_Control(pDX, IDC_LIST_SSCAPS, m_list_ss_caps);
}

BEGIN_MESSAGE_MAP(DlgDataTranslationBoard, CDialog)
	ON_CBN_SELCHANGE(IDC_BOARD, &DlgDataTranslationBoard::on_sel_change_board)
	ON_LBN_SELCHANGE(IDC_LIST_BOARDCAPS, &DlgDataTranslationBoard::on_lbn_sel_change_list_board_caps)
END_MESSAGE_MAP()

BOOL DlgDataTranslationBoard::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_subsystem_in = m_p_analog_in->GetSubSysType();
	m_p_dt_acq32 = m_p_analog_in;
	find_dt_open_layers_boards();
	on_sel_change_board();
	return TRUE;
}

BOOL DlgDataTranslationBoard::find_dt_open_layers_boards()
{
	m_cb_board.ResetContent();
	const UINT ui_num_boards = m_p_dt_acq32->GetNumBoards();
	const auto flag = (ui_num_boards > 0 ? TRUE : FALSE);
	if (ui_num_boards == 0)
		m_cb_board.AddString(_T("No Board"));
	else
	{
		for (short i = 0; i < static_cast<short>(ui_num_boards); i++)
			m_cb_board.AddString(m_p_dt_acq32->GetBoardList(i));
	}
	m_cb_board.SetCurSel(0);
	return flag;
}

void DlgDataTranslationBoard::on_sel_change_board()
{
	const auto i_sel = m_cb_board.GetCurSel();
	m_cb_board.GetLBText(i_sel, m_board_name_);
	m_n_subsystems_ = get_board_capabilities();
}

#define SS_LIST_SIZE 6
#define	SS_CODES {OLSS_AD, OLSS_DA, OLSS_DIN, OLSS_DOUT, OLSS_CT, OLSS_SRL}
#define	SS_LIST	 {OLDC_ADELEMENTS, OLDC_DAELEMENTS, OLDC_DINELEMENTS,OLDC_DOUTELEMENTS, OLDC_CTELEMENTS, OLDC_SRLELEMENTS}
#define SS_TEXT	 {_T("Analog Inputs"), _T("Analog outputs"), _T("Digital Inputs"),_T("Digital Outputs"),	_T("Counter/Trigger"), _T("Serial Port")}

int DlgDataTranslationBoard::get_board_capabilities()
{
	int n_sub_systems = 0;

	try
	{
		m_p_dt_acq32->SetBoard(m_board_name_);
		m_list_board_caps.ResetContent();

		n_sub_systems = m_p_dt_acq32->GetNumSubSystems();

		const CString subsystem_text[SS_LIST_SIZE] = SS_TEXT;
		constexpr short ss_list[SS_LIST_SIZE] = SS_LIST;

		for (auto i = 0; i < SS_LIST_SIZE; i++)
		{
			const int number = m_p_dt_acq32->GetDevCaps(ss_list[i]);
			CString cs;
			cs.Format(_T("\n  %i: "), number);
			auto board_text = cs + subsystem_text[i];
			m_list_board_caps.AddString(board_text);
			m_list_board_caps.SetItemData(i, number);
		}
	}
	catch (COleDispatchException* e)
	{
		DispatchException(e);
		return n_sub_systems;
	}
	return n_sub_systems;
}

void DlgDataTranslationBoard::change_subsystem(int index)
{
	constexpr int ss_codes[SS_LIST_SIZE] = SS_CODES;
	const DWORD ss_info = ss_codes[index];
	const int num_items = m_list_board_caps.GetItemData(index);
	const auto ol_ss = (ss_info & 0xffff);
	//UINT ui_element = (ss_info >> 16) & 0xff;
	if (num_items > 0 && static_cast<unsigned long>(m_p_dt_acq32->GetSubSysType()) != ol_ss)
	{
		try
		{
			if (ol_ss == static_cast<unsigned long>(m_p_analog_out->GetSubSysType()))
				m_p_dt_acq32 = m_p_analog_out;
			else
				m_p_dt_acq32 = m_p_analog_in;
			m_p_dt_acq32->SetSubSysType(static_cast<short>(ol_ss));
			m_p_dt_acq32->SetSubSysElement(0);
		}
		catch (COleDispatchException* e)
		{
			DispatchException(e);
		}
	}
	get_subsystem_yn_capabilities(num_items);
	get_subsystem_numerical_capabilities(num_items);
}

void DlgDataTranslationBoard::get_subsystem_yn_capabilities(int num_items)
{
	m_list_ss_caps.ResetContent();
	if (num_items > 0)
	{
		constexpr OLSSC ol_ssc[SUB_COUNT] = SUB_CAP;
		const CString cap_text[SUB_COUNT] = SUB_TEXT;
		try
		{
			for (UINT i = 0; i < SUB_COUNT; i++)
				if (m_p_dt_acq32->GetSSCaps(ol_ssc[i]) != 0)
					m_list_ss_caps.AddString(cap_text[i]);
		}
		catch (COleDispatchException* e)
		{
			DispatchException(e);
		}
	}
	m_list_ss_caps.Invalidate();
}

void DlgDataTranslationBoard::get_subsystem_numerical_capabilities(int num_items)
{
	m_list_ss_num_caps.ResetContent();
	if (num_items > 0)
	{
		constexpr OLSSC ol_ssc_num[SS_NUM_COUNT] = SS_NUM_CAP;
		const CString num_text[SS_NUM_COUNT] = SS_NUM_TEXT;
		try
		{
			for (UINT i = 0; i < SS_NUM_COUNT; i++)
			{
				const UINT capability = m_p_dt_acq32->GetSSCaps(ol_ssc_num[i]);
				if (capability != 0)
				{
					CString cs;
					cs.Format(_T(": %d"), capability);
					m_list_ss_num_caps.AddString(num_text[i] + cs);
				}
			}
		}
		catch (COleDispatchException* e)
		{
			DispatchException(e);
		}
	}
	m_list_ss_num_caps.Invalidate();
}

void DlgDataTranslationBoard::on_lbn_sel_change_list_board_caps()
{
	const auto i_sel = m_list_board_caps.GetCurSel();
	if (i_sel >= 0)
	{
		change_subsystem(i_sel);
	}
}

void DlgDataTranslationBoard::DispatchException(COleDispatchException* e)
{
	CString my_error;
	my_error.Format(_T("DT-Open Layers Error: %i "), static_cast<int>(e->m_scError));
	my_error += e->m_strDescription;
	AfxMessageBox(my_error);
	e->Delete();
}
