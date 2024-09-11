#pragma once

class PaneldbFilterToolBar : public CMFCToolBar
{
	void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler) override
	{
		auto* pTarget = static_cast<CFrameWnd*>(GetOwner());
		CMFCToolBar::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);
	}
	BOOL AllowShowOnList() const override { return FALSE; }

	afx_msg virtual void OnSize(UINT n_type, int cx, int cy);
	DECLARE_MESSAGE_MAP()
};
