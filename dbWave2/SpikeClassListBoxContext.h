#pragma once
#include <afx.h>
class SpikeClassListBoxContext :
    public CObject
{
public:
	int m_left_column_width = 20;
	int m_row_height = 20;
	int m_widthSeparator = 5;
	int m_widthText = -1;
	int m_widthSpikes = -1;
	int m_widthBars = -1;
	int m_topIndex = -1;
	COLORREF m_color_text = RGB(0, 0, 0);
	COLORREF m_color_background = GetSysColor(COLOR_SCROLLBAR);;
	CBrush m_brush_background;
};

