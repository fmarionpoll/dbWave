#include "StdAfx.h"

#include "NotedocCntrItem.h"
#include "NoteDoc.h"
//#include "ViewNotedoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_SERIAL(CNotedocCntrItem, CRichEditCntrItem, 0)

CNotedocCntrItem::CNotedocCntrItem(REOBJECT* preo, CNoteDoc* pContainer)
	: CRichEditCntrItem(preo, pContainer)
{
	// TODO: add one-time construction code here
}

CNotedocCntrItem::~CNotedocCntrItem()
{
	// TODO: add cleanup code here
}


#ifdef _DEBUG
void CNotedocCntrItem::AssertValid() const
{
	CRichEditCntrItem::AssertValid();
}

void CNotedocCntrItem::Dump(CDumpContext& dc) const
{
	CRichEditCntrItem::Dump(dc);
}
#endif
