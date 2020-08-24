//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// QuadStateTree.h - Header file for the CQuadStateTree class                               //
//                                                                                          //
// Written by : PJ Arends http://www.codeproject.com/script/Membership/View.aspx?mid=8817   //
//                                                                                          //
// Licence : CodeProject Open Licence http://www.codeproject.com/info/cpol10.aspx           //
//                                                                                          //
// Web: http://www.codeproject.com/Articles/847799/CQuadStateTree                           //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#if _WIN32_WINNT < 0x0600
#error _WIN32_WINNT must be greater than or equal to 0x0600
#endif

#if _WIN32_IE <= 0x0600
#error _WIN32_IE must be greater than 0x0600
#endif

// The TVN_CHECK notification code notifies the parent window that an
// item's check box state is about to change. This notification is sent
// in the form of a WM_NOTIFY message. Information about the change is
// contained in a NMTVNCHECK structure. Setting the return value, via
// pResult, to a non-zero value will stop the checkbox state from
// changing, if the tree item is the one that was initially triggered
// by a user action or a call to SetCheck(). This can be checked by
// comparing the values of NMTVNCHECK::hTreeItem and NMTVNCHECK::TriggerItem
// and seeing if they are the same value.
//
// According to my version of commctrl.h (version 1.2) there is nothing
// defined between TVN_LAST (0U-499U) and TTN_FIRST (0U-520U) so I am
// assuming that using TVN_LAST - 1 (0U-500U) should be a safe, conflict
// free value to use.

#define TVN_CHECK TVN_LAST - 1      // Custom WM_NOTIFY notification code

// Message map macro for the TVN_CHECK notification
#define ON_TVN_CHECK(nID,memberFxn) ON_NOTIFY(TVN_CHECK,(nID),(memberFxn))

enum TVCS_CHECKSTATE                // Checkbox state codes, compatible with CTreeCtrl::SetCheck()
{
	TVCS_NONE = -1,        // No checkbox associated with this item
	TVCS_UNCHECKED = 0,        // BST_UNCHECKED equivalent
	TVCS_CHECKED = 1,        // BST_CHECKED
	TVCS_INDETERMINATE = 2         // BST_INDETERMINATE
};

typedef struct tagTVNCHECK          // Information for the TVN_CHECK notification code
{
	NMHDR			hdr;            // Standard NMHDR structure
	HTREEITEM		hTreeItem;      // Handle to the tree item that is changing
	LPARAM			lParam;         // Extra data associated with the tree item
	TVCS_CHECKSTATE OldCheckState;  // Old Checkbox state
	TVCS_CHECKSTATE NewCheckState;  // New checkbox state
	HTREEITEM		hTriggerItem;   // Handle of the tree item that was initially triggered
} NMTVNCHECK, * LPNMTVNCHECK;

// CQuadStateTree
class CQuadStateTree : public CTreeCtrl
{
	DECLARE_DYNAMIC(CQuadStateTree)
	DECLARE_MESSAGE_MAP()

public:
	CQuadStateTree();
	virtual ~CQuadStateTree();

	TVCS_CHECKSTATE GetCheck(HTREEITEM hTreeItem) const;
	BOOL            SetCheck(HTREEITEM hTreeItem, TVCS_CHECKSTATE NewCheckState = TVCS_CHECKED);

	virtual BOOL Create(DWORD dw_style, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual BOOL CreateEx(DWORD dwExStyle, DWORD dw_style, const RECT& rect, CWnd* pParentWnd, UINT nID);

private:
	void    BuildBitmap();
	void    ToggleCheck(HTREEITEM hTreeItem);
	void    SetTriggerItem(HTREEITEM hTreeItem);
	BOOL    SetCheckInternal(HTREEITEM hTreeItem, TVCS_CHECKSTATE NewCheckState);
	LRESULT SendTVNCheck(HTREEITEM hTreeItem, TVCS_CHECKSTATE NewCheckState, TVCS_CHECKSTATE OldCheckState);

	CBitmap    m_Bitmap;
	CImageList m_ImageList;
	bool       m_bIgnoreIndeterminateState;
	bool       m_bTvnCheckReturnedNonzero;
	bool       m_bSettingChildItems;
	HTREEITEM  m_hTriggerItem;

public:
	afx_msg BOOL    OnNMClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL    OnNMTvStateImageChanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL    OnTvnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL    OnTvnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnTvmSetitem(WPARAM wp, LPARAM lp);

protected:
	virtual void PreSubclassWindow();
};
