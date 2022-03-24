#pragma once

// CntrItem.h : interface of the CNotedocCntrItem class
//

#include <afxrich.h>		// MFC rich edit classes

class CNoteDoc;
class ViewNoteDoc;

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
	{
		return (CNoteDoc*)CRichEditCntrItem::GetDocument();
	}

	ViewNoteDoc* GetActiveView()
	{
		return (ViewNoteDoc*)CRichEditCntrItem::GetActiveView();
	}

	// Implementation
public:
	~CNotedocCntrItem() override;
#ifdef _DEBUG
	void AssertValid() const override;
	void Dump(CDumpContext& dc) const override;
#endif
};
