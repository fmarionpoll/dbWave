// ScopeScreenPropsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Editctrl.h"
#include "resource.h"
#include "ChartWnd.h"
//#include "Editctrl.h"
#include "DlgChartProps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DlgChartProps::DlgChartProps(CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent), m_pscope(nullptr), m_crScopeFill(0)
{
	m_xyticks = 0;
	m_xytickline = 0;
	m_xcells = 0;
	m_ycells = 0;
	m_crScopeGrid = 0;
}

void DlgChartProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT4, m_xyticks);
	DDX_Text(pDX, IDC_EDIT5, m_xytickline);
	DDX_Text(pDX, IDC_EDIT3, m_xcells);
	DDX_Text(pDX, IDC_EDIT6, m_ycells);
}

BEGIN_MESSAGE_MAP(DlgChartProps, CDialog)
	ON_EN_CHANGE(IDC_EDIT3, OnEnChangeXCells)
	ON_EN_CHANGE(IDC_EDIT4, OnEnChangeXYTicks)
	ON_EN_CHANGE(IDC_EDIT5, OnEnChangeXYTicksLine)
	ON_EN_CHANGE(IDC_EDIT6, OnEnChangeYCells)
	ON_BN_CLICKED(IDC_BUTTON1, OnBackgroundColor)
	ON_BN_CLICKED(IDC_BUTTON2, OnGridColor)
END_MESSAGE_MAP()

BOOL DlgChartProps::OnInitDialog()
{
	CDialog::OnInitDialog();
	SCOPESTRUCT* pStruct = m_pscope->GetScopeParameters();
	m_xcells = pStruct->iXCells;
	m_ycells = pStruct->iYCells;
	m_xyticks = pStruct->iXTicks;
	m_xytickline = pStruct->iXTickLine;

	VERIFY(mm_xcells.SubclassDlgItem(IDC_EDIT3, this));
	VERIFY(mm_ycells.SubclassDlgItem(IDC_EDIT6, this));
	VERIFY(mm_xyticks.SubclassDlgItem(IDC_EDIT4, this));
	VERIFY(mm_xytickline.SubclassDlgItem(IDC_EDIT5, this));

	mm_xcells.ShowScrollBar(SB_VERT);
	mm_ycells.ShowScrollBar(SB_VERT);
	mm_xyticks.ShowScrollBar(SB_VERT);
	mm_xytickline.ShowScrollBar(SB_VERT);

	UpdateData(FALSE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DlgChartProps::OnEnChangeXCells()
{
	if (mm_xcells.m_bEntryDone)
	{
		const auto x_cells = m_xcells;
		mm_xcells.OnEnChange(this, m_xcells, 1, -1);

		// check boundaries
		if (m_xcells < 1)
			m_xcells = 1;

		if (m_xcells != x_cells)
		{
			SCOPESTRUCT* pStruct = m_pscope->GetScopeParameters();
			pStruct->xScaleUnitValue = pStruct->xScaleUnitValue * x_cells / m_xcells;
			m_xcells = x_cells;
			m_pscope->SetNxScaleCells(m_xcells, m_xyticks, m_xytickline);
			m_pscope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::OnEnChangeXYTicks()
{
	if (mm_xyticks.m_bEntryDone)
	{
		const auto xy_ticks = m_xyticks;
		mm_xyticks.OnEnChange(this, m_xyticks, 1, -1);

		// check boundaries
		if (m_xyticks < 0) m_xyticks = 0;
		if (m_xyticks != xy_ticks)
		{
			if (m_xcells > 0)
				m_pscope->SetNxScaleCells(m_xcells, m_xyticks, m_xytickline);
			if (m_ycells > 0)
				m_pscope->SetNyScaleCells(m_ycells, m_xyticks, m_xytickline);
			m_pscope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::OnEnChangeXYTicksLine()
{
	if (mm_xytickline.m_bEntryDone)
	{
		const auto xy_tick_line = m_xytickline;
		mm_xytickline.OnEnChange(this, m_xytickline, 1, -1);

		if (m_xytickline < 0) m_xytickline = 0;
		if (m_xytickline != xy_tick_line)
		{
			if (m_xcells > 0)
				m_pscope->SetNxScaleCells(m_xcells, m_xyticks, m_xytickline);
			if (m_ycells > 0)
				m_pscope->SetNyScaleCells(m_ycells, m_xyticks, m_xytickline);
			m_pscope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::OnEnChangeYCells()
{
	if (mm_ycells.m_bEntryDone)
	{
		auto y_cells = m_ycells;
		mm_ycells.OnEnChange(this, m_ycells, 1, -1);

		if (m_ycells < 1) m_ycells = 1;
		if (m_ycells != y_cells)
		{
			SCOPESTRUCT* pStruct = m_pscope->GetScopeParameters();
			pStruct->yScaleUnitValue = pStruct->yScaleUnitValue * y_cells / m_ycells;
			m_pscope->SetNyScaleCells(m_ycells, m_xyticks, m_xytickline);
			m_pscope->Invalidate();
		}
		UpdateData(FALSE);
	}
}

void DlgChartProps::OnBackgroundColor()
{
	CColorDialog dlg(m_crScopeFill, CC_RGBINIT, nullptr);
	if (IDOK != dlg.DoModal())
		return;
	(m_pscope->GetScopeParameters())->crScopeFill = dlg.GetColor();
	m_pscope->Invalidate();
}

void DlgChartProps::OnGridColor()
{
	CColorDialog dlg(m_crScopeGrid, CC_RGBINIT, nullptr);
	if (IDOK != dlg.DoModal())
		return;
	(m_pscope->GetScopeParameters())->crScopeGrid = dlg.GetColor();
	m_pscope->Invalidate();
}
