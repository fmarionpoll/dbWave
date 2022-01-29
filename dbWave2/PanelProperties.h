// PropertiesWnd.h

#pragma once
#include "dbWaveDoc.h"

class CPropertiesToolBar : public CMFCToolBar
{
public:
	void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler) override
	{
		CMFCToolBar::OnUpdateCmdUI(static_cast<CFrameWnd*>(GetOwner()), bDisableIfNoHndler);
	}

	BOOL AllowShowOnList() const override { return FALSE; }
};

class CPropertiesWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CPropertiesWnd)
public:
	CPropertiesWnd();
	~CPropertiesWnd() override;
	void AdjustLayout() override;

protected:
	CPropertiesToolBar m_wndToolBar;
	CMFCPropertyGridCtrl m_wndPropList;

	// Implementation
public:
protected:
	CdbWaveDoc* m_pDoc;
	CdbWaveDoc* m_pDocOld;
	CFont m_fntPropList;

	static int m_noCol[]; // [26] succession of fields
	static int m_propCol[NTABLECOLS]; // no col (0 to 28), no group (1, 2, 3 or -1 if not displayed) and type ()
	CUIntArray m_typeProps;
	CUIntArray m_iIDProps;
	CUIntArray m_groupProps;
	int m_wndEditInfosHeight;
	BOOL m_bUpdateCombos;
	BOOL m_bchangedProperty;

	int InitGroupFromTable(CMFCPropertyGridProperty* pGroup, int icol0);
	void UpdateGroupPropFromTable(CMFCPropertyGridProperty* pGroup);
	void InitPropList();
	void UpdatePropList();
	void SetPropListFont();
	void UpdateTableFromProp();
	void UpdateTableFromGroupProp(CMFCPropertyGridProperty* pGroup);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnBnClickedEditinfos();
	afx_msg void OnUpdateBnEditinfos(CCmdUI* pCmdUI);
	afx_msg void OnBnClickedUpdateinfos();
	afx_msg void OnUpdateBnUpdateinfos(CCmdUI* pCmdUI);
	afx_msg LRESULT OnPropertyChanged(WPARAM, LPARAM);

public:
	afx_msg void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
