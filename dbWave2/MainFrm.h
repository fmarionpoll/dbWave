#pragma once

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "dbIndexTable.h"
#include "dbMainTable.h"
#include "dbWdatabase.h"
#include "dbWaveDoc.h"

#include "PanelFilter.h"
#include "PanelProperties.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
	UINT			m_SecondToolBarID;
	CMFCToolBar*	m_pSecondToolBar;
	void		ActivatePropertyPane(BOOL bDisplay);
	void		ActivateFilterPane(BOOL bDisplay);

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
		CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	BOOL	m_bPropertiesPaneVisible;
	BOOL	m_bFilterPaneVisible;

	CMFCRibbonStatusBar	m_wndStatusBar;
	CMFCRibbonBar		m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages	m_UserImages;	
	CPropertiesWnd		m_wndProperties;
	CFilterWnd			m_wndFilter;
	CMFCOutlookBar		m_wndOutlookBar;
	CMFCOutlookBarPane	m_wndOutlookPane;

	BOOL				CreateOutlookBar();
	BOOL				CreateDockingPropertiesPanes();
	void				SetDockingPropertiesPanesIcons(BOOL bHiColorIcons);
	CdbWaveDoc*			GetMDIActiveDocument();

// Generated message map functions
	DECLARE_MESSAGE_MAP()
protected:
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
	afx_msg void OnUpdateCheckFilterpane(CCmdUI *pCmdUI);
	afx_msg void OnCheckPropertiespane();
	afx_msg void OnUpdateCheckPropertiespane(CCmdUI *pCmdUI);
};


