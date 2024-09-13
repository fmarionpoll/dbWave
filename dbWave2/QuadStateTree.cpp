// QuadStateTree.cpp - Implementation file for the CQuadStateTree class                     //
//                                                                                          //
// Written by : PJ Arends http://www.codeproject.com/script/Membership/View.aspx?mid=8817   //
//                                                                                          //
// Licence : CodeProject Open Licence http://www.codeproject.com/info/cpol10.aspx           //
//                                                                                          //
// Web: http://www.codeproject.com/Articles/847799/CQuadStateTree                           //

// TVN_CHECK methods derived from:
// http://www.apijunkie.com/APIJunkie/blog/post/2007/11/Handling-tree-control-check-box-selectionde-selection-in-Win32MFC.aspx

#include "StdAfx.h"
#include "QuadStateTree.h"

//#define ACTIVATE_VIEWER
//#include <ImageViewer.h>

// These macros are used to convert between TVITEM::state image bits
// (bits 12 to 15) and TVCS_CHECKSTATE
#define TVCS2STATE(T) ((UINT)(((INT)(T)+1)<<12)&TVIS_STATEIMAGEMASK)
#define STATE2TVCS(S) ((TVCS_CHECKSTATE)((INT)((S&TVIS_STATEIMAGEMASK)>>12)-1))

IMPLEMENT_DYNAMIC(CQuadStateTree, CTreeCtrl)

BEGIN_MESSAGE_MAP(CQuadStateTree, CTreeCtrl)
	ON_NOTIFY_REFLECT_EX(NM_CLICK, &CQuadStateTree::OnNMClick)
	ON_NOTIFY_REFLECT_EX(NM_TVSTATEIMAGECHANGING, &CQuadStateTree::on_nm_tv_state_image_changing)
	ON_NOTIFY_REFLECT_EX(TVN_KEYDOWN, &CQuadStateTree::on_tvn_keydown)
	ON_NOTIFY_REFLECT_EX(TVN_ITEMCHANGED, &CQuadStateTree::on_tvn_item_changed)
	ON_MESSAGE(TVM_SETITEM, &CQuadStateTree::on_tvm_set_item)
END_MESSAGE_MAP()

CQuadStateTree::CQuadStateTree()
	: m_b_ignore_indeterminate_state_(false)
	  , m_b_tvn_check_returned_nonzero_(false)
	  , m_b_setting_child_items_(true)
	  , m_h_trigger_item_(nullptr)
{
	build_bitmap();
}

CQuadStateTree::~CQuadStateTree()
{
	m_image_list_.DeleteImageList();
	m_bitmap_.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::GetCheck()  (public member function)
//   Retrieve the current checkbox state of the specified tree item
//
// Parameters:
//   hTreeItem - The handle of the tree item
//
// Returns:
//   A TVCS_CHECKSTATE enum value that specifies the current checkbox state
//
// Note:
//   CQuadStateTree::GetCheck is functionally identical to CTreeCtrl::GetCheck.
//   The only difference is that CTreeCtrl::GetCheck returns a BOOL value while
//   here we return a TVCS_CHECKSTATE value.
//
/////////////////////////////////////////////////////////////////////////////

TVCS_CHECKSTATE CQuadStateTree::get_check(const HTREEITEM h_tree_item) const
{
	TVITEM item;
	item.mask = TVIF_HANDLE | TVIF_STATE;
	item.hItem = h_tree_item;
	item.stateMask = TVIS_STATEIMAGEMASK;
	::SendMessage(m_hWnd, TVM_GETITEM, 0, reinterpret_cast<LPARAM>(&item));

	return STATE2TVCS(item.state);
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::OnTvmSetitem()  (public message handler function)
//   Handles the TVM_SETITEM message
//
// Parameters:
//   wp - Normally unused and set to zero. Here we use it as a flag to tell
//        if this message came from CQuadStateTree::SetCheckInternal
//   lp - A pointer to a TVITEM structure with information about the tree
//        item being set
//
// Returns:
//   A non-zero value if successful, zero if blocked or failed
//
// Note:
//   Handle the TVM_SETITEM message in case this control has been cast to the
//   base CTreeCtrl class and CTreeCtrl::SetCheck() was called. Here we route
//   the call to our own CQuadStateTree::SetCheck() function.
//
/////////////////////////////////////////////////////////////////////////////

LRESULT CQuadStateTree::on_tvm_set_item(const WPARAM wp, const LPARAM lp)
{
	const auto p_tvi = reinterpret_cast<LPTVITEM>(lp);

	// SetCheckInternal sets this value; CTreeCtrl::SetCheck() sets it to zero
	if (wp != 0xFEB1)
	{
		// Make sure we are dealing with state and state image attributes
		if ((p_tvi->mask & (TVIF_STATE | TVIF_HANDLE)) == (TVIF_STATE | TVIF_HANDLE))
		{
			if ((p_tvi->stateMask & TVIS_STATEIMAGEMASK) == TVIS_STATEIMAGEMASK)
			{
				// pass control to our own SetCheck()
				return set_check(p_tvi->hItem, STATE2TVCS(p_tvi->state));
			}
		}
	}

	return Default();
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::SetCheck()  (public member function)
//   Set the checkbox state of the specified tree item
//
// Parameters:
//   hTreeItem     - The handle of the tree item
//   NewCheckState - The state to set the checkbox
//
// Returns:
//   TRUE if successful, FALSE if blocked or failed
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::set_check(const HTREEITEM h_tree_item, const TVCS_CHECKSTATE new_check_state)
{
	if (new_check_state == TVCS_INDETERMINATE)
	{
		// Cannot set a tree item's check state to indeterminate.
		// Indeterminate can only be set in response to a parent
		// tree item having its child tree items in different states
		return FALSE;
	}

	// Set TriggerItem so we know which tree item started this in case
	// this action results in a bunch of parent and child tree items
	// having to also change checkbox states. SetCheckInternal does
	// all the actual work.
	set_trigger_item(h_tree_item);
	return set_check_internal(h_tree_item, new_check_state);
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::SetCheckInternal()  (private member function)
//   Set the checkbox state of the specified tree item
//
// Parameters:
//   hTreeItem     - The handle of the tree item
//   NewCheckState - The state to set the checkbox
//
// Returns:
//   TRUE if successful, FALSE if blocked or failed
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::set_check_internal(const HTREEITEM h_tree_item, const TVCS_CHECKSTATE new_check_state)
{
	const auto old_check_state = get_check(h_tree_item);
	if (old_check_state == new_check_state)
	{
		// no change, do nothing
		return TRUE;
	}

	if (new_check_state == TVCS_NONE)
	{
		// TVCS_NONE state is only allowed at the top of the tree
		// Do not allow TVCS_NONE state under a non-TVCS_NONE item
		const auto parent = GetParentItem(h_tree_item);
		if (nullptr != parent && get_check(parent) != TVCS_NONE)
		{
			return FALSE;
		}
	}

	// IgnoreIndeterminateState flag is set in ToggleCheck when the checkbox
	// state is set to TVCS_INDETERMINATE in order for the default handler
	// to cycle to the next state; TVCS_UNCHECKED. Do not send a TVNCHECK
	// notification in that case.
	if (!m_b_ignore_indeterminate_state_)
	{
		send_tvn_check(h_tree_item, new_check_state, old_check_state);
	}

	// TvnCheckReturnedNonzero is set in SendTVNCheck if the handler of the
	// TVNCHECK notification returned a non-zero value in order to stop the
	// checkbox state from changing
	if (!m_b_tvn_check_returned_nonzero_)
	{
		TVITEM item;
		item.mask = TVIF_HANDLE | TVIF_STATE;
		item.hItem = h_tree_item;
		item.stateMask = TVIS_STATEIMAGEMASK;
		item.state = TVCS2STATE(new_check_state);

		// Set wParam to a 0xFEB1 to flag this message in our OnTvmSetitem handler
		return static_cast<BOOL>(::SendMessage(m_hWnd, TVM_SETITEM, 0xFEB1, reinterpret_cast<LPARAM>(&item)));
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::SetTriggerItem()  (private member function)
//   Set the internal variables for when a user changes a checkbox
//   state and all parent and child checkboxes will be changed accordingly
//
// Parameters:
//   hTreeItem - The handle of the tree item to be set as the trigger item
//
/////////////////////////////////////////////////////////////////////////////

void CQuadStateTree::set_trigger_item(HTREEITEM h_tree_item)
{
	m_h_trigger_item_ = h_tree_item;

	// 'SettingChildItems' is a flag used in On_Tvn_Item_changed() to control
	// whether child items or parent items are being set in response
	// to the 'TriggerItem' item checkbox being changed
	m_b_setting_child_items_ = true;
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::ToggleCheck()  (private member function)
//   Toggle the checkbox state in response to a mouse click or space bar press
//
// Parameters:
//   hTreeItem - The handle of the tree item whose checkbox is to be toggled
//
// Note:
//   The default handler for this action does not send a TVM_SETITEM message
//   so we can not handle this action in our OnTvmSetitem handler
//
/////////////////////////////////////////////////////////////////////////////

void CQuadStateTree::toggle_check(const HTREEITEM h_tree_item)
{
	// By default, the tree control will cycle through it's check states;
	// unchecked -> checked -> indeterminate -> unchecked -> ...
	// but we only want to go from indeterminate or checked to unchecked and
	// unchecked to checked, we do not want to toggle to indeterminate state.
	const auto old_check_state = get_check(h_tree_item);
	auto new_check_state = TVCS_CHECKED;
	if (old_check_state == TVCS_CHECKED || old_check_state == TVCS_INDETERMINATE)
	{
		new_check_state = TVCS_UNCHECKED;
	}

	// TVN_CHECK Notify handler can call GetCheck() to get old check state
	// Set TriggerItem so we know who started this mess
	set_trigger_item(h_tree_item);
	send_tvn_check(h_tree_item, new_check_state, old_check_state);

	if (old_check_state == TVCS_CHECKED && !m_b_tvn_check_returned_nonzero_)
	{
		// Set to indeterminate state so when these functions return
		// the default handler will set the state to unchecked. Use
		// the IgnoreIndeterminateState flag to prevent SetCheckInternal
		// from firing a TVNCHECK notification
		m_b_ignore_indeterminate_state_ = true;
		set_check_internal(h_tree_item, TVCS_INDETERMINATE);
		m_b_ignore_indeterminate_state_ = false;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::SendTVNCheck()  (private member function)
//   Send a TVN_CHECK notification to the parent window
//
// Parameters:
//   hTreeItem     - The handle of the tree item whose checkbox state is changing
//   NewCheckState - The new checkbox state
//   OldCheckState - The old checkbox state
//
// Returns:
//   Zero if the checkbox state is allowed to change, non-zero to block the change
//
// Note:
//   TvnCheckReturnedNonzero is set to true if this function returns non-zero
//
/////////////////////////////////////////////////////////////////////////////

LRESULT CQuadStateTree::send_tvn_check(const HTREEITEM h_tree_item, const TVCS_CHECKSTATE new_check_state, const TVCS_CHECKSTATE old_check_state)
{
	if (IsWindow(GetParent()->m_hWnd))
	{
		NMTVNCHECK tvn = {nullptr};
		tvn.hdr.code = TVN_CHECK;
		tvn.hdr.hwndFrom = GetSafeHwnd();
		tvn.hdr.idFrom = GetDlgCtrlID();
		tvn.h_tree_item = h_tree_item;
		tvn.l_param = static_cast<LPARAM>(GetItemData(h_tree_item));
		tvn.new_check_state = new_check_state;
		tvn.old_check_state = old_check_state;
		tvn.h_trigger_item = m_h_trigger_item_;

		if (0 != GetParent()->SendMessage(WM_NOTIFY, tvn.hdr.idFrom, reinterpret_cast<LPARAM>(&tvn))
			&& m_h_trigger_item_ == h_tree_item)
		{
			// Setting this to true stops the checkbox from changing
			// Handled and reset in OnMNTvStateImageChanging
			m_b_tvn_check_returned_nonzero_ = true;
			return 1;
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::OnNMClick()  (public notification message handler)
//   Handle a left mouse button click (NM_CLICK notification)
//
// Parameters:
//   pNMHDR  - Not used here
//   pResult - Result return parameter, set to zero and ignored
//
// Returns
//   FALSE - to allow parent window to also handle the NM_CLICK notification
//
// Note:
//   MSDN documentation says that returning a non-zero value via pResult
//   will block the default action. However, as this is not the case with
//   the TVN_KEYDOWN notification when a space bar is pressed, and both
//   these actions have to be handled the same, I have decided to do the
//   blocking in OnNMTvStateImageChanging instead of here.
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::OnNMClick(NMHDR* p_nmhdr, LRESULT* p_result)
{
	*p_result = 0;
	UNREFERENCED_PARAMETER(p_nmhdr);

	UINT flags = 0;
	const auto pos = GetMessagePos();
	CPoint point(GET_X_LPARAM(pos), GET_Y_LPARAM(pos));
	ScreenToClient(&point);
	const auto h_tree_item = HitTest(point, &flags);
	// Was the click on a checkbox?
	if (nullptr != h_tree_item && (flags & TVHT_ONITEMSTATEICON) == TVHT_ONITEMSTATEICON)
	{
		toggle_check(h_tree_item);
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::OnTvnKeydown()  (public notification message handler)
//   Handle key presses (TVN_KEYDOWN notification)
//
// Parameters:
//   pNMHDR  - Pointer to a NMTV_KEYDOWN structure containing information
//             about the key press
//   pResult - Result return parameter, set to zero and ignored
//
// Returns
//   FALSE - to allow parent window to also handle the TVN_KEYDOWN notification
//
// Note:
//   pResult is ignored by the default handler so any blocking is done
//   in OnNMTvStateImageChanging instead of here.
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::on_tvn_keydown(NMHDR* p_nmhdr, LRESULT* p_result)
{
	*p_result = 0;
	const auto p_tv_key_down = reinterpret_cast<LPNMTVKEYDOWN>(p_nmhdr);

	if (p_tv_key_down->wVKey == VK_SPACE)
	{
		const auto h_tree_item = GetSelectedItem();
		// KeyDown is called before StateImageChanging so check for TVCS_NONE state
		if (nullptr != h_tree_item && get_check(h_tree_item) != TVCS_NONE)
		{
			toggle_check(h_tree_item);
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::OnNMTvStateImageChanging  (public notification message handler)
//   Handle NM_TVSTATEIMAGECHANGING notification
//
// Parameters:
//   pNMHDR  - Pointer to a NMTVSTATEIMAGECHANGING structure containing
//             information about the checkbox changes
//   pResult - Result return parameter, set to non-zero to block the change
//
// Returns
//   TRUE  - to block the NM_TVSTATEIMAGECHANGING notification from going to parent window
//   FALSE - to allow parent window to also handle the NM_TVSTATEIMAGECHANGING notification
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::on_nm_tv_state_image_changing(NMHDR* p_nmhdr, LRESULT* p_result)
{
	// If there is no checkbox in the HTREE_ITEM prevent a press on
	// the space bar from creating one by returning a non-zero value
	// This has the effect of locking the TVCS_NONE state
	// Also check for the TvnCheckReturnedNonzero flag to see if the
	// check state is allowed to change
	*p_result = 0;
	const auto p_sic = reinterpret_cast<NMTVSTATEIMAGECHANGING*>(p_nmhdr);

	if (p_sic->iOldStateImageIndex - 1 == TVCS_NONE || m_b_tvn_check_returned_nonzero_)
	{
		m_b_tvn_check_returned_nonzero_ = false; // reset
		*p_result = 1; // block the check state from changing
		return TRUE; // block parent notification
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::OnTvnItemchanged()  (public notification message handler)
//   Handle the TVN_ITEMCHANGED notification
//
// Parameters:
//   pNMHDR  - Pointer to a NMTVITEMCHANGE structure containing
//             information about the checkbox changes
//   pResult - Result return parameter, set to zero and ignored
//
// Returns
//   TRUE  - to block the TVN_ITEMCHANGED notification from going to parent window
//   FALSE - to allow parent window to also handle the TVN_ITEMCHANGED notification
//
// Note:
//   Here is where we keep all parent item and child item checkboxes in sync
//   Every time a child or parent item checkbox is changed it generates
//   another TVN_ITEMCHANGED notification. The flag 'SettingChildItems'
//   and the 'TriggerItem' handle are used to control whether to set
//   parent or child item checkboxes.
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::on_tvn_item_changed(NMHDR* p_nmhdr, LRESULT* p_result)
{
	*p_result = 0;
	auto pIC = reinterpret_cast<NMTVITEMCHANGE*>(p_nmhdr);

	if ((pIC->uChanged & TVIF_STATE) == TVIF_STATE)
	{
		const auto old_check_state = STATE2TVCS(pIC->uStateOld);
		const auto new_check_state = STATE2TVCS(pIC->uStateNew);

		if (old_check_state == new_check_state || (new_check_state == TVCS_INDETERMINATE && m_b_ignore_indeterminate_state_
			== true))
		{
			return TRUE; // no change; block parent notification
		}

		if (m_b_setting_child_items_)
		{
			if ((new_check_state == TVCS_CHECKED || new_check_state == TVCS_UNCHECKED) && ItemHasChildren(pIC->hItem))
			{
				// Set all children to same state
				// Will recursively end up back here for each level down
				auto child = GetChildItem(pIC->hItem);
				while (child != nullptr)
				{
					set_check_internal(child, new_check_state);
					child = GetNextSiblingItem(child);
				}
			}
		}

		if (pIC->hItem == m_h_trigger_item_)
		{
			// All child item checkboxes of 'TriggerItem' have been set to
			// the same state, so now we can set the parent item checkboxes
			m_b_setting_child_items_ = false;
		}

		if (!m_b_setting_child_items_)
		{
			const auto parent_item = GetParentItem(pIC->hItem);
			if (parent_item != nullptr && get_check(parent_item) != TVCS_NONE)
			{
				// Set parent state depending on all it's child states
				// Will recursively end up back here for each level up
				auto child = GetChildItem(parent_item);
				const auto state = get_check(child);
				auto check = state;
				while (child != nullptr && check == state)
				{
					check = get_check(child);
					child = GetNextSiblingItem(child);
				}
				set_check_internal(parent_item, check == state ? check : TVCS_INDETERMINATE);
			}
		}
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::BuildBitmap()  (private member function)
//   Create the Bitmap and Image_list for the checkboxes. The default one
//   does not have an indeterminate state
//
// http://www.codeproject.com/Tips/845393/Convert-a-Binary-File-to-a-Hex-Encoded-Text-File
//
/////////////////////////////////////////////////////////////////////////////

void CQuadStateTree::build_bitmap()
{
	BYTE BitmapInfoData[] = {
		0x28, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xEC, 0x07, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	BYTE BitmapBitsData[] = {
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F,
		0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F,
		0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F,
		0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F,
		0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF5, 0xF5, 0xF5, 0xF9, 0xF9, 0xF9, 0xF8, 0xF8, 0xF8, 0xF5,
		0xF5, 0xF5, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F,
		0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F,
		0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xCC, 0xCB, 0xCA, 0xD5, 0xD4, 0xD4, 0xDC, 0xDB, 0xDB, 0xE1, 0xE1,
		0xE0, 0xE7, 0xE7, 0xE6, 0xEB, 0xEB, 0xEA, 0xEC, 0xEC, 0xEB, 0xEC, 0xEB, 0xEB, 0xEA, 0xE9, 0xE9,
		0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xCC, 0xCB, 0xCA, 0xDB,
		0xDA, 0xDA, 0xE9, 0xE2, 0xDF, 0xBA, 0x99, 0x8C, 0xBD, 0x9D, 0x90, 0xF6, 0xF3, 0xF2, 0xED, 0xED,
		0xEC, 0xEC, 0xEB, 0xEB, 0xEA, 0xE9, 0xE9, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0xF4, 0xF4, 0xF4, 0xCC, 0xCB, 0xCA, 0xD5, 0xD4, 0xD4, 0xDC, 0xDB, 0xDB, 0xE1, 0xE1, 0xE0, 0xE7,
		0xE7, 0xE6, 0xEB, 0xEB, 0xEA, 0xEC, 0xEC, 0xEB, 0xEC, 0xEB, 0xEB, 0xEA, 0xE9, 0xE9, 0xF4, 0xF4,
		0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4,
		0xF4, 0xC6, 0xC4, 0xC2, 0xE9, 0xE9, 0xE9, 0xED, 0xED, 0xED, 0xF0, 0xF0, 0xF0, 0xF4, 0xF4, 0xF4,
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xE6, 0xE6, 0xE6, 0xF4, 0xF4, 0xF4, 0x8F,
		0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xCA, 0xC8, 0xC6, 0xF0, 0xEC, 0xEA, 0xBB, 0x99,
		0x8B, 0x97, 0x5E, 0x49, 0x98, 0x60, 0x4B, 0xD1, 0xB9, 0xB0, 0xF9, 0xF9, 0xF9, 0xF6, 0xF6, 0xF6,
		0xE6, 0xE6, 0xE6, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xC6,
		0xC4, 0xC2, 0xE9, 0xE9, 0xE9, 0xED, 0xED, 0xED, 0xF0, 0xF0, 0xF0, 0xF4, 0xF4, 0xF4, 0xF6, 0xF6,
		0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xE6, 0xE6, 0xE6, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xC2, 0xBF, 0xBC,
		0xE5, 0xE4, 0xE3, 0xE9, 0xE9, 0xE9, 0xED, 0xED, 0xED, 0xF2, 0xF2, 0xF2, 0xF4, 0xF4, 0xF4, 0xF5,
		0xF5, 0xF5, 0xF4, 0xF4, 0xF4, 0xE2, 0xE2, 0xE1, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F,
		0x8E, 0xF4, 0xF4, 0xF4, 0xD1, 0xCF, 0xCD, 0xE9, 0xE1, 0xDE, 0x95, 0x5C, 0x47, 0x96, 0x5E, 0x48,
		0x97, 0x5F, 0x4A, 0xA4, 0x72, 0x60, 0xFA, 0xF9, 0xF8, 0xF4, 0xF4, 0xF4, 0xE2, 0xE2, 0xE1, 0xF4,
		0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xC2, 0xBF, 0xBC, 0xE5, 0xE4,
		0xE3, 0xBB, 0x99, 0x8B, 0xA7, 0x77, 0x66, 0xA7, 0x77, 0x66, 0xA7, 0x77, 0x66, 0xD1, 0xB9, 0xB0,
		0xF4, 0xF4, 0xF4, 0xE2, 0xE2, 0xE1, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xBF, 0xBB, 0xB8, 0xE1, 0xDF, 0xDD, 0xE5,
		0xE5, 0xE4, 0xEA, 0xEA, 0xEA, 0xEF, 0xEF, 0xEF, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
		0xF2, 0xDE, 0xDD, 0xDC, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4,
		0xE1, 0xE0, 0xDE, 0xAA, 0x7E, 0x6D, 0x94, 0x5B, 0x46, 0xE2, 0xD4, 0xCF, 0xA7, 0x77, 0x66, 0x97,
		0x5F, 0x4A, 0xD5, 0xBF, 0xB7, 0xF6, 0xF6, 0xF6, 0xDE, 0xDD, 0xDC, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F,
		0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xBF, 0xBB, 0xB8, 0xE1, 0xDF, 0xDD, 0xAA, 0x7E, 0x6D,
		0x95, 0x5C, 0x47, 0x95, 0x5C, 0x47, 0x95, 0x5C, 0x47, 0xA7, 0x77, 0x66, 0xF2, 0xF2, 0xF2, 0xDE,
		0xDD, 0xDC, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F,
		0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xBC, 0xB7, 0xB2, 0xDC, 0xD8, 0xD5, 0xDF, 0xDC, 0xDA, 0xE3, 0xE1,
		0xE0, 0xE8, 0xE8, 0xE8, 0xEC, 0xEC, 0xEC, 0xED, 0xED, 0xED, 0xED, 0xED, 0xED, 0xD6, 0xD5, 0xD4,
		0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xCD, 0xC9, 0xC5, 0xDD,
		0xCF, 0xC9, 0xC8, 0xAE, 0xA3, 0xEE, 0xEE, 0xED, 0xD5, 0xC1, 0xBA, 0x96, 0x5D, 0x48, 0xA5, 0x75,
		0x63, 0xF8, 0xF8, 0xF8, 0xD6, 0xD5, 0xD5, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0xF4, 0xF4, 0xF4, 0xBC, 0xB7, 0xB2, 0xDC, 0xD8, 0xD5, 0xAA, 0x7E, 0x6D, 0x94, 0x5B, 0x46, 0x95,
		0x5C, 0x47, 0x95, 0x5C, 0x47, 0xA7, 0x77, 0x66, 0xED, 0xED, 0xED, 0xD6, 0xD5, 0xD4, 0xF4, 0xF4,
		0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4,
		0xF4, 0xB9, 0xB3, 0xAE, 0xD7, 0xD1, 0xCD, 0xD9, 0xD4, 0xD0, 0xDB, 0xD7, 0xD4, 0xDF, 0xDD, 0xDB,
		0xE3, 0xE2, 0xE1, 0xE6, 0xE6, 0xE5, 0xE8, 0xE8, 0xE8, 0xCD, 0xCD, 0xCC, 0xF4, 0xF4, 0xF4, 0x8F,
		0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xDD, 0xD9, 0xD5, 0xE5, 0xE2,
		0xDF, 0xDC, 0xD8, 0xD5, 0xF4, 0xF3, 0xF2, 0xA1, 0x70, 0x5D, 0x94, 0x5B, 0x46, 0xD6, 0xC3, 0xBC,
		0xDC, 0xDC, 0xDB, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9,
		0xB3, 0xAE, 0xD7, 0xD1, 0xCD, 0xAA, 0x7E, 0x6D, 0x95, 0x5C, 0x47, 0x95, 0x5C, 0x47, 0x95, 0x5C,
		0x47, 0xA7, 0x77, 0x66, 0xE8, 0xE8, 0xE8, 0xCD, 0xCD, 0xCC, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE,
		0xD5, 0xCF, 0xCB, 0xD5, 0xCF, 0xCB, 0xD6, 0xD1, 0xCD, 0xDA, 0xD5, 0xD2, 0xDE, 0xDB, 0xD8, 0xE1,
		0xDF, 0xDD, 0xE4, 0xE3, 0xE2, 0xC8, 0xC7, 0xC6, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F,
		0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xD5, 0xCF, 0xCB, 0xD5, 0xCF, 0xCB, 0xD6, 0xD1, 0xCD,
		0xE6, 0xE2, 0xE0, 0xCF, 0xB8, 0xAF, 0x92, 0x59, 0x44, 0xA5, 0x76, 0x64, 0xE8, 0xE7, 0xE7, 0xF4,
		0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xD5, 0xCF,
		0xCB, 0xBB, 0x99, 0x8B, 0xC8, 0xAE, 0xA3, 0xC8, 0xAE, 0xA3, 0xC8, 0xAE, 0xA3, 0xD1, 0xB9, 0xB0,
		0xE4, 0xE3, 0xE2, 0xC8, 0xC7, 0xC6, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xD5, 0xCF, 0xCB, 0xD5,
		0xCF, 0xCB, 0xD5, 0xCF, 0xCB, 0xD5, 0xCF, 0xCB, 0xD8, 0xD3, 0xD0, 0xDC, 0xD8, 0xD5, 0xDF, 0xDD,
		0xDB, 0xC5, 0xC3, 0xC1, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4,
		0xB9, 0xB3, 0xAE, 0xD5, 0xCF, 0xCB, 0xD5, 0xCF, 0xCB, 0xD5, 0xCF, 0xCB, 0xD6, 0xD0, 0xCC, 0xF1,
		0xEE, 0xED, 0x9D, 0x69, 0x56, 0x92, 0x59, 0x43, 0xD0, 0xBF, 0xB9, 0xF6, 0xF6, 0xF6, 0x8F, 0x8F,
		0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xD5, 0xCF, 0xCB, 0xD5, 0xCF, 0xCB,
		0xD5, 0xCF, 0xCB, 0xD5, 0xCF, 0xCB, 0xD8, 0xD3, 0xD0, 0xDC, 0xD8, 0xD5, 0xDF, 0xDD, 0xDB, 0xC5,
		0xC3, 0xC1, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F,
		0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3,
		0xAE, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xBA, 0xB4, 0xAF, 0xBD, 0xB9, 0xB4, 0xC1, 0xBE, 0xBB,
		0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xB9,
		0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xD0, 0xCC, 0xC9, 0xC0, 0xA7,
		0x9D, 0xAB, 0x86, 0x76, 0xE4, 0xDF, 0xDC, 0xF5, 0xF5, 0xF5, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0xF4, 0xF4, 0xF4, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xB9,
		0xB3, 0xAE, 0xB9, 0xB3, 0xAE, 0xBA, 0xB4, 0xAF, 0xBD, 0xB9, 0xB4, 0xC1, 0xBE, 0xBB, 0xF4, 0xF4,
		0xF4, 0x8F, 0x8F, 0x8E, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x8F,
		0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF8, 0xF8, 0xF8, 0xF9, 0xF9, 0xF9,
		0xF6, 0xF6, 0xF6, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4,
		0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0xF4, 0x8F, 0x8F, 0x8E,
		0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24,
		0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C,
		0xED, 0x24, 0x1C, 0xED, 0x24, 0x1C, 0xED, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F,
		0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F,
		0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F,
		0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F,
		0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E,
		0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E, 0x8F, 0x8F, 0x8E
	};

	const auto p_bitmap_info = reinterpret_cast<BITMAPINFO*>(BitmapInfoData);
	const auto p_bitmap_info_data = reinterpret_cast<BITMAPINFOHEADER*>(BitmapInfoData);

	const auto h_bitmap = CreateDIBitmap(::GetDC(nullptr), p_bitmap_info_data, CBM_INIT,
	                                     BitmapBitsData, p_bitmap_info, DIB_RGB_COLORS);
	//ShowGraphic(hBitmap);

	if (nullptr != h_bitmap && m_bitmap_.Attach(h_bitmap))
	{
		if (m_image_list_.Create(p_bitmap_info_data->biHeight, p_bitmap_info_data->biHeight, p_bitmap_info_data->biBitCount,
		                       p_bitmap_info_data->biWidth / p_bitmap_info_data->biHeight, 0))
		{
			m_image_list_.Add(&m_bitmap_, RGB(0xED, 0x1C, 0x24));
			//ShowGraphic(ImageList);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::PreSubclassWindow  (protected virtual member function)
//   Called by the framework before the window is subclassed
//   At this point m_hWnd is valid
//
/////////////////////////////////////////////////////////////////////////////

void CQuadStateTree::PreSubclassWindow()
{
	CTreeCtrl::PreSubclassWindow();
	SetImageList(&m_image_list_, TVSIL_STATE);
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::Create  (public member function)
//   Create this control and set the image_list
//
// Parameters:
//   dw_style    - Window styles
//   rect       - Window rectangle
//   pParentWnd - Parent window
//   nID        - Control ID number
//
// Returns:
//   TRUE if successful; FALSE on failure
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::Create(const DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd, const UINT n_id)
{
	return CreateEx(0, dw_style, rect, p_parent_wnd, n_id);
}

/////////////////////////////////////////////////////////////////////////////
//
// CQuadStateTree::CreateEx  (public member function)
//   Create this control and set the image_list
//
// Parameters:
//   dwStyleEx  - Extended Window styles
//   dw_style    - Window styles
//   rect       - Window rectangle
//   pParentWnd - Parent window
//   nID        - Control ID number
//
// Returns:
//   TRUE if successful; FALSE on failure
//
/////////////////////////////////////////////////////////////////////////////

BOOL CQuadStateTree::CreateEx(const DWORD dw_ex_style, const DWORD dw_style, const RECT& rect, CWnd* p_parent_wnd, const UINT n_id)
{
	const auto ret = CTreeCtrl::CreateEx(dw_ex_style, dw_style | TVS_CHECKBOXES, rect, p_parent_wnd, n_id);
	if (FALSE != ret)
	{
		SetImageList(&m_image_list_, TVSIL_STATE);
	}
	return ret;
}
