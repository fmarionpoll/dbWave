#pragma once

// CntrItem.h : interface of the CNotedocCntrItem class
//

#include <afxrich.h>		// MFC rich edit classes

class CNoteDoc;
class CViewNoteDoc;

class CNotedocCntrItem : public CRichEditCntrItem
{
	DECLARE_SERIAL(CNotedocCntrItem)

// Constructors
public:
	CNotedocCntrItem(REOBJECT* preo = nullptr, CNoteDoc* pContainer = nullptr);
		// Note: pContainer is allowed to be NULL to enable IMPLEMENT_SERIALIZE.
		//  IMPLEMENT_SERIALIZE requires the class have a constructor with
		//  zero arguments.  Normally, OLE items are constructed with a
		//  non-NULL document pointer.

// Attributes
public:
	CNoteDoc* GetDocument()
		{ return (CNoteDoc*)CRichEditCntrItem::GetDocument(); }
	CViewNoteDoc* GetActiveView()
		{ return (CViewNoteDoc*)CRichEditCntrItem::GetActiveView(); }


// Implementation
public:
	~CNotedocCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};
