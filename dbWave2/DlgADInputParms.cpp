// ADInputParmsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "./include/DataTranslation/Olxdadefs.h"
#include "GridCtrl/GridCell.h"
#include "GridCtrl/GridCellCombo_FMP.h"
#include "GridCtrl/GridCellNumeric.h"
#include "GridCtrl/GridCellCheck.h"
#include "DlgADInputParms.h"
#include "CyberAmp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CADInputParmsDlg dialog

// Static data : rows headers, content of comboboxes (high pass filter, AD gain, amplifier, headstage)
// pszRowTitle: content of the row header
// if string starts with char, print BOLD and left aligned
// if string starts with space, print normal and right aligned
// the numbers below are used in a "switch" to take action according to the row item
// within OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)

TCHAR* CADInputParmsDlg::pszRowTitle[] = {
	_T("Title"),
	// 1: // title
	_T("A/D channel (0-7) "),
	_T(" gain"),
	// 2: // A/D channel (0-7), (0-16) 3: // A/D gain
	_T("Amplifier"),
	_T(" channel"),
	_T(" gain"),
	_T(" notch filter"),
	_T(" IN+ filter (Hz)"),
	_T(" DC Offset (mV)"), //_T(" IN- filter (Hz)"),
	_T(" low pass filter (Hz)"),
	// 4: // ampli 5: // ampli out chan 6: // ampli gain 7: // notch 8:// IN+ 9: // IN- 10: //low pass
	_T("Probe"),
	_T(" gain"),
	// 11: headstage 12: // headstage gain
	_T("Signal max/min (mV)"),
	_T(" total gain"),
	_T(" resolution (µV)"),
	// 13: // max range 14: // total gain 15:// bin resolution
	_T("") };
TCHAR* CADInputParmsDlg::pszHighPass[] = { _T("GND"),_T("DC"),_T("0.1"),_T("1"),_T("10"),_T("30"),_T("100"),_T("300"),
											_T(""),_T(""),_T(""),_T(""),_T(""),_T(""),_T(""),_T("") };
TCHAR* CADInputParmsDlg::pszADGains[] = { _T("1"),_T("2"),_T("4"),_T("8"),_T(""),_T(""),_T(""),_T("") };
TCHAR* CADInputParmsDlg::pszAmplifier[] = { _T("CyberAmp 320"),_T("Neurolog"),_T("IDAC"),_T("Dagan"),_T("Alligator"),  _T("none"),_T(""),_T("") };
TCHAR* CADInputParmsDlg::pszProbeType[] = { _T("Syntech"),_T("DTP02"),_T("AI401"),_T("Neurolog"),_T("AD575"),_T("Dagan"),_T("none"),_T("") };
TCHAR* CADInputParmsDlg::pszEncoding[] = { _T("Offset binary"),_T("Two's complement"),_T("Unknown"),_T(""),_T(""),_T(""),_T(""),_T("") };
int		CADInputParmsDlg::iEncoding[] = { OLx_ENC_BINARY, OLx_ENC_2SCOMP, -1 };

IMPLEMENT_DYNAMIC(CADInputParmsDlg, CDialog)
CADInputParmsDlg::CADInputParmsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CADInputParmsDlg::IDD, pParent)
	, m_nacqchans(0)
	, m_maxchans(0)
{
	m_OldSize = CSize(-1, -1);
	m_iNBins = 4096;
	m_xVoltsMax = 10.0f;
	m_bchainDialog = FALSE;
	m_pwFormat = nullptr;
	m_pchArray = nullptr;
	m_numchansMAXDI = 8;
	m_numchansMAXSE = 16;
	m_bcommandAmplifier = FALSE;
	m_inputlistmax = 32;				// chan list size for DT9800
	m_postmessage = 0;
	m_bchantype = OLx_CHNT_SINGLEENDED;
	m_pAlligatorAmplifier = nullptr;
	p_alligatordevice_ptr_array = nullptr;

	m_rowADchannel = 0;
	m_row_ADgain = 0;
	m_row_headstagegain = 1;
	m_row_ampgain = 1;
	m_row_readonly = 1;
}

CADInputParmsDlg::~CADInputParmsDlg()
= default;

void CADInputParmsDlg::DoDataExchange(CDataExchange * pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NACQCHANS, m_nacqchans);
	DDX_Text(pDX, IDC_MAXLIST, m_inputlistmax);
	DDX_Control(pDX, IDC_GRID, m_Grid);
	DDX_Control(pDX, IDC_RESOLUTION, m_resolutionCombo);
	DDX_Control(pDX, IDC_ENCODING, m_encodingCombo);
}

BEGIN_MESSAGE_MAP(CADInputParmsDlg, CDialog)
	ON_EN_CHANGE(IDC_NACQCHANS, OnEnChangeNacqchans)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_SINGLEENDED, OnBnClickedSingleended)
	ON_BN_CLICKED(IDC_DIFFERENTIAL, OnBnClickedDifferential)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID, OnGridEndEdit)
	ON_CBN_SELCHANGE(IDC_RESOLUTION, OnCbnSelchangeResolution)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_ADINTERVALS, OnBnClickedAdintervals)
END_MESSAGE_MAP()

// CADInputParmsDlg message handlers

BOOL CADInputParmsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	CRect rect;
	GetClientRect(rect);
	m_OldSize = CSize(rect.Width(), rect.Height());

	// display/hide chain dialog buttons
	auto show_window = SW_HIDE;
	if (m_bchainDialog)
		show_window = SW_SHOW;
	GetDlgItem(IDC_ADCHANNELS)->ShowWindow(show_window);
	GetDlgItem(IDC_ADINTERVALS)->ShowWindow(show_window);

	// display nb of available channels according to parameter
	if (m_bchantype == OLx_CHNT_SINGLEENDED)
	{
		((CButton*)GetDlgItem(IDC_SINGLEENDED))->SetCheck(BST_CHECKED);
		m_maxchans = m_numchansMAXSE;
	}
	else
	{
		((CButton*)GetDlgItem(IDC_DIFFERENTIAL))->SetCheck(BST_CHECKED);
		m_maxchans = m_numchansMAXDI;
	}

	// set spin  max list: dt9800 = up to 32 channels
	((CSpinButtonCtrl*)(GetDlgItem(IDC_SPIN1)))->SetRange(1, m_inputlistmax);
	// load encoding combo with data
	int i = 0;
	do {
		auto j = m_encodingCombo.AddString(pszEncoding[i]);
		m_encodingCombo.SetItemData(j, static_cast<DWORD>(iEncoding[i]));
		i++;
	} while (pszEncoding[i] != _T(""));

	// eventually load data
	if (m_pwFormat == nullptr)
	{
		m_nacqchans = 1;
		// init other dialog box items
		m_resolutionCombo.SetCurSel(1);	// 12 bits
		m_iNBins = 1 << 12; // 2 exp (nbits)
		m_encodingCombo.SetCurSel(0);
	}
	else
	{
		// load A/D card parameters
		m_nacqchans = m_pwFormat->scan_count;
		GetDlgItem(IDC_ADCARDNAME)->SetWindowText(m_pwFormat->csADcardName);
		// get precision and compute corresponding nb of bits
		m_iNBins = m_pwFormat->binspan + 1;
		int i_bins = m_iNBins;
		auto n_bits = 0;
		do {
			i_bins = i_bins / 2;
			n_bits++;
		} while (i_bins > 1);

		// select corresponding entry in the combobox
		CString cs;
		cs.Format(_T("%i"), n_bits);
		const auto i_found = m_resolutionCombo.FindStringExact(-1, cs);
		if (i_found != CB_ERR)
			m_resolutionCombo.SetCurSel(i_found);

		// select encoding mode according to parameter in waveFormat
		int i_current;
		for (i_current = 0; i_current < m_encodingCombo.GetCount(); i_current++)
		{
			if (m_encodingCombo.GetItemData(i_current) == static_cast<DWORD>(m_pwFormat->mode_encoding))
				break;
		}
		if (i_current == m_encodingCombo.GetCount())
			i_current--;
		m_encodingCombo.SetCurSel(i_current);
	}

	/////////////////////////////////////////////////////////////////////////
	// initialise grid properties
	/////////////////////////////////////////////////////////////////////////

	m_Grid.EnableDragAndDrop(TRUE);
	m_Grid.GetDefaultCell(FALSE, FALSE)->SetBackClr(RGB(0xFF, 0xFF, 0xFF));
	m_Grid.SetColumnCount(m_nacqchans + 1);
	m_Grid.SetFixedColumnCount(1);
	m_Grid.SetFixedRowCount(1);

	// get font and create bold face
	auto p_font = m_Grid.GetFont();
	ASSERT(p_font != NULL);
	LOGFONT lf;
	p_font->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;

	// set row headers
	auto n_rows = 0;				// count number of rows
	do {
		n_rows++;
	} while (pszRowTitle[n_rows - 1] != _T(""));
	m_Grid.SetRowCount(n_rows);

	// init row headers with descriptors
	int col = 0;
	int row = 0;
	do {
		row++;
		m_Grid.SetItemText(row, col, pszRowTitle[row - 1]);
		if (pszRowTitle[row - 1][0] == ' ')
			m_Grid.SetItemFormat(row, col, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		else
			m_Grid.SetItemFont(row, col, &lf);
	} while (pszRowTitle[row] != _T(""));
	CString cs;
	cs.Format(_T("A/D channel (0-%i)"), m_maxchans - 1);
	m_Grid.SetItemText(2, 0, cs);

	// init data columns
	for (col = 1; col <= m_nacqchans; col++)
	{
		InitGridColumnDefaults(col);
		if (m_pwFormat != nullptr)
			LoadChanData(col);
	}

	m_Grid.AutoSizeColumn(0);
	m_Grid.AutoSizeRows();
	m_Grid.ExpandRowsToFit();

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CADInputParmsDlg::OnEnChangeNacqchans()
{
	if (!::IsWindow(m_Grid.m_hWnd))
		return;
	UpdateData(TRUE);

	// make sure that the number of channels is appropriate
	if (m_nacqchans < 1)
		m_nacqchans = 1;
	if (m_nacqchans > m_inputlistmax)
		m_nacqchans = m_inputlistmax;

	// update title of row 2 and refresh cell
	CString cs;
	cs.Format(_T("A/D channel (0-%i)"), m_maxchans - 1);
	m_Grid.SetItemText(m_rowADchannel, 0, cs);
	m_Grid.RedrawCell(m_rowADchannel, 0, nullptr);

	// update combos / acq channels
	for (int i = 1; i <= m_nacqchans; i++)
		InitADchannelCombo(i, -1);

	AdjustGridSize();
	UpdateData(FALSE);
}

void CADInputParmsDlg::AdjustGridSize()
{
	const auto n_old_columns = m_Grid.GetColumnCount();
	// reduce size of the grid; delete last columns
	if (n_old_columns > m_nacqchans + 1)
	{
		for (auto i = n_old_columns - 1; i > m_nacqchans; i--)
			m_Grid.DeleteColumn(i);
		m_Grid.Refresh();
	}
	// increase size of the grid
	else if (n_old_columns < m_nacqchans + 1)
	{
		m_Grid.SetColumnCount(m_nacqchans + 1);
		for (int i = n_old_columns; i < m_nacqchans + 1; i++)
			InitGridColumnDefaults(i);
		m_Grid.Refresh();
	}
}

BOOL CALLBACK EnumProc(const HWND h_wnd, LPARAM lParam)
{
	auto p_wnd = CWnd::FromHandle(h_wnd);
	auto* p_translate = reinterpret_cast<CSize*>(lParam);

	auto* p_dlg = (CADInputParmsDlg*)p_wnd->GetParent();
	if (!p_dlg) return FALSE;

	CRect rect;
	p_wnd->GetWindowRect(rect);
	p_dlg->ScreenToClient(rect);
	if (h_wnd == p_dlg->m_Grid.GetSafeHwnd())
	{
		// move grid
		if (((rect.top >= 7 && p_translate->cy > 0) || rect.Height() > 20) && ((rect.left >= 7 && p_translate->cx > 0) || rect.Width() > 20))
			p_wnd->MoveWindow(rect.left, rect.top, rect.Width() + p_translate->cx, rect.Height() + p_translate->cy, FALSE);
		else
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top + p_translate->cy, rect.Width(), rect.Height(), FALSE);
	}
	else
	{
		const auto i_id = p_wnd->GetDlgCtrlID();
		switch (i_id)
		{
		case IDC_SIZEBOX:     // move size box on the lower right corner
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top + p_translate->cy,
				rect.Width(), rect.Height(), FALSE);
			break;
		case IDOK:	// other windows to move with the right size switch..
		case IDCANCEL:
		case IDC_ADCHANNELS:
		case IDC_ADINTERVALS:
			p_wnd->MoveWindow(rect.left + p_translate->cx, rect.top, rect.Width(), rect.Height(), FALSE);
			break;
		default:
			break;
		}
	}
	p_dlg->Invalidate();

	return TRUE;
}

void CADInputParmsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (cx <= 1 || cy <= 1)
		return;

	CSize Translate(cx - m_OldSize.cx, cy - m_OldSize.cy);
	::EnumChildWindows(GetSafeHwnd(), EnumProc, (LPARAM)&Translate);
	m_OldSize = CSize(cx, cy);

	CWnd* p_wnd = GetDlgItem(IDC_SIZEBOX);
	if (p_wnd)
		p_wnd->ShowWindow((nType == SIZE_MAXIMIZED) ? SW_HIDE : SW_SHOW);
}

//////////////////////////////////////////////////////
// set column with proper header, combos, etc and data

void CADInputParmsDlg::InitADchannelCombo(int col, int iselect)
{
	// build string array with channel list
	CStringArray csArrayOptions;
	CString cs;
	for (int i = 0; i < m_maxchans; i++)
	{
		cs.Format(_T("%i"), i);
		csArrayOptions.Add(cs);
	}
	csArrayOptions.Add(_T("16 (Din)"));

	// select cell and corresponding combo
	auto p_cell = (CGridCellCombo*)m_Grid.GetCell(m_rowADchannel, col);
	if (p_cell != nullptr)
	{
		// if iselect negative, get current selection
		if (iselect < 0)
			iselect = p_cell->GetCurSel();
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		p_cell->SetCurSel(iselect);
	}
}

BOOL CADInputParmsDlg::InitGridColumnDefaults(int col)
{
	if (col < 1 || col >= m_Grid.GetColumnCount())
		return FALSE;

	// get font and create bold face
	auto p_font = m_Grid.GetFont();
	ASSERT(p_font != NULL);
	LOGFONT lf;
	p_font->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;

	// set columns headers
	auto row = 0;
	GV_ITEM item;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.row = row;
	item.col = col;
	item.nFormat = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
	item.strText.Format(_T("ch %i"), col);
	m_Grid.SetItem(&item);
	m_Grid.SetItemFont(row, col, &lf);

	// set cells content
	//"Title",
	row++;
	item.row = row;
	item.strText.Format(_T("channel %i"), col);
	item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
	m_Grid.SetItem(&item);

	//"A/D channel" - (col-1)
	row++;
	m_rowADchannel = row;
	if (m_Grid.SetCellType(m_rowADchannel, col, RUNTIME_CLASS(CGridCellCombo)))
		InitADchannelCombo(col, col - 1);

	// "A/D gain" - combo
	row++;
	m_row_ADgain = row;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray csArrayOptions;
		auto i = 0;
		do {
			csArrayOptions.Add(pszADGains[i]);
			i++;
		} while (pszADGains[i] != _T(""));
		auto* p_cell = (CGridCellCombo*)m_Grid.GetCell(row, col);
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN);
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(0);
	}

	// "Amplifier type" - combo
	row++;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray cs_array_options;
		auto i = 0;
		do {
			cs_array_options.Add(pszAmplifier[i]);
			i++;
		} while (pszAmplifier[i] != _T(""));
		auto* p_cell = (CGridCellCombo*)m_Grid.GetCell(row, col);
		p_cell->SetOptions(cs_array_options);
		p_cell->SetStyle(CBS_DROPDOWN);
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(0);
	}

	// " output channel" - numeric
	row++;
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	// init value
	item.row = row;
	item.strText.Format(_T("%d"), col);
	m_Grid.SetItem(&item);

	// " gain" - numeric
	row++;
	m_row_ampgain = row;				// save index for later
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	// init value
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
	else
		m_Grid.SetItemText(row, col, _T("500"));

	// " notch filter" - check box
	row++;
	item.strText = _T("50 Hz");
	item.row = row;
	m_Grid.SetItem(&item);
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCheck));

	// " IN+" - combo
	row++;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray csArrayOptions;
		int i = 0;
		do {
			csArrayOptions.Add(pszHighPass[i]);
			i++;
		} while (pszHighPass[i] != _T(""));
		auto p_cell = (CGridCellCombo*)m_Grid.GetCell(row, col);
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(1);
	}

	// DC Offset (mV) " IN-" - combo
	row++;
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
	else
		m_Grid.SetItemText(row, col, _T("0.00"));

	// " low pass filter" - numeric
	row++;
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
	else
		m_Grid.SetItemText(row, col, _T("2800"));

	// "Probe type" - combo
	row++;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray csArrayOptions;
		int i = 0;
		do {
			csArrayOptions.Add(pszProbeType[i]);
			i++;
		} while (pszProbeType[i] != _T(""));
		auto p_cell = (CGridCellCombo*)m_Grid.GetCell(row, col);
		p_cell->SetOptions(csArrayOptions);
		p_cell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
		else
			p_cell->SetCurSel(0);
	}

	// " gain" - numeric
	row++;
	m_row_headstagegain = row;				// save index for later
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	// init value
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
	else
		m_Grid.SetItemText(row, col, _T("10"));

	// read only / numeric fields
	m_row_readonly = row + 1;
	const auto clr = m_Grid.GetDefaultCell(TRUE, TRUE)->GetBackClr();
	for (auto i = 0; i < 3; i++)
	{
		row++;
		m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
		m_Grid.SetItemState(row, col, m_Grid.GetItemState(1, 1) | GVIS_READONLY);
		m_Grid.SetItemBkColour(row, col, clr);
	}
	InitGridColumnReadOnlyFields(col);

	return TRUE;
}

void CADInputParmsDlg::InitGridColumnReadOnlyFields(int col)
{
	// "Signal total gain" - read only / numeric
	int row = m_row_readonly;
	CString cs = m_Grid.GetItemText(m_row_ampgain, col);
	auto gain = _ttoi(cs);
	cs = m_Grid.GetItemText(m_row_headstagegain, col);
	gain *= _ttoi(cs);
	cs = m_Grid.GetItemText(m_row_ADgain, col);
	gain *= _ttoi(cs);
	cs.Format(_T("%1.3f"), (m_xVoltsMax * 1000.f / float(gain)));
	m_Grid.SetItemText(row, col, cs);

	// "total gain" - read only / numeric
	row++;
	cs.Format(_T("%li"), gain);		// = ampgain * probegain * ADgain
	m_Grid.SetItemText(row, col, cs);

	// " resolution (µV)" - read only
	row++;
	cs.Format(_T("%1.3f"), (float(m_xVoltsMax) * float(1.E6) / float(gain) * 2 / float(m_iNBins)));
	m_Grid.SetItemText(row, col, cs);
}

void CADInputParmsDlg::OnBnClickedSingleended()
{
	m_maxchans = m_numchansMAXSE;
	m_bchantype = OLx_CHNT_SINGLEENDED;
	OnEnChangeNacqchans();
}

void CADInputParmsDlg::OnBnClickedDifferential()
{
	m_maxchans = m_numchansMAXDI;
	m_bchantype = OLx_CHNT_DIFFERENTIAL;
	OnEnChangeNacqchans();
}

void CADInputParmsDlg::OnGridEndEdit(NMHDR * pNotifyStruct, LRESULT * pResult)
{
	const auto p_item = reinterpret_cast<NM_GRIDVIEW*>(pNotifyStruct);
	switch (p_item->iRow)
	{
		//case 1: // title - no action
		//case 2: // A/D channel (0-7), (0-15)
	case 3: // A/D gain
		InitGridColumnReadOnlyFields(p_item->iColumn);
		break;
	case 4: // ampli
	case 5: // ampli out chan
		SaveChanData(p_item->iColumn);
		SetAmplifierParms(p_item->iColumn);
		break;
	case 6: // ampli gain
		SaveChanData(p_item->iColumn);
		SetAmplifierParms(p_item->iColumn);
		InitGridColumnReadOnlyFields(p_item->iColumn);
		break;
	case 7: // notch
	case 8: // IN+
	case 9: // low pass
	case 10:
		SaveChanData(p_item->iColumn);
		SetAmplifierParms(p_item->iColumn);
		break;
		//case 11: // headstage
	case 12:  // headstage gain
		InitGridColumnReadOnlyFields(p_item->iColumn);
		break;
		//case 13: // max range
		//case 14: // total gain
		//case 15: // bin resolution

	default:
		break;
	}
	// Accept change and refresh grid
	*pResult = TRUE;
	m_Grid.Refresh();
}

void CADInputParmsDlg::OnCbnSelchangeResolution()
{
	CString cs;
	m_resolutionCombo.GetWindowText(cs);
	const auto n_bits = _ttoi(cs);
	const auto old_n_bins = m_iNBins;
	m_iNBins = 1 << n_bits; // 2 exp (nbits)
	// change input resolution if value has changed
	if (m_iNBins != old_n_bins)
	{
		for (auto i = 1; i <= m_nacqchans; i++)
			InitGridColumnReadOnlyFields(i);
		m_Grid.Refresh();
	}
}

// load column data (index = 1, n)
void CADInputParmsDlg::LoadChanData(int col)
{
	// select channel and load parameters CComboBox
	ASSERT(col <= m_pchArray->chanArray_getSize());
	const auto p_chan = m_pchArray->get_p_channel(col - 1);

	// settings for the col
	GV_ITEM item;
	item.mask = GVIF_TEXT | GVIF_FORMAT;
	item.col = col;
	// ----------------------------------------
	// AD channel comment
	item.row = 1;
	item.nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX;
	item.strText = p_chan->am_csComment;
	if (p_chan->am_csComment != _T(""))
		m_Grid.SetItem(&item);
	// AD input channel number
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_adchannel);
	m_Grid.SetItem(&item);
	// " ADgain" - combo
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_gainAD);
	m_Grid.SetItem(&item);

	GetAmplifierParms(col); // get amplifier current settings

	// ----------------------------------------
	// amplifier name
	item.row++;
	item.strText = p_chan->am_csamplifier;
	m_Grid.SetItem(&item);
	// amplifier channel
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_amplifierchan);
	m_Grid.SetItem(&item);
	// amplifier gain
	item.row++;
	const auto gain = double(p_chan->am_gaintotal) / (double(p_chan->am_gainheadstage) * double(p_chan->am_gainAD));
	auto i_gain = static_cast<int>(gain);
	if (i_gain == 0)
		i_gain = 1;
	item.strText.Format(_T("%d"), i_gain);
	m_Grid.SetItem(&item);
	// amplifier notch filter
	item.row++;
	((CGridCellCheck*)m_Grid.GetCell(item.row, item.col))->SetCheck(p_chan->am_notchfilt);
	// IN+
	item.row++;
	item.strText = p_chan->am_csInputpos;
	m_Grid.SetItem(&item);
	// DC Offset (mV) // IN-
	item.row++;
	//Item.strText=pchan->am_csInputneg;
	item.strText.Format(_T("%0.3f"), p_chan->am_offset);
	m_Grid.SetItem(&item);
	// amplifier low pass filter
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_lowpass);
	m_Grid.SetItem(&item);
	// ----------------------------------------
	// headstage
	item.row++;
	item.strText = p_chan->am_csheadstage;
	m_Grid.SetItem(&item);
	// headstage gain
	item.row++;
	item.strText.Format(_T("%i"), p_chan->am_gainheadstage);
	m_Grid.SetItem(&item);

	// set read only fields
	InitGridColumnReadOnlyFields(col);
}

void CADInputParmsDlg::SaveData()
{
	// if number of acq channels changed, change corresp structures
	if (m_nacqchans != m_pwFormat->scan_count)
	{
		m_pwFormat->scan_count = m_nacqchans;
		m_pchArray->chanArray_setSize(m_nacqchans);
		const WORD ch_buffer_size = m_pwFormat->buffersize / m_nacqchans;
		m_pwFormat->buffersize = ch_buffer_size * m_nacqchans;
	}
	// general acq parameters
	GetDlgItem(IDC_ADCARDNAME)->GetWindowText(m_pwFormat->csADcardName);
	m_pwFormat->binspan = m_iNBins;

	// save each channel
	for (auto i = 1; i <= m_nacqchans; i++)
	{
		SaveChanData(i);
		SetAmplifierParms(i);
	}
}

// save column data (index = 1, n)
void CADInputParmsDlg::SaveChanData(int col)
{
	// select channel and load parameters CComboBox
	if (m_pchArray->chanArray_getSize() < col)
		m_pchArray->chanArray_setSize(col);
	auto p_chan = m_pchArray->get_p_channel(col - 1);

	// AD channel comment
	auto row = 1;
	p_chan->am_csComment = m_Grid.GetItemText(row, col);
	// "AD input channel" - combo
	row++;
	auto cs = m_Grid.GetItemText(row, col);
	p_chan->am_adchannel = _ttoi(cs);	// digital channels "16 [DIN]" is OK because of the space after "16"

	// " ADgain" - combo
	row++;
	cs = m_Grid.GetItemText(row, col);
	p_chan->am_gainAD = _ttoi(cs);

	// amplifier name
	row++;
	p_chan->am_csamplifier = m_Grid.GetItemText(row, col);

	// amplifier channel
	row++;
	cs = m_Grid.GetItemText(row, col);
	p_chan->am_amplifierchan = _ttoi(cs);

	// amplifier gain
	row++;
	cs = m_Grid.GetItemText(row, col);
	p_chan->am_gainpre = _ttoi(cs);
	p_chan->am_gainpost = 1;

	// amplifier notch filter
	row++;
	p_chan->am_notchfilt = ((CGridCellCheck*)m_Grid.GetCell(row, col))->GetCheck();

	// amplifier high pass filter (IN+)
	row++;
	p_chan->am_csInputpos = m_Grid.GetItemText(row, col);
	// DC Offset(mV) // IN-
	row++;
	cs = m_Grid.GetItemText(row, col);
	p_chan->am_offset = (float)_tstof(cs);
	// amplifier low pass filter
	row++;
	cs = m_Grid.GetItemText(row, col);
	p_chan->am_lowpass = _ttoi(cs);

	// headstage
	row++;
	p_chan->am_csheadstage = m_Grid.GetItemText(row, col);
	// headstage gain
	row++;
	cs = m_Grid.GetItemText(row, col);
	p_chan->am_gainheadstage = _ttoi(cs);
	p_chan->am_gainamplifier = double(p_chan->am_gainheadstage) * (double)p_chan->am_gainpre * (double)p_chan->am_gainpost;
	p_chan->am_gaintotal = p_chan->am_gainamplifier * p_chan->am_gainAD;
	// compute dependent parameters
	p_chan->am_resolutionV = m_pwFormat->fullscale_Volts / p_chan->am_gaintotal / m_pwFormat->binspan;
}

void CADInputParmsDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	SaveData();
	OnOK();
}

void CADInputParmsDlg::OnBnClickedAdintervals()
{
	m_postmessage = IDC_ADINTERVALS;
	UpdateData(TRUE);
	SaveData();
	OnOK();
}

void CADInputParmsDlg::SetAmplifierParms(int col)
{
	if (!m_bcommandAmplifier)
		return;

	// transfer data into structure
	auto p_chan = m_pchArray->get_p_channel(col - 1);

	if (p_chan->am_csamplifier.Find(_T("CyberAmp")) >= 0
		|| p_chan->am_csamplifier.Find(_T("Axon")) >= 0)
	{
		// cyberAmp declared: check if connected -if not, exit
		CCyberAmp cyber_amp;
		if (!(cyber_amp.Initialize() == NULL))
			return;

		p_chan->am_csInputneg = pszHighPass[0];
		/*auto error_code = */cyber_amp.SetWaveChanParms(p_chan);
	}

	if (p_chan->am_csamplifier.Find(_T("Alligator")) >= 0)
	{
		//m_pAlligatorAmplifier->SetWaveChanParms(pchan, m_pdevice1);
	}
}

void CADInputParmsDlg::GetAmplifierParms(int col)
{
	if (!m_bcommandAmplifier)
		return;

	// transfer data into structure
	const auto p_chan = m_pchArray->get_p_channel(col - 1);

	// exit if cyberAmp not declared - if not, exit
	if (p_chan->am_csamplifier.Find(_T("CyberAmp")) >= 0
		|| p_chan->am_csamplifier.Find(_T("Axon")) >= 0)
	{
		// cyberAmp declared: check if connected -if not, exit
		CCyberAmp cyber_amp;
		const BOOL b_cyber_present = (cyber_amp.Initialize() == NULL);
		if (!b_cyber_present)
			return;
	}

	if (p_chan->am_csamplifier.Find(_T("Alligator")) >= 0)
	{
		const auto nAlligatorDescriptors = p_alligatordevice_ptr_array->GetCount();
		USBPxxPARAMETERS* p_device = nullptr;
		for (auto i = 0; i < nAlligatorDescriptors; i++)
		{
			auto* ptr = p_alligatordevice_ptr_array->GetAt(i);
			if (p_chan->am_amplifierchan == ptr->ChannelNumber)
			{
				p_device = ptr;
				break;
			}
		}
		if (p_device != nullptr)
		{
			m_pAlligatorAmplifier->GetWaveChanParms(p_chan, p_device);
		}
	}
}