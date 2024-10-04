#pragma once

class PaneldbFilterToolBar : public CMFCToolBar
{
	void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHandler) override
	{
		auto* pTarget = static_cast<CFrameWnd*>(GetOwner());
		CMFCToolBar::OnUpdateCmdUI(pTarget, bDisableIfNoHandler);
	}
	BOOL AllowShowOnList() const override { return FALSE; }

public:
	CMFCToolBarComboBoxButton* get_combo() const
	{
		return static_cast<CMFCToolBarComboBoxButton*>(GetButton(3));
	}

private:
	afx_msg virtual void OnSize(UINT n_type, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
