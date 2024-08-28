#pragma once
#include <afx.h>

#include "ColorNames.h"

class SpikeClassListBoxContext :
    public CObject
{
public:
	int			m_left_column_width {20};
	int			m_row_height {20};
	int			m_width_separator {5};
	int			m_width_text {-1};
	int			m_width_spikes {-1};
	int			m_width_bars {-1};
	int			m_top_index {-1};
	COLORREF	m_color_text {col_black};
	COLORREF	m_color_background {GetSysColor(COLOR_SCROLLBAR)};;
	CBrush		m_brush_background;
};

