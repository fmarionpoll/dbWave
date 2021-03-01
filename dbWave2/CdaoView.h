#pragma once


// CdaoView view

class CdaoView : public CView
{
	DECLARE_DYNCREATE(CdaoView)

protected:
	CdaoView();           // protected constructor used by dynamic creation
	virtual ~CdaoView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
};


