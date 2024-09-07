#pragma once
#include <afxtoolbar.h>
class SpikeClassGridToolBar :
    public CMFCToolBar
{
	DECLARE_DYNAMIC(SpikeClassGridToolBar)

public:
	void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler) override
	{
		CMFCToolBar::OnUpdateCmdUI(static_cast<CFrameWnd*>(GetOwner()), bDisableIfNoHndler);
	}
	BOOL AllowShowOnList() const override { return FALSE; }

	afx_msg void on_expand_all_properties();
	afx_msg void on_update_expand_all_properties(CCmdUI* p_cmd_ui);
	afx_msg void on_sort_properties();
	afx_msg void on_update_sort_properties(CCmdUI* p_cmd_ui);
	afx_msg void on_bn_clicked_edit_infos();
	afx_msg void on_update_bn_edit_infos(CCmdUI* p_cmd_ui);
	afx_msg void on_bn_clicked_update_infos();
	afx_msg void on_update_bn_update_infos(CCmdUI* p_cmd_ui);

	DECLARE_MESSAGE_MAP()
};

