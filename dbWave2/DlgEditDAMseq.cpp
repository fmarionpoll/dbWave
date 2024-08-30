#include "StdAfx.h"
//#include "dbWave.h"
#include "DlgEditDAMseq.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(DlgEditDAMseq, CDialogEx)

DlgEditDAMseq::DlgEditDAMseq(CWnd* p_parent /*=NULL*/)
	: CDialogEx(IDD, p_parent)
{
	m_p_parent_ = p_parent;
}

DlgEditDAMseq::~DlgEditDAMseq()
{
}

void DlgEditDAMseq::DoDataExchange(CDataExchange* p_dx)
{
	CDialogEx::DoDataExchange(p_dx);

	DDX_Text(p_dx, IDC_MSEQ_RATIO, m_m_seq_ratio);
	DDX_Text(p_dx, IDC_MSEQ_DELAY, m_m_seq_delay);
	DDX_Text(p_dx, IDC_MSEQ_SEED, m_m_seq_seed);
	DDX_Control(p_dx, IDC_MSEQ_RANDOM_SEED, m_m_seq_random_seed);
}

BEGIN_MESSAGE_MAP(DlgEditDAMseq, CDialogEx)

	ON_BN_CLICKED(IDC_MSEQ_RANDOM_SEED, &DlgEditDAMseq::on_bn_clicked_m_seq_random_seed)

END_MESSAGE_MAP()

BOOL DlgEditDAMseq::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_m_seq_ratio = m_out_d_parameters.m_seq_i_ratio;
	m_m_seq_delay = m_out_d_parameters.m_seq_i_delay;
	m_m_seq_seed = m_out_d_parameters.m_seq_i_seed;

	UpdateData(FALSE);
	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgEditDAMseq::OnOK()
{
	UpdateData(TRUE);

	m_out_d_parameters.m_seq_i_ratio = m_m_seq_ratio;
	m_out_d_parameters.m_seq_i_delay = m_m_seq_delay;
	m_out_d_parameters.m_seq_i_seed = m_m_seq_seed;

	CDialogEx::OnOK();
}

void DlgEditDAMseq::on_bn_clicked_m_seq_random_seed()
{
	UpdateData(TRUE);

	auto the_time = CTime::GetCurrentTime();
	const UINT seed = ((the_time.GetSecond() << 8) + (the_time.GetMinute() << 4) + (the_time.GetHour())) * 10000000 &
		0x7fffffff;
	m_m_seq_seed = seed;
	UpdateData(FALSE);
}
