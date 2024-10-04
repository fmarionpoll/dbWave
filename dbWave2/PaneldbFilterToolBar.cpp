#include "stdafx.h"
#include "PaneldbFilterToolBar.h"


BEGIN_MESSAGE_MAP(PaneldbFilterToolBar, CMFCToolBar)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void PaneldbFilterToolBar::OnSize(const UINT n_type, const int cx, const int cy)
{
	CMFCToolBar::OnSize(n_type, cx, cy);
	//TRACE("onsize n_type=%i cx=%i cy=%i \n", n_type, cx, cy);
	const auto p_combo = static_cast<CMFCToolBarComboBoxButton*>(GetButton(3));
}

