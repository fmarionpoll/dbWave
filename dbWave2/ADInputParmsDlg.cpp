// ADInputParmsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "dbWave.h"
#include <Olxdadefs.h>
#include "GridCtrl\GridCell.h"
#include "GridCtrl\GridCellCombo_FMP.h"
#include "GridCtrl\GridCellNumeric.h"
#include "GridCtrl\GridCellCheck.h"
#include "adinputparmsdlg.h"

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
	_T("")};
TCHAR*	CADInputParmsDlg::pszHighPass[]	= { _T("GND"),_T("DC"),_T("0.1"),_T("1"),_T("10"),_T("30"),_T("100"),_T("300"),
											_T(""),_T(""),_T(""),_T(""),_T(""),_T(""),_T(""),_T("")};
TCHAR*	CADInputParmsDlg::pszADGains[]	= { _T("1"),_T("2"),_T("4"),_T("8"),_T(""),_T(""),_T(""),_T("")};
TCHAR*	CADInputParmsDlg::pszAmplifier[]	= { _T("CyberAmp 320"),_T("Neurolog"),_T("IDAC"),_T("Dagan"),_T("none"),_T(""),_T(""),_T("")};
TCHAR*	CADInputParmsDlg::pszProbeType[]	= { _T("Syntech"),_T("DTP02"),_T("AI401"),_T("Neurolog"),_T("AD575"),_T("Dagan"),_T("none"),_T("")};
TCHAR*	CADInputParmsDlg::pszEncoding[]	= { _T("Offset binary"),_T("Two's complement"),_T("Unknown"),_T(""),_T(""),_T(""),_T(""),_T("")};
int		CADInputParmsDlg::iEncoding[]= { OLx_ENC_BINARY, OLx_ENC_2SCOMP, -1};

IMPLEMENT_DYNAMIC(CADInputParmsDlg, CDialog)
CADInputParmsDlg::CADInputParmsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CADInputParmsDlg::IDD, pParent)
	, m_nacqchans(0)
	, m_maxchans(0)
{
	m_OldSize = CSize(-1,-1);
	m_iNBins = 4096;
	m_xVoltsMax = 10.0f;
	m_bchainDialog = FALSE;
	m_pwFormat=NULL;
	m_pchArray=NULL;
	m_numchansMAXDI = 8;
	m_numchansMAXSE = 16;
	m_bcommandAmplifier=FALSE;
	m_inputlistmax = 32;				// chan list size for DT9800
}

CADInputParmsDlg::~CADInputParmsDlg()
{
}

void CADInputParmsDlg::DoDataExchange(CDataExchange* pDX)
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
	int showWindow = SW_HIDE;
	if (m_bchainDialog)
		showWindow = SW_SHOW;
	GetDlgItem(IDC_ADCHANNELS)->ShowWindow(showWindow);
	GetDlgItem(IDC_ADINTERVALS)->ShowWindow(showWindow);
	
	// display nb of available channels according to parameter
	if (m_bchantype == OLx_CHNT_SINGLEENDED)
	{
		((CButton*) GetDlgItem(IDC_SINGLEENDED))->SetCheck(BST_CHECKED);
		m_maxchans = m_numchansMAXSE;
	}
	else
	{
		((CButton*) GetDlgItem(IDC_DIFFERENTIAL))->SetCheck(BST_CHECKED);
		m_maxchans = m_numchansMAXDI;
	}

	// set spin  max list: dt9800 = up to 32 channels
	((CSpinButtonCtrl*) GetDlgItem(IDC_SPIN1))->SetRange(1, m_inputlistmax);
	// load encoding combo with data
	int i= 0;
	do {
		int j=m_encodingCombo.AddString(pszEncoding[i]);
		m_encodingCombo.SetItemData(j, (DWORD) iEncoding[i]);
		i++;
	} while (pszEncoding[i] != _T(""));

	// eventually load data
	if (m_pwFormat==NULL)
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
		m_iNBins = m_pwFormat->binspan+1;
		int ibins = m_iNBins;
		int nbits = 0;
		do {
			ibins = ibins/2;
			nbits++;
		} while (ibins > 1);
		
		// select corresponding entry in the combobox
		CString cs;
		cs.Format(_T("%i"), nbits);
		int i = m_resolutionCombo.FindStringExact(-1, cs);	
		if (i != CB_ERR)
			m_resolutionCombo.SetCurSel(i);
		
		// select encoding mode according to parameter in waveFormat
		int icur=0;
		for (int icur = 0; icur < m_encodingCombo.GetCount(); icur++)
			if (m_encodingCombo.GetItemData(icur) == (DWORD) (m_pwFormat->mode_encoding ))
				break;
		if (icur == m_encodingCombo.GetCount())
			icur --;
		m_encodingCombo.SetCurSel(icur);
	}

	/////////////////////////////////////////////////////////////////////////
	// initialise grid properties
	/////////////////////////////////////////////////////////////////////////

	m_Grid.EnableDragAndDrop(TRUE);
	m_Grid.GetDefaultCell(FALSE, FALSE)->SetBackClr(RGB(0xFF, 0xFF, 0xFF));
	m_Grid.SetColumnCount (m_nacqchans +1); 
	m_Grid.SetFixedColumnCount(1);
	m_Grid.SetFixedRowCount(1);

	// get font and create bold face 
	CFont *pFont = m_Grid.GetFont();
	ASSERT (pFont != NULL);
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;

	// set row headers
	int nrows = 0;				// count number of rows
	do {
		nrows++;
	} while (pszRowTitle[nrows-1] != _T(""));
	m_Grid.SetRowCount(nrows);

	// init row headers with descriptors
	int col = 0;
	int row = 0;
	do {
		row++;
		m_Grid.SetItemText(row, col, pszRowTitle[row-1]);
		if (pszRowTitle[row-1][0] == ' ')
			m_Grid.SetItemFormat(row, col, DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
		else
			m_Grid.SetItemFont(row, col, &lf);

	} while (pszRowTitle[row] != _T(""));
	CString cs; 
	cs.Format(_T("A/D channel (0-%i)"), m_maxchans-1);
	m_Grid.SetItemText(2, 0, cs);

	// init data columns
	for (col=1; col<= m_nacqchans; col++)
	{
		InitGridColumnDefaults(col);
		if (m_pwFormat!=NULL)
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
	cs.Format(_T("A/D channel (0-%i)"), m_maxchans-1);
	m_Grid.SetItemText(m_rowADchannel, 0, cs);
	m_Grid.RedrawCell(m_rowADchannel, 0, NULL);

	// update combos / acq channels
	for (int i=1; i<= m_nacqchans; i++)
		InitADchannelCombo(i, -1);

	AdjustGridSize();
	UpdateData(FALSE);
}

void CADInputParmsDlg::AdjustGridSize()
{
	int noldcols = m_Grid.GetColumnCount();
	// reduce size of the grid; delete last columns
	if (noldcols > m_nacqchans+1)
	{
		for (int i = noldcols-1; i> m_nacqchans; i--)
			m_Grid.DeleteColumn(i);
		m_Grid.Refresh();
	}
	// increase size of the grid
	else if (noldcols < m_nacqchans+1)
	{
		m_Grid.SetColumnCount(m_nacqchans +1);
		for (int i= noldcols; i< m_nacqchans+1; i++)
			InitGridColumnDefaults(i);
		m_Grid.Refresh();
	}
}

BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lParam)
{
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	CSize* pTranslate = (CSize*) lParam;

	CADInputParmsDlg* pDlg = (CADInputParmsDlg*) pWnd->GetParent();
	if (!pDlg) return FALSE;

	CRect rect;
	pWnd->GetWindowRect(rect);
	pDlg->ScreenToClient(rect);
	if (hwnd == pDlg->m_Grid.GetSafeHwnd())
	{
		// move grid
		if (  ((rect.top >= 7 && pTranslate->cy > 0) || rect.Height() > 20) && ((rect.left >= 7 && pTranslate->cx > 0) || rect.Width() > 20)   )
			pWnd->MoveWindow(rect.left, rect.top,		rect.Width()+pTranslate->cx,	rect.Height()+pTranslate->cy,	FALSE);
		else
			pWnd->MoveWindow(rect.left+pTranslate->cx,	rect.top+pTranslate->cy,		rect.Width(), rect.Height(),	FALSE);
	}
	else 
	{
		int iID = pWnd->GetDlgCtrlID();
		switch (iID)
		{
		case IDC_SIZEBOX:     // move size box on the lower right corner
			pWnd->MoveWindow(rect.left+pTranslate->cx, rect.top+pTranslate->cy, 
							 rect.Width(), rect.Height(), FALSE);
			break;
		case IDOK:	// other windows to move with the right size switch..
		case IDCANCEL:
		case IDC_ADCHANNELS:
		case IDC_ADINTERVALS:
			pWnd->MoveWindow(rect.left+pTranslate->cx, rect.top, rect.Width(), rect.Height(), FALSE);
		break;
	default:
		break;
		}
	}
	pDlg->Invalidate();

	return TRUE;
}

void CADInputParmsDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (cx <= 1 || cy <= 1 ) 
		return;

	CSize Translate(cx - m_OldSize.cx, cy - m_OldSize.cy);
	::EnumChildWindows(GetSafeHwnd(), EnumProc, (LPARAM)&Translate);
	m_OldSize = CSize(cx,cy);

	CWnd *pWnd = GetDlgItem(IDC_SIZEBOX);
	if (pWnd)
		pWnd->ShowWindow( (nType == SIZE_MAXIMIZED)? SW_HIDE : SW_SHOW);
}


//////////////////////////////////////////////////////
// set column with proper header, combos, etc and data

void CADInputParmsDlg::InitADchannelCombo(int col, int iselect)
{
	// build string array with channel list
	CStringArray csArrayOptions;
	CString cs;
	for (int i= 0; i < m_maxchans; i++)
	{
		cs.Format(_T("%i"), i);
		csArrayOptions.Add(cs);	
	}
	csArrayOptions.Add(_T("16 (Din)"));

	// select cell and corresponding combo
	CGridCellCombo *pCell = (CGridCellCombo*) m_Grid.GetCell(m_rowADchannel, col);
	if (pCell != NULL)
	{
		// if iselect negative, get current selection
		if (iselect < 0)
			iselect = pCell->GetCurSel();
		pCell->SetOptions(csArrayOptions);
		pCell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		pCell->SetCurSel(iselect);
	}
}

BOOL CADInputParmsDlg::InitGridColumnDefaults(int col)
{
	if (col < 1 || col >= m_Grid.GetColumnCount())
		return FALSE;

	// get font and create bold face 
	CFont *pFont = m_Grid.GetFont();
	ASSERT (pFont != NULL);
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;

	// set columns headers
	int row = 0;
	GV_ITEM Item;
	Item.mask = GVIF_TEXT|GVIF_FORMAT;
	Item.row = row;
	Item.col = col;
	Item.nFormat = DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX;
	Item.strText.Format(_T("ch %i"),col);
	m_Grid.SetItem(&Item);
	m_Grid.SetItemFont(row, col, &lf);
	
	// set cells content
	//"Title",
	row++;
	Item.row = row;
	Item.strText.Format(_T("channel %i"),col);
	Item.nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX;
	m_Grid.SetItem(&Item);

	//"A/D channel" - (col-1)
	row++;
	m_rowADchannel = row;
	if (m_Grid.SetCellType(m_rowADchannel, col, RUNTIME_CLASS(CGridCellCombo)))
		InitADchannelCombo(col, col-1);

	// "A/D gain" - combo
	row++;
	m_row_ADgain = row;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray csArrayOptions;
		int i= 0;
		do {
			csArrayOptions.Add(pszADGains[i]);
			i++;
		} while (pszADGains[i] != _T(""));
		CGridCellCombo *pCell = (CGridCellCombo*) m_Grid.GetCell(row, col);
		pCell->SetOptions(csArrayOptions);
		pCell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
		else
			pCell->SetCurSel(0);
	}

	// "Amplifier type" - combo
	row++;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray csArrayOptions;
		int i= 0;
		do {
			csArrayOptions.Add(pszAmplifier[i]);
			i++;
		} while (pszAmplifier[i] != _T(""));
		CGridCellCombo *pCell = (CGridCellCombo*) m_Grid.GetCell(row, col);
		pCell->SetOptions(csArrayOptions);
		pCell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
		else
			pCell->SetCurSel(0);
	}

	// " output channel" - numeric
	row++;
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	// init value
	Item.row = row;
	Item.strText.Format(_T("%d"),col);
	m_Grid.SetItem(&Item);

	// " gain" - numeric
	row++;
	m_row_ampgain = row;				// save index for later
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	// init value
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
	else
		m_Grid.SetItemText(row, col, _T("500"));

	// " notch filter" - check box
	row++;
	Item.strText = _T("50 Hz");
	Item.row = row;
	m_Grid.SetItem(&Item);
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCheck));

	// " IN+" - combo
	row++;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray csArrayOptions;
		int i= 0;
		do {
			csArrayOptions.Add(pszHighPass[i]);
			i++;
		} while (pszHighPass[i] != _T(""));
		CGridCellCombo *pCell = (CGridCellCombo*) m_Grid.GetCell(row, col);
		pCell->SetOptions(csArrayOptions);
		pCell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
		else
			pCell->SetCurSel(1);
	}

	// DC Offset (mV) " IN-" - combo
	row++;
	//if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	//{
	//	CStringArray csArrayOptions;
	//	int i= 0;
	//	do {
	//		csArrayOptions.Add(pszHighPass[i]);
	//		i++;
	//	} while (pszHighPass[i] != _T(""));
	//	CGridCellCombo *pCell = (CGridCellCombo*) m_Grid.GetCell(row, col);
	//	pCell->SetOptions(csArrayOptions);
	//	pCell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
	//	// init value
	//	if (col > 1)
	//		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
	//	else
	//		pCell->SetCurSel(1);
	//} 
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col - 1));
	else
		m_Grid.SetItemText(row, col, _T("0.00"));

	// " low pass filter" - numeric
	row++;
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
	else
		m_Grid.SetItemText(row, col, _T("2800"));

	// "Probe type" - combo
	row++;
	if (m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellCombo)))
	{
		CStringArray csArrayOptions;
		int i= 0;
		do {
			csArrayOptions.Add(pszProbeType[i]);
			i++;
		} while (pszProbeType[i] != _T(""));
		CGridCellCombo *pCell = (CGridCellCombo*) m_Grid.GetCell(row, col);
		pCell->SetOptions(csArrayOptions);
		pCell->SetStyle(CBS_DROPDOWN); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		// init value
		if (col > 1)
			m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
		else
			pCell->SetCurSel(0);
	}

	// " gain" - numeric
	row++;
	m_row_headstagegain = row;				// save index for later
	m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
	// init value
	if (col > 1)
		m_Grid.SetItemText(row, col, m_Grid.GetItemText(row, col-1));
	else
		m_Grid.SetItemText(row, col, _T("10"));

	// read only / numeric fields
	m_row_readonly = row+1;
	COLORREF clr = m_Grid.GetDefaultCell(TRUE, TRUE)->GetBackClr();
	for (int i=0; i<3; i++)
	{
		row++;
		m_Grid.SetCellType(row, col, RUNTIME_CLASS(CGridCellNumeric));
		m_Grid.SetItemState(row, col,  m_Grid.GetItemState(1,1) | GVIS_READONLY);
		m_Grid.SetItemBkColour(row, col, clr);
	}
	InitGridColumnReadOnlyFields(col);

	return TRUE;
}
void CADInputParmsDlg::InitGridColumnReadOnlyFields(int col)
{
	// "Signal total gain" - read only / numeric
	int row = m_row_readonly;
	CString cs;
	int igain;
	cs = m_Grid.GetItemText(m_row_ampgain, col);
	igain = _ttoi(cs);
	cs = m_Grid.GetItemText(m_row_headstagegain, col);
	igain *= _ttoi(cs);
	cs = m_Grid.GetItemText(m_row_ADgain, col);
	igain *= _ttoi(cs);
	cs.Format(_T("%1.3f"), (m_xVoltsMax*1000.f / float(igain)));
	m_Grid.SetItemText(row, col, cs);

	// "total gain" - read only / numeric
	row++;
	cs.Format(_T("%li"), igain);		// = ampgain * probegain * ADgain
	m_Grid.SetItemText(row, col, cs);

	// " resolution (µV)" - read only
	row++;
	cs.Format(_T("%1.3f"), (m_xVoltsMax * 1.E6 / float(igain*2) / float(m_iNBins)));
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

void CADInputParmsDlg::OnGridEndEdit(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
	switch (pItem->iRow) 
	{
	//case 1: // title - no action
	//case 2: // A/D channel (0-7), (0-15) 
	case 3: // A/D gain
		InitGridColumnReadOnlyFields(pItem->iColumn);
		break;
	case 4: // ampli
	case 5: // ampli out chan 
		SaveChanData(pItem->iColumn);
		SetAmplifierParms(pItem->iColumn);
		break;
	case 6: // ampli gain
		SaveChanData(pItem->iColumn);
		SetAmplifierParms(pItem->iColumn);
		InitGridColumnReadOnlyFields(pItem->iColumn);
		break;
	case 7: // notch 
	case 8: // IN+ 
	case 9: // low pass
	case 10:
		SaveChanData(pItem->iColumn);
		SetAmplifierParms(pItem->iColumn);
		break;
	//case 11: // headstage 
	case 12:  // headstage gain
		InitGridColumnReadOnlyFields(pItem->iColumn);
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
	int cursel = m_resolutionCombo.GetCurSel();
	m_resolutionCombo.GetWindowText(cs);
	int nbits = _ttoi(cs);
	UINT oldNBins = m_iNBins;
	m_iNBins = 1 << nbits; // 2 exp (nbits)
	// change input resolution if value has changed
	if (m_iNBins != oldNBins)
	{
		for (int i= 1; i<= m_nacqchans; i++)
			InitGridColumnReadOnlyFields(i);
		m_Grid.Refresh();
	}
}

// load column data (index = 1, n)
void CADInputParmsDlg::LoadChanData(int col)
{
	// select channel and load parameters CComboBox
	ASSERT(col <= m_pchArray->ChannelGetnum());
	CWaveChan* pchan = m_pchArray->GetWaveChan(col-1);

	// settings for the col
	GV_ITEM Item;
	Item.mask = GVIF_TEXT|GVIF_FORMAT;
	Item.col = col;
	// ----------------------------------------
	// AD channel comment
	Item.row = 1;
	Item.nFormat = DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX;
	Item.strText = pchan->am_csComment;
	if (pchan->am_csComment != _T(""))
		m_Grid.SetItem(&Item);
	// AD input channel number
	Item.row++;
	Item.strText.Format(_T("%i"),pchan->am_adchannel);
	m_Grid.SetItem(&Item);
	// " ADgain" - combo
	Item.row++;
	Item.strText.Format(_T("%i"),pchan->am_adgain);
	m_Grid.SetItem(&Item);
	// ----------------------------------------
	// amplifier name
	Item.row++;
	Item.strText = pchan->am_csamplifier;	
	m_Grid.SetItem(&Item);
	// amplifier channel
	Item.row++;
	Item.strText.Format(_T("%i"),pchan->am_amplifierchan);
	m_Grid.SetItem(&Item);
	// amplifier gain
	Item.row++;
	double gain = (pchan->am_totalgain)/(pchan->am_gainheadstage*pchan->am_adgain);
	int igain = (int) gain;
	if (igain == 0)
		igain = 1;
	Item.strText.Format(_T("%d"), igain);
	m_Grid.SetItem(&Item);
	// amplifier notch filter
	Item.row++;
	((CGridCellCheck*) m_Grid.GetCell(Item.row, Item.col))->SetCheck(pchan->am_notchfilt);
	// IN+
	Item.row++;
	Item.strText=pchan->am_csInputpos;
	m_Grid.SetItem(&Item);
	// DC Offset (mV) // IN-
	Item.row++;
	//Item.strText=pchan->am_csInputneg;
	Item.strText.Format(_T("%0.3f"), pchan->am_offset);
	m_Grid.SetItem(&Item);
	// amplifier low pass filter
	Item.row++;
	Item.strText.Format(_T("%i"),pchan->am_lowpass);
	m_Grid.SetItem(&Item);
	// ----------------------------------------
	// headstage
	Item.row++;
	Item.strText=pchan->am_csheadstage;
	m_Grid.SetItem(&Item);
	// headstage gain
	Item.row++;
	Item.strText.Format(_T("%i"),pchan->am_gainheadstage);
	m_Grid.SetItem(&Item);

	// set read only fields
	InitGridColumnReadOnlyFields(col);
}

void CADInputParmsDlg::SaveData()
{
	// if number of acq channels changed, change corresp structures
	if (m_nacqchans != m_pwFormat->scan_count)
	{
		m_pwFormat->scan_count = m_nacqchans;
		m_pchArray->ChannelSetnum(m_nacqchans);
		WORD chbuffersize = m_pwFormat->buffersize/m_nacqchans;
		m_pwFormat->buffersize = chbuffersize *m_nacqchans;
	}
	// general acq parameters
	GetDlgItem(IDC_ADCARDNAME)->GetWindowText(m_pwFormat->csADcardName);
	m_pwFormat->binspan = m_iNBins;

	// save each channel
	for (int i=1; i<= m_nacqchans; i++)
	{
		SaveChanData(i);
		SetAmplifierParms(i);
	}
}

// save column data (index = 1, n)
void CADInputParmsDlg::SaveChanData(int col)
{
	// select channel and load parameters CComboBox
	if (m_pchArray->ChannelGetnum() < col)
		m_pchArray->ChannelSetnum(col);
	CWaveChan* pchan = m_pchArray->GetWaveChan(col-1);
	CString cs;

	// AD channel comment
	int row = 1;
	pchan->am_csComment = m_Grid.GetItemText(row, col);
	// "AD input channel" - combo
	row++;
	cs = m_Grid.GetItemText(row, col);
	pchan->am_adchannel = _ttoi(cs);	// digital channels "16 [DIN]" is OK because of the space after "16"
	
	// " ADgain" - combo
	row++;
	cs = m_Grid.GetItemText(row, col);
	pchan->am_adgain = _ttoi(cs);

	// amplifier name
	row++;
	pchan->am_csamplifier = m_Grid.GetItemText(row, col);

	// amplifier channel
	row++;
	cs = m_Grid.GetItemText(row, col);
	pchan->am_amplifierchan = _ttoi(cs);

	// amplifier gain
	row++;
	cs = m_Grid.GetItemText(row, col);
	pchan->am_gainpre = _ttoi(cs);
	pchan->am_gainpost = 1;
	//pchan->am_gainfract = _ttoi(cs);

	// amplifier notch filter
	row++;
	pchan->am_notchfilt= ((CGridCellCheck*) m_Grid.GetCell(row, col))->GetCheck();

	// amplifier high pass filter (IN+)
	row++;
	pchan->am_csInputpos= m_Grid.GetItemText(row, col);
	// DC Offset(mV) // IN-
	row++;
	//pchan->am_csInputneg= m_Grid.GetItemText(row, col);
	cs = m_Grid.GetItemText(row, col);
	pchan->am_offset = (float) _tstof(cs);
	// amplifier low pass filter
	row++;
	cs = m_Grid.GetItemText(row, col);
	pchan->am_lowpass = _ttoi(cs);

	// headstage
	row++;
	pchan->am_csheadstage = m_Grid.GetItemText(row, col);
	// headstage gain
	row++;
	cs = m_Grid.GetItemText(row, col);
	pchan->am_gainheadstage= _ttoi(cs);
	pchan->am_gainfract = pchan->am_gainheadstage * (float)pchan->am_gainpre * (float)pchan->am_gainpost;
	pchan->am_totalgain = pchan->am_gainfract * pchan->am_adgain;
	// compute dependent parameters
	pchan->am_resolutionV = m_pwFormat->fullscale_Volts / pchan->am_totalgain / m_pwFormat->binspan;
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


#include "cyberAmp.h"

void CADInputParmsDlg::SetAmplifierParms(int col)
{
	if (!m_bcommandAmplifier)
		return;

	MessageBox(_T("set ampli parms"));
	// transfer data into structure
	CWaveChan* pchan = m_pchArray->GetWaveChan(col-1);

	// exit if cyberAmp not declared - if not, exit
	int a = pchan->am_csamplifier.Find(_T("CyberAmp"));
	int b = pchan->am_csamplifier.Find(_T("Axon")); 
	if (a != 0 && b != 0) 
		return;

	// cyberAmp declared: check if connected -if not, exit
	CCyberAmp cyberAmp;
	BOOL bcyberPresent = (cyberAmp.Initialize() == NULL);
	if (!bcyberPresent)
		return;

	// chan, gain, filter +, lowpass, notch	
	cyberAmp.SetHPFilter(pchan->am_amplifierchan, C300_POSINPUT, pchan->am_csInputpos);	
	cyberAmp.SetHPFilter(pchan->am_amplifierchan, C300_NEGINPUT, pszHighPass[0]);
	cyberAmp.SetmVOffset(pchan->am_amplifierchan, pchan->am_offset);
	cyberAmp.SetNotchFilter(pchan->am_amplifierchan, pchan->am_notchfilt);
	double gain = pchan->am_totalgain/(pchan->am_gainheadstage*pchan->am_adgain);
	cyberAmp.SetGain(pchan->am_amplifierchan, (int) gain);
	cyberAmp.SetLPFilter(pchan->am_amplifierchan, (int) (pchan->am_lowpass));
	int errorcode = cyberAmp.C300_FlushCommandsAndAwaitResponse();
}


