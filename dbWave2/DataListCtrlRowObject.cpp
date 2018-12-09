// DataListCtrlRowObject.cpp : implementation file
//

#include "StdAfx.h"
#include "dbWave.h"
#include "DataListCtrlRowObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CDataListCtrlRowObject, CObject, 0 /* schema number*/ )

// CDataListCtrlRowObject

CDataListCtrlRowObject::CDataListCtrlRowObject()
{
	bInit = FALSE;
	index = 0;
	pdataWnd = nullptr;
	pspikeWnd = nullptr;
	pdataDoc = nullptr;
	pspikeDoc = nullptr;
	wversion = 0;
}

CDataListCtrlRowObject::CDataListCtrlRowObject(int i)
{
	bInit = FALSE;
	index = i;
	pdataWnd = nullptr;
	pspikeWnd = nullptr;
	wversion = 0;
}

CDataListCtrlRowObject::~CDataListCtrlRowObject()
{
	if (pdataWnd != nullptr)	
	{
		pdataWnd->DestroyWindow();
		delete pdataWnd;
	}
	if (pspikeWnd != nullptr)
	{
		pspikeWnd->DestroyWindow();
		delete pspikeWnd;
	}
	SAFE_DELETE(pdataDoc);
	SAFE_DELETE(pspikeDoc);
	csComment.Empty();
	csDatafileName.Empty();
	csSpikefileName.Empty();
	csSensillumname.Empty();
	csStim1.Empty();
	csConc1.Empty();
	csStim2.Empty();
	csConc2.Empty();
	csNspk.Empty();
	csFlag.Empty();
	csDate.Empty();
}

void CDataListCtrlRowObject::operator = (const CDataListCtrlRowObject& arg)
{
	bInit = arg.bInit;
	index = arg.index;
	insectID = arg.insectID;
	csComment = arg.csComment ;
	csDatafileName = arg.csDatafileName ;
	csSpikefileName = arg.csSpikefileName ;
	csSensillumname = arg.csSensillumname ;
	csStim1 = arg.csStim1;
	csConc1 = arg.csConc1;
	csStim2 = arg.csStim2;
	csConc2 = arg.csConc2;
	csNspk = arg.csNspk ;
	csFlag = arg.csFlag ;
	csDate = arg.csDate ;
	pdataWnd = arg.pdataWnd;
	pspikeWnd = arg.pspikeWnd;
	pdataDoc = arg.pdataDoc;
	pspikeDoc = arg.pspikeDoc;
}

void CDataListCtrlRowObject::Serialize(CArchive& ar)
{
	// not serialized: 
	// CAcqDataDoc*	pdataDoc;
	// CSpikeDoc*	pspikeDoc;

	if (ar.IsStoring())
	{
		wversion = 2;
		ar << wversion;
		ar << index;

		int nstr = 8;
		ar << nstr;
		ar << csComment;
		ar << csDatafileName;
		ar << csSensillumname;
		ar << csStim1;
		ar << csConc1;
		ar << csStim2;
		ar << csConc2;
		ar << csNspk;
		ar << csFlag;
		ar << csDate;

		int nobj = 3;
		ar << nobj;
		pdataWnd->Serialize(ar);
		pspikeWnd->Serialize(ar);
		ar << insectID;
	} 
	else
	{
		ar >> wversion;
		ar >> index;

		int nstr;
		ar >> nstr;
		ar >> csComment; nstr--;
		ar >> csDatafileName; nstr--;
		ar >> csSensillumname; nstr--;
		ar >> csStim1; nstr--;
		ar >> csConc1; nstr--;
		if (wversion > 1)
		{
			ar >> csStim2; nstr--;
			ar >> csConc2; nstr--;
		}
		ar >> csNspk; nstr--;
		ar >> csFlag; nstr--;
		ar >> csDate; nstr--;

		int nobj;
		ar >> nobj;
		ASSERT(nobj >= 2);
		pdataWnd->Serialize(ar);
		pspikeWnd->Serialize(ar);
		nobj -= 2;
		nobj--; if (nobj>0)  ar >> insectID;
	}
}
