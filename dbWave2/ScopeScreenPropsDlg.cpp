// ScopeScreenPropsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "editctrl.h"
#include "resource.h"
#include "scopescr.h"
#include "editctrl.h"
#include "ScopeScreenPropsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CScopeScreenPropsDlg dialog


CScopeScreenPropsDlg::CScopeScreenPropsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScopeScreenPropsDlg::IDD, pParent)
{
	m_xyticks = 0;
	m_xytickline = 0;
	m_xcells = 0;
	m_ycells = 0;
	m_crScopeGrid = 0;
}


void CScopeScreenPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT4, m_xyticks);
	DDX_Text(pDX, IDC_EDIT5, m_xytickline);
	DDX_Text(pDX, IDC_EDIT3, m_xcells);
	DDX_Text(pDX, IDC_EDIT6, m_ycells);
}


BEGIN_MESSAGE_MAP(CScopeScreenPropsDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT3, OnEnChangeXCells)
	ON_EN_CHANGE(IDC_EDIT4, OnEnChangeXYTicks)
	ON_EN_CHANGE(IDC_EDIT5, OnEnChangeXYTicksLine)
	ON_EN_CHANGE(IDC_EDIT6, OnEnChangeYCells)
	ON_BN_CLICKED(IDC_BUTTON1, OnBackgroundColor)
	ON_BN_CLICKED(IDC_BUTTON2, OnGridColor)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScopeScreenPropsDlg message handlers

BOOL CScopeScreenPropsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_xcells = m_pscope->m_parms.iXCells;
	m_ycells = m_pscope->m_parms.iYCells;
	m_xyticks = m_pscope->m_parms.iXTicks;
	m_xytickline = m_pscope->m_parms.iXTickLine;

	VERIFY(mm_xcells.SubclassDlgItem(IDC_EDIT3, this));
	VERIFY(mm_ycells.SubclassDlgItem(IDC_EDIT6, this));
	VERIFY(mm_xyticks.SubclassDlgItem(IDC_EDIT4, this));
	VERIFY(mm_xytickline.SubclassDlgItem(IDC_EDIT5, this));

	mm_xcells.ShowScrollBar(SB_VERT);
	mm_ycells.ShowScrollBar(SB_VERT);
	mm_xyticks.ShowScrollBar(SB_VERT);
	mm_xytickline.ShowScrollBar(SB_VERT);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CScopeScreenPropsDlg::OnEnChangeXCells() 
{
	if (!mm_xcells.m_bEntryDone)
		return;

	int xcells = m_xcells;
	switch (mm_xcells.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:
		UpdateData(TRUE);
		xcells = m_xcells;
		break;
	case VK_UP:
	case VK_PRIOR:
		xcells++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		xcells--;
		break;
	}
	// check boundaries
	if (xcells < 1) xcells = 1;

	// change display if necessary
	mm_xcells.m_bEntryDone=FALSE;	// clear flag
	mm_xcells.m_nChar=0;			// empty buffer
	mm_xcells.SetSel(0, -1);		// select all text
	if (m_xcells != xcells)
	{
		m_pscope->m_parms.xScaleUnitValue = m_pscope->m_parms.xScaleUnitValue * m_xcells / xcells;
		m_xcells=xcells;
		m_pscope->SetNxScaleCells(m_xcells, m_xyticks, m_xytickline);
		m_pscope->Invalidate();
	}
	UpdateData(FALSE);
	return;
}

void CScopeScreenPropsDlg::OnEnChangeXYTicks() 
{
	if (!mm_xyticks.m_bEntryDone)
		return;

	int xyticks = m_xyticks;
	switch (mm_xyticks.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:
		UpdateData(TRUE);
		xyticks = m_xyticks;
		break;
	case VK_UP:
	case VK_PRIOR:
		xyticks++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		xyticks--;
		break;
	}
	// check boundaries
	if (xyticks < 0) xyticks = 0;

	// change display if necessary
	mm_xyticks.m_bEntryDone=FALSE;	// clear flag
	mm_xyticks.m_nChar=0;			// empty buffer
	mm_xyticks.SetSel(0, -1);		// select all text
	if (m_xyticks != xyticks)
	{
		m_xyticks=xyticks;
		if (m_xcells > 0)
			m_pscope->SetNxScaleCells(m_xcells, m_xyticks, m_xytickline);
		if (m_ycells > 0)
			m_pscope->SetNyScaleCells(m_ycells, m_xyticks, m_xytickline);
		m_pscope->Invalidate();
	}
	UpdateData(FALSE);
	return;
}

void CScopeScreenPropsDlg::OnEnChangeXYTicksLine() 
{
	if (!mm_xytickline.m_bEntryDone)
		return;

	int xytickline = m_xytickline;
	switch (mm_xytickline.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:
		UpdateData(TRUE);
		xytickline = m_xytickline;
		break;
	case VK_UP:
	case VK_PRIOR:
		xytickline++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		xytickline--;
		break;
	}
	// check boundaries
	if (xytickline < 0)	xytickline = 0;

	// change display if necessary
	mm_xytickline.m_bEntryDone=FALSE;	// clear flag
	mm_xytickline.m_nChar=0;			// empty buffer
	mm_xytickline.SetSel(0, -1);		// select all text
	if (m_xytickline != xytickline)
	{
		m_xytickline = xytickline;
		if (m_xcells > 0)
			m_pscope->SetNxScaleCells(m_xcells, m_xyticks, m_xytickline);
		if (m_ycells > 0)
			m_pscope->SetNyScaleCells(m_ycells, m_xyticks, m_xytickline);
		m_pscope->Invalidate();
	}
	UpdateData(FALSE);
	return;
}

void CScopeScreenPropsDlg::OnEnChangeYCells() 
{
	if (!mm_ycells.m_bEntryDone)
		return;

	int ycells = m_ycells;
	switch (mm_ycells.m_nChar)
	{				// load data from edit controls
	case VK_RETURN:
		UpdateData(TRUE);
		ycells = m_ycells;
		break;
	case VK_UP:
	case VK_PRIOR:
		ycells++;
		break;
	case VK_DOWN:
	case VK_NEXT:
		ycells--;
		break;
	}
	// check boundaries
	if (ycells < 1) ycells = 1;

	// change display if necessary
	mm_ycells.m_bEntryDone=FALSE;	// clear flag
	mm_ycells.m_nChar=0;			// empty buffer
	mm_ycells.SetSel(0, -1);		// select all text
	if (m_ycells != ycells)
	{
		m_pscope->m_parms.yScaleUnitValue = m_pscope->m_parms.yScaleUnitValue * m_ycells / ycells;
		m_ycells=ycells;		
		m_pscope->SetNyScaleCells(m_ycells, m_xyticks, m_xytickline);
		m_pscope->Invalidate();
	}
	UpdateData(FALSE);
	return;
}

void CScopeScreenPropsDlg::OnBackgroundColor() 
{
	CColorDialog dlg(m_crScopeFill, CC_RGBINIT, nullptr);
	if(IDOK != dlg.DoModal())
		return;
	m_pscope->m_parms.crScopeFill= dlg.GetColor();
	m_pscope->Invalidate();
	
}

void CScopeScreenPropsDlg::OnGridColor() 
{
	CColorDialog dlg(m_crScopeGrid, CC_RGBINIT, nullptr);
	if(IDOK != dlg.DoModal())
		return;
	m_pscope->m_parms.crScopeGrid= dlg.GetColor();
	m_pscope->Invalidate();
}


