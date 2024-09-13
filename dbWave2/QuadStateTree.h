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

enum TVCS_CHECKSTATE // Checkbox state codes, compatible with CTreeCtrl::SetCheck()
{
	TVCS_NONE = -1, 		// No checkbox associated with this item
	TVCS_UNCHECKED = 0, 	// BST_UNCHECKED equivalent
	TVCS_CHECKED = 1, 		// BST_CHECKED
	TVCS_INDETERMINATE = 2	// BST_INDETERMINATE
};

typedef struct tagTVNCHECK // Information for the TVN_CHECK notification code
{
	NMHDR hdr; 
	HTREEITEM h_tree_item; 
	LPARAM l_param; 
	TVCS_CHECKSTATE old_check_state; 
	TVCS_CHECKSTATE new_check_state; 
	HTREEITEM h_trigger_item; 
} NMTVNCHECK, *LPNMTVNCHECK;

// CQuadStateTree
class CQuadStateTree : public CTreeCtrl
{
	DECLARE_DYNAMIC(CQuadStateTree)

public:
	CQuadStateTree();
	~CQuadStateTree() override;

	TVCS_CHECKSTATE get_check(HTREEITEM h_tree_item) const;
	BOOL set_check(HTREEITEM h_tree_item, TVCS_CHECKSTATE new_check_state = TVCS_CHECKED);

	virtual BOOL Create(DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd, UINT n_id);
	virtual BOOL CreateEx(DWORD dw_ex_style, DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd, UINT n_id);

protected:
	void PreSubclassWindow() override;

private:
	void build_bitmap();
	void toggle_check(HTREEITEM h_tree_item);
	void set_trigger_item(HTREEITEM h_tree_item);
	BOOL set_check_internal(HTREEITEM h_tree_item, TVCS_CHECKSTATE new_check_state);
	LRESULT send_tvn_check(HTREEITEM h_tree_item, TVCS_CHECKSTATE new_check_state, TVCS_CHECKSTATE old_check_state);

	CBitmap m_bitmap_;
	CImageList m_image_list_;
	bool m_b_ignore_indeterminate_state_;
	bool m_b_tvn_check_returned_nonzero_;
	bool m_b_setting_child_items_;
	HTREEITEM m_h_trigger_item_;

public:
	afx_msg BOOL OnNMClick(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg BOOL on_nm_tv_state_image_changing(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg BOOL on_tvn_keydown(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg BOOL on_tvn_item_changed(NMHDR* p_nmhdr, LRESULT* p_result);
	afx_msg LRESULT on_tvm_set_item(WPARAM wp, LPARAM lp);

	DECLARE_MESSAGE_MAP()
};
