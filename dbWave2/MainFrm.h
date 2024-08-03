#pragma once

#include "dbWaveDoc.h"
#include "PanelFilter.h"
#include "PanelProperties.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
	CMainFrame();

	// Attributes
	UINT m_SecondToolBarID;
	CMFCToolBar* m_pSecondToolBar;
	void ActivatePropertyPane(BOOL bActivate);
	void ActivateFilterPane(BOOL bActivate);

	// Overrides
	BOOL PreCreateWindow(CREATESTRUCT& cs) override;
	BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) override;

	// Implementation
	~CMainFrame() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif

protected: // control bar embedded members
	BOOL is_properties_panel_visible_;
	BOOL is_filter_pane_visible_;

	CMFCRibbonStatusBar status_bar_;
	CMFCRibbonBar ribbon_bar_;
	CMFCRibbonApplicationButton ribbon_application_button_;
	CMFCToolBarImages user_images_;
	CPropertiesPanel panel_properties_;
	CFilterPanel panel_filter_;
	CMFCOutlookBar outlook_bar_;
	CMFCOutlookBarPane outlook_pane_;

	BOOL CreateOutlookBar();
	BOOL CreateDockingPropertiesPanes();
	void SetDockingPropertiesPanesIcons(BOOL bHiColorIcons);
	CdbWaveDoc* GetMDIActiveDocument();

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnViewPropertiesWindow();
	afx_msg void OnUpdateViewPropertiesWindow(CCmdUI* pCmdUI);
	afx_msg void OnViewFilterWindow();
	afx_msg void OnUpdateViewFilterWindow(CCmdUI* pCmdUI);
	afx_msg void OnOptions();
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnDestroy();
	afx_msg void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	afx_msg void OnCheckFilterpane();
	afx_msg void OnUpdateCheckFilterpane(CCmdUI* pCmdUI);
	afx_msg void OnCheckPropertiespane();
	afx_msg void OnUpdateCheckPropertiespane(CCmdUI* pCmdUI);

	DECLARE_MESSAGE_MAP()
};
