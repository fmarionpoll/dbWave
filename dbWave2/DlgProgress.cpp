
#include "StdAfx.h"
#include "resource.h"
#include "DlgProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgProgress::DlgProgress(const UINT n_caption_id)
{
	if (n_caption_id != 0)
		m_n_caption_id_ = n_caption_id;
}

DlgProgress::~DlgProgress()
{
	if (m_hWnd != nullptr)
		DestroyWindow();
}

BOOL DlgProgress::DestroyWindow()
{
	re_enable_parent();
	return CDialog::DestroyWindow();
}

void DlgProgress::re_enable_parent()
{
	if (m_b_parent_disabled_ && (m_pParentWnd != nullptr))
		m_pParentWnd->EnableWindow(TRUE);
	m_b_parent_disabled_ = FALSE;
}

BOOL DlgProgress::Create(CWnd* p_parent)
{
	// Get the true parent of the dialog
	m_pParentWnd = GetSafeOwner(p_parent);

	// m_bParentDisabled is used to re-enable the parent window
	// when the dialog is destroyed. So we don't want to set
	// it to TRUE unless the parent was already enabled.

	if ((m_pParentWnd != nullptr) && m_pParentWnd->IsWindowEnabled())
	{
		m_pParentWnd->EnableWindow(FALSE);
		m_b_parent_disabled_ = TRUE;
	}

	if (!CDialog::Create(IDD, p_parent))
	{
		re_enable_parent();
		return FALSE;
	}

	return TRUE;
}

void DlgProgress::DoDataExchange(CDataExchange* p_dx)
{
	CDialog::DoDataExchange(p_dx);
	DDX_Control(p_dx, CG_IDC_PROGDLG_PROGRESS, m_progress);
}

BEGIN_MESSAGE_MAP(DlgProgress, CDialog)

END_MESSAGE_MAP()

void DlgProgress::set_status(const LPCTSTR lpsz_message) const
{
	ASSERT(m_hWnd); // Don't call this _before_ the dialog has
	// been created. Can be called from OnInitDialog
	auto p_wnd_status = GetDlgItem(CG_IDC_PROGDLG_STATUS);

	// Verify that the static text control exists
	ASSERT(p_wnd_status != NULL);
	p_wnd_status->SetWindowText(lpsz_message);
}

void DlgProgress::OnCancel()
{
	m_b_cancel_ = TRUE;
}

void DlgProgress::set_range(const int n_lower, const int n_upper)
{
	m_n_lower_ = n_lower;
	m_n_upper_ = n_upper;
	m_progress.SetRange(static_cast<short>(n_lower), static_cast<short>(n_upper));
}

int DlgProgress::set_pos(const int n_pos)
{
	pump_messages();
	const auto i_result = m_progress.SetPos(n_pos);
	update_percent(n_pos);
	return i_result;
}

int DlgProgress::set_step(const int n_step)
{
	m_n_step_ = n_step; // Store for later use in calculating percentage
	return m_progress.SetStep(n_step);
}

int DlgProgress::offset_pos(const int n_pos)
{
	pump_messages();
	const auto i_result = m_progress.OffsetPos(n_pos);
	update_percent(i_result + n_pos);
	return i_result;
}

int DlgProgress::step_it()
{
	pump_messages();
	const auto i_result = m_progress.StepIt();
	update_percent(i_result + m_n_step_);
	return i_result;
}

void DlgProgress::pump_messages()
{
	// Must call Create() before using the dialog
	ASSERT(m_hWnd != NULL);

	MSG msg;
	// Handle dialog messages
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

BOOL DlgProgress::check_cancel_button()
{
	// Process all pending messages
	pump_messages();

	// Reset m_bCancel to FALSE so that
	// CheckCancelButton returns FALSE until the user
	// clicks Cancel again. This will allow you to call
	// CheckCancelButton and still continue the operation.
	// If m_bCancel stayed TRUE, then the next call to
	// CheckCancelButton would always return TRUE

	const auto b_result = m_b_cancel_;
	m_b_cancel_ = FALSE;

	return b_result;
}

void DlgProgress::update_percent(const int nNewPos) const
{
	auto p_wnd_percent = GetDlgItem(CG_IDC_PROGDLG_PERCENT);

	const auto n_divisor = m_n_upper_ - m_n_lower_;
	ASSERT(n_divisor > 0); // m_nLower should be smaller than m_nUpper

	const auto n_dividend = (nNewPos - m_n_lower_);
	ASSERT(n_dividend >= 0); // Current position should be greater than m_nLower

	auto n_percent = n_dividend * 100 / n_divisor;

	// Since the Progress Control wraps, we will wrap the percentage
	// along with it. However, don't reset 100% back to 0%
	if (n_percent != 100)
		n_percent %= 100;

	// Display the percentage
	CString str_buf;
	str_buf.Format(_T("%d%c"), n_percent, _T('%'));

	CString str_cur; // get current percentage
	p_wnd_percent->GetWindowText(str_cur);

	if (str_cur != str_buf)
		p_wnd_percent->SetWindowText(str_buf);
}

/////////////////////////////////////////////////////////////////////////////
// CProgressDlg message handlers

BOOL DlgProgress::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_progress.SetRange(static_cast<short>(m_n_lower_), static_cast<short>(m_n_upper_));
	m_progress.SetStep(m_n_step_);
	m_progress.SetPos(m_n_lower_);

	CString str_caption;
	VERIFY(str_caption.LoadString(m_n_caption_id_));
	SetWindowText(str_caption);

	return TRUE;
}
