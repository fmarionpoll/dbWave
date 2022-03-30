// GridDropTarget.cpp : implementation file
//
// MFC Grid Control - Drag/Drop target implementation
//
// GridDropTarget is an OLE drop target for GridCtrl. All it does
// is handle the drag and drop windows messages and pass them
// directly onto the grid control.
//
// Written by Chris Maunder <chris@codeproject.com>
// Copyright (c) 1998-2005. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is
// not sold for profit without the authors written consent, and
// providing that this notice and the authors name and all copyright
// notices remains intact.
//
// An email letting me know how you are using it would be nice as well.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// For use with GridCtrl v2.10+
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCtrl.h"

#ifndef GRIDCONTROL_NO_DRAGDROP
#include "GridDropTarget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GridDropTarget

GridDropTarget::GridDropTarget()
{
	m_pGridCtrl = nullptr;
	m_bRegistered = FALSE;
}

GridDropTarget::~GridDropTarget()
{
}

// Overloaded Register() function performs the normal COleDropTarget::Register
// but also serves to connect this COleDropTarget with the parent grid control,
// where all drop messages will ultimately be forwarded.
BOOL GridDropTarget::Register(GridCtrl* pGridCtrl)
{
	if (m_bRegistered)
		return FALSE;

	// Stop re-entry problems
	static BOOL bInProcedure = FALSE;
	if (bInProcedure)
		return FALSE;
	bInProcedure = TRUE;

	ASSERT(pGridCtrl->IsKindOf(RUNTIME_CLASS(GridCtrl)));
	ASSERT(pGridCtrl);

	if (!pGridCtrl || !pGridCtrl->IsKindOf(RUNTIME_CLASS(GridCtrl)))
	{
		bInProcedure = FALSE;
		return FALSE;
	}

	m_pGridCtrl = pGridCtrl;

	m_bRegistered = COleDropTarget::Register(pGridCtrl);

	bInProcedure = FALSE;
	return m_bRegistered;
}

void GridDropTarget::Revoke()
{
	m_bRegistered = FALSE;
	COleDropTarget::Revoke();
}

BEGIN_MESSAGE_MAP(GridDropTarget, COleDropTarget)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GridDropTarget message handlers

DROPEFFECT GridDropTarget::OnDragScroll(CWnd* p_wnd, DWORD dwKeyState, CPoint /*point*/)
{
	if (m_pGridCtrl && ((p_wnd->GetSafeHwnd() == m_pGridCtrl->GetSafeHwnd())))
	{
		if (dwKeyState & MK_CONTROL)
			return DROPEFFECT_COPY;
		return DROPEFFECT_MOVE;
	}
	return DROPEFFECT_NONE;
}

DROPEFFECT GridDropTarget::OnDragEnter(CWnd* p_wnd, COleDataObject* pDataObject,
                                        DWORD dwKeyState, CPoint point)
{
	if ((m_pGridCtrl) && (p_wnd->GetSafeHwnd() == m_pGridCtrl->GetSafeHwnd()))
		return m_pGridCtrl->OnDragEnter(pDataObject, dwKeyState, point);
	return DROPEFFECT_NONE;
}

void GridDropTarget::OnDragLeave(CWnd* p_wnd)
{
	if ((m_pGridCtrl) && (p_wnd->GetSafeHwnd() == m_pGridCtrl->GetSafeHwnd()))
		m_pGridCtrl->OnDragLeave();
}

DROPEFFECT GridDropTarget::OnDragOver(CWnd* p_wnd, COleDataObject* pDataObject,
                                       DWORD dwKeyState, CPoint point)
{
	if ((m_pGridCtrl) && (p_wnd->GetSafeHwnd() == m_pGridCtrl->GetSafeHwnd()))
		return m_pGridCtrl->OnDragOver(pDataObject, dwKeyState, point);
	return DROPEFFECT_NONE;
}

BOOL GridDropTarget::OnDrop(CWnd* p_wnd, COleDataObject* pDataObject,
                             DROPEFFECT dropEffect, CPoint point)
{
	if ((m_pGridCtrl) && (p_wnd->GetSafeHwnd() == m_pGridCtrl->GetSafeHwnd()))
		return m_pGridCtrl->OnDrop(pDataObject, dropEffect, point);
	return FALSE;
}

#endif // GRIDCONTROL_NO_DRAGDROP
