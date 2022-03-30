//////////////////////////////////////////////////////////////////////
// GridDropTarget.h : header file
//
// MFC Grid Control - Drag/Drop target implementation
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
//////////////////////////////////////////////////////////////////////

#pragma once

#include <afxole.h>

class GridCtrl;

/////////////////////////////////////////////////////////////////////////////
// GridDropTarget command target

class GridDropTarget : public COleDropTarget
{
public:
	GridDropTarget();
	~GridDropTarget() override;

	// Attributes
public:
	GridCtrl* m_pGridCtrl;
	BOOL m_bRegistered;

	// Operations
public:
	BOOL Register(GridCtrl* pGridCtrl);
	void Revoke() override;

	BOOL OnDrop(CWnd* p_wnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) override;
	DROPEFFECT OnDragEnter(CWnd* p_wnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override;
	void OnDragLeave(CWnd* p_wnd) override;
	DROPEFFECT OnDragOver(CWnd* p_wnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override;
	DROPEFFECT OnDragScroll(CWnd* p_wnd, DWORD dwKeyState, CPoint point) override;

protected:
	DECLARE_MESSAGE_MAP()
};
