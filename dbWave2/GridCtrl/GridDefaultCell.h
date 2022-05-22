#pragma once
#include <afx.h>
#include "GridCell.h"


// This class is for storing grid default values. It's a little heavy weight, so
// don't use it in bulk
class GridDefaultCell : public GridCell
{
	DECLARE_DYNCREATE(GridDefaultCell)

	// Construction/Destruction
public:
	GridDefaultCell();
	~GridDefaultCell() override;

public:
	virtual DWORD GetStyle() const { return m_dwStyle; }
	virtual void SetStyle(DWORD dw_style) { m_dwStyle = dw_style; }
	virtual int GetWidth() const { return m_Size.cx; }
	virtual int GetHeight() const { return m_Size.cy; }
	virtual void SetWidth(int nWidth) { m_Size.cx = nWidth; }
	virtual void SetHeight(int nHeight) { m_Size.cy = nHeight; }

	// Disable these properties
	void SetData(LPARAM /*lParam*/) override
	{
		ASSERT(FALSE);
	}

	void SetState(DWORD /*nState*/) override
	{
		ASSERT(FALSE);
	}

	DWORD GetState() const override { return GridCell::GetState() | GVIS_READONLY; }

	void SetCoords(int /*row*/, int /*col*/) override
	{
		ASSERT(FALSE);
	}

	void SetFont(const LOGFONT* /*plf*/) override;
	LOGFONT* GetFont() const override;
	CFont* GetFontObject() const override;

protected:
	CSize m_Size; // Default GetRectSize
	CFont m_Font; // Cached font
	DWORD m_dwStyle; // Cell Style - unused
};

