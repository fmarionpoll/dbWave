// DataListCtrlRowObject.cpp : implementation file
//

#include "StdAfx.h"
//#include "dbWave.h"
#include "DataListCtrlRowObject.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL(CDataListCtrlRowObject, CObject, 0 /* schema number*/)

// CDataListCtrlRowObject

CDataListCtrlRowObject::CDataListCtrlRowObject() : bChanged(0), insectID(0)
{
	index = 0;
}

CDataListCtrlRowObject::CDataListCtrlRowObject(int i) : bChanged(0), insectID(0), pdataDoc(nullptr), pspikeDoc(nullptr)
{
	index = i;
}

CDataListCtrlRowObject::~CDataListCtrlRowObject()
{
	if (pChartDataWnd != nullptr)
	{
		pChartDataWnd->DestroyWindow();
		delete pChartDataWnd;
	}
	if (pSpikeChartWnd != nullptr)
	{
		pSpikeChartWnd->DestroyWindow();
		delete pSpikeChartWnd;
	}
	SAFE_DELETE(pdataDoc);
	SAFE_DELETE(pspikeDoc);
	cs_comment.Empty();
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

CDataListCtrlRowObject& CDataListCtrlRowObject::operator = (const CDataListCtrlRowObject& arg)
{
	if (this != &arg) {
		bInit = arg.bInit;
		index = arg.index;
		insectID = arg.insectID;
		cs_comment = arg.cs_comment;
		csDatafileName = arg.csDatafileName;
		csSpikefileName = arg.csSpikefileName;
		csSensillumname = arg.csSensillumname;
		csStim1 = arg.csStim1;
		csConc1 = arg.csConc1;
		csStim2 = arg.csStim2;
		csConc2 = arg.csConc2;
		csNspk = arg.csNspk;
		csFlag = arg.csFlag;
		csDate = arg.csDate;
		pChartDataWnd = arg.pChartDataWnd;
		pSpikeChartWnd = arg.pSpikeChartWnd;
		pdataDoc = arg.pdataDoc;
		pspikeDoc = arg.pspikeDoc;
		pdbWaveDoc = arg.pdbWaveDoc;
	}
	return *this;
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

		const auto nstr = static_cast<int>(8);
		ar << nstr;
		ar << cs_comment;
		ar << csDatafileName;
		ar << csSensillumname;
		ar << csStim1;
		ar << csConc1;
		ar << csStim2;
		ar << csConc2;
		ar << csNspk;
		ar << csFlag;
		ar << csDate;

		const auto nobj = static_cast<int>(3);
		ar << nobj;
		pChartDataWnd->Serialize(ar);
		pSpikeChartWnd->Serialize(ar);
		ar << insectID;
	}
	else
	{
		ar >> wversion;
		ar >> index;

		int nstr;
		ar >> nstr;
		ar >> cs_comment; nstr--;
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
		pChartDataWnd->Serialize(ar);
		pSpikeChartWnd->Serialize(ar);
		nobj -= 2;
		nobj--; if (nobj > 0)  ar >> insectID;
	}
}