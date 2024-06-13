#pragma once
#include "QuadStateTree.h"

class CFilterToolBar : public CMFCToolBar
{
	void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler) override
	{
		CMFCToolBar::OnUpdateCmdUI(static_cast<CFrameWnd*>(GetOwner()), bDisableIfNoHndler);
	}

	BOOL AllowShowOnList() const override { return FALSE; }
};

class CFilterPanel : public CDockablePane
{
	// Construction
public:
	CFilterPanel();
	void AdjustLayout() override;

	// Attributes
protected:
	CQuadStateTree m_wndFilterView;
	CFilterToolBar m_wndToolBar;

	CdbWaveDoc* m_pDoc{nullptr};
	CdbWaveDoc* m_pDocOld{nullptr};
	static int m_noCol[]; // [26] succession of fields that can be filtered
	HTREEITEM m_htreeitem[26]{};

	void InitFilterList();
	void PopulateItemFromTableLong(DB_ITEMDESC* pdesc);
	void PopulateItemFromLinkedTable(DB_ITEMDESC* pdesc);
	void PopulateItemFromTablewithDate(DB_ITEMDESC* pdesc);
	void InsertAlphabetic(const CString& cs, CStringArray& csArray);
	void BuildFilterItemIndirectionFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem);
	void BuildFilterItemLongFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem);
	void BuildFilterItemDateFromTree(DB_ITEMDESC* pdesc, HTREEITEM startItem);
	void SelectNext(BOOL bNext);

	// Implementation
public:
	~CFilterPanel() override;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* p_wnd, CPoint point);
	afx_msg void OnUpdateTree();
	afx_msg void OnApplyFilter();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSortRecords();
	afx_msg void OnSelectNext();
	afx_msg void OnSelectPrevious();

public:
	afx_msg void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
