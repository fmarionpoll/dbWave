#pragma once
#include <afxtoolbar.h>

#include "SpikeClassGridList.h"

class SpikeClassGridToolBar :
    public CMFCToolBar
{

public:
	void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler) override
	{
		CMFCToolBar::OnUpdateCmdUI(static_cast<CFrameWnd*>(GetOwner()), bDisableIfNoHndler);
	}
	BOOL AllowShowOnList() const override { return FALSE; }
};

